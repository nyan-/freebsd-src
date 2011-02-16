#!/bin/sh
# $FreeBSD: src/tools/regression/geom_nop/test-2.t,v 1.2.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $

. `dirname $0`/conf.sh

us=45
src=`mktemp /tmp/$base.XXXXXX` || exit 1
dst=`mktemp /tmp/$base.XXXXXX` || exit 1

echo "1..1"

dd if=/dev/random of=${src} bs=1m count=1 >/dev/null 2>&1

mdconfig -a -t malloc -s 1M -u $us || exit 1

gnop create /dev/md${us} || exit 1

dd if=${src} of=/dev/md${us}.nop bs=1m count=1 >/dev/null 2>&1
dd if=/dev/md${us}.nop of=${dst} bs=1m count=1 >/dev/null 2>&1

if [ `md5 -q ${src}` != `md5 -q ${dst}` ]; then
	echo "not ok 1"
else
	echo "ok 1"
fi

gnop destroy md${us}.nop
mdconfig -d -u $us
rm -f ${src} ${dst}
