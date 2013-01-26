#!/bin/bash

LOOP_COUNT=128
TEMPFILE="/tmp/timecntr"
TEMPFILETOTALS="/tmp/totals"


objc_run_test() {
	TEST_NAME="$1"
	COUNTER="$2"
	
	if [ "${COUNTER}" -eq "${LOOP_COUNT}" ] ; then
		return 0
	fi
	
	TIME=`./${TEST_NAME}`
	echo "${TEST_NAME}[${COUNTER}] - 0.$TIME"
	
	COUNTER="`expr "${COUNTER}" + 1`"
	
	TOTAL="`cat $TEMPFILE`"
	TOTAL="`expr "$TOTAL" + "$TIME"`"
	echo $TOTAL > $TEMPFILE
	
	objc_run_test "${TEST_NAME}" "${COUNTER}"
}

for TEST in ./*-test; do
	echo '0' > $TEMPFILE
	
	objc_run_test $TEST 0
	
	TIME="`cat $TEMPFILE`"
	TIME="`expr "$TIME" / "${LOOP_COUNT}"`"
	
	echo "$TEST[average] = 0.$TIME"
	echo "$TEST[average] = 0.$TIME" >> $TEMPFILETOTALS
done

echo "======================"
cat $TEMPFILETOTALS

rm $TEMPFILE $TEMPFILETOTALS

