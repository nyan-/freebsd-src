#!/bin/sh
# $FreeBSD: src/tools/regression/lib/libutil/test-trimdomain-nodomain.t,v 1.1.10.1 2009/04/15 03:14:26 kensmith Exp $

cd `dirname $0`

executable=`basename $0 .t`

make $executable 2>&1 > /dev/null

exec ./$executable
