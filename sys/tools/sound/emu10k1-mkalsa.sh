# $FreeBSD: src/sys/tools/sound/emu10k1-mkalsa.sh,v 1.1.2.1.6.1 2010/12/21 17:09:25 kensmith Exp $

GREP=${GREP:-grep}
CC=${CC:-cc}
AWK=${AWK:-awk}
MV=${MV:=mv}
RM=${RM:=rm}
IN=$1
OUT=$2

trap "${RM} -f $OUT.tmp" EXIT

$GREP -v '#include' $IN | \
$CC -E -D__KERNEL__ -dM -  | \
$AWK -F"[     (]" '
/define/  {
	print "#ifndef " $2;
	print;
	print "#endif";
}' > $OUT.tmp
${MV} -f $OUT.tmp $OUT
