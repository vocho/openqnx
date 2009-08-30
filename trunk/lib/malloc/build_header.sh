
test_func()
{
	index=`expr $count + 1`
	echo "		if (__prev) { \\"
	echo "			if (__malloc_bt_depth > $count) { \\"
	echo "				__my_builtin_return_address_n(__line, $index, __prev); \\"
	echo "				__prev = __line; \\"
	echo "				callerd[$index] = (unsigned *)__prev; \\"
	echo "				__total++; \\"

	count=`expr $count + 1`
	if [ $count -lt 123 ]
	then
		test_func
	fi
	echo "			} /* $index */\\"
	echo "		} /* $index */\\"
}

test_func2()
{
	test_func
}

cat << EOF 
/*
 * \$QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * \$
 */

/* This file is generated do not edit */
#define MALLOC_GETBT(callerd) \\
{ \\
	int __line=0; \\
	int __prev=0; \\
	int __total=0; \\
		__my_builtin_return_address_n(__line, 0, __prev); \\
		__prev = __line; \\
		callerd[0] = (unsigned *)__prev; \\
		__total++; \\
EOF

count=0
test_func

cat << EOF
}
EOF
