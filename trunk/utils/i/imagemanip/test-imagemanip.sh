#!/bin/sh
# Test script, designed to see whether or not imagemanip works properly.  

# NOTE: Can't use /tmp because it's a RAM disk, and we create some very large files there
# which would exhaust it.  This needs to be on a physical disk with at least 128MB free.
TMPDIR=/opt/test
TESTDIR=`pwd`
EXEDIR=$TESTDIR
LOG=$TESTDIR/run-tests.log
TESTID="utils"
DOIT=${DOIT:-""}
REALTESTID=${REALTESTID:-"0"}
TOTALTESTS=0
FAILEDTESTS=0

pass=PASSX
fail=FAILX
bugid="bugid $xfail "

# Function when we assume test should pass
testfunc_pass()
{
	# $1 is the test number
	# $2..$n are the function and its parameters 
	TESTNO=$1
	shift
	TOTALTESTS=`expr $TOTALTESTS + 1`

	printf "Test ${TESTNO}...\r"
	echo "===== TEST ${TESTNO} =====" >>$LOG
	echo "RUNNING: " $* >>$LOG
	$DOIT $* >>$LOG 2>&1
	ERRCODE=$?
	
	if [ $ERRCODE -ne 0 ] ; then
		echo "!! FAILED: ERROR CODE" $ERRCODE >>$LOG
		printf "%s:  %s%s %s ${abuild} ${aresult} %s\n" $fail "$bugid" $REALTESTID "test-imagemanip.sh" $TESTNO
		printf "%s: Command = $*\n\n" $fail
		FAILEDTESTS=`expr $FAILEDTESTS + 1`
		return
	else
		echo "SUCCESS: ERROR CODE" $ERRCODE >>$LOG
	fi
}	

# Function used to check results of a test; does not increment test number
testfunc_test()
{
	# $1 is the test number
	# $2..$n are the function and its parameters 
	TESTNO=$1
	shift

	printf "Test ${TESTNO}...\r"
	echo "===== TEST ${TESTNO} =====" >>$LOG
	echo "RUNNING: " $* >>$LOG
	$DOIT $* >>$LOG 2>&1
	ERRCODE=$?

	if [ $ERRCODE -ne 0 ] ; then
		echo "!! FAILED: ERROR CODE" $ERRCODE >>$LOG
		printf "%s:  %s%s %s ${abuild} ${aresult} %s\n" $fail "$bugid" $REALTESTID "test-imagemanip.sh" $TESTNO
		printf "%s: Command = $*\n\n" $fail
		FAILEDTESTS=`expr $FAILEDTESTS + 1`
		return
	else
		echo "SUCCESS: ERROR CODE" $ERRCODE >>$LOG	
	fi
}	

# Function when we assume test should fail
testfunc_fail()
{
	# $1 is the test number
	# $2..$n are the function and its parameters 
	TESTNO=$1
	shift
	TOTALTESTS=`expr $TOTALTESTS + 1`

	printf "Test ${TESTNO}...\r"
	echo "===== TEST ${TESTNO} =====" >>$LOG
	echo "RUNNING: " $* >>$LOG
	$DOIT $* >>$LOG 2>&1
	ERRCODE=$?

	if [ $ERRCODE -eq 0 ] ; then
		echo "!! FAILED: ERROR CODE" $ERRCODE " (Expecting failure)" >>$LOG
		printf "%s:  %s%s %s ${abuild} ${aresult} %s\n" $fail "$bugid" $REALTESTID "test-imagemanip.sh" $TESTNO
		printf "%s: Command = $*\n\n" $fail
		FAILEDTESTS=`expr $FAILEDTESTS + 1`
		return
	else
		echo "SUCCESS: ERROR CODE" $ERRCODE " (Expecting failure)" >>$LOG
	fi
}	

# Function when we expect certain output
testfunc_expect()
{
	# $1 is the test number
	# $2 is the expected string (must appear somewhere in output)
	# $3..$n are the function and its parameters 
	TESTNO=$1
	shift
	EXPECT=$2
	shift
	TOTALTESTS=`expr $TOTALTESTS + 1`

	printf "Test ${TESTNO}...\r"
	echo "===== TEST ${TESTNO} =====" >>$LOG
	echo "RUNNING: " $* >>$LOG
	$DOIT $* 2>&1 | grep -F $EXPECT
	ERRCODE=$?

	if [ $ERRCODE -eq 0 ] ; then
		echo "!! FAILED: ERROR CODE" $ERRCODE " (Expecting " $EXPECT ")" >>$LOG
		printf "%s:  %s%s %s ${abuild} ${aresult} %s\n" $fail "$bugid" $REALTESTID "test-imagemanip.sh" $TESTNO
		printf "%s: Command = $*\n\n" $fail
		FAILEDTESTS=`expr $FAILEDTESTS + 1`
		return
	else
		echo "SUCCESS: ERROR CODE" $ERRCODE " (Expecting " $EXPECT ")" >>$LOG
	fi
}	

# Function when we *don't* expect certain output
testfunc_notexpect()
{
	# $1 is the test number
	# $2 is the non-expected string (must NOT appear in output)
	# $3..$n are the function and its parameters 
	TESTNO=$1
	shift
	EXPECT=$2
	shift
	TOTALTESTS=`expr $TOTALTESTS + 1`

	printf "Test ${TESTNO}...\r"
	echo "===== TEST ${TESTNO} =====" >>$LOG
	echo "RUNNING: " $* >>$LOG
	$DOIT $* 2>&1 | grep -F -v $EXPECT
	ERRCODE=$?

	if [ $ERRCODE -eq 0 ] ; then
		echo "!! FAILED: ERROR CODE" $ERRCODE " (Not expecting " $EXPECT ")" >>$LOG
		printf "%s:  %s%s %s ${abuild} ${aresult} %s\n" $fail "$bugid" $REALTESTID "test-imagemanip.sh" $TESTNO
		printf "%s: Command = $*\n\n" $fail
		FAILEDTESTS=`expr $FAILEDTESTS + 1`
		return
	else
		echo "SUCCESS: ERROR CODE" $ERRCODE " (Not expecting " $EXPECT ")" >>$LOG
	fi
}	

printf "START:  $0, pid $$\n"

	#
	# Clean up from last run
	#
	rm -f $LOG
	rm -f $TMPDIR/im-out.*

	#
	# create data files
	#
	mkdir -p $TMPDIR
	if test -w $TMPDIR
	then
		printf "Creating test files...\n"
		cd $TMPDIR

		if ! test -r im-test.1byte
		then
			echo -n "a" > im-test.1byte
		fi
		
		if ! test -r im-test.256bytes
		then
			echo -n "abcdefghijklmnopqrstuvwxyz;:,.?/" > im-test.256bytes
			echo -n "123456789A123456789B123456789C12" >>im-test.256bytes
			echo -n "!QUOIOIUVNXCVKPAKLJSDJFLQJHFIUWE" >>im-test.256bytes
			echo -n "ZYXWVUTSRQPONMLKJIHGFEDCBA@*)(][" >>im-test.256bytes
			echo -n "TbcdefThijklTnopqrTtuvwxTz;:,.T/" >>im-test.256bytes
			echo -n "1T345T7T9A1T3T567T9T123T5T789T12" >>im-test.256bytes
			echo -n "!QTOTOIUTNTCVKTATLJSTJTLQJTFTUWE" >>im-test.256bytes
			echo -n "ZYXTVUTSRTPONMLTJIHGFTDCBA@T)(][" >>im-test.256bytes
		fi
		
		if ! test -r im-test.64kb
		then
			head -c -n 65536 /dev/urandom > im-test.64kb
		fi
		
		if ! test -r im-test.256kb
		then
			head -c -n 262144 /dev/urandom > im-test.256kb
		fi
			
		if ! test -r im-test.1mb
		then
			head -c -n 1048576 /dev/urandom > im-test.1mb
		fi
			
		if ! test -r im-test.96kb
		then
			head -c -n 98304 /dev/urandom > im-test.96kb
		fi

		if ! test -r im-test.be32kbmid
		then
			head -c -n 65536 im-test.96kb | tail -c -n -32768 > im-test.32kbmid
		fi
		
		if ! test -r im-test.16mb
		then
			cat im-test.1mb im-test.1mb im-test.1mb > im-test.16mb
			head -c -n 1048576 /dev/urandom >> im-test.16mb
			cat im-test.1mb im-test.1mb im-test.1mb >> im-test.16mb
			head -c -n 1048576 /dev/urandom >> im-test.16mb
			cat im-test.1mb im-test.1mb im-test.1mb >> im-test.16mb
			head -c -n 1048576 /dev/urandom >> im-test.16mb
			cat im-test.1mb im-test.1mb im-test.1mb >> im-test.16mb
			head -c -n 1048576 /dev/urandom >> im-test.16mb
		fi

		if ! test -r im-test.1mbits-head
		then
			head -c -n 131072 im-test.1mb > im-test.1mbits-head
		fi
		
		if ! test -r im-test.1m-head
		then
			head -c -n 1048576 im-test.16mb > im-test.1m-head
		fi

		if ! test -r im-test.64kb-head
		then
			head -c -n 65536 im-test.96kb >im-test.64kb-head
		fi

		if ! test -r im-test.524032-FFs
		then
			awk 'BEGIN {for (i=0;i<524032;i++) {printf "\xff"}}' > im-test.524032-FFs
		fi
		
		head -c -n 1 im-test.256bytes > im-test.1byte-head
		head -c -n 4 im-test.256bytes > im-test.4byte-head
		head -c -n 128 im-test.1mb > im-test.1kbits-head
		head -c -n 1024 im-test.1mb > im-test.1k-head
		head -c -n 256 /dev/zero >im-test.zero-head
		echo -n "@a" >im-test.at-a
		echo -n "a@" >im-test.a-at
		cat im-test.256bytes im-test.256bytes im-test.256bytes>im-out.768bytes-cat
		echo -n "A@" >im-test.A-at
		echo -n "AA@@" >im-test.A2-at2
		echo -n "AAAA@@@@" >im-test.A4-at4
		echo -n "A@A@A@" >im-test.3-A-at
		echo -n "abcdefghijklmnopqrstuvwxyz" >im-test.alpha
		echo -n "ABCDEFGHIJKLMNOPQRSTUVWXYZ" >im-test.ALPHA
		echo -n "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ" >im-test.aAlLpPhHaA
		echo -n "ABCDEF" >im-test.ABCDEF
		echo -n "AB" >im-test.AB
		echo -n "CD" >im-test.CD
		echo -n "EF" >im-test.EF
		echo -n "ABCDEFGHIJKL" >im-test.ABCDEFGHIJKL
		echo -n "ABGH" >im-test.ABGH
		echo -n "CDIJ" >im-test.CDIJ
		echo -n "EFKL" >im-test.EFKL
		echo -n "ABCD" >im-test.ABCD
		echo -n "EFGH" >im-test.EFGH
		echo -n "IJKL" >im-test.IJKL
		echo -n "BADCFEHGJILK" >im-test.BADCFEHGJILK
		echo -n "DCBAHGFELKJI" >im-test.DCBAHGFELKJI
		
		echo -n "This is a test file" > im-test-readme.ansi
		echo -n "T\0h\0i\0s\0 \0i\0s\0 \0a\0 \0t\0e\0s\0t\0 \0f\0i\0l\0e\0" >im-test-readme.unicode
		cat im-test-readme.unicode /dev/zero | head -c -n 256 > im-test-readme.unicode256
				
		printf "Done creating test files.\n"
		cd $TESTDIR
	fi
	
	#
	# start the test offically
	#
	printf "POINT: %s %s\n" $REALTESTID "test-imagemanip.sh"

	#
	# Tests
	#

	STARTTIME=`date -t`
	
	# 1   : File Input/Output
	
	# 1.1 : Test -i and -o with non-existent files
	testfunc_fail 1.1.1 $EXEDIR/imagemanip -i this.file.must.not.exist 
	testfunc_fail 1.1.2 $EXEDIR/imagemanip -o $TMPDIR////
	
	# 1.2 : Test -i with 1 byte file
	testfunc_pass 1.2.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.1byte -o $TMPDIR/im-out.1byte
	testfunc_test 1.2.1 cmp $TMPDIR/im-test.1byte $TMPDIR/im-out.1byte
	testfunc_pass 1.2.2 $EXEDIR/imagemanip -m 1 -i $TMPDIR/im-test.1byte -o $TMPDIR/im-out.1byte2
	testfunc_test 1.2.2 cmp $TMPDIR/im-test.1byte $TMPDIR/im-out.1byte2
	testfunc_pass 1.2.3 $EXEDIR/imagemanip -i $TMPDIR/im-test.1byte -m 1 -o $TMPDIR/im-out.1byte3
	testfunc_test 1.2.3 cmp $TMPDIR/im-test.1byte $TMPDIR/im-out.1byte3
	
	# 1.3 : Test -i with 256 byte file
	testfunc_pass 1.3.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -o $TMPDIR/im-out.256bytes
	testfunc_test 1.3.1 cmp $TMPDIR/im-test.256bytes $TMPDIR/im-out.256bytes
	testfunc_pass 1.3.2 $EXEDIR/imagemanip -m 256 -i $TMPDIR/im-test.256bytes -o $TMPDIR/im-out.256bytes2
	testfunc_test 1.3.2 cmp $TMPDIR/im-test.256bytes $TMPDIR/im-out.256bytes2
	testfunc_pass 1.3.3 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -m 256 -o $TMPDIR/im-out.256bytes3
	testfunc_test 1.3.3 cmp $TMPDIR/im-test.256bytes $TMPDIR/im-out.256bytes3

	# 1.4 : Test -i with 64K file
	testfunc_pass 1.4.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.64kb -o $TMPDIR/im-out.64kb
	testfunc_test 1.4.1 cmp $TMPDIR/im-test.64kb $TMPDIR/im-out.64kb
	testfunc_pass 1.4.2 $EXEDIR/imagemanip -m 64k -i $TMPDIR/im-test.64kb -o $TMPDIR/im-out.64kb2
	testfunc_test 1.4.2 cmp $TMPDIR/im-test.64kb $TMPDIR/im-out.64kb2
	testfunc_pass 1.4.3 $EXEDIR/imagemanip -i $TMPDIR/im-test.64kb -m 64k -o $TMPDIR/im-out.64kb3
	testfunc_test 1.4.3 cmp $TMPDIR/im-test.64kb $TMPDIR/im-out.64kb3
	
	# 1.5 : Test -i with 256K file
	testfunc_pass 1.5.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256kb -o $TMPDIR/im-out.256kb
	testfunc_test 1.5.1 cmp $TMPDIR/im-test.256kb $TMPDIR/im-out.256kb
	testfunc_pass 1.5.2 $EXEDIR/imagemanip -m 256k -i $TMPDIR/im-test.256kb -o $TMPDIR/im-out.256kb2
	testfunc_test 1.5.2 cmp $TMPDIR/im-test.256kb $TMPDIR/im-out.256kb2
	testfunc_pass 1.5.3 $EXEDIR/imagemanip -i $TMPDIR/im-test.256kb -m 256k -o $TMPDIR/im-out.256kb3
	testfunc_test 1.5.3 cmp $TMPDIR/im-test.256kb $TMPDIR/im-out.256kb3
	
	# 1.6 : Test -i with 1MB file
	testfunc_pass 1.6.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.1mb -o $TMPDIR/im-out.1mb
	testfunc_test 1.6.1 cmp $TMPDIR/im-test.1mb $TMPDIR/im-out.1mb
	testfunc_pass 1.6.2 $EXEDIR/imagemanip -m 1m -i $TMPDIR/im-test.1mb -o $TMPDIR/im-out.1mb2
	testfunc_test 1.6.2 cmp $TMPDIR/im-test.1mb $TMPDIR/im-out.1mb2
	testfunc_pass 1.6.3 $EXEDIR/imagemanip -i $TMPDIR/im-test.1mb -m 1m -o $TMPDIR/im-out.1mb3
	testfunc_test 1.6.3 cmp $TMPDIR/im-test.1mb $TMPDIR/im-out.1mb3
	
	# 2   : Test human size designators
	
	# 2.1 : Test bits
	testfunc_pass 2.1.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -m 8bits -o $TMPDIR/im-out.1byte-out
	testfunc_test 2.1.1 cmp $TMPDIR/im-test.1byte-head $TMPDIR/im-out.1byte-out
		
	testfunc_pass 2.1.2 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -m 32bits -o $TMPDIR/im-out.4byte-out
	testfunc_test 2.1.2 cmp $TMPDIR/im-test.4byte-head $TMPDIR/im-out.4byte-out
		
	# 2.2 : Test K and Kbits
	testfunc_pass 2.2.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.1mb -m 1kbits -o $TMPDIR/im-out.1kbits-out
	testfunc_test 2.2.1 cmp $TMPDIR/im-test.1kbits-head $TMPDIR/im-out.1kbits-out
		
	testfunc_pass 2.2.2 $EXEDIR/imagemanip -i $TMPDIR/im-test.1mb -m 1K -o $TMPDIR/im-out.1k-out
	testfunc_test 2.2.2 cmp $TMPDIR/im-test.1k-head $TMPDIR/im-out.1k-out

	# 2.3 : Test M and Mbits
	testfunc_pass 2.3.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.1mb -m 1mbits -o $TMPDIR/im-out.1mbits-out
	testfunc_test 2.3.1 cmp $TMPDIR/im-test.1mbits-head $TMPDIR/im-out.1mbits-out

	testfunc_pass 2.3.2 $EXEDIR/imagemanip -i $TMPDIR/im-test.16mb -m 1M -o $TMPDIR/im-out.1m-out
	testfunc_test 2.3.2 cmp $TMPDIR/im-test.1m-head $TMPDIR/im-out.1m-out

	# 2.4 : Test G
	# NOTE: In order not to waste everyone's time and disk space, we just test
	# that the "G" option can be given and that it sets an upper bound correctly
	testfunc_expect 2.4.1 "1024M bytes" $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -m 1G -o $TMPDIR/im-out.256-out
	
	# 3   : Test pseudo-file ops
	
	# 3.1 : Test -p file to create whole file
	testfunc_pass 3.1.1 $EXEDIR/imagemanip -p0 -m 256 -o $TMPDIR/im-out.zero-out
	testfunc_test 3.1.1 cmp $TMPDIR/im-test.zero-head $TMPDIR/im-out.zero-out
	
	# 3.2 : Test -p to prepend data
	testfunc_pass 3.2.1 $EXEDIR/imagemanip -m1 -p64 -m0 -i $TMPDIR/im-test.1byte -o $TMPDIR/im-out.at-a
	testfunc_test 3.2.1 cmp $TMPDIR/im-test.at-a $TMPDIR/im-out.at-a
	
	# 3.3 : Test -p to append data
	testfunc_pass 3.3.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.1byte -m1 -p64 -m0 -o $TMPDIR/im-out.a-at
	testfunc_test 3.3.1 cmp $TMPDIR/im-test.a-at $TMPDIR/im-out.a-at
	
	# 3.4 : Test -d to delete whole file
	testfunc_pass 3.4.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -d
	testfunc_expect 3.4.2 "Percent filled= 0.0" $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -d
		
	# 3.5 : Test -d to chop beginning off file
	testfunc_pass 3.5.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.at-a -m1 -d -m0 -o $TMPDIR/im-out.a
	testfunc_test 3.5.1 cmp $TMPDIR/im-test.1byte $TMPDIR/im-out.a
	
	# 3.6 : Test -d to truncate file
	testfunc_pass 3.6.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.a-at -m1 -o $TMPDIR/im-out.a -m0 -d
	testfunc_test 3.6.1 cmp $TMPDIR/im-test.1byte $TMPDIR/im-out.a
	
	# 4   : Test file maximum

	# 4.1 : Test maximum input
	testfunc_pass 4.1.1 $EXEDIR/imagemanip -m64k -i $TMPDIR/im-test.96kb -m0 -o $TMPDIR/im-out.64kb-in
	testfunc_test 4.1.1 cmp $TMPDIR/im-out.64kb-in $TMPDIR/im-test.64kb-head
	
	# 4.2 : Test maximum output
	testfunc_pass 4.2.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.96kb -m64k -o $TMPDIR/im-out.64kb-out
	testfunc_test 4.2.1 cmp $TMPDIR/im-out.64kb-out $TMPDIR/im-test.64kb-head
	
	# 4.3 : Test unlimited input/output
	testfunc_pass 4.3.1 $EXEDIR/imagemanip -m0 -i $TMPDIR/im-test.96kb -m0 -o $TMPDIR/im-out.96kb-io
	testfunc_test 4.3.1 cmp $TMPDIR/im-test.96kb $TMPDIR/im-out.96kb-io

	testfunc_pass 4.3.2 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -i $TMPDIR/im-test.256bytes -i $TMPDIR/im-test.256bytes -o $TMPDIR/im-out.768bytes-out
	testfunc_test 4.3.2 cmp $TMPDIR/im-out.768bytes-cat $TMPDIR/im-out.768bytes-out
	
	# 5   : Test count
	
	# 5.1 : Test 1 byte in count
	testfunc_pass 5.1.1 $EXEDIR/imagemanip -c 1 -p 65 -c 1 -p 64 -m2 -o $TMPDIR/im-out.A-at
	testfunc_test 5.1.1 cmp $TMPDIR/im-test.A-at $TMPDIR/im-out.A-at

	testfunc_pass 5.1.2 $EXEDIR/imagemanip -c 1 -p 65 -c 1 -p 64 -m6 -o $TMPDIR/im-out.3-A-at
	testfunc_test 5.1.2 cmp $TMPDIR/im-test.3-A-at $TMPDIR/im-out.3-A-at

	testfunc_pass 5.1.3 $EXEDIR/imagemanip -c 1 -i $TMPDIR/im-test.alpha -c 1 -i $TMPDIR/im-test.ALPHA -o $TMPDIR/im-out.aAlLpPhHaA
	testfunc_test 5.1.3 cmp $TMPDIR/im-test.aAlLpPhHaA $TMPDIR/im-out.aAlLpPhHaA
		
	# 5.2 : Test 2 byte in count
	testfunc_pass 5.2.1 $EXEDIR/imagemanip -c 2 -p 65 -c 2 -p 64 -m4 -o $TMPDIR/im-out.A2-at2
	testfunc_test 5.2.1 cmp $TMPDIR/im-test.A2-at2 $TMPDIR/im-out.A2-at2
	
	# 5.3 : Test 4 byte in count
	testfunc_pass 5.3.1 $EXEDIR/imagemanip -c 4 -p 65 -c 4 -p 64 -m8 -o $TMPDIR/im-out.A4-at4
	testfunc_test 5.3.1 cmp $TMPDIR/im-test.A4-at4 $TMPDIR/im-out.A4-at4
	
	# 5.4 : Test 1 byte out count
	testfunc_pass 5.4.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.aAlLpPhHaA -c 1 -o $TMPDIR/im-out.alpha -c 1 -o $TMPDIR/im-out.ALPHA
	testfunc_test 5.4.1 cmp $TMPDIR/im-test.alpha $TMPDIR/im-out.alpha
	testfunc_test 5.4.1 cmp $TMPDIR/im-test.ALPHA $TMPDIR/im-out.ALPHA
	
	# 5.5 : Test 2 byte out count
	testfunc_pass 5.5.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.ABCDEF -c 2 -o $TMPDIR/im-out.AB -c 2 -o $TMPDIR/im-out.CD -c 2 -o $TMPDIR/im-out.EF
	testfunc_test 5.5.1 cmp $TMPDIR/im-test.AB $TMPDIR/im-out.AB
	testfunc_test 5.5.1 cmp $TMPDIR/im-test.CD $TMPDIR/im-out.CD
	testfunc_test 5.5.1 cmp $TMPDIR/im-test.EF $TMPDIR/im-out.EF
	
	testfunc_pass 5.5.2 $EXEDIR/imagemanip -i $TMPDIR/im-test.ABCDEFGHIJKL -c 2 -o $TMPDIR/im-out.ABGH -c 2 -o $TMPDIR/im-out.CDIJ -c 2 -o $TMPDIR/im-out.EFKL
	testfunc_test 5.5.2 cmp $TMPDIR/im-test.ABGH $TMPDIR/im-out.ABGH
	testfunc_test 5.5.2 cmp $TMPDIR/im-test.CDIJ $TMPDIR/im-out.CDIJ
	testfunc_test 5.5.2 cmp $TMPDIR/im-test.EFKL $TMPDIR/im-out.EFKL
	
	# 5.6 : Test 4 byte out count
	testfunc_pass 5.6.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.ABCDEFGHIJKL -c 32bits -o $TMPDIR/im-out.ABCD -c 4 -o $TMPDIR/im-out.EFGH -c 4 -o $TMPDIR/im-out.IJKL
	testfunc_test 5.6.1 cmp $TMPDIR/im-test.ABCD $TMPDIR/im-out.ABCD
	testfunc_test 5.6.1 cmp $TMPDIR/im-test.EFGH $TMPDIR/im-out.EFGH
	testfunc_test 5.6.1 cmp $TMPDIR/im-test.IJKL $TMPDIR/im-out.IJKL
	
	# 6   : Endian tests
	
	# 6.1 : Test no endian swapping
	testfunc_pass 6.1.1 $EXEDIR/imagemanip -e1 -i $TMPDIR/im-test.ABCDEFGHIJKL -o $TMPDIR/im-out.ABCDEFGHIJKL
	testfunc_test 6.1.1 cmp $TMPDIR/im-test.ABCDEFGHIJKL $TMPDIR/im-out.ABCDEFGHIJKL
	
	# 6.2 : Test 2 byte endian swap
	testfunc_pass 6.2.1 $EXEDIR/imagemanip -e2 -i $TMPDIR/im-test.ABCDEFGHIJKL -o $TMPDIR/im-out.BADCFEHGJILK
	testfunc_test 6.2.1 cmp $TMPDIR/im-test.BADCFEHGJILK $TMPDIR/im-out.BADCFEHGJILK
		
	# 6.3 : Test 4 byte endian swap
	testfunc_pass 6.3.1 $EXEDIR/imagemanip -e4 -i $TMPDIR/im-test.ABCDEFGHIJKL -o $TMPDIR/im-out.DCBAHGFELKJI
	testfunc_test 6.3.1 cmp $TMPDIR/im-test.DCBAHGFELKJI $TMPDIR/im-out.DCBAHGFELKJI
	
	# 7   : Test examples
	
	# 7.1 : Split 16MB image file into two 8MB files
	testfunc_pass 7.1.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.16mb -m 8M -o $TMPDIR/im-out.first8mb -o $TMPDIR/im-out.second8mb
	testfunc_expect 7.1.1 "8388608" ls -l $TMPDIR/im-out.first8mb
	testfunc_expect 7.1.1 "8388608" ls -l $TMPDIR/im-out.second8mb
	cat $TMPDIR/im-out.first8mb $TMPDIR/im-out.second8mb >$TMPDIR/im-out.16mb
	testfunc_test 7.1.1 cmp $TMPDIR/im-test.16mb $TMPDIR/im-out.16mb
	
	# 7.2 : Combine above two half flash images back into one
	testfunc_pass 7.2.1 $EXEDIR/imagemanip -i $TMPDIR/im-out.first8mb -i $TMPDIR/im-out.second8mb -o $TMPDIR/im-out.16mb
	testfunc_test 7.2.1 cmp $TMPDIR/im-test.16mb $TMPDIR/im-out.16mb

	# 7.3 : Split 16MB image into two 8MB files, the first supplying the low 16-bits
	# 		and the second the high 16-bits for a 32-bit bus
	testfunc_pass 7.3.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.16mb -m 8M -c16bits -o $TMPDIR/im-out.low16bits -o $TMPDIR/im-out.high16bits
	testfunc_expect 7.3.1 "8388608" ls -l $TMPDIR/im-out.low16bits
	testfunc_expect 7.3.1 "8388608" ls -l $TMPDIR/im-out.high16bits
	# Right now, we don't actually test the individual chunks, but we do reassemble
	# them in the next test and expect them to equal the original image

	# 7.4 : Combine above two low/high flash images back into one
	testfunc_pass 7.4.1 $EXEDIR/imagemanip -c 16bits -i $TMPDIR/im-out.low16bits -i $TMPDIR/im-out.high16bits -o $TMPDIR/im-out.16mb
	testfunc_test 7.4.1 cmp $TMPDIR/im-test.16mb $TMPDIR/im-out.16mb

	# 7.5 : Pad file to 4 megabits with FF characters
	testfunc_pass 7.5.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -p 0xFF -m 4Mbits -o $TMPDIR/im-out.4Mbits
	testfunc_expect 7.5.1 "524288" ls -l $TMPDIR/im-out.4Mbits
	head -c -n 256 $TMPDIR/im-out.4Mbits > $TMPDIR/im-out.4Mbits-head
	tail -c -n -524032 $TMPDIR/im-out.4Mbits > $TMPDIR/im-out.4Mbits-tail
	testfunc_test 7.5.1 cmp $TMPDIR/im-test.256bytes $TMPDIR/im-out.4Mbits-head
	testfunc_test 7.5.1 cmp $TMPDIR/im-test.524032-FFs $TMPDIR/im-out.4Mbits-tail

	# 7.6 : Pad a text file to 256 bytes, interleaving zeros to convert ANSI to Unicode
	testfunc_pass 7.6.1 $EXEDIR/imagemanip -c 1 -i $TMPDIR/im-test-readme.ansi -p 0 -m 256 -o $TMPDIR/im-out-readme.unicode256
	testfunc_test 7.6.1 cmp $TMPDIR/im-out-readme.unicode256 $TMPDIR/im-test-readme.unicode256

	# 7.7 : Extract the middle 32K in big endian format from a 96K little-endian file
	testfunc_pass 7.7.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.96kb -m32K -d -e4 -o $TMPDIR/im-out.be32kbmid -d
	# NOTE: We don't test this yet...
	# testfunc_test 7.7.1 cmp $TMPDIR/im-test.be32kbmid $TMPDIR/im-out.be32kbmid

	# 8   : Test verbosity
	
	# 8.1 : Test no verbosity
	testfunc_notexpect "" 8.1.1 $EXEDIR/imagemanip -i $TMPDIR/im-test.256bytes -o $TMPDIR/im-out.256bytes

	# 8.2 : Test -v prints warnings of unconsumed data
	testfunc_expect "not all input data was read" 8.2.1 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m 250 -o $TMPDIR/im-out.250bytes
	testfunc_expect "output data files are not completely full" 8.2.2 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m300 -o $TMPDIR/im-out.300bytes
	
 	# 8.3 : Test -vv reports capacities and consumption
	testfunc_expect "Bytes read    = 250 bytes" 8.3.1 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m 250 -o $TMPDIR/im-out.250bytes
	testfunc_expect "Bytes written = 250 bytes" 8.3.2 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m 250 -o $TMPDIR/im-out.250bytes
	testfunc_expect "Percent filled= 100.0 %%"   8.3.3 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m 250 -o $TMPDIR/im-out.250bytes

	testfunc_expect "Bytes read    = 256 bytes" 8.3.4 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m300 -o $TMPDIR/im-out.300bytes
	testfunc_expect "Bytes written = 256 bytes" 8.3.5 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m300 -o $TMPDIR/im-out.300bytes
	testfunc_expect "Percent filled= 85.3 %%" 8.3.6 $EXEDIR/imagemanip -v -i $TMPDIR/im-test.256bytes -m300 -o $TMPDIR/im-out.300bytes

	# 8.4 : Test -vvv prints all arguments	
	# We don't bother testing this		

	STOPTIME=`date -t`

	printf "STOP:  $0, pid $$\n"
	printf "TOTALS: ${TOTALTESTS} tests run, ${FAILEDTESTS} tests FAILED\n"
	printf "LOG: Logfile of run is in $LOG\n"
	printf "RUNTIME: %d seconds\n" `expr $STOPTIME - $STARTTIME`
	
	if [ $FAILEDTESTS -eq 0 ] ; then
		rm -f $TMPDIR/im-out.*
		rm -f $TMPDIR/im-test.*
	fi
	
	exit
