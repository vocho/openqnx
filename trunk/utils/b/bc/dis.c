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




#include <libc.h>
#include "bc.h"
#include "parse.h"

#define	p	bc_printf

void _disasm(int offs, int *start, int len);

void disasm(int *start, int len)
{
	_disasm(0, start, len);
}

void _disasm(int offs, int *start, int len)
{
	int            *cur = start;
	int            *end = start + len;

#define genoffs(cur)	((cur)-start+offs)
#define j(s) p("\t%s\t0x%x {.+%d}\n", s, genoffs(cur)+*cur, *cur)

	while (cur < end) {
		if (offs == -1)
			p("* ");
		else
			p("%4.8x:", genoffs(cur));
		switch (*cur++) {
		case JMP:
			j("jmp");
			cur++;
			break;
		case CALL:
			p("\tcall\t%c {%u}\n", *cur + 'a', *cur);
			cur++;
			break;
		case RETURN:
			p("\treturn\n");
			break;
		case JMPZ:
			j("jz");
			cur++;
			break;
		case JMPNZ:
			j("jnz");
			cur++;
			break;
		case PRINT:
			p("\tprint\n");
			break;
		case '\"':
			p("\tstr\t%s\n", *((char **) cur++));
			break;
		case DISCARD:
			p("\tdiscard\n");
			break;
		case CLRSTK:
			p("\tclearstk\n");
			break;
		case REPLACE:
			p("\treplace\n");
			break;
		case SAVEVAR:
			p("\tsavev\t%c\n", 'a' + *cur++);
			break;
		case RESTVAR:
			p("\trestv\t%c\n", 'a' + *cur++);
			break;
		case SAVEARRAY:
			p("\tsavea\t%c\n", 'A' + *cur++);
			break;
		case RESTARRAY:
			p("\tresta\t%c\n", 'A' + *cur++);
			break;
		case INDEX:
			p("\tindex\n");
			break;
		case PUSHVAR:
			p("\tpushv\t%c\n", 'a' + *cur++);
			break;
		case PUSHCONST:
			p("\tpushc\n");
		   cur++;
			break;
		case PUSHARRAY:
			p("\tpusha\t%c\n", 'A' + *cur++);
			break;
		case PUSHSPECIAL:
			switch (*cur++) {
			case IBASE:
				p("\tpush\tibase\n");
				break;
			case OBASE:
				p("\tpush\tobase\n");
				break;
			case SCALE:
				p("\tpush\tscale\n");
				break;
			default:
				p("\tpush\tunknown\n");
				break;
			}
			break;
		case POPVAR:
			p("\tpopv\t%c\n", 'a' + *cur++);
			break;
		case POPARRAY:
			p("\tpopa\t%c\n", 'A' + *cur++);
			break;
		case UMINUS:
			p("\tnegate\n");
			break;
		case PLUSPLUS | PREFIX:
			p("\t++(pre)\n");
			break;
		case MINUSMINUS | PREFIX:
			p("\t--(pre)\n");
			break;
		case PLUSPLUS | POSTFIX:
			p("\t(post)++\n");
			break;
		case MINUSMINUS | POSTFIX:
			p("\t(post)--\n");
			break;
		case '=':
			p("\t=\n");
			break;
		case ASG_PLUS:
			p("\t+=\n");
			break;
		case ASG_MINUS:
			p("\t-=\n");
			break;
		case ASG_STAR:
			p("\t*=\n");
			break;
		case ASG_DIV:
			p("\t/=\n");
			break;
		case ASG_MOD:
			p("\t%=\n");
			break;
		case ASG_EXP:
			p("\t^=\n");
			break;
			/* binary operators */
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '^':
			p("\t%c\n", cur[-1]);
			break;
			/* relational operators */
		case EQ:
			p("\t==\n");
			break;
		case NE:
			p("\t!=\n");
			break;
		case LE:
			p("\t<=\n");
			break;
		case '<':
			p("\t<\n");
			break;
		case '>':
			p("\t>\n");
			break;
		case GE:
			p("\t>=\n");
			break;
			/* built-ins */
		case SCALE:
			p("\tscale\n");
			break;
		case LENGTH:
			p("\tlength\n");
			break;
		case SQRT:
			p("\tsqrt\n");
			break;
		default:
			p("\t	dw\t{%x=%d}\n", cur[-1], cur[-1]);
		}
	}
}
