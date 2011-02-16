#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/chown/06.t,v 1.1.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $

desc="chown returns ELOOP if too many symbolic links were encountered in translating the pathname"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..6"

n0=`namegen`
n1=`namegen`

expect 0 symlink ${n0} ${n1}
expect 0 symlink ${n1} ${n0}
expect ELOOP chown ${n0}/test 65534 65534
expect ELOOP chown ${n1}/test 65534 65534
expect 0 unlink ${n0}
expect 0 unlink ${n1}
