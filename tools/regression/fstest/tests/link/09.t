#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/link/09.t,v 1.1.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $

desc="link returns ENOENT if the source file does not exist"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..5"

n0=`namegen`
n1=`namegen`

expect 0 create ${n0} 0644
expect 0 link ${n0} ${n1}
expect 0 unlink ${n0}
expect 0 unlink ${n1}
expect ENOENT link ${n0} ${n1}
