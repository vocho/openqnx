$! vmstest.com -- DCL script to perform test/Makefile actions for VMS
$!
$! Usage:
$!  $ set default [-.test]
$!  $ @[-.vms]vmstest.com bigtest
$! This assumes that newly built gawk.exe is in the next directory up.
$!
$	echo	= "write sys$output"
$	cmp	= "diff/Output=_NL:/Maximum=1"
$	rm	= "delete/noConfirm/noLog"
$	gawk = "$sys$disk:[-]gawk"
$	AWKPATH_srcdir = "define/User AWKPATH sys$disk:[]"
$
$	if p1.eqs."" then  p1 = "bigtest"
$	gosub 'p1'
$	if p2.nes."" then  gosub 'p2'
$	if p3.nes."" then  gosub 'p3'
$	if p4.nes."" then  gosub 'p4'
$	if p5.nes."" then  gosub 'p5'
$	if p6.nes."" then  gosub 'p6'
$	if p7.nes."" then  gosub 'p7'
$	if p8.nes."" then  gosub 'p8'
$	exit
$
$all:
$bigtest:	bigtest_list = "basic unix_tests gawk_ext vms_tests"
$		echo "bigtest"
$bigtest_loop:	bigtest_test = f$element(0," ",bigtest_list)
$		bigtest_list = bigtest_list - bigtest_test - " "
$		if bigtest_test.nes." " then  gosub 'bigtest_test'
$		if bigtest_list.nes.""	then  goto   bigtest_loop
$		return
$
$basic:		basic_lst1 = "msg swaplns messages argarray longwrds" -
		  + " getline fstabplus compare arrayref rs fsrs rand" -
		  + " fsbs negexp asgext anchgsub splitargv awkpath nfset" -
		  + " reparse convfmt arrayparm paramdup nonl defref" -
		  + " nofmtch litoct resplit rswhite prmarscl sclforin" -
		  + " sclifin intprec childin noeffect numsubstr pcntplus" -
		  + " prmreuse math fldchg fldchgnf reindops sprintfc" -
		  + " backgsub tweakfld clsflnam mmap8k fnarray dynlj" -
		  + " substr eofsplit prt1eval splitwht back89 tradanch"
$		basic_lst2 = "nlfldsep splitvar intest nfldstr nors" -
		  + " fnarydel noparms funstack clobber delarprm prdupval" -
		  + " nasty zeroflag getnr2tm getnr2tb printf1" -
		  + " funsmnam fnamedat numindex subslash opasnslf" -
		  + " opasnidx arynocls getlnbuf arysubnm fnparydl"
$		echo "basic"
$basic_loop1:	basic_test = f$element(0," ",basic_lst1)
$		basic_lst1 = basic_lst1 - basic_test - " "
$		if basic_test.nes." " then  gosub 'basic_test'
$		if basic_lst1.nes.""  then  goto   basic_loop1
$basic_loop2:	basic_test = f$element(0," ",basic_lst2)
$		basic_lst2 = basic_lst2 - basic_test - " "
$		if basic_test.nes." " then  gosub 'basic_test'
$		if basic_lst2.nes.""  then  goto   basic_loop2
$		return
$
$unix_tests:	unix_tst_list = "poundbang fflush getlnhd pipeio1" -
		  + " pipeio2 strftlng pid"
$		echo "unix_tests"
$unix_tst_loop: unix_tst_test = f$element(0," ",unix_tst_list)
$		unix_tst_list = unix_tst_list - unix_tst_test - " "
$		if unix_tst_test.nes." " then  gosub 'unix_tst_test'
$		if unix_tst_list.nes.""  then  goto   unix_tst_loop
$		return
$
$gawk_ext:	gawk_ext_list = "fieldwdth ignrcase posix manyfiles" -
		  + " igncfs argtest badargs strftime gensub gnureops reint" -
		  + " igncdym"		! + " nondec"
$		echo "gawk_ext (gawk.extensions)"
$gawk_ext_loop: gawk_ext_test = f$element(0," ",gawk_ext_list)
$		gawk_ext_list = gawk_ext_list - gawk_ext_test - " "
$		if gawk_ext_test.nes." " then  gosub 'gawk_ext_test'
$		if gawk_ext_list.nes.""  then  goto   gawk_ext_loop
$		return
$
$vms_tests:	vms_tst_list = "vms_io1"
$		echo "vms_tests"
$vms_tst_loop: vms_tst_test = f$element(0," ",vms_tst_list)
$		vms_tst_list = vms_tst_list - vms_tst_test - " "
$		if vms_tst_test.nes." " then  gosub 'vms_tst_test'
$		if vms_tst_list.nes.""  then  goto   vms_tst_loop
$		return
$
$extra:		extra_list = "regtest inftest"
$		echo "extra"
$		gosub "regtest"
$		gosub "inftest"
$		return
$
$poundbang:
$	echo "poundbang:  useless for VMS, so skipped"
$	return
$
$msg:
$	echo "Any output from ""DIFF"" is bad news, although some differences"
$	echo "in floating point values are probably benign -- in particular,"
$	echo "some systems may omit a leading zero and the floating point"
$	echo "precision may lead to slightly different output in a few cases."
$	return
$
$swaplns:	echo "swaplns"
$	gawk -f swaplns.awk swaplns.in >tmp.
$	cmp swaplns.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$messages:	echo "messages"
$	set noOn
$	gawk -f messages.awk > out2 >& out3
$	cmp out1.ok out1.
$	if $status then  rm out1.;
$	cmp out2.ok out2.
$	if $status then  rm out2.;
$	cmp out3.ok out3.
$	if $status then  rm out3.;
$	set On
$	return
$
$argarray:	echo "argarray"
$	define/User TEST "test"			!this is useless...
$	gawk -f argarray.awk ./argarray.in - >tmp.
just a test
$	cmp argarray.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fstabplus:	echo "fstabplus"
$	gawk -f fstabplus.awk >tmp.
1		2
$	cmp fstabplus.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fsrs:		echo "fsrs"
$	gawk -f fsrs.awk fsrs.in >tmp.
$	cmp fsrs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$igncfs:	echo "igncfs"
$	gawk -f igncfs.awk igncfs.in >tmp.
$	cmp igncfs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$longwrds:	echo "longwrds"
$	gawk -f longwrds.awk manpage >tmp.too
$	sort tmp.too tmp.
$	cmp longwrds.ok tmp.
$	if $status then  rm tmp.;,tmp.too;
$	return
$
$fieldwdth:	echo "fieldwdth"
$	gawk -v "FIELDWIDTHS=2 3 4" "{ print $2}" >tmp.
123456789
$	cmp fieldwdth.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ignrcase:	echo "ignrcase"
$	gawk -v "IGNORECASE=1" "{ sub(/y/, """"); print}" >tmp.
xYz
$	cmp ignrcase.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$regtest:
$  if f$search("regtest.com").eqs.""
$  then echo "regtest:  not available"
$  else echo "regtest"
$	echo "Some of the output from regtest is very system specific, do not"
$	echo "be distressed if your output differs from that distributed."
$	echo "Manual inspection is called for."
$	@regtest.com
$ endif
$	return
$
$posix: echo "posix"
$	gawk -f posix.awk >tmp.
1:2,3 4
$	cmp posix.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$manyfiles:	echo "manyfiles"
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]").eqs."" then  create/Dir/Prot=(O:rwed) [.junk]
$	gawk "BEGIN { for (i = 1; i <= 300; i++) print i, i}" >tmp.
$	echo "This may take quite a while..."
$	echo ""
$	gawk -f manyfiles.awk tmp. tmp.
$	define/User sys$error _NL:
$	define/User sys$output tmp.too
$	search/Match=Nor/Output=_NL:/Log [.junk]*.* ""
$!/Log output: "%SEARCH-S-NOMATCH, <filename> - <#> records" plus 1 line summary
$	gawk "$4!=2{++count}; END{if(NR!=301||count!=1){print ""Failed!""}}" tmp.too
$	rm tmp.;,tmp.too;,[.junk]*.*;*,[]junk.dir;
$	return
$
$compare:	echo "compare"
$	gawk -f compare.awk 0 1 compare.in >tmp.
$	cmp compare.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arrayref:	echo "arrayref"
$	gawk -f arrayref.awk >tmp.
$	cmp arrayref.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rs:		echo "rs"
$	gawk -v "RS=" "{ print $1, $2}" rs.in >tmp.
$	cmp rs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fsbs:		echo "fsbs"
$	gawk -v "FS=\" "{ print $1, $2 }" fsbs.in >tmp.
$	cmp fsbs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$inftest:	echo "inftest"
$     !!  echo "This test is very machine specific..."
$	gawk -f inftest.awk >tmp.
$     !!  cmp inftest.ok tmp.		!just care that gawk doesn't crash...
$	if $status then  rm tmp.;
$	return
$
$getline:	echo "getline"
$	gawk -f getline.awk getline.awk getline.awk >tmp.
$	cmp getline.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rand:		echo "rand"
$	echo "The following line should just be 19 random numbers between 1 and 100"
$	echo ""
$	gawk -f rand.awk
$	return
$
$negexp:	echo "negexp"
$	gawk "BEGIN { a = -2; print 10^a }" >tmp.
$	cmp negexp.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$asgext:	echo "asgext"
$	gawk -f asgext.awk asgext.in >tmp.
$	cmp asgext.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$anchgsub:	echo "anchgsub"
$	gawk -f anchgsub.awk anchgsub.in >tmp.
$	cmp anchgsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitargv:	echo "splitargv"
$	gawk -f splitargv.awk splitargv.in >tmp.
$	cmp splitargv.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$awkpath:	echo "awkpath"
$	define/User AWK_LIBRARY [],[.lib]
$	gawk -f awkpath.awk >tmp.
$	cmp awkpath.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nfset:		echo "nfset"
$	gawk -f nfset.awk nfset.in >tmp.
$	cmp nfset.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reparse:	echo "reparse"
$	gawk -f reparse.awk reparse.in >tmp.
$	cmp reparse.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$argtest:	echo "argtest"
$	gawk -f argtest.awk -x -y abc >tmp.
$	cmp argtest.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$badargs:	echo "badargs"
$	on error then continue
$	gawk -f 2>&1 >tmp.too
$!	search/Match=Nor tmp. "patchlevel" /Output=tmp.
$	gawk "/patchlevel/{next}; {gsub(""\"""",""'""); print}" <tmp.too >tmp.
$	cmp badargs.ok tmp.
$	if $status then  rm tmp.;,tmp.too;
$	return
$
$convfmt:	echo "convfmt"
$	gawk -f convfmt.awk >tmp.
$	cmp convfmt.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arrayparm:	echo "arrayparm"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f arrayparm.awk >tmp. 2>&1
$	set On
$	cmp arrayparm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$paramdup:	echo "paramdup"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f paramdup.awk >tmp. 2>&1
$	set On
$	cmp paramdup.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nonl:		echo "nonl"
$	! This one might fail, depending on which C run-time library is used.
$	! If VAXCRTL is used by the program that unpacks the distribution,
$	! then nonl.awk will actually end with a newline.  Even when that's
$	! not the case, if gawk itself uses VAXCRTL, an absent newline will
$	! be fabricated by the library when gawk reads the file.  DECC$SHR
$	! doesn't behave this way....
$	AWKPATH_srcdir
$	gawk --lint -f nonl.awk _NL: >tmp. 2>&1
$	cmp nonl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$defref:	echo "defref"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f defref.awk >tmp. 2>&1
$	set On
$	cmp defref.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nofmtch:	echo "nofmtch"
$	AWKPATH_srcdir
$	gawk --lint -f nofmtch.awk >tmp. 2>&1
$	cmp nofmtch.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$strftime:	echo "strftime"
$	! this test could fail on slow machines or on a second boundary,
$	! so if it does, double check the actual results
$!!	date | gawk -v "OUTPUT"=tmp. -f strftime.awk
$	! note: this test is simpler to implement for VMS
$	gawk -v "OUTPUT"=tmp. -
 "BEGIN {""show time"" | getline; print >""strftime.ok""; print strftime(""  %v %T"") >OUTPUT}"
$	set noOn
$	cmp strftime.ok tmp.
$	if $status then  rm tmp.;,strftime.ok;*
$	set On
$	return
$
$litoct:	echo "litoct"
$	gawk --traditional -f litoct.awk >tmp.
ab
$	cmp litoct.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$gensub:	echo "gensub"
$	gawk -f gensub.awk gensub.in >tmp.
$	cmp gensub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$resplit:	echo "resplit"
$	gawk -- "{ FS = "":""; $0 = $0; print $2 }" >tmp.
a:b:c d:e:f
$	cmp resplit.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rswhite:	echo "rswhite"
$	gawk -f rswhite.awk rswhite.in >tmp.
$	cmp rswhite.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prmarscl:	echo "prmarscl"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f prmarscl.awk >tmp. 2>&1
$	set On
$	cmp prmarscl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sclforin:	echo "sclforin"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f sclforin.awk >tmp. 2>&1
$	set On
$	cmp sclforin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sclifin:	echo "sclifin"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f sclifin.awk >tmp. 2>&1
$	set On
$	cmp sclifin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$intprec:	echo "intprec"
$	gawk -f intprec.awk >tmp. 2>&1
$	cmp intprec.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$childin:	echo "childin:  currently fails for the VMS port, so skipped"
$	return
$! note: this `childin' test currently [gawk 3.0.3] fails for vms
$!!childin:	echo "childin"
$	echo "note: type ``hi<return><ctrl/Z>'",-
	     "' if testing appears to hang in `childin'"
$!!	@echo hi | gawk "BEGIN { ""cat"" | getline; print; close(""cat"") }" >tmp.
$	gawk "BEGIN { ""type sys$input:"" | getline; print; close(""type sys$input:"") }" >tmp.
hi
$	cmp childin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$noeffect:	echo "noeffect"
$	AWKPATH_srcdir
$	gawk --lint -f noeffect.awk >tmp. 2>&1
$	cmp noeffect.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$numsubstr:	echo "numsubstr"
$	AWKPATH_srcdir
$	gawk -f numsubstr.awk numsubstr.in >tmp.
$	cmp numsubstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$gnureops:	echo "gnureops"
$	gawk -f gnureops.awk >tmp.
$	cmp gnureops.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pcntplus:	echo "pcntplus"
$	gawk -f pcntplus.awk >tmp.
$	cmp pcntplus.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prmreuse:	echo "prmreuse"
$	if f$search("prmreuse.ok").eqs."" then  create prmreuse.ok
$	gawk -f prmreuse.awk >tmp.
$	cmp prmreuse.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$math:		echo "math"
$	gawk -f math.awk >tmp.
$	cmp math.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fflush:
$	echo "fflush:  hopeless for VMS, so skipped"
$	return
$!!fflush:	echo "fflush"
$	! hopelessly Unix-specific
$!!	@fflush.sh >tmp.
$	cmp fflush.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fldchg:	echo "fldchg"
$	gawk -f fldchg.awk fldchg.in >tmp.
$	cmp fldchg.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fldchgnf:	echo "fldchgnf"
$	gawk -f fldchgnf.awk fldchgnf.in >tmp.
$	cmp fldchgnf.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reindops:	echo "reindops"
$	gawk -f reindops.awk reindops.in >tmp.
$	cmp reindops.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sprintfc:	echo "sprintfc"
$	gawk -f sprintfc.awk sprintfc.in >tmp.
$	cmp sprintfc.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getlnhd:
$	echo "getlnhd:  uses Unix-specific command so won't work on VMS"
$	return
$!!getlnhd:	echo "getlnhd"
$	gawk -f getlnhd.awk >tmp.
$	cmp getlnhd.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$backgsub:	echo "backgsub"
$	gawk -f backgsub.awk backgsub.in >tmp.
$	cmp backgsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$tweakfld:	echo "tweakfld"
$	gawk -f tweakfld.awk tweakfld.in >tmp.
$	if f$search("errors.cleanup").nes."" then  rm errors.cleanup;*
$	cmp tweakfld.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clsflnam:	echo "clsflnam"
$	if f$search("clsflnam.ok").eqs."" then  create clsflnam.ok
$	gawk -f clsflnam.awk clsflnam.in >tmp.
$	cmp clsflnam.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$mmap8k:	echo "mmap8k"
$	gawk "{ print }" mmap8k.in >tmp.
$	cmp mmap8k.in tmp.
$	if $status then  rm tmp.;
$	return
$
$fnarray:	echo "fnarray"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f fnarray.awk >tmp. 2>&1
$	set On
$	cmp fnarray.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$dynlj:		echo "dynlj"
$	gawk -f dynlj.awk >tmp.
$	cmp dynlj.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$substr:	echo "substr"
$	gawk -f substr.awk >tmp.
$	cmp substr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$eofsplit:	echo "eofsplit"
$	if f$search("eofsplit.ok").eqs."" then  create eofsplit.ok
$	gawk -f eofsplit.awk >tmp.
$	cmp eofsplit.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prt1eval:	echo "prt1eval"
$	gawk -f prt1eval.awk >tmp.
$	cmp prt1eval.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitwht:	echo "splitwht"
$	gawk -f splitwht.awk >tmp.
$	cmp splitwht.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$back89:		echo "back89"
$	gawk "/a\8b/" back89.in >tmp.
$	cmp back89.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$tradanch:	echo "tradanch"
$	if f$search("tradanch.ok").eqs."" then  create tradanch.ok
$	gawk --traditional -f tradanch.awk tradanch.in >tmp.
$	cmp tradanch.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nlfldsep:	echo "nlfldsep"
$	gawk -f nlfldsep.awk nlfldsep.in >tmp.
$	cmp nlfldsep.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitvar:	echo "splitvar"
$	gawk -f splitvar.awk splitvar.in >tmp.
$	cmp splitvar.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$intest:	echo "intest"
$	gawk -f intest.awk >tmp.
$	cmp intest.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pid:		echo "pid"
$	if f$search("pid.ok").eqs."" then  create pid.ok
$	open/Write ftmp _pid.in
$	write ftmp f$integer("%x" + f$getjpi("","PID"))
$	write ftmp f$integer("%x" + f$getjpi("","OWNER"))
$	close ftmp
$	gawk -f pid.awk _pid.in >tmp.
$	rm _pid.in;
$	cmp pid.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$strftlng:	echo "strftlng"
$	define/User TZ "UTC"		!useless
$	gawk -f strftlng.awk >tmp.
$	cmp strftlng.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nfldstr:	echo "nfldstr"
$	if f$search("nfldstr.ok").eqs."" then  create nfldstr.ok
$	gawk "$1 == 0 { print ""bug"" }" >tmp.

$	cmp nfldstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nors:		echo "nors"
$!! there's no straightforward way to supply non-terminated input on the fly
$!!	@echo A B C D E | tr -d '\12' | $(AWK) '{ print $$NF }' - $(srcdir)/nors.in > _$@
$!! so just read a line from sys$input instead
$	gawk "{ print $NF }" - nors.in >tmp.
A B C D E
$	cmp nors.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnarydel:	echo "fnarydel"
$	gawk -f fnarydel.awk >tmp.
$	cmp fnarydel.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reint:		echo "reint"
$	gawk --re-interval -f reint.awk reint.in >tmp.
$	cmp reint.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$noparms:	echo "noparms"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f noparms.awk >tmp. 2>&1
$	set On
$	cmp noparms.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pipeio1:	echo "pipeio1"
$	cat = "TYPE"	!close enough, as long as we avoid .LIS default suffix
$	define/User test1 []test1.
$	define/User test2 []test2.
$	gawk -f pipeio1.awk >tmp.
$	rm test1.;,test2.;
$	cmp pipeio1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pipeio2:
$	echo "pipeio2:  uses Unix-specific command so won't work on VMS"
$	return
$!!pipeio2:	echo "pipeio2"
$	cat = "gawk -- {print}"
$	tr  = "??"	!unfortunately, no trivial substitution available...
$	gawk -v "SRCDIR=." -f pipeio2.awk >tmp.
$	cmp pipeio2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$funstack:	echo "funstack"
$	gawk -f funstack.awk funstack.in >tmp.
$	cmp funstack.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clobber:	echo "clobber"
$	gawk -f clobber.awk >tmp.
$	cmp clobber.ok seq.
$	if $status then  rm seq.;*
$	cmp clobber.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$delarprm:	echo "delarprm"
$	gawk -f delarprm.awk >tmp.
$	cmp delarprm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prdupval:	echo "prdupval"
$	gawk -f prdupval.awk prdupval.in >tmp.
$	cmp prdupval.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nasty:	echo "nasty"
$	gawk -f nasty.awk >tmp.
$	if f$file_attrib("nasty.ok","LRL").eq.0 then  convert nasty.ok *.*
$	if f$file_attrib("tmp.",    "LRL").eq.0 then  convert tmp. *.*
$	cmp nasty.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$zeroflag:	echo "zeroflag"
$	gawk -f zeroflag.awk >tmp.
$	cmp zeroflag.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getnr2tm:	echo "getnr2tm"
$	gawk -f getnr2tm.awk getnr2tm.in >tmp.
$	cmp getnr2tm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getnr2tb:	echo "getnr2tb"
$	gawk -f getnr2tb.awk getnr2tb.in >tmp.
$	cmp getnr2tb.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$printf1:	echo "printf1"
$	gawk -f printf1.awk >tmp.
$	cmp printf1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$funsmnam:	echo "funsmnam"
$	set noOn
$	gawk -f funsmnam.awk >tmp. 2>&1
$	set On
$	cmp funsmnam.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnamedat:	echo "fnamedat"
$	set noOn
$	gawk -f fnamedat.awk < fnamedat.in >tmp. 2>&1
$	set On
$	cmp fnamedat.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$numindex:	echo "numindex"
$	set noOn
$	gawk -f numindex.awk < numindex.in >tmp. 2>&1
$	set On
$	cmp numindex.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$subslash:	echo "subslash"
$	set noOn
$	gawk -f subslash.awk >tmp. 2>&1
$	set On
$	cmp subslash.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$opasnslf:	echo "opasnslf"
$	set noOn
$	gawk -f opasnslf.awk >tmp. 2>&1
$	set On
$	cmp opasnslf.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$opasnidx:	echo "opasnidx"
$	set noOn
$	gawk -f opasnidx.awk >tmp. 2>&1
$	set On
$	cmp opasnidx.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arynocls:	echo "arynocls"
$	gawk -v "INPUT"=arynocls.in -f arynocls.awk >tmp.
$	cmp arynocls.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$igncdym:	echo "igncdym"
$	gawk -f igncdym.awk igncdym.in >tmp.
$	cmp igncdym.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getlnbuf:	echo "getlnbuf"
$	gawk -f getlnbuf.awk getlnbuf.in >tmp.
$	gawk -f gtlnbufv.awk getlnbuf.in >tmp2.
$	cmp getlnbuf.ok tmp.
$	if $status then  rm tmp.;
$	cmp getlnbuf.ok tmp2.
$	if $status then  rm tmp2.;
$	return
$
$arysubnm:	echo "arysubnm"
$	gawk -f arysubnm.awk >tmp.
$	cmp arysubnm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnparydl:	echo "fnparydl"
$	gawk -f fnparydl.awk >tmp.
$	cmp fnparydl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nondec:	echo "nondec"
$ !	gawk -f nondec.awk >tmp.
$ !	cmp nondec.ok tmp.
$ !	if $status then  rm tmp.;
$	return
$
$vms_io1:	echo "vms_io1"
$	if f$search("vms_io1.ok").eqs.""
$	then create vms_io1.ok
Hello
$	endif
$ !	define/User dbg$input sys$command:
$	gawk /Input=sys$input _NL: /Output=tmp.
# prior to 3.0.4, gawk crashed doing any redirection after closing stdin
BEGIN { print "Hello" >"/dev/stdout" }
$	cmp vms_io1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clean:
$	if f$search("tmp.")	 .nes."" then  rm tmp.;*
$	if f$search("tmp.too")	 .nes."" then  rm tmp.too;*
$	if f$search("out%.")	 .nes."" then  rm out%.;*
$	if f$search("strftime.ok").nes."" then  rm strftime.ok;*
$	if f$search("test%.")	 .nes."" then  rm test%.;*
$	if f$search("seq.")	 .nes."" then  rm seq.;*
$	if f$search("_pid.in")	 .nes."" then  rm _pid.in;*
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]")	 .nes."" then  rm []junk.dir;1
$	return
$
$!NOTREACHED
$ exit
