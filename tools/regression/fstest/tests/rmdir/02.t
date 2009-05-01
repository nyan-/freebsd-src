#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/rmdir/02.t,v 1.1.8.1 2009/04/15 03:14:26 kensmith Exp $

desc="rmdir returns ENAMETOOLONG if a component of a pathname exceeded 255 characters"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..4"

expect 0 mkdir ${name255} 0755
expect 0 rmdir ${name255}
expect ENOENT rmdir ${name255}
expect ENAMETOOLONG rmdir ${name256}
