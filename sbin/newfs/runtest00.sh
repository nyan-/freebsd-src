#!/bin/sh
# $FreeBSD: src/sbin/newfs/runtest00.sh,v 1.1.34.1 2009/04/15 03:14:26 kensmith Exp $

set -e

MD=99
(
for s in 1m 4m 60m 120m 240m 1g
do
	(
	mdconfig -d -u $MD || true
	mdconfig -a -t malloc -s $s -u $MD
	disklabel -r -w md$MD auto
	./newfs -R /dev/md${MD}c
	) 1>&2
	md5 < /dev/md${MD}c
done
mdconfig -d -u $MD 1>&2 || true
) 
