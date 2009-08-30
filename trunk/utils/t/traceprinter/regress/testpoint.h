/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */





#ifndef __TESTPOINT_H
#define __TESTPOINT_H
/*****************************************************************************
*
*	File:	testpoint.h
*
******************************************************************************
*
*   Contents:	QA Neutrino test suite macros
*
*	Date:		May	30, 1995.
*
*	Author:		Kirk Russell.
*
*	$Id: testpoint.h 153052 2008-08-13 01:17:50Z coreos $
*
*****************************************************************************/

/*--------------------------------------------------------------------------*
 *								STANDARD HEADERS 							*
 *--------------------------------------------------------------------------*/
/*
 * assert() must not be disabled.
 */
#ifdef NDEBUG
#error NDEBUG cannot be defined
#endif

/*
 *  The default is to use stdio for the output of the test macros.
 *
 */
#include <stdio.h>

#ifndef PRNT
#define PRNT	printf
#define _prnt_errcount 0
#endif
#ifndef SPRNT
#define SPRNT	snprintf
#endif

#if defined (__cplusplus) || defined(__CPLUSPLUS__)
 extern "C" {
#endif

/*
 * Define this here, instead of including extra stuff
 */
extern char *  rindex(const char *s, int c);

/****************************************************************************
*
*						Macro teststart
*
*****************************************************************************
*
*	Purpose:	Use this macro once at the start of your Neutrino program.
*				A "START" message will be added to the log file.
*
*	Input:
*		name:	A string containing the name of the executable.
*
****************************************************************************/
extern void teststart(const char*);
#define teststart(name) \
	PRNT("START: %s, pid %d\n", \
		name, getpid())


/****************************************************************************
*
*						Macro teststop
*
*****************************************************************************
*
*	Purpose:	Use this macro once at the end of your Neutrino program.
*				A "STOP" message will be added to the log file.
*
*	Input:
*		name:	A string containing the name of the executable.
*
****************************************************************************/
extern void teststop(const char*);
#define teststop(name) \
	PRNT("STOP:  %s, pid %d\n", \
		name, getpid())

/****************************************************************************
*
*						Macro testpntbegin
*
*****************************************************************************
*
*	Purpose:	Use this macro once before testing a test point.
*				A "POINT" message will be added to the log file.
*
*	Input:
*		summary:	A string containing a brief summary of what is to be
*					tested.
*
*	Notes:
*		The macros 'testpntbegin' and 'testpntend' are special macros.
*		They shall appear in pairs within the same lexical scope.
*		Treat the 'testpntbegin' macro as expanding to '{' token and
*		the 'testpntend' macro as expanding to the coresponding '}' token.
*
*		One of the macros 'testpntpass' or 'testpntfail' must be used
*		before 'testpntend'.
*
*		The __fbname member is included to print only the basename of the
*		source file.  Most of our makefiles use VPATH, which appears to
*		cause __FILE__ to be set to the large absolute path of the file.
*
****************************************************************************/
extern void testpntbegin(const char*);
#define testpntbegin(summary) \
	{ \
	int __xfail = 0; \
	unsigned __errsave = _prnt_errcount; \
	int __pntcnt = 0; \
	char* __xfailmsg = ""; \
	const volatile char* volatile __fbname = ((char*)0);	\
	__fbname = ((__fbname= rindex(__FILE__,'/')) ? __fbname+1 : __FILE__);\
	PRNT("POINT: file %s, line %d %s\n", \
		__fbname, __LINE__, summary)

/****************************************************************************
*
*						Macro testpntend
*
*****************************************************************************
*
*	Purpose:	Use this macro once after testing a test point.
*
*	Notes:
*		The macros 'testpntbegin' and 'testpntend' are special macros.
*		They shall appear in pairs within the same lexical scope.
*		Treat the 'testpntbegin' macro as expanding to '{' token and
*		the 'testpntend' macro as expanding to the coresponding '}' token.
*
****************************************************************************/
extern void testpntend();
#define testpntend() \
	{ if (__errsave != _prnt_errcount) testwarning("internal i/o error"); } \
	}


/****************************************************************************
*
*						Macro testpntsetupxfail
*
*****************************************************************************
*
*	Purpose:	Indicate that the test point is expect to fail.
*
*	Input:
*		msg:	A string containing the GNATS bugid.
*
*	Notes:
*		This macro should only be used within the scope created by
*		the macros 'testpntbegin' and 'testpntend' are special macros.
*
*		This test point is expected to fail.  Now the 'testpntpass' and
*		'testpntfail' macros will produce "PASSX" and "FAILX" messages
*		respectively.  The will allow the tester to distinguish between
*		expected failures from other failures.
*
*		If the test point has another outcome, for example a real failure,
*		you can use the macro "testpntclearxfail" to clear the expected
*		failure mechanism.
*
****************************************************************************/
extern void testpntsetupxfail(const char*);
#define testpntsetupxfail(msg) \
	__xfail = 1, \
	__xfailmsg = msg


/****************************************************************************
*
*						Macro testpntclearxfail
*
*****************************************************************************
*
*	Purpose:	Cancel an expected failure setup by the 'testpntsetupxfail'
*				macro.
*
*	Notes:
*		This macro should only be used within the scope created by
*		the macros 'testpntbegin' and 'testpntend' are special macros.
*
****************************************************************************/
extern void testpntclearxfail();
#define testpntclearxfail()\
	__xfail = 0, \
	__xfailmsg = ""


/****************************************************************************
*
*						Macro testpntpass
*
*****************************************************************************
*
*	Purpose:	This test point finished successfully.
*				A "PASS" ("PASSX", if the failure was expected) message will
*				be added to the log file.
*
*	Input:
*		msg:	A string containing a brief summary of why the test passed.
*
*	Notes:
*		This macro should only be used within the scope created by
*		the macros 'testpntbegin' and 'testpntend' are special macros.
*
****************************************************************************/
extern void testpntpass(const char*);
#define testpntpass(msg) \
	PRNT("%s%s, line %d%s%s %s\n", \
	(__xfail) ? "PASSX: file " : "PASS:  file ", \
	__fbname, __LINE__, (__xfail) ? " bugid " : "", __xfailmsg, msg), \
	__pntcnt++


/****************************************************************************
*
*						Macro testpntfail
*
*****************************************************************************
*
*	Purpose:	This test point finished unsuccessfully.
*				A "FAIL" ("FAILX", if the failure was expected) message will
*				be added to the log file.
*
*	Input:
*		msg:	A string containing a brief summary of why the test failed.
*
*	Notes:
*		This macro should only be used within the scope created by
*		the macros 'testpntbegin' and 'testpntend' are special macros.
*
****************************************************************************/
extern void testpntfail(const char*);
#define testpntfail(msg) \
	PRNT("%s%s, line %d%s%s %s\n", \
	(__xfail) ? "FAILX: file " : "FAIL:  file ", \
	__fbname, __LINE__, (__xfail) ? " bugid " : "", __xfailmsg, msg), \
	__pntcnt++

/****************************************************************************
*
*						Macro testpntunres
*
*****************************************************************************
*
*	Purpose:	This test point did not fail or pass.  The test point
*				is to be considered unresolved.
*
*	Input:
*		msg:	A string containing a brief summary of why the test is
*				to be considered unresolved.
*
*	Notes:
*		This macro should only be used within the scope created by
*		the macros 'testpntbegin' and 'testpntend' are special macros.
*
****************************************************************************/
extern void testpntunres(const char*);
#define testpntunres(msg) \
	__xfail = __xfail, \
	__xfailmsg = __xfailmsg, \
	PRNT("UNRES: file %s, line %d %s\n", __fbname, __LINE__, msg), \
	__pntcnt++


/****************************************************************************
*
*						Macro __testnote
*
*****************************************************************************
*
*	Purpose:	Append a message to the log file.
*				For internal use in this include file only.
*				This should be considered a private member function.
*
*	Input:
*		msg:	A string containing a brief message.
*		type:	A five character string containing the message id
*
****************************************************************************/
extern void __testnotes(const char*, const char*);
#define __testnote(msg, type) \
	PRNT("%s  file %s, line %d %s\n", type, \
	(rindex(__FILE__,'/')) ? rindex(__FILE__,'/')+1 : __FILE__, __LINE__, msg)


/****************************************************************************
*
*						Macro testnote
*
*****************************************************************************
*
*	Purpose:	Append an informational message to the log file.
*				A "NOTE" message will be added to the log file.
*
*	Input:
*		msg:	A string containing a brief informational message.
*
****************************************************************************/
extern void testnote(const char*);
#define testnote(msg) __testnote(msg, "NOTE:")

/****************************************************************************
*
*						Macro testwarning
*
*****************************************************************************
*
*	Purpose:	A minor error was discovered.  The test can recover from
*				this error.
*				A "WARN" message will be added to the log file.
*
*	Input:
*		msg:	A string containing a brief warning message.
*
****************************************************************************/
extern void testwarning(const char*);
#define testwarning(msg) __testnote(msg, "WARN:")


/****************************************************************************
*
*						Macro testwarning
*
*****************************************************************************
*
*	Purpose:	A severe error was discovered.  The test cannot recover from
*				this error.
*				A "ERR" message will be added to the log file.
*
*	Input:
*		msg:	A string containing a brief warning message.
*
****************************************************************************/
extern void testerror(const char*);
#define testerror(msg) __testnote(msg, "ERR: ")

#if defined (__cplusplus) || defined(__CPLUSPLUS__)
 };
#endif

#endif
