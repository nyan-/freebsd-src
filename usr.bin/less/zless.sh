#!/bin/sh
#
# $FreeBSD: src/usr.bin/less/zless.sh,v 1.1.24.1.6.1 2010/12/21 17:09:25 kensmith Exp $
#

export LESSOPEN="|/usr/bin/lesspipe.sh %s"
exec /usr/bin/less "$@"
