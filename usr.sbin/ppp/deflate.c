/*-
 * Copyright (c) 1997 Brian Somers <brian@Awfulhak.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: deflate.c,v 1.6.2.2 1998/11/26 07:14:40 jkh Exp $
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "defs.h"
#include "mbuf.h"
#include "log.h"
#include "timer.h"
#include "lqr.h"
#include "hdlc.h"
#include "fsm.h"
#include "lcp.h"
#include "ccp.h"
#include "deflate.h"

/* Our state */
struct deflate_state {
    u_short seqno;
    int uncomp_rec;
    int winsize;
    z_stream cx;
};

static char garbage[10];
static u_char EMPTY_BLOCK[4] = { 0x00, 0x00, 0xff, 0xff };

#define DEFLATE_CHUNK_LEN 1024		/* Allocate mbufs this size */

static void
DeflateResetOutput(void *v)
{
  struct deflate_state *state = (struct deflate_state *)v;

  state->seqno = 0;
  state->uncomp_rec = 0;
  deflateReset(&state->cx);
  log_Printf(LogCCP, "Deflate: Output channel reset\n");
}

static int
DeflateOutput(void *v, struct ccp *ccp, struct link *l, int pri, u_short proto,
              struct mbuf *mp)
{
  struct deflate_state *state = (struct deflate_state *)v;
  u_char *wp, *rp;
  int olen, ilen, len, res, flush;
  struct mbuf *mo_head, *mo, *mi_head, *mi;

  ilen = mbuf_Length(mp);
  log_Printf(LogDEBUG, "DeflateOutput: Proto %02x (%d bytes)\n", proto, ilen);
  log_DumpBp(LogDEBUG, "DeflateOutput: Compress packet:", mp);

  /* Stuff the protocol in front of the input */
  mi_head = mi = mbuf_Alloc(2, MB_HDLCOUT);
  mi->next = mp;
  rp = MBUF_CTOP(mi);
  if (proto < 0x100) {			/* Compress the protocol */
    rp[0] = proto & 0377;
    mi->cnt = 1;
  } else {				/* Don't compress the protocol */
    rp[0] = proto >> 8;
    rp[1] = proto & 0377;
    mi->cnt = 2;
  }

  /* Allocate the initial output mbuf */
  mo_head = mo = mbuf_Alloc(DEFLATE_CHUNK_LEN, MB_HDLCOUT);
  mo->cnt = 2;
  wp = MBUF_CTOP(mo);
  *wp++ = state->seqno >> 8;
  *wp++ = state->seqno & 0377;
  log_Printf(LogDEBUG, "DeflateOutput: Seq %d\n", state->seqno);
  state->seqno++;

  /* Set up the deflation context */
  state->cx.next_out = wp;
  state->cx.avail_out = DEFLATE_CHUNK_LEN - 2;
  state->cx.next_in = MBUF_CTOP(mi);
  state->cx.avail_in = mi->cnt;
  flush = Z_NO_FLUSH;

  olen = 0;
  while (1) {
    if ((res = deflate(&state->cx, flush)) != Z_OK) {
      if (res == Z_STREAM_END)
        break;			/* Done */
      log_Printf(LogWARN, "DeflateOutput: deflate returned %d (%s)\n",
                res, state->cx.msg ? state->cx.msg : "");
      mbuf_Free(mo_head);
      mbuf_FreeSeg(mi_head);
      state->seqno--;
      return 1;			/* packet dropped */
    }

    if (flush == Z_SYNC_FLUSH && state->cx.avail_out != 0)
      break;

    if (state->cx.avail_in == 0 && mi->next != NULL) {
      mi = mi->next;
      state->cx.next_in = MBUF_CTOP(mi);
      state->cx.avail_in = mi->cnt;
      if (mi->next == NULL)
        flush = Z_SYNC_FLUSH;
    }

    if (state->cx.avail_out == 0) {
      mo->next = mbuf_Alloc(DEFLATE_CHUNK_LEN, MB_HDLCOUT);
      olen += (mo->cnt = DEFLATE_CHUNK_LEN);
      mo = mo->next;
      mo->cnt = 0;
      state->cx.next_out = MBUF_CTOP(mo);
      state->cx.avail_out = DEFLATE_CHUNK_LEN;
    }
  }

  olen += (mo->cnt = DEFLATE_CHUNK_LEN - state->cx.avail_out);
  olen -= 4;		/* exclude the trailing EMPTY_BLOCK */

  /*
   * If the output packet (including seqno and excluding the EMPTY_BLOCK)
   * got bigger, send the original.
   */
  if (olen >= ilen) {
    mbuf_Free(mo_head);
    mbuf_FreeSeg(mi_head);
    log_Printf(LogDEBUG, "DeflateOutput: %d => %d: Uncompressible (0x%04x)\n",
              ilen, olen, proto);
    ccp->uncompout += ilen;
    ccp->compout += ilen;	/* We measure this stuff too */
    return 0;
  }

  mbuf_Free(mi_head);

  /*
   * Lose the last four bytes of our output.
   * XXX: We should probably assert that these are the same as the
   *      contents of EMPTY_BLOCK.
   */
  for (mo = mo_head, len = mo->cnt; len < olen; mo = mo->next, len += mo->cnt)
    ;
  mo->cnt -= len - olen;
  if (mo->next != NULL) {
    mbuf_Free(mo->next);
    mo->next = NULL;
  }

  ccp->uncompout += ilen;
  ccp->compout += olen;

  log_Printf(LogDEBUG, "DeflateOutput: %d => %d bytes, proto 0x%04x\n",
            ilen, olen, proto);

  hdlc_Output(l, PRI_NORMAL, ccp_Proto(ccp), mo_head);
  return 1;
}

static void
DeflateResetInput(void *v)
{
  struct deflate_state *state = (struct deflate_state *)v;

  state->seqno = 0;
  state->uncomp_rec = 0;
  inflateReset(&state->cx);
  log_Printf(LogCCP, "Deflate: Input channel reset\n");
}

static struct mbuf *
DeflateInput(void *v, struct ccp *ccp, u_short *proto, struct mbuf *mi)
{
  struct deflate_state *state = (struct deflate_state *)v;
  struct mbuf *mo, *mo_head, *mi_head;
  u_char *wp;
  int ilen, olen;
  int seq, flush, res, first;
  u_char hdr[2];

  log_DumpBp(LogDEBUG, "DeflateInput: Decompress packet:", mi);
  mi_head = mi = mbuf_Read(mi, hdr, 2);
  ilen = 2;

  /* Check the sequence number. */
  seq = (hdr[0] << 8) + hdr[1];
  log_Printf(LogDEBUG, "DeflateInput: Seq %d\n", seq);
  if (seq != state->seqno) {
    if (seq <= state->uncomp_rec)
      /*
       * So the peer's started at zero again - fine !  If we're wrong,
       * inflate() will fail.  This is better than getting into a loop
       * trying to get a ResetReq to a busy sender.
       */
      state->seqno = seq;
    else {
      log_Printf(LogWARN, "DeflateInput: Seq error: Got %d, expected %d\n",
                seq, state->seqno);
      mbuf_Free(mi_head);
      ccp_SendResetReq(&ccp->fsm);
      return NULL;
    }
  }
  state->seqno++;
  state->uncomp_rec = 0;

  /* Allocate an output mbuf */
  mo_head = mo = mbuf_Alloc(DEFLATE_CHUNK_LEN, MB_IPIN);

  /* Our proto starts with 0 if it's compressed */
  wp = MBUF_CTOP(mo);
  wp[0] = '\0';

  /*
   * We set avail_out to 1 initially so we can look at the first
   * byte of the output and decide whether we have a compressed
   * proto field.
   */
  state->cx.next_in = MBUF_CTOP(mi);
  state->cx.avail_in = mi->cnt;
  state->cx.next_out = wp + 1;
  state->cx.avail_out = 1;
  ilen += mi->cnt;

  flush = mi->next ? Z_NO_FLUSH : Z_SYNC_FLUSH;
  first = 1;
  olen = 0;

  while (1) {
    if ((res = inflate(&state->cx, flush)) != Z_OK) {
      if (res == Z_STREAM_END)
        break;			/* Done */
      log_Printf(LogWARN, "DeflateInput: inflate returned %d (%s)\n",
                res, state->cx.msg ? state->cx.msg : "");
      mbuf_Free(mo_head);
      mbuf_Free(mi);
      ccp_SendResetReq(&ccp->fsm);
      return NULL;
    }

    if (flush == Z_SYNC_FLUSH && state->cx.avail_out != 0)
      break;

    if (state->cx.avail_in == 0 && mi && (mi = mbuf_FreeSeg(mi)) != NULL) {
      /* underflow */
      state->cx.next_in = MBUF_CTOP(mi);
      ilen += (state->cx.avail_in = mi->cnt);
      if (mi->next == NULL)
        flush = Z_SYNC_FLUSH;
    }

    if (state->cx.avail_out == 0) {
      /* overflow */
      if (first) {
        if (!(wp[1] & 1)) {
          /* 2 byte proto, shuffle it back in output */
          wp[0] = wp[1];
          state->cx.next_out--;
          state->cx.avail_out = DEFLATE_CHUNK_LEN-1;
        } else
          state->cx.avail_out = DEFLATE_CHUNK_LEN-2;
        first = 0;
      } else {
        olen += (mo->cnt = DEFLATE_CHUNK_LEN);
        mo->next = mbuf_Alloc(DEFLATE_CHUNK_LEN, MB_IPIN);
        mo = mo->next;
        state->cx.next_out = MBUF_CTOP(mo);
        state->cx.avail_out = DEFLATE_CHUNK_LEN;
      }
    }
  }

  if (mi != NULL)
    mbuf_Free(mi);

  if (first) {
    log_Printf(LogWARN, "DeflateInput: Length error\n");
    mbuf_Free(mo_head);
    ccp_SendResetReq(&ccp->fsm);
    return NULL;
  }

  olen += (mo->cnt = DEFLATE_CHUNK_LEN - state->cx.avail_out);

  *proto = ((u_short)wp[0] << 8) | wp[1];
  mo_head->offset += 2;
  mo_head->cnt -= 2;
  olen -= 2;

  ccp->compin += ilen;
  ccp->uncompin += olen;

  log_Printf(LogDEBUG, "DeflateInput: %d => %d bytes, proto 0x%04x\n",
            ilen, olen, *proto);

  /*
   * Simulate an EMPTY_BLOCK so that our dictionary stays in sync.
   * The peer will have silently removed this!
   */
  state->cx.next_out = garbage;
  state->cx.avail_out = sizeof garbage;
  state->cx.next_in = EMPTY_BLOCK;
  state->cx.avail_in = sizeof EMPTY_BLOCK;
  inflate(&state->cx, Z_SYNC_FLUSH);

  return mo_head;
}

static void
DeflateDictSetup(void *v, struct ccp *ccp, u_short proto, struct mbuf *mi)
{
  struct deflate_state *state = (struct deflate_state *)v;
  int res, flush, expect_error;
  u_char *rp;
  struct mbuf *mi_head;
  short len;

  log_Printf(LogDEBUG, "DeflateDictSetup: Got seq %d\n", state->seqno);

  /*
   * Stuff an ``uncompressed data'' block header followed by the
   * protocol in front of the input
   */
  mi_head = mbuf_Alloc(7, MB_HDLCOUT);
  mi_head->next = mi;
  len = mbuf_Length(mi);
  mi = mi_head;
  rp = MBUF_CTOP(mi);
  if (proto < 0x100) {			/* Compress the protocol */
    rp[5] = proto & 0377;
    mi->cnt = 6;
    len++;
  } else {				/* Don't compress the protocol */
    rp[5] = proto >> 8;
    rp[6] = proto & 0377;
    mi->cnt = 7;
    len += 2;
  }
  rp[0] = 0x80;				/* BITS: 100xxxxx */
  rp[1] = len & 0377;			/* The length */
  rp[2] = len >> 8;
  rp[3] = (~len) & 0377;		/* One's compliment of the length */
  rp[4] = (~len) >> 8;

  state->cx.next_in = rp;
  state->cx.avail_in = mi->cnt;
  state->cx.next_out = garbage;
  state->cx.avail_out = sizeof garbage;
  flush = Z_NO_FLUSH;
  expect_error = 0;

  while (1) {
    if ((res = inflate(&state->cx, flush)) != Z_OK) {
      if (res == Z_STREAM_END)
        break;			/* Done */
      if (expect_error && res == Z_BUF_ERROR)
        break;
      log_Printf(LogERROR, "DeflateDictSetup: inflate returned %d (%s)\n",
                res, state->cx.msg ? state->cx.msg : "");
      log_Printf(LogERROR, "DeflateDictSetup: avail_in %d, avail_out %d\n",
                state->cx.avail_in, state->cx.avail_out);
      ccp_SendResetReq(&ccp->fsm);
      mbuf_FreeSeg(mi_head);		/* lose our allocated ``head'' buf */
      return;
    }

    if (flush == Z_SYNC_FLUSH && state->cx.avail_out != 0)
      break;

    if (state->cx.avail_in == 0 && mi && (mi = mi->next) != NULL) {
      /* underflow */
      state->cx.next_in = MBUF_CTOP(mi);
      state->cx.avail_in = mi->cnt;
      if (mi->next == NULL)
        flush = Z_SYNC_FLUSH;
    }

    if (state->cx.avail_out == 0) {
      if (state->cx.avail_in == 0)
        /*
         * This seems to be a bug in libz !  If inflate() finished
         * with 0 avail_in and 0 avail_out *and* this is the end of
         * our input *and* inflate() *has* actually written all the
         * output it's going to, it *doesn't* return Z_STREAM_END !
         * When we subsequently call it with no more input, it gives
         * us Z_BUF_ERROR :-(  It seems pretty safe to ignore this
         * error (the dictionary seems to stay in sync).  In the worst
         * case, we'll drop the next compressed packet and do a
         * CcpReset() then.
         */
        expect_error = 1;
      /* overflow */
      state->cx.next_out = garbage;
      state->cx.avail_out = sizeof garbage;
    }
  }

  ccp->compin += len;
  ccp->uncompin += len;

  state->seqno++;
  state->uncomp_rec++;
  mbuf_FreeSeg(mi_head);		/* lose our allocated ``head'' buf */
}

static const char *
DeflateDispOpts(struct lcp_opt *o)
{
  static char disp[7];		/* Must be used immediately */

  sprintf(disp, "win %d", (o->data[0]>>4) + 8);
  return disp;
}

static void
DeflateInitOptsOutput(struct lcp_opt *o, const struct ccp_config *cfg)
{
  o->len = 4;
  o->data[0] = ((cfg->deflate.out.winsize - 8) << 4) + 8;
  o->data[1] = '\0';
}

static int
DeflateSetOptsOutput(struct lcp_opt *o)
{
  if (o->len != 4 || (o->data[0] & 15) != 8 || o->data[1] != '\0')
    return MODE_REJ;

  if ((o->data[0] >> 4) + 8 > 15) {
    o->data[0] = ((15 - 8) << 4) + 8;
    return MODE_NAK;
  }

  return MODE_ACK;
}

static int
DeflateSetOptsInput(struct lcp_opt *o, const struct ccp_config *cfg)
{
  int want;

  if (o->len != 4 || (o->data[0] & 15) != 8 || o->data[1] != '\0')
    return MODE_REJ;

  want = (o->data[0] >> 4) + 8;
  if (cfg->deflate.in.winsize == 0) {
    if (want < 8 || want > 15) {
      o->data[0] = ((15 - 8) << 4) + 8;
    }
  } else if (want != cfg->deflate.in.winsize) {
    o->data[0] = ((cfg->deflate.in.winsize - 8) << 4) + 8;
    return MODE_NAK;
  }

  return MODE_ACK;
}

static void *
DeflateInitInput(struct lcp_opt *o)
{
  struct deflate_state *state;

  state = (struct deflate_state *)malloc(sizeof(struct deflate_state));
  if (state != NULL) {
    state->winsize = (o->data[0] >> 4) + 8;
    state->cx.zalloc = NULL;
    state->cx.opaque = NULL;
    state->cx.zfree = NULL;
    state->cx.next_out = NULL;
    if (inflateInit2(&state->cx, -state->winsize) == Z_OK)
      DeflateResetInput(state);
    else {
      free(state);
      state = NULL;
    }
  }

  return state;
}

static void *
DeflateInitOutput(struct lcp_opt *o)
{
  struct deflate_state *state;

  state = (struct deflate_state *)malloc(sizeof(struct deflate_state));
  if (state != NULL) {
    state->winsize = (o->data[0] >> 4) + 8;
    state->cx.zalloc = NULL;
    state->cx.opaque = NULL;
    state->cx.zfree = NULL;
    state->cx.next_in = NULL;
    if (deflateInit2(&state->cx, Z_DEFAULT_COMPRESSION, 8,
                     -state->winsize, 8, Z_DEFAULT_STRATEGY) == Z_OK)
      DeflateResetOutput(state);
    else {
      free(state);
      state = NULL;
    }
  }

  return state;
}

static void
DeflateTermInput(void *v)
{
  struct deflate_state *state = (struct deflate_state *)v;

  inflateEnd(&state->cx);
  free(state);
}

static void
DeflateTermOutput(void *v)
{
  struct deflate_state *state = (struct deflate_state *)v;

  deflateEnd(&state->cx);
  free(state);
}

const struct ccp_algorithm PppdDeflateAlgorithm = {
  TY_PPPD_DEFLATE,	/* pppd (wrongly) expects this ``type'' field */
  CCP_NEG_DEFLATE24,
  DeflateDispOpts,
  {
    DeflateSetOptsInput,
    DeflateInitInput,
    DeflateTermInput,
    DeflateResetInput,
    DeflateInput,
    DeflateDictSetup
  },
  {
    DeflateInitOptsOutput,
    DeflateSetOptsOutput,
    DeflateInitOutput,
    DeflateTermOutput,
    DeflateResetOutput,
    DeflateOutput
  },
};

const struct ccp_algorithm DeflateAlgorithm = {
  TY_DEFLATE,		/* rfc 1979 */
  CCP_NEG_DEFLATE,
  DeflateDispOpts,
  {
    DeflateSetOptsInput,
    DeflateInitInput,
    DeflateTermInput,
    DeflateResetInput,
    DeflateInput,
    DeflateDictSetup
  },
  {
    DeflateInitOptsOutput,
    DeflateSetOptsOutput,
    DeflateInitOutput,
    DeflateTermOutput,
    DeflateResetOutput,
    DeflateOutput
  },
};
