/*
 * $QNXLicenseC:
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
 * $
 */

typedef enum {
    DM_BASIC_TYPE = 0x00,
    DM_CHAR_ENCODING,
    DM_CHAR_ENCODING_SIGNED,
    DM_SET_INDEX,
    DM_POINTER,
    DM_ZAP_SPACE,
    DM_EMIT_SPACE,
    DM_RESET_INDEX,
    DM_ARRAY_PREFIX,
    DM_ARRAY_SIZE,
    DM_ARRAY_SUFFIX,
    DM_ARRAY,
    DM_OPEN_PAREN,
    DM_CLOSE_PAREN,
    DM_FUNCTION,
    DM_FUNCTION_PREFIX,
    DM_FUNCTION_SUFFIX,
    DM_FUNCTION_ARG_SEPARATOR,
    DM_THIS_FUNCTION,
    DM_UNMODIFIED_TYPE,
    DM_BASED_ENCODING,
    DM_BASED_SUFFIX,
    DM_BASED_SELF,
    DM_BASED_VOID,
    DM_BASED_STRING_PREFIX,
    DM_IDENTIFIER,
    DM_COPY_STRING,
    DM_BASED_STRING_SUFFIX,
    DM_MODIFIER_LIST,
    DM_TYPE_ENCODING,
    DM_TEMPLATE_ARG,
    DM_INTEGER,
    DM_RECURSE_BEGIN,
    DM_RECURSE_END,
    DM_TEMPLATE_NAME,
    DM_TEMPLATE_PREFIX,
    DM_TEMPLATE_SUFFIX,
    DM_TEMPLATE_ARG_SEPARATOR,
    DM_WATCOM_OBJECT,
    DM_ANONYMOUS_ENUM,
    DM_OPERATOR_PREFIX,
    DM_OPERATOR_FUNCTION,
    DM_RELATIONAL_FUNCTION,
    DM_ASSIGNMENT_FUNCTION,
    DM_OPERATOR_NEW,
    DM_OPERATOR_DELETE,
    DM_DESTRUCTOR_CHAR,
    DM_DESTRUCTOR,
    DM_CONSTRUCTOR,
    DM_OPERATOR_CONVERT,
    DM_CTOR_DTOR_NAME,
    DM_SCOPED_NAME,
    DM_NAME,
    DM_SCOPE,
    DM_SCOPE_SEPARATOR,
    DM_MANGLED_NAME,
    DM_TRUNCATED_NAME,
    DM_OPERATOR_DELETE_ARRAY,
    DM_OPERATOR_NEW_ARRAY,
    DM_INVALID
} dm_pts;
