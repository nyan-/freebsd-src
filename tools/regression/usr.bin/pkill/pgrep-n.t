#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/pkill/pgrep-n.t,v 1.1.20.1 2009/04/15 03:14:26 kensmith Exp $

base=`basename $0`

echo "1..1"

name="pgrep -n"
sleep=`mktemp /tmp/$base.XXXXXX` || exit 1
ln -sf /bin/sleep $sleep
$sleep 5 &
oldpid=$!
$sleep 5 &
sleep 0.3
newpid=$!
pid=`pgrep -f -n $sleep`
if [ "$pid" = "$newpid" ]; then
	echo "ok - $name"
else
	echo "not ok - $name"
fi
kill $oldpid
kill $newpid
rm -f $sleep
