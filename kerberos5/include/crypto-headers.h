/* $FreeBSD: src/kerberos5/include/crypto-headers.h,v 1.3.2.1.6.1 2010/12/21 17:09:25 kensmith Exp $ */
#ifndef __crypto_headers_h__
#define __crypto_headers_h__
#define OPENSSL_DES_LIBDES_COMPATIBILITY
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/rc4.h>
#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/ui.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/pkcs12.h>
#include <openssl/hmac.h>
#endif /* __crypto_headers_h__ */
