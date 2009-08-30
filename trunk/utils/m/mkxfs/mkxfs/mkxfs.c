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



// Hidden option 'N' to allow for more than 4 bootstrap executables.
// Document later...
#ifdef __USAGE
%-mkxfs

%C - make a image/flash file system

%C	-t type [-r root] [-l input] [-s section] [-nv] [in-file [out-file]]

Options:
 -t ffs2|ffs3|ifs|etfs Set the type of the output file system.
 -l input              Prefix a line to the input-file.
 -n                    No timestamps. Allows for binary identical images. One
                       'n' will strip timestamps from files which vary from run
                       to run. More than one will strip ALL time information
                       which is necessary on Windows NTFS with daylight savings
                       time.
 -r root               Search the default paths in this directory before the
                       default.
 -s section            Do not strip the named section from ELF executable 
                       when creating an IFS image.
 -v                    Operate verbosely.
 -a suffix             Append suffix to symbol files generated via [+keeplinked]
%-mkefs

%C - make an embedded (flash) file system

%C	[-l inputline] [-nv] [input-file [output-file]]

Options:
 -t ffs2|ffs3   Set the type of the output file system.
 -c cache_dir   Cache compressed files in cache_dir.
 -l inputline   Prefix a line to the input-file.
 -n             No timestamps. Allows for binary identical images.  One 'n'
                will strip timestamps from files which vary from run to run.
                More than one will strip ALL time information which is 
                necessary on Windows NTFS with daylight savings time.
 -v             Operate verbosely.
%-mketfs

%C - make an embedded transaction file system

%C	[-l inputline] [-nv] [input-file [output-file]]

Options:
 -l inputline   Prefix a line to the input-file.
 -n             No timestamps. Allows for binary identical images.  One 'n'
                will strip timestamps from files which vary from run to run.
                More than one will strip ALL time information which is 
                necessary on Windows NTFS with daylight savings time.
 -v             Operate verbosely.
%-mkifs

%C - make an image file system

%C	[-r root] [-l input] [-s section] [-nv] [in-file [out-file]]

Options:
 -l input       Prefix a line to the input-file.
 -n             No timestamps. Allows for binary identical images.  One 'n'
                will strip timestamps from files which vary from run to run.
                More than one will strip ALL time information which is 
                necessary on Windows NTFS with daylight savings time.
 -p             Do not strip PhAB resource information. (experimental)
 -r root        Search the default paths in this directory before the default.
 -s section     Do not strip the named section from ELF executable 
                when creating an IFS image.
 -v             Operate verbosely.
#endif

#include <fcntl.h>
#include <lib/compat.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "struct.h"
#include <malloc.h>


#if defined(VARIANT_le)
	#define ENDIAN_STRING	"le"
#elif defined(VARIANT_be)
	#define ENDIAN_STRING	"be"
#endif

#if defined(DEFAULT_CPU)
	//Nothing to do
#elif defined(__386__) || defined(__X86__)
	#define DEFAULT_CPU	"x86"
#elif defined(__SH__)
	#define DEFAULT_CPU	"sh" ENDIAN_STRING
#elif defined(__ARM__)
	#define DEFAULT_CPU	"arm" ENDIAN_STRING
#elif defined(__MIPS__)
	#define DEFAULT_CPU	"mips" ENDIAN_STRING
#elif defined(__PPC__)
	#define DEFAULT_CPU	"ppc" ENDIAN_STRING
#elif defined(__SOLARIS__) || defined(linux)
	#define DEFAULT_CPU	"mipsbe"
#else
	#error DEFAULT_CPU not defined
#endif

struct token_state	*token;

#define GLOBENVC		100
char				*globenvv[GLOBENVC];
int					 globenvc;

char				*boot_attr_buf;
char				*option_lines;

char				*symfile_suffix;

struct input_buffer {
	char			*data;
	unsigned		len;
};

#define INPUT_FILE_LEN	4000
struct input_buffer	input_file;

#define INPUT_SCRIPT_LEN	1000
struct input_buffer	input_script;

#define INPUT_BOOT_LEN		1000
struct input_buffer	input_boot;

struct tmpfile_entry		*tmpfile_list;

struct attr_file_entry		 file_attr;
struct attr_file_list		*attr_file_list;

struct attr_script_entry	 script_attr;
struct attr_script_list		*attr_script_list;

struct file_entry	 *file_list;

int					 verbose;
int					 host_endian;	// 0 - little,  1 - big
int					 target_endian = -1;// 0 - little,  1 - big,  -1 - unknown
FILE				*debug_fp;
FILE				*script_fp;
unsigned			 line_num;
char				*cache_dir;
int					new_style_bootstrap;
int					ext_sched = SCRIPT_SCHED_EXT_NONE;

int no_time;

struct {
				char	name[SCRIPT_APS_PARTITION_NAME_LENGTH + 1];
				int		budget;
}		globapsv[SCRIPT_APS_MAX_PARTITIONS] = {
			{ SCRIPT_APS_SYSTEM_PARTITION_NAME, 100 }
		};
int		globapsc = SCRIPT_APS_SYSTEM_PARTITION_ID;

int aps_lookup(char *aps)
{
int		i;

	for (i = 0; i <= globapsc; ++i)
		if (!strcmp(globapsv[i].name, aps))
			return(i);
	return(-1);
}
char *aps_parse(char *token1, char **pname, char *token2, int *budget, char *token3, int *critical)
{
char	*cp;

	if (strlen(*pname = token1) > SCRIPT_APS_PARTITION_NAME_LENGTH)
		return("APS partition name too long.\n");
	else if (strchr(*pname, '/') != NULL)
		return("Invalid APS partition name.\n");
	else if (aps_lookup(*pname) != -1)
		return("Duplicate APS partition name.\n");
	else if (globapsc >= SCRIPT_APS_MAX_PARTITIONS - 1)
		return("Too many APS partitions defined.\n");
	*budget = strtol(token2, &cp, 10);
	if (cp[0] == '%')
		cp += 1;
	if (*cp != '\0' || *budget < 0 || *budget > 100)
		return("Invalid APS partition budget.\n");
	else if (*budget > globapsv[SCRIPT_APS_SYSTEM_PARTITION_ID].budget)
		return("Total APS budget exceeds 100%%.\n");
	*critical = strtol(token3, &cp, 10);
	if (cp[0] == 'm' && cp[1] == 's')
		cp += 2;
	if (*cp != '\0' || *critical < 0 || *critical > 1000)
		return("Invalid APS partition critical time.\n");
	++globapsc;
	strncpy(globapsv[globapsc].name, *pname, SCRIPT_APS_PARTITION_NAME_LENGTH + 1);
	globapsv[SCRIPT_APS_SYSTEM_PARTITION_ID].budget -= (globapsv[globapsc].budget = *budget);
	return(NULL);
}

static void (*parse_file_init)(struct attr_file_entry *attrp);
static void (*parse_file_attr)(int tokenc, char *tokenv[], struct attr_file_entry *attrp);
static int (*need_seekable)(struct file_entry *list);
static unsigned (*make_fsys)(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname);

#include "xplatform.h"

void
set_cpu(const char *name, int overwrite) {
	char		*new;
	unsigned	n;
	
	setenv("PROCESSOR", name, overwrite);
	n = strlen(name);
	if(n > 2
	 && name[n-1] == 'e'
	 && (name[n-2] == 'b' || name[n-2] == 'l')) {
		 new = alloca(n - 1);
		 n -= 2;
		 memcpy(new, name, n);
		 name = new;
		 new[n] = '\0';
	} 
	setenv("CPU_BASE", name, overwrite);
}

#ifdef __GNUC__
void error_exit(char *format, ...) __attribute__((noreturn, format(printf, 1, 2)));
#endif
void
error_exit(char *format, ...) {
	va_list arglist;

	if(line_num != 0) {
		fprintf(stderr, "Line %u: ", line_num);
	}
	va_start(arglist, format);
	vfprintf(stderr, format, arglist);
	va_end(arglist);

	exit(1);
}

static void
rm_tmpfiles() {
	struct tmpfile_entry	*tmp;

	for(tmp = tmpfile_list ; tmp ; tmp = tmp->next)
		unlink(tmp->name);
}

static void
die(int signum) {
	rm_tmpfiles();
	_exit(1);
}
	

static void
add_data(char *string) {
	unsigned	size;
	char		*new;

	size = (option_lines == NULL) ? 0 : strlen(option_lines);
	new = realloc(option_lines, size + strlen(string) + 1);
	if(new == NULL) {
		error_exit("No memory for -l option.\n");
	}
	option_lines = new;
	strcpy(&new[size], string);
}


static void
set_target_name(struct file_entry *fip) {
	char *	tbuf = NULL;
	char	*s;
	char	*d;

	s = fip->targpath;
	if(!IS_ABSPATH(s)) {
		tbuf = malloc(strlen(fip->attr->prefix) + strlen(s) + 2);
		if (tbuf == NULL) {
			error_exit("No memory for file path.\n");
		}
		sprintf(tbuf, "%s/%s", fip->attr->prefix, s);
	} else {
		tbuf = strdup(s);
	}
	s = d = tbuf;
	s = SKIP_DRIVE(d);		// Strip off any driver letter
	while(*s != '\0') {
		if (IS_DIRSEP(*s)) *s = '/'; // Switch to forward slashes 
		// Strip the silly '.' out of stuff like /prefix/./path on the target.
		if(s[0] == '/' && s[1] == '.'  && (IS_DIRSEP(s[2]) || s[2] == '\0')) {
			s += 2;
		}
		*d++ = *s++;
	}
	*d = '\0';
	free(fip->targpath);
	s = tbuf;
	while(*s == '/') ++s;
	fip->targpath = strdup(s);
	if (tbuf) free (tbuf);
}


char **
env_lookup(char *env, int envc, char *envv[]) {
	int 	i, n;
	char	*p;

	p = strchr(env, '=');
	if(p == NULL) {
		n = strlen(env);
	} else {
		n = p - env;
	}
	for(i = 0 ; i < envc ; ++i) {
		p = envv[i];
		if(p && strncmp(env, p, n) == 0 && p[n] == '=') {
			return(&envv[i]);
		}
	}

	return(NULL);
}


void
env_add(char *env) {
	char	**sp;
	int		i;

	if( (sp = env_lookup(env, globenvc, globenvv)) ) {
		*sp = strdup(env);
		return;
	}

	for(i = 0 ; i < GLOBENVC ; ++i)
		if(globenvv[i] == NULL) {
			globenvv[i] = strdup(env);
			if(++i > globenvc) globenvc = i;
			break;
		}
}


char *
checkenv(char *name) {
	char	**sp;

	if( (sp = env_lookup(name, globenvc, globenvv)) ) {
		return(strchr(*sp, '=') + 1);
	}
	return(getenv(name));
}

//
// Recursive getenv - Expand out strings of the form ${name} recursively.
// Search mkifs variables as well as environ.
//

#define MAX_DEPTH	20

char *
rgetenv(char *string) {
	char				*p;
	char				*dest = NULL;
	unsigned			size = 0;
	unsigned			i = 0;
	int					env_depth;
	int					inp_depth;
	unsigned			env_start[MAX_DEPTH];
	struct {
		char		*p;
	}					in[MAX_DEPTH];
	char				c;

	env_depth = -1;
	inp_depth = 0;
	in[0].p = string;
	for( ;; ) {
		if(i >= size) {
			size += 32;
			p = realloc(dest, size);
			if(p == NULL) {
				error_exit("No memory to expand variable.\n");
			}
			dest = p;
		}
		c = *in[inp_depth].p++;
		switch(c) {
		case '\0':
			if(inp_depth == 0) {
				dest[i] = '\0';
				return(dest);
			}
			--inp_depth;
			break;
		case '}':
			if(env_depth >= 0) {
				dest[i] = '\0';
				i = env_start[env_depth--];
				p = checkenv(&dest[i]);
				if(p!=NULL) {
					if(++inp_depth > MAX_DEPTH) {
						error_exit("Too many nested expansions.\n");
					}
					in[inp_depth].p = p;
				}
			}
			break;
		case '$':
			if(*in[inp_depth].p == '{') {
				if(++env_depth > MAX_DEPTH) {
					error_exit("Too many nested expansions.\n");
				}
				env_start[env_depth] = i;
				in[inp_depth].p++;
				break;
			}
			/* fall through */
		default:
			dest[i++] = c;
			break;
		}
	}
}


unsigned long
getsize(char *str, char **dst) {
	unsigned long v;
	
	v = strtoul(str, &str, 0);
	switch(*str) {
	case 'G':
	case 'g':
		v *= 1024L;
	case 'M':
	case 'm':
		v *= 1024L;
	case 'K':
	case 'k':
		v *= 1024L;
		str++;
		break;
	}
	if (dst) *dst = str;
	return v;
}


short int
swap16(int target_endian, int val) {

	if(target_endian < 0)
		error_exit("Target endian is not known.\n");

	return(host_endian != target_endian ? SWAP16(val) : val);
}


long int
swap32(int target_endian, int val) {

	if(target_endian < 0)
		error_exit("Target endian is not known.\n");

	return(host_endian != target_endian ? SWAP32(val) : val);
}



char *
mk_tmpfile() {
	char					*tmpname;
	struct tmpfile_entry	*tmp = NULL;
#if defined (__WIN32__) || defined(__NT__)
	char 					*dostmp;
#endif

	if( !(tmpname = tmpnam(NULL)) || !(tmp = malloc(sizeof(*tmp))) )
		error_exit("Unable to get a tmp file name.\n");
/*
 * The hack in find_file() to make sure that paths are done correctly breaks
 * win32 in the situation where a temporary file is created in the current
 * working directory.  ie. find_file() fails to find the created temporary file.
 * The solution (for now) is to make sure there is a '.\' in front of the 
 * temporary filename.  The other solution may be to figure out why find_file()
 * doesn't work in this case.  It's all just so UGLY!! :-)
 */		
#if defined(__WIN32__) || defined(__NT__)
	if(tmpname[0] != '\\' && strchr(tmpname,'\\')){ /* is there a pathsep char in it? */
		if( !(tmpname = strdup(tmpname)) )
			error_exit("Unable to get memory for a temp file.\n");
	}
	else {
		if( !(dostmp=(char *)calloc(strlen(tmpname)+3,sizeof(char))) )
			error_exit("Unable to get memory for a temp file.\n");
		if( tmpname[0] == '\\' ) {
		    sprintf(dostmp,".%s",tmpname);
		} else {
		    sprintf(dostmp,".\\%s",tmpname);
		}
		tmpname=dostmp;
	}
#else
	if( !(tmpname = strdup(tmpname)) )
		error_exit("Unable to get memory for a tmp file.\n");
#endif
	tmp->next = tmpfile_list;
	tmp->name = tmpname;
	tmpfile_list = tmp;

	return(tmpname);
}



int
decode_attr(int report_err, struct attr_types *atp, char *name, int *ivalp, char **svalp) {
	char	*s;
	int		 n;
	int		 conditional;

	if(name[1] == '\0')
		error_exit("Malformed attribute.\n");
		
	if( name[0] == '?' ) {
		++name;
		conditional = 1;
	} else {
		conditional = 0;
	}

	if(*name == '+'  ||  *name == '-') {
		s = name + 1;
		n = strlen(s);
		*ivalp = (*name == '+') ? 1 : 0;
	} else if( (s = strchr(name, '=')) ){
		*ivalp = getsize(s + 1, NULL);
		*svalp = s + 1;
		n = (s - name) + 1;
		s = name;
	} else {
		error_exit("Malformed attribute: %s.\n", name);
		n = 0;
	}

	while(atp->name) {
		if(strncmp(s, atp->name, n) == 0) {
			int		 attr;

			attr = atp->code & ~ATTRIBUTE_SET;
			if(conditional && (atp->code & ATTRIBUTE_SET)) {
				attr = -2;
			}
			atp->code |= ATTRIBUTE_SET;
			return(attr);
		}
		++atp;
	}

	if(report_err) error_exit("Unknown attribute: %s.\n", name);
	return(-1);
}


struct token_state *
push_token_state() {
	struct token_state	*new;

	new = malloc(sizeof(*new));
	if(new == NULL) {
		error_exit("No memory for token state.\n");
	}
	new->prev = token;
	token = new;
	return(token->prev);
}

void
pop_token_state() {
	struct token_state	*old;

	old = token;
	token = old->prev;
	free(old);
}


//
// Take an input buffer and tokenize it into another buffer with an argv
// array pointing to each token. We return where the scan stopped.
//
char *
tokenize(char *input, char *space, char term) {
	unsigned n, len;
	char *s1, *s2, quoting;

	for(s1 = input, s2 = token->buf, n = 0, len = 0; *s1 && *s1 != term; ++s1) {
		if(line_num != 0 && *s1 == '\n') ++line_num;
		if(strchr(space, *s1)) {
			// Skip leading white space
			continue;
		}
		if(*s1 == '#') {
			//Eat comment
			s1 = strchr(s1, '\n') - 1;
			continue;
		}

		if(n >= TOKENC)
			error_exit("Too many arguments.\n");

		token->v[n] = s2;		// Save pointer to start of token
		quoting = 0;
		while(*s1  &&  (quoting  ||  !strchr(space, *s1))) {
            
        		if(len>=TOKENLEN)
            			error_exit("Arguments too long.\n");

			if(*s1 == '\\'  &&  
					(*(s1 + 1) == '\\' ||
			    	 *(s1 + 1) == '\n' || 
			    	 *(s1 + 1) == '$'  || 
			    	 *(s1 + 1) == term || 
					 *(s1 + 1) == '"')) {
		                len++;
				if((*s2++ = *++s1) == '\n') {
					len--;
					--s2;	// Eat escaped newline.
				}
					
			// Handle env vars
			} else if(!quoting  &&  *s1 == '$'  &&  *(s1 + 1)) {
				char *s3, delim = '\0', *delim_p;

				/* temporarily null terminate the string
				 * for rgetenv, restoring delimiter at end
				 * and setting s1 appropriately (fix for
				 * PR:9834)*/
				delim_p = strpbrk(s1+1, "$][/\\ \n\t}");
				if(delim_p){
					if(*delim_p == '}')
						delim_p++;
					delim = *delim_p;
					*delim_p = '\0';
				}
				if( (s3 = rgetenv(s1)) ){
					strncpy(s2, s3, TOKENLEN-len);
					s2[TOKENLEN-len-1]='\0';
					s2 += strlen(s3);
                    			len += strlen(s3);
					free(s3);
				}
				if(delim_p){
					*delim_p = delim;
					s1 = delim_p - 1;
				}

			// Handle quotes
			} else if(*s1 == '"') {
				quoting = !quoting;

			// Check for termination character
			} else if((*s1 == term) && (!quoting || (term=='\n'))) {
				--s1;
				break;

			// Default is to just copy the character
			} else {
				*s2++ = *s1;
                		len++;
			}

			if(*s1) ++s1;
		}

		*s2++ = '\0';
        	len++;
		++n;
	}

	token->c = n;

	return(s1);
}

/* Determine if an attribute is already present in the list */
struct attr_file_list *
attr_in_attr_file_list(struct attr_file_entry *attrp) {
	struct attr_file_list *list;

	// Try and match an existing attribute.
	for(list = attr_file_list; list; list = list->next)
		if(memcmp(attrp, &list->attr, sizeof(*attrp)) == 0)
			return(list);
	return(NULL);
}

/* Copy the attribute to the list of attributes return pointer to new one*/
struct attr_file_list *
copy_attr_to_attr_file_list(struct attr_file_entry *attrp) {
	struct attr_file_list *list;

	list = calloc(1, sizeof(*list));
	if(list == NULL) {
		error_exit("No memory for attribute list.\n");
	}
	memcpy(&list->attr, attrp, sizeof(*attrp));
	list->next = attr_file_list;
	attr_file_list = list;
	return(list);
}

struct attr_file_list *
add_attr(struct attr_file_entry *attrp) {
	struct attr_file_list	*new;

	new = attr_in_attr_file_list(attrp);
	if(new == NULL) new = copy_attr_to_attr_file_list(attrp);
	return(new);
}

/* Determine if a directory has already been added to the file_entry_list */
struct file_entry * 
dir_in_file_list(char *hostpath, char *targpath) {
	struct file_entry *tmp_entry;

	tmp_entry = file_list;
	while (tmp_entry) {
		if ((strcmp(tmp_entry->hostpath, hostpath) == 0) &&
		    (strcmp(tmp_entry->targpath, targpath) == 0)) {
			return(tmp_entry);
		}
		tmp_entry = tmp_entry->next;
	}
	return(NULL);
}

/* Used for directory cycle detection */
void
push_stack(struct inode_stack *is, ino_t inode) {
	if (!is->inodes || is->count >= is->size) {
		is->size += STACK_GROW;	
		is->inodes = (ino_t*)realloc(is->inodes, is->size*sizeof(ino_t));
		if(is->inodes == NULL) {
			error_exit("No memory for directory cycle detection stack.\n");
		}
	}
	is->inodes[is->count] = inode;
	is->count++;
}

int 
inode_in_stack(struct inode_stack *is, ino_t inode) {
//Windows, inodes?  who knows what they look like
#if !defined(__WIN32__) && !defined(__NT__)
	int indx;
	for (indx = is->count-1; indx >= 0; indx--) {
		if (is->inodes[indx] == inode) {
			return(1);
		}
	}
#endif
	return(0);
}

void 
pop_stack (struct inode_stack *is) {
	if (is->inodes && is->count > 0)
		is->count--;
}

void
destroy_stack(struct inode_stack *is) {
	if (is->inodes)
		free(is->inodes);
	is->inodes = NULL;
	is->size = 0;
	is->count = 0;
}


char *
find_file(char *search, char *hbuf, struct stat *sbuf, char *host, int optional) {
	char	*ifsp;
	char	*start;
	char	*tempsearch;

	/*
	 * The path wars...  If and only if there is a DIRSEP character anywhere
	 * in the host filename (ie: under qnx 'blah/blah' has a '/'), then the current 
	 * working directory ('./') is searched first.
	 */
	if(!IS_ABSPATH(host)) {
		if(search == NULL) search = file_attr.search_path;
		ifsp = start = rgetenv(search);
		if(HAS_PATH(host)) {
			tempsearch = (char*)malloc(strlen(ifsp) + strlen(PATHSEP_STR) + 1);
			if (tempsearch == NULL) {
				error_exit("No memory for temporary path.\n");
			}
			strcpy(tempsearch, PATHSEP_STR);
			strcat(tempsearch, ifsp);
			ifsp = start = tempsearch;
		}
		if (verbose > 5) {
    		fprintf(debug_fp,"search path %s\n", ifsp);
		}
		for( ;; ) {
			unsigned	len;
			char		*end;

			end = strchr(ifsp, PATHSEP_CHR);
			if(end == NULL) {
				len = strlen(ifsp);
			} else {
				len = end - ifsp;
			}
			memcpy(hbuf, ifsp, len);
			if(len > 0 && !IS_DIRSEP(hbuf[len-1])) hbuf[len++] = '/';
			strcpy(&hbuf[len], host);
			if(stat(hbuf, sbuf) != -1) break;
			if(end == NULL) {
				if(optional) {
					return(NULL);
				}
				error_exit("Host file '%s' not available.\n", host);
			}
			ifsp = end + 1;
		}
		free(start);
		host = hbuf;
	} else if(stat(host, sbuf) == -1) {
		if(optional)
			return(NULL);
		error_exit("Host file '%s' not available.\n", host);
	}
	return(host);
}

static void
collect_file(struct input_buffer *buf, unsigned grow, char *type, FILE *fp) {
	int					n;
	char				*new;
	unsigned			new_size;

	//
	// Collect file into a single memory buffer.
	//
	n = 0;
	for( ;; ) {
		new_size = buf->len + grow;
		new = realloc(buf->data, new_size);
		if(new == NULL) {
			error_exit("No memory for input %s file buffer.\n", type);
		}
		buf->data = new;
		n = fread(&new[buf->len], 1, grow, fp);
		if(n == -1) {
			error_exit("Error reading %s file: %s.\n", type, strerror(errno));
		}
		if(n < grow) break;
		buf->len = new_size;
	}
	buf->len += n;
	if((buf->len + 1) >= new_size) {
		new_size = buf->len + 2;
		new = realloc(buf->data, new_size);
		if(new == NULL) {
			error_exit("No memory for input %s file buffer.\n", type);
		}
		buf->data = new;
	}
	new[buf->len+0] = '\n'; //Make sure there's a terminating newline
	new[buf->len+1] = '\0';
}

char *
cache_file(char *host, char *target, struct attr_file_entry *attrp)
{
	static char cfile[1024];
	char cmd[1024], *p;
	int i=0;
	struct stat hbuf, cbuf;

	sprintf(cfile, "%s/%s", cache_dir, target ? target : host);

	/* First we check that the path is there, creating intermediate
	   directories if need be. */
	strcpy(cmd, cfile);
	strcpy(cmd, dirname(cmd));
	if(cmd[1] == ':')
		p = &cmd[3];
	else{
		while(cmd[i] && (cmd[i] == '/' || cmd[i] == '\\'))
			i++;
		p = &cmd[i];
	}

	while(1){
		if(*p == '/' || *p == '\\' || *p == '\0'){
			char c = *p;
			struct stat buf;
			*p = '\0';
			if(stat(cmd, &buf) == -1 && errno == ENOENT){
				if(gen_mkdir(cmd, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
					error_exit("Unable to make directory %s in cache directory %s.\n", cmd, cache_dir);
			}
			*p = c;
		}
		if(*p == '\0')
			break;
		p++;
	}

	if(stat(cfile, &cbuf) == 0 && stat(host, &hbuf) == 0 && hbuf.st_mtime <= cbuf.st_mtime)
		return cfile;

	sprintf(cmd, "%s <%s >%s", attrp->filter, host, cfile);
#if defined (__WIN32__) || defined(__NT__)
	fixenviron(cmd, sizeof(cmd));
#endif
	if(system(cmd) != 0)
		error_exit("Filter %s failed.\n", cmd);
	return cfile;
}

struct file_entry *
add_file(struct file_entry **list, char *host, char *target,
			struct attr_file_entry *attrp, struct stat *sbuf) {
	static struct file_entry	*end = NULL;
	struct file_entry			*fip;
	char						*orig_host;

	orig_host = host;
	//
	// Look for a filter to run on the file.
	// TF Ammended: Don't filter links and directories, 
	//              filter should be set to null in any case
	//
	if(!S_ISDIR(attrp->mode) && attrp->mode != S_IFLNK && attrp->filter) {
		char *tfile, cmd[1024];

		if(cache_dir && strstr(attrp->filter, "flashcmp") != NULL){
			tfile = cache_file(host, target, attrp);
		}
		else {
			tfile = mk_tmpfile();
			sprintf(cmd, "%s <%s >%s", attrp->filter, host, tfile);
#if defined (__WIN32__) || defined(__NT__)
			fixenviron(cmd, sizeof(cmd));
#endif
			if(system(cmd) != 0)
				error_exit("Filter %s failed.\n", cmd);
		}
		host = tfile;
	}

	fip = calloc(1, sizeof(*fip));
	if(fip == NULL) {
		error_exit("No memory for file list.\n");
	}
	if(target == NULL) {
		target = orig_host;
		if(!S_ISFIFO(attrp->mode)) target = basename(target);
	}
	/* Populate with mode, permissions etc.  Make these
	   user overridable by the attribute structure */
	fip->targpath = strdup(target);
	fip->hostpath = strdup(host);

	fip->attr = attrp;
	fip->host_perms = sbuf->st_mode & ~S_IFMT;
	fip->host_uid = sbuf->st_uid;
	fip->host_gid = sbuf->st_gid;
	fip->host_mtime = no_time > 1 ? 0 : sbuf->st_mtime;

	if(S_ISREG(attrp->mode) && !(attrp->scriptfile)) {
		if (crc32_fn(fip->hostpath, &fip->host_file_crc) != -1) {
			fip->flags |= FILE_FLAGS_CRC_VALID;
		}
	}

	if(no_time){ /* strip timestamps from tmpfiles */
		struct tmpfile_entry *t = tmpfile_list;
		while(t){
			if(strcmp(t->name, host) == 0){
				fip->host_mtime = 0;
				break;
			}
		t = t->next;
		}
	}

	if(end)
		end->next = fip;
	else
		*list = fip;

	end = fip;

	return(fip);
}


void
parse_boot_cmd(int tokenc, char *tokenv[], struct attr_file_entry *attrp) {
	int						i, argc, envc, n;
	struct file_entry		*fip;
	struct bootargs_entry	*bap;
	char					buf[PATH_MAX+1], hbuf[PATH_MAX+1], *host;
	static int				have_startup;
	struct stat				sbuf;


	// If no file is specified we set the global attributes.
	if(tokenc == 0) {
		file_attr = *attrp;
		return;
	}
	// Look for leading env variables and calculate argc and envc
	for(envc = 0 ; envc < tokenc ; ++envc) {
		if(strchr(tokenv[envc], '=') == NULL)
			break;
	}
	argc = tokenc - envc;

	// If no command then we are setting global env variables
	if(argc == 0) {
		for(i = 0 ; i < envc ; ++i)
			env_add(tokenv[i]);
		return;
	}

	// Append global env vars to end of tokenv (do not duplicate any on cmd)
	for(i = 0 ; i < globenvc ; ++i)
		if(env_lookup(globenvv[i], envc, tokenv) == NULL)
			tokenv[tokenc++] = globenvv[i];

	// Guard against buffer overrun
	if (strlen(tokenv[envc]) > PATH_MAX) {
		error_exit("File name too long.\n");
	}

	// Search for host file.
	host = find_file(attrp->search_path, hbuf, &sbuf, tokenv[envc], 0);

	// Add a file entry.
	tokenv[envc] = basename(tokenv[envc]);
	fip = add_file(&file_list, host, NULL, attrp, &sbuf);

	n = 0;
	for(i = envc ; i < tokenc ; ++i) {
		n += snprintf(buf + n, PATH_MAX - n, "%s", tokenv[i]) + 1;
	}
	for(i = 0 ; i < envc ; ++i) {
		n += snprintf(buf + n, PATH_MAX - n, "%s", tokenv[i]) + 1;
	}

	// Guard against buffer overrun
	if (n >= PATH_MAX) {
		error_exit("Argument list too long.\n");
	}

	fip->flags |= FILE_FLAGS_BOOT;
	if(!have_startup) {
		fip->flags |= FILE_FLAGS_STARTUP;
		have_startup = 1;
	}
	fip->bootargs = bap = calloc(1, sizeof(struct bootargs_entry) + n);
	if (bap == NULL) {
		error_exit("No memory for boot arguments.\n");
	}
	memcpy(bap->args, buf, n);
	n += offsetof(struct bootargs_entry, args) + 1;
	bap->size_lo = n;
	bap->size_hi = n >> 8;
	bap->argc = argc;
	bap->envc = envc;
}


void
parse_boot(FILE *src_fp) {
	char					*s;
	struct attr_file_entry	attr;
	struct attr_file_list	*list;

	collect_file(&input_boot, INPUT_BOOT_LEN, "boot", src_fp);

	//
	// Parse the file.
	//
	for(s = input_boot.data; *s ; ++s) {
		// Skip white space
		if(isspace(*s))
			continue;

		// Look for attr's
		attr = file_attr;
		if(*s == '[') {
			++s;
			s = tokenize(s, " \t\n\r", ']');
			if(*s++ != ']')
				error_exit("Missing ].\n");

			// Assign global attr's which the routine may modify.
			parse_file_attr(token->c, token->v, &attr);
		}

		// Skip white space
		while(*s == ' '  ||  *s == '\t')
			++s;

		// Look for command
		if(*s == '[')
			error_exit("Missing command.\n");
		s = tokenize(s, " \t\r", '\n');

		/* 
		// Try and match an existing attribute 
		for(list = attr_file_list; list; list = list->next)
			if(memcmp(&attr, &list->attr, sizeof(attr)) == 0)
				break;

		// If no match then we create a new one.
		if(list == NULL) {
			list = calloc(1, sizeof(*list));
			list->attr = attr;
			list->next = attr_file_list;
			attr_file_list = list;
		}
		*/
		// Try and match an existing attribute & create if no match
		list = add_attr(&attr);

		parse_boot_cmd(token->c, token->v, &list->attr);
	}
}


static unsigned
parse_check(char *string) {
	unsigned check;

	check = strtoul(string, &string, 0) * 10;
	if(string[0] == '.' && isdigit(string[1])) {
		check += string[1] - '0';
	}
	if( check >= 0x10000) check = 0xffff;
	return(check);
}

#define DEFAULT_CHECKS	50

void
parse_script_cmd(FILE *dst_fp, int tokenc, char *tokenv[], struct attr_script_entry *attrp) {
	int						i, argc, envc, n;
	union script_cmd		cmd;
	unsigned				size;
	unsigned				flags;

	// If no command is specified we set the global attributes.
	if(tokenc == 0) {
		script_attr = *attrp;
		return;
	}
	size = 0;
	memset(&cmd, 0, sizeof(cmd));

	flags = attrp->flags;

	// Look for background &. We insist it be the last arg.
	if(tokenc && tokenv[tokenc-1][0] == '&' && tokenv[tokenc-1][1] == '\0') {
		--tokenc;
		flags |= SCRIPT_FLAGS_BACKGROUND;
	}

	for(envc = 0 ; envc < tokenc ; ++envc) {
		if(strchr(tokenv[envc], '=') == NULL)
			break;
	}
	argc = tokenc - envc;

	// If no command then we are setting global env variables
	// If no command or env vars we are setting global attributes
	if(argc == 0) {
		if(envc) {
			for(i = 0 ; i < envc ; ++i)
				env_add(tokenv[i]);
		} else {
			script_attr = *attrp;
		}
		return;
	}
	n = 0;
	if(!attrp->external) {
		char		*fname;
		unsigned	checks;

		if(strcmp(tokenv[envc], "waitfor") == 0) {
			//
			// Handle the "waitfor" command
			//
			switch(argc) {
			case 1:
				error_exit("Missing filename for waitfor command.\n");
				checks = 0;
				break;
			case 2:
				checks = DEFAULT_CHECKS;
				break;
			default:
				checks = parse_check(tokenv[envc+2]);
				break;
			}
			fname = tokenv[envc+1];
			cmd.hdr.type = SCRIPT_TYPE_WAITFOR;
			goto waitfor_reopen; //Ugh.
		} else if(strcmp(tokenv[envc], "reopen") == 0) {
			//
			// Handle the "reopen" command
			//
			checks = DEFAULT_CHECKS;
			fname = "/dev/console";
			switch(argc) {
			default:
				checks = parse_check(tokenv[envc+2]);
				/* fall through */
			case 2:
				fname = tokenv[envc+1];
				break;
			case 1:
				break;
			}
			cmd.hdr.type = SCRIPT_TYPE_REOPEN;
waitfor_reopen:
			size = n = offsetof(union script_cmd, waitfor_reopen.fname);
			size += strlen(fname) + 1;
			size = RUP(size, 4);			// 32bit align
			cmd.hdr.size_lo = size & 0xff;
			cmd.hdr.size_hi = size >> 8;
			cmd.waitfor_reopen.checks_lo = checks & 0xff;
			cmd.waitfor_reopen.checks_hi = checks >> 8;
			fwrite(&cmd, 1, n, dst_fp);
			n += fwrite(fname, 1, strlen(fname) + 1, dst_fp);
		} else if(strcmp(tokenv[envc], "display_msg") == 0) {
			cmd.hdr.type = SCRIPT_TYPE_DISPLAY_MSG;
			size = n = offsetof(union script_cmd, display_msg.msg);
			for(i = envc+1; i < tokenc; ++i) {
				size += strlen(tokenv[i]) + 1;
			}
			++size;
			size = RUP(size, 4);			// 32bit align
			cmd.hdr.size_lo = size & 0xff;
			cmd.hdr.size_hi = size >> 8;
			fwrite(&cmd, 1, n, dst_fp);
			for(i = envc+1; i < tokenc-1; ++i) {
				n += fwrite(tokenv[i], 1, strlen(tokenv[i]), dst_fp) + 1;
				putc(' ', dst_fp);
			}
			if ((tokenc > 1) && tokenv[i]) { 
				n += fwrite(tokenv[i], 1, strlen(tokenv[i]), dst_fp) + 1; 
				putc('\n', dst_fp);
			}
			n += 1;
			putc('\0', dst_fp);
		} else if(strcmp(tokenv[envc], "procmgr_symlink") == 0) {
			cmd.hdr.type = SCRIPT_TYPE_PROCMGR_SYMLINK;
			if(argc != 3) {
				error_exit("Missing filenames for procmgr_link command.\n");
			}
				
			size = n = offsetof(union script_cmd, procmgr_symlink.src_dest);
			i = envc+1;
			size += strlen(tokenv[i+0]) + 1;
			size += strlen(tokenv[i+1]) + 1;
			size = RUP(size, 4);			// 32bit align
			cmd.hdr.size_lo = size & 0xff;
			cmd.hdr.size_hi = size >> 8;
			fwrite(&cmd, 1, n, dst_fp);
			n += fwrite(tokenv[i+0], 1, strlen(tokenv[i+0])+1, dst_fp);
			n += fwrite(tokenv[i+1], 1, strlen(tokenv[i+1])+1, dst_fp);
		}
		else if (!strcmp(tokenv[envc], "sched_aps")) {
		char	*err, *pname;
		int		budget, critical;

			if (ext_sched != SCRIPT_SCHED_EXT_NONE && ext_sched != SCRIPT_SCHED_EXT_APS)
				error_exit("Invalid combination of SCHED_EXT features.\n");
			ext_sched = SCRIPT_SCHED_EXT_APS;
			if ((err = aps_parse((argc >= 2) ? tokenv[envc + 1] : SCRIPT_APS_SYSTEM_PARTITION_NAME, &pname, (argc >= 3) ? tokenv[envc + 2] : "0", &budget, (argc >= 4) ? tokenv[envc + 3] : "0", &critical)) != NULL)
				error_exit(err);
			cmd.hdr.type = SCRIPT_TYPE_EXTSCHED_APS;
			cmd.extsched_aps.parent = SCRIPT_APS_SYSTEM_PARTITION_ID;
			cmd.extsched_aps.budget = budget;
			cmd.extsched_aps.critical_lo = critical & 0xFF;
			cmd.extsched_aps.critical_hi = critical >> 8;
			cmd.extsched_aps.id = aps_lookup(pname);
			size = n = offsetof(union script_cmd, extsched_aps.pname);
			size += strlen(pname) + 1;
			size = RUP(size, 4);
			cmd.hdr.size_lo = size & 0xff;
			cmd.hdr.size_hi = size >> 8;
			fwrite(&cmd, 1, n, dst_fp);
			n += fwrite(pname, 1, strlen(pname) + 1, dst_fp);
		}
	}
	if(cmd.hdr.type == SCRIPT_TYPE_EXTERNAL) {
		//
		// Handle an external command
		//
	
		// Insert argv[0] value between file name & remainder of args
		for(i = tokenc; i > envc; --i) tokenv[i] = tokenv[i-1];
		tokenv[envc+1] = (attrp->argv0 != NULL) ? attrp->argv0 : basename(tokenv[envc]);
		++tokenc;
	
		// Append global env vars to end of tokenv (do not duplicate any on cmd)
		for(i = 0 ; i < globenvc ; ++i) {
			if(env_lookup(globenvv[i], envc, tokenv) == NULL)
				tokenv[tokenc++] = globenvv[i];
		}
	
		// Stuff the header and calculate the size of the entry
		cmd.external.flags = flags;
		cmd.external.cpu = attrp->cpu;
		memcpy(&cmd.external.extsched, &attrp->extsched, sizeof(cmd.external.extsched));
		cmd.external.policy = attrp->policy;
		cmd.external.priority = attrp->priority;
		cmd.external.argc = argc;
		cmd.external.envc = tokenc - argc - 1;
		size = n = offsetof(union script_cmd, external.args);
		for(i = 0 ; i < tokenc ; ++i)
			size += strlen(tokenv[i]) + 1;
		size = RUP(size, 4);			// 32bit align
	
		cmd.hdr.size_lo = size & 0xff;
		cmd.hdr.size_hi = size >> 8;
		fwrite(&cmd, 1, n, dst_fp);
	
		// Put out the args then env vars and null pad to 32 bit align
		for(i = envc ; i < tokenc ; ++i) {
			n += fprintf(dst_fp, "%s%c", tokenv[i], '\0');
		}
		for(i = 0 ; i < envc ; ++i) {
			n += fprintf(dst_fp, "%s%c", tokenv[i], '\0');
		}
	}
	while(n < size) {
		putc(0, dst_fp);
		++n;
	}
}



void
parse_script(FILE *src_fp, FILE *dst_fp) {
	char				*s;
	struct attr_script_entry	attr;
	struct attr_script_list		*list;

	collect_file(&input_script, INPUT_SCRIPT_LEN, "script", src_fp);

	//
	// Parse the file.
	//
	push_token_state();
	for(s = input_script.data; *s ; ++s) {
		// Skip white space
		if(isspace(*s))
			continue;

		// Look for attr's
		attr = script_attr;
		if(*s == '[') {
			++s;
			s = tokenize(s, " \t\n\r", ']');
			if(*s++ != ']')
				error_exit("Missing ].\n");

			// Assign global attr's which the routine may modify.
			parse_script_attr(token->c, token->v, &attr);
		}

		// Skip white space
		while(*s == ' '  ||  *s == '\t')
			++s;

		// Look for command
		if(*s == '[')
			error_exit("Missing command.\n");
		s = tokenize(s, " \t\r", '\n');

		// Try and match an existing attribute.
		for(list = attr_script_list ; list ; list = list->next)
			if(memcmp(&attr, &list->attr, sizeof(attr)) == 0)
				break;

		// If no match then we create a new one.
		if(list == NULL) {
			list = calloc(1, sizeof(*list));
			if(list == NULL) {
				error_exit("No memory for attribute list.\n");
			}
			list->attr = attr;
			list->next = attr_script_list;
			attr_script_list = list;
		}
		parse_script_cmd(dst_fp, token->c, token->v, &list->attr);
	}
	pop_token_state();
	free(input_script.data);
	input_script.data = NULL;
	input_script.len = 0;
}





void
collect_dir(char *host, char *target, struct attr_file_entry *attrp, int callindex) {
	DIR				*dp;
	struct dirent	*dirp;
	struct stat		lsbuf, sbuf;
	char			*sh;
	char			*st;
	unsigned		len;

	struct attr_file_entry my_attr;				
	struct attr_file_list *list;
	static struct inode_stack  inode_list;

	/*
	  Create a new attribute structure for us only when we 
	  have a directory or a link entry (since we will need
	  to change the mode attribute from a REG->DIR/LNK).  
	*/
	stat(host, &sbuf);
	lstat(host, &lsbuf);
	memcpy(&my_attr, attrp, sizeof(*attrp));

	if ((callindex != 0) && !attrp->follow_sym_link && S_ISLNK(lsbuf.st_mode)) {
		//We have a link to a dir and want to store links as links,
		//With callindex != 0 we know this is a recursive call
		if (verbose > 5)
			fprintf(debug_fp, "Adding Link to Directory\n\tHOST:%s\n\tTARGET:%s\n", host, target);
		sbuf = lsbuf;
	}
	else {
		//We either have a directory, or a directory link we want to follow
		//If it is a link we should check to see we haven't already recursed
		//through the resolved directory since that would cause a cycle.
		if (dir_in_file_list(host, target) != NULL) {
			if (verbose)
				fprintf(debug_fp, "Ignorning second inclusion of %s\n", host);
			return;
		}

		if (verbose > 5)
			fprintf(debug_fp, "Adding Directory\n\tHOST:%s\n\tTARGET:%s\n", host, target);
	}
	my_attr.mode = sbuf.st_mode & S_IFMT;

	//Add the directory entry to the file entry list
	list = add_attr(&my_attr);
	add_file(&file_list, host, target, &list->attr, &sbuf);

	//We don't recurse down links if we add them
	if (S_ISLNK(my_attr.mode)) 
		return;		

	//Now continue processing the directory contents
	len = strlen(host);
	sh = &host[len];
	if(len > 0 && !IS_DIRSEP(sh[-1])) {
		*sh++ = '/';
		*sh = '\0';
	}
	len = strlen(target);
	st = &target[len];
	if(len > 0 && !IS_DIRSEP(st[-1])) {
		*st++ = '/';
	}

	if((dp = opendir(host)) == NULL) {
		error_exit("Unable to open '%s': %s.\n", host, strerror(errno));
	}


	while( (dirp = readdir(dp)) ) {
		if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;

		strcpy(sh, dirp->d_name);
		strcpy(st, dirp->d_name);

		lstat(host, &lsbuf);
		stat(host, &sbuf);

		if (!attrp->follow_sym_link && S_ISLNK(lsbuf.st_mode) & !S_ISDIR(sbuf.st_mode)) {
			//We don't want to resolve file links ... handle dir's recursively
			if (verbose > 5)
				fprintf(debug_fp, "Adding Link to File\n\tHOST:%s\n\tTARGET:%s\n", host, target);
			my_attr.mode = lsbuf.st_mode & S_IFMT;

			list = add_attr(&my_attr);
			add_file(&file_list, host, target, &list->attr, &sbuf);
		}		
		else if (S_ISDIR(sbuf.st_mode)) {
			if (inode_in_stack(&inode_list, sbuf.st_ino)) {
				if (verbose)
					fprintf(debug_fp, "Warning! Cycle detected in path included by \n\t%s\n",host);
			}
			else {
				push_stack(&inode_list, sbuf.st_ino);
				collect_dir(host, target, attrp, 1);
				pop_stack(&inode_list);
			}
		}
		else if (S_ISFIFO(sbuf.st_mode)) {
			my_attr.mode = sbuf.st_mode & S_IFMT;
			list = add_attr(&my_attr);
			add_file(&file_list, host, target, &list->attr, &sbuf);
		}
		else {
			add_file(&file_list, host, target, attrp, &sbuf);
		}
	}
	*sh = '\0';
	*st = '\0';

	closedir(dp);
	if (callindex == 0)
		destroy_stack(&inode_list);
	return;
}


/*
  A filename specification must conform to one of the following forms:
  (The leading "/" in the destination actually refers to the mount point
  of the image file system).

  hostfile
    file                  - src in MKIFS_PATH/file , dst in /prefix/basename(file)
    /file                 - src in /file           , dst in /prefix/basename(file)
  
  hostdir
    dir                   - src in MKIFS_PATH/dir  , dst in /prefix/path...
    /dir                  - src in /dir            , dst in /path...
  
  targetfile = hostfile
    file1      file2      - src in MKIFS_PATH/file2, dst in /prefix/file1
    file1      /file2     - src in MKIFS_PATH/file2, dst in /file1
    /file1     file2      - src in /file2          , dst in /prefix/file1
    /file1     /file2     - src in /file2          , dst in /file1
  
  targetdir = hostdir
    dir1       dir2       - src in MKIFS_PATH/dir2, dst in /prefix/dir1/path...
    dir1       /dir2      - src in MKIFS_PATH/dir2, dst in /dir1/path...
    /dir1      dir2       - src in /dir2          , dst in /prefix/dir1/path...
    /dir1      /dir2      - src in /dir2          , dst in /dir1/path...
  
  targetfile = {....}
    file                  - src inline             , dst in prefix/file
    /file                 - src inline             , dst in /file
*/
void
parse_file_name(int tokenc, char *tokenv[], struct attr_file_entry *attrp) {
	struct file_entry	*fip;
	struct stat			sbuf;
	char				hbuf[PATH_MAX + 1];
	char				tbuf[PATH_MAX + 1];
	char				linkbuf[PATH_MAX + 1];
	char				*host;
	char				*target;
	FILE				*src_fp;
	int					flags = 0;

	// If no filename is specified we set the global attributes.
	if(tokenc == 0) {
		attrp->bootfile = 0;
		file_attr = *attrp;
		return;
	}

	// Find where it is on the host
	if(tokenc == 1) {
		/* this allows "[type=dir] /some_dir" to work as expected */
		if(attrp->newdir){
			host = tokenv[0];
			target = tokenv[0];
		}
		else{
			host = tokenv[0];
			target = NULL;
		}
	} else {
		host = tokenv[1];
		target = tokenv[0];
	}

	// Guard against buffer overrun
	if ((host) && (strlen(host) > PATH_MAX)) {
		error_exit("File name too long.\n");
	}
	if ((target) && (strlen(target) > PATH_MAX)) {
		error_exit("File name too long.\n");
	}

	switch(attrp->mode) {
		char	*full;

	case S_IFREG:
	case S_IFDIR:
		if(attrp->newdir){ /* don't bother searching if we're forcing directory creation */
			sbuf.st_mode = S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
			sbuf.st_uid = 0;
			sbuf.st_gid = 0;
			sbuf.st_mtime = 0;
			fip = add_file(&file_list, host, target, attrp, &sbuf);
			return;
		}
		full = find_file(attrp->search_path, hbuf, &sbuf, host, attrp->optional);
		if(full == NULL) {
			fprintf(stderr, "Warning: Host file '%s' missing.\n", host);
			return;
		}
		if(S_ISDIR(sbuf.st_mode)) {
			strcpy(hbuf, full);
			if(target != NULL) {
				strcpy(tbuf, target);
			} else {
				tbuf[0] = '\0';
			}
			collect_dir(hbuf, tbuf, attrp, 0);
			return;
		}
		if(!attrp->follow_sym_link){
			struct attr_file_entry tmp = *attrp;
			struct attr_file_list *lst;
			int len;

			if( (len = readlink(full, linkbuf, sizeof(linkbuf))) != -1 ){
				linkbuf[len] = '\0';
				tmp.mode = S_IFLNK;
				lst = add_attr(&tmp);
				attrp = &lst->attr;
				sbuf.st_mode = S_IRWXU | S_IRWXG | S_IRWXO;
				sbuf.st_uid = 0;
				sbuf.st_gid = 0;
				sbuf.st_mtime = no_time ? 0 : time(NULL);
				host = linkbuf;
			}
		} else {
			host = full;
		}
		break;
	case S_IFIFO:
		sbuf.st_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		sbuf.st_uid = 0;
		sbuf.st_gid = 0;
		sbuf.st_mtime = no_time ? 0 : time(NULL);
		break;
	case S_IFLNK:
		sbuf.st_mode = S_IRWXU | S_IRWXG | S_IRWXO;
		sbuf.st_uid = 0;
		sbuf.st_gid = 0;
		sbuf.st_mtime = no_time ? 0 : time(NULL);
		break;
	}

	// Check for the special bootstrap file
	if(attrp->bootfile) {
		if(booter.processed) {
			error_exit("%s: Only one bootstrap file allowed in an image.\n", host);
		}
		booter.processed = 1;
		src_fp = fopen(host, "rb");
		if(src_fp == NULL)
			error_exit("Unable to open '%s': %s.\n", host, strerror(errno));

		parse_boot(src_fp);

		fclose(src_fp);
		return;
	}

	// Check for the special script file
	if(attrp->scriptfile) {
		char	*src_file = host;

		if(script_fp == NULL) {
			flags |= FILE_FLAGS_SCRIPT;
	
			host = mk_tmpfile();
			script_fp = fopen(host, "wb");
			if(script_fp == NULL) {
				error_exit("Unable to open '%s': %s.\n", host, strerror(errno));
			}
		} else {
			host = NULL;
		}
	
		src_fp = fopen(src_file, "r");
		if(src_fp == NULL)
			error_exit("Unable to open '%s': %s.\n", host, strerror(errno));

		parse_script(src_fp, script_fp);

		fclose(src_fp);
	}

	if(host != NULL) {
		fip = add_file(&file_list, host, target, attrp, &sbuf);
		fip->flags |= flags;
	}

}

char *
collect_inline_file(char *input, char *tmpname) {
	char	*s;
	FILE	*fp;

	fp = fopen(tmpname, "wb");
	if(fp == NULL) {
		error_exit("Unable to open '%s': %s.\n", tmpname, strerror(errno));
	}

	for(s = input ; *s ; ++s) {
		if(*s == '}') {
			++s;
			break;
		}
		if(*s == '\n' && line_num != 0) ++line_num;

		if(*s == '\\' && *(s + 1))
			++s;

		putc(*s, fp);
	}

	fclose(fp);

	return(s);
}


static void
parse_one_file(char *s) {
	struct attr_file_entry	attr;
	struct attr_file_list	*list;

	for( ; *s ; ++s) {
		if(*s == '\n' && line_num != 0) ++line_num;

		// Skip white space
		if(isspace(*s))
			continue;

		// Look for attr's
		attr = file_attr;
		if(*s == '[') {
			++s;
			s = tokenize(s, " \t\n\r", ']');
			if(*s++ != ']')
				error_exit("Missing ].\n");

			// Assign global attr's which the routine may modify.
			parse_file_attr(token->c, token->v, &attr);
		}

		// Skip white space
		while(*s == ' '  ||  *s == '\t')
			++s;

		// Look for filename
		if(*s == '[')
			error_exit("Missing filename.\n");
		s = tokenize(s, " \t=\r", '\n');
		if(token->c > 2)
			error_exit("Improper filename specification.\n");

		if(token->c == 2  &&  strcmp(token->v[1], "{") == 0) {
			if(*s++ != '\n')
				error_exit("Missing inline file data.\n");

			if (line_num != 0)
				++line_num;

			token->v[1] = mk_tmpfile();
			s = collect_inline_file(s, token->v[1]);
		}

		list = add_attr(&attr);
		
		parse_file_name(token->c, token->v, &list->attr);

		if(strcmp(attr.cd, file_attr.cd) != 0)
			chdir(file_attr.cd);

		if(line_num != 0 && *s == '\n') ++line_num;
	}
}

void
parse_file(FILE *src_fp) {

	collect_file(&input_file, INPUT_FILE_LEN, "control", src_fp);
	//
	// Parse the input buffer(s).
	//
	if(option_lines != NULL) parse_one_file(option_lines);
	line_num = 1;
	parse_one_file(input_file.data);
	line_num = 0;
	if(boot_attr_buf != NULL) parse_one_file(boot_attr_buf);
}


void
add_tree(struct tree_entry *parent, struct file_entry *fip) {
	struct tree_entry	*prev, *trp;
	char				*pp, *np;
	char				 name[PATH_MAX + 1];

	/* 
	   The root of the tree will have a targetpath of ""
	   This is implied so don't bother with it.
	*/
	if (strlen(fip->targpath) == 0) {
		return;
	}

	trp = NULL;
	for(pp = fip->targpath; *pp ; ) {
		// Collect a single path component.
		for(np = name ; *pp  &&  *pp != '/' ; ++pp, ++np) {
			*np = *pp;
			if (np == &name[PATH_MAX]) {
				error_exit("File name too long.\n");
			}
		}
		*np = '\0';

		// Search the tree at the current level for a match
		for(prev = (void *)&parent->child ; (trp = prev->sibling) ; prev = trp)
			if(strcmp(trp->name, name) == 0) break;

		//If no match we build the tree piecewise
		if(trp == NULL) {
			static struct file_entry		file;
			static struct attr_file_entry	attr;

			trp = malloc(sizeof(*trp) + strlen(name));
			if (trp == NULL) {
				error_exit("No memory for tree entry.\n");
			}
			trp->parent = parent;
			trp->child = NULL;
			trp->sibling = NULL;
			trp->flags = 0;
			strcpy(trp->name, name);
			prev->sibling = trp;
        	
			/* Create dummy directory entry which will get
			   filled in later.   This is required because 
			   files may come before their directory entries 
			   if the list gets shuffled */
			if(file.attr == NULL) {
				file.attr = &attr;
				file.attr->mode = S_IFDIR;

	#define PERM_READ		(S_IRUSR|S_IRGRP|S_IROTH)
	#define PERM_WRITE		(S_IWUSR|S_IWGRP|S_IWOTH)
	#define PERM_EXECUTE	(S_IXUSR|S_IXGRP|S_IXOTH)
	#define PERM_BITS		(S_ISUID|S_ISGID|S_ISVTX)
				file.attr->dperms_mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
				file.host_perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
				file.host_uid = 0;
				file.host_gid = 0;
				file.host_mtime = no_time ? 0 : time(NULL);
			}
			trp->fip = &file;
		}

		parent = trp;
		if(*pp) ++pp;
	}
	/*After going through and making the path, set the last 
	  tree pointer to our file pointer.  This will guarantee 
	  that we get the proper attributes. Root is an exception */
	trp->fip = fip;
}


//
// A debugging routine. It will not normally be called.
//
void
print_tree(struct tree_entry *trp, int level) {
	int i;

	while(trp) {
		for(i = 0 ; i < level ; ++i) printf("  ");
		if(trp->child) {
			printf("%s\n", trp->name);
			print_tree(trp->child, level + 1);
		} else {
			printf("%s\n", trp->name);
		}

		trp = trp->sibling;
	}
}


int
main(int argc, char *argv[]) {
	int					n;
	FILE				*src_fp;
	FILE				*dst_fp;
	char				*specified_dest;
	char				*intermediate_dest;
	char 				*output_dest;
	unsigned			startup_offset;
	struct file_entry	*fip;
	char				*cmd;
	char				*type = NULL;
	mode_t				old_mask;
	char				*rootdir = NULL;


	cmd = basename(argv[0]);
	if(IS_EXE_NAME(cmd, "mkifs")) {
		type = "ifs";
	} else if(IS_EXE_NAME(cmd, "mkefs")) {
		type = "ffs3";
	} else if(IS_EXE_NAME(cmd, "mketfs")) {
		type = "etfs";
	}

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

	// Get the right permissions on temp files
	old_mask = umask(0);

	while((n = getopt(argc, argv, "a:c:r:l:nNps:t:v")) != -1) {
		switch(n) {
		case 'a':
			symfile_suffix = strdup( optarg );
			break;
		case 'c':
			cache_dir = optarg;
			break;
		case 'l':
			add_data(optarg);
			add_data("\n");
			break;
		case 'n':
			no_time++;
			break;
		case 'N':
			new_style_bootstrap++;
			break;
		case 'p':
			// Place holder for old, experimental switch
			break;
		case 's':
			ifs_section(NULL, optarg);
			break;
		case 't':
			type = optarg;
			break;
		case 'r':
			if(rootdir) {
				error_exit("Only one rootdir can be passed\n");
			}
			rootdir = optarg;
			break;
		case 'v':
			++verbose;
			break;
		default:
			exit(1);
		}
	}

	if(type == NULL) {
		error_exit("Output file system type not specified\n");
	} else if(strcmp(type, "ifs") == 0) {
		parse_file_init = ifs_parse_init;
		parse_file_attr = ifs_parse_attr;
		need_seekable   = ifs_need_seekable;
		make_fsys       = ifs_make_fsys;
	} else if(strcmp(type, "ffs3") == 0) {
		parse_file_init = ffs_parse_init;
		parse_file_attr = ffs_parse_attr;
		need_seekable   = ffs_need_seekable;
		make_fsys       = ffs_make_fsys_3;
	} else if(strcmp(type, "ffs2") == 0) {
		parse_file_init = ffs_parse_init;
		parse_file_attr = ffs_parse_attr;
		need_seekable   = ffs_need_seekable;
		make_fsys       = ffs_make_fsys_2;
		fprintf(stderr, "Warning: FFS2 no longer supported. ");
		fprintf(stderr, "Use for backwards compatability only.\n");
	} else if(strcmp(type, "ffs2-quiet") == 0) {
		// Same as above, but no warning message.
		parse_file_init = ffs_parse_init;
		parse_file_attr = ffs_parse_attr;
		need_seekable   = ffs_need_seekable;
		make_fsys       = ffs_make_fsys_2;
	} else if(strcmp(type, "etfs") == 0) {
		parse_file_init = etfs_parse_init;
		parse_file_attr = etfs_parse_attr;
		need_seekable   = etfs_need_seekable;
		make_fsys       = etfs_make_fsys;
	} else {
		error_exit("Unknown file system '%s'\n", type);
	}

	set_cpu(DEFAULT_CPU, 0);
#if defined(__WIN32__) || defined(__NT__)
	{
		char *p, *qnx_target = getenv("QNX_TARGET");
		if(!qnx_target) {
			qnx_target = getenv("QSSL_TARGET");
		}
		if ( qnx_target ) {
			if ( qnx_target[0] == '\"' ) { 
				qnx_target++;
				if ( (p = strrchr( qnx_target, '\"')) ) {
					*p = '\0';
				}
			}
		} else {
			error_exit("QNX_TARGET environment variable must be set\n");
		}
		setenv( "QNX_TARGET", qnx_target, 0 );
	}
#else
	{
		char *qnx_target = getenv("QNX_TARGET");
		if(!qnx_target) {
			qnx_target = getenv("QSSL_TARGET");
		}
		if(qnx_target) {
			setenv("QNX_TARGET",qnx_target, 0);
		} else {
			error_exit("QNX_TARGET environment variable must be set\n");
		}
    }
#endif
    if(rootdir) {
		setenv("_ROOTDIR_", rootdir, 1);

		/* @@@ This should not duplicate the paths, but for now it is the least risky */
		setenv("MKIFS_PATH",
	       // If there is something in sbin, and also somewhere else
	       // we probably want the sbin version so look there first
	       "${_ROOTDIR_}/${PROCESSOR}/sbin"   
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/usr/sbin"

	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/boot/sys"

	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/bin"
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/usr/bin"
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/lib"
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/lib/dll"
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/usr/lib"
	       
	       // Feel free to get rid of this if /usr/photon/bin goes away
	       PATHSEP_STR "${_ROOTDIR_}/${PROCESSOR}/usr/photon/bin"


	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/sbin"   
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/sbin"

	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/boot/sys"

	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/bin"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/bin"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/lib"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/lib/dll"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/lib"
	       
	       // Feel free to get rid of this if /usr/photon/bin goes away
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/photon/bin"
	       , 0 );
	} else {
		setenv("MKIFS_PATH",
	       // If there is something in sbin, and also somewhere else
	       // we probably want the sbin version so look there first
	       "${QNX_TARGET}/${PROCESSOR}/sbin"   
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/sbin"

	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/boot/sys"

	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/bin"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/bin"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/lib"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/lib/dll"
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/lib"
	       
	       // Feel free to get rid of this if /usr/photon/bin goes away
	       PATHSEP_STR "${QNX_TARGET}/${PROCESSOR}/usr/photon/bin"
	       , 0 );
	}

	src_fp = stdin;
	dst_fp = stdout;
	debug_fp = stderr;
	specified_dest = NULL;

	atexit(rm_tmpfiles);

	signal(SIGHUP, die);
#if !defined(__WIN32__) && !defined(__NT__)
	signal(SIGPIPE, die);
#endif
	signal(SIGINT, die);
	signal(SIGQUIT, die);
	signal(SIGTERM, die);

	n = argc - optind;
	if(n) {
		if(strcmp(argv[optind], "-") != 0) {
			src_fp = fopen(argv[optind], "r");
			if(src_fp == NULL)
				error_exit("Unable to open '%s': %s.\n", argv[optind], strerror(errno));
		}

		if(n > 1) {
			if(strcmp(argv[optind + 1], "-") != 0) {
				specified_dest = argv[optind + 1];
			}
		}
	}

	push_token_state();

	parse_file_init(&file_attr);
	parse_script_init(&script_attr);

	parse_file(src_fp);

	if(script_fp != NULL) {
		int		n;

		// Terminate list with a size of 0
		n = 0;
		fwrite(&n, sizeof(n), 1, script_fp);
		fclose(script_fp);
	}

	for(fip = file_list; fip != NULL; fip = fip->next) {
		//NYI: should run the list & check for duplicate target names
		if(fip->attr->prefix == NULL) {
			fip->attr->prefix = booter.name ? "proc/boot" : "";
		}
		set_target_name(fip);
	}

	if(booter.copy_filter) {
		intermediate_dest = mk_tmpfile();
	} else if((specified_dest == NULL)
	    && ((booter.filter_spec != NULL) || need_seekable(file_list))) {
		intermediate_dest = mk_tmpfile();
	} else {
		intermediate_dest = specified_dest;
	}

	// Final output file should be created with original umask()
	umask(old_mask);

	if(intermediate_dest != NULL) {
		dst_fp = fopen(intermediate_dest, "w+b");
		if(dst_fp == NULL) {
			error_exit("Unable to open '%s': %s.\n", intermediate_dest, strerror(errno));
		}
	}

	// make_fsys() might create more temp files
	umask(0);

	if(specified_dest == NULL) MAKE_BINARY_FP(stdout);

	startup_offset = make_fsys(dst_fp, file_list, mountpoint, intermediate_dest);

	if(dst_fp != stdout) fclose(dst_fp);

	output_dest = intermediate_dest;
	if(booter.filter_spec != NULL) {
		if(specified_dest != NULL) {
			output_dest = specified_dest;
		} else if(booter.copy_filter) {
			output_dest = mk_tmpfile();
		}
		proc_booter_filter(startup_offset, intermediate_dest, output_dest);
	}
	if(specified_dest == NULL && (dst_fp != stdout)) {
		dst_fp = fopen(output_dest, "rb");
		if(dst_fp == NULL) {
			error_exit("Can not open '%s' for input.\n", intermediate_dest);
		}
		while((n = fread(token->buf, 1, sizeof(token->buf), dst_fp)) > 0) {
			fwrite(token->buf, 1, n, stdout);
		}
	}
	return(0);
}

struct tree_entry *
make_tree(struct file_entry *list) {
	struct file_entry	*fip;
	struct tree_entry	root;

	// Build a tree from the file list.
	root.child = NULL;
	for(fip = list ; fip ; fip = fip->next)
		add_tree(&root, fip);

	return(root.child);
}

__SRCVERSION("mkxfs.c $Rev: 200993 $");
