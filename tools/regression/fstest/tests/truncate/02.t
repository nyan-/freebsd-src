#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/truncate/02.t,v 1.1.8.1 2009/04/15 03:14:26 kensmith Exp $

desc="truncate returns ENAMETOOLONG if a component of a pathname exceeded 255 characters"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..5"

expect 0 create ${name255} 0644
expect 0 truncate ${name255} 123
expect 123 stat ${name255} size
expect 0 unlink ${name255}
expect ENAMETOOLONG truncate ${name256} 123
