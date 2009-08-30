/* This file contains stubs of all (I think) ANSI C library functions.
 * This is intended to serve as a reference; there's no point in compiling it.
 * The idea is that you put this file in an accessible but out-of-the-way
 * location, and run ctags on it.  Then you set the TAGPATH environment
 * variable to include this file's directory.  From that point on, you can
 * use the normal tag searching functions to see the declaration for any
 * of these functions.  In particular, the "ref" program distributed with
 * elvis is handy for this.
 */

/* <stdlib.h> */
#define NULL 0

/* <stdio.h> */
typedef struct _filestruct *FILE;
FILE *stdin, *stdout, *stderr;

/* <time.h> time/date breakdown */
struct tm
{
	int	tm_sec;		/* second, 0-59 */
	int	tm_min;		/* minute, 0-59 */
	int	tm_hour;	/* hour, 0-23 */
	int	tm_mday;	/* day of month, 1-31 */
	int	tm_mon;		/* month, 0-11 */
	int	tm_year;	/* year, 1970-2038 */
	int	tm_wday;	/* day of week, 0=sunday - 6=saturday */
	int	tm_yday;	/* day of year, 0-364 (365 on leap years) */
	int	tm_isdst;	/* daylight savings time flag */
};

/* <locale.h> formats of numbers and money */
struct lconv
{
	/* NUMERIC */
	char *decimal_point;	/* Decimal point character  */
	char *thousands_sep;	/* Thousands separator  */
	char *grouping;		/* one-byte ints giving groups, end with 0 to
				   repeat or CHAR_MAX to stop grouping */

	/* MONETARY */
	char *int_curr_symbol;	/* 3-byte ISO-4217 synbol, 1-byte separator */
	char *currency_symbol;	/* Local currency symbol  */
	char *mon_decimal_point;/* Decimal point character  */
	char *mon_thousands_sep;/* Thousands separator.  */
	char *mon_grouping;	/* Like `grouping' element (above).  */
	char *positive_sign;	/* Sign for positive values.  */
	char *negative_sign;	/* Sign for negative values.  */
	char int_frac_digits;	/* Int'l fractional digits.  */
	char frac_digits;	/* Local fractional digits.  */
	char p_cs_precedes;	/* currency_symbol preceeds positive amount? */
	char p_sep_by_space;	/* space between currency_symbol & positive? */
	char n_cs_precedes;	/* currency_symbol preceeds negative amount? */
	char n_sep_by_space;	/* space between currency_symbol & negative? */
	char p_sign_posn;	/* 0=parentheses, 1=before both, 2=after both,*/
	char n_sign_posn;	/*   3=before cuurency_symbol, 4=after it */
};

/* <stdlib.h> Returned by div() */
typedef struct
{
	int quot;		/* Quotient */
	int rem;		/* Remainder */
} div_t;

/* <stdlib.h> Returned by ldiv() */
typedef struct
{
	long quot;		/* Quotient */
	long rem;		/* Remainder */
} ldiv_t;

/* <stdlib.h> wide characters, typically `unsigned short' */
typedef OPAQUE	wchar_t;

/* <stdlib.h> type returned by sizeof operator, traditionally `int' */
typedef OPAQUE	size_t;

/* <time.h> type returned by clock(), typically `long int' */
typedef OPAQUE	clock_t;

/* <time.h> type returned by time(), traditionally `long int' */
typedef OPAQUE	time_t;

/* <stdio.h> type returned by fgetpos(), typically `long int' */
typedef OPAQUE	fpos_t;

/* <stdio.h> type returned by ftell(), traditionally `long int'.  Apparently
 * ANSI continues to use `long int' instead of `off_t', but `off_t' is still
 * preferred because 32-bit offsets aren't big enough anymore.  `off_t' may be
 * a larger data type than `long int'.  This doesn't necessarily mean that
 * your libraries and OS support the larger offsets, of course.
 */
typedef OPAQUE	off_t;

/* <stdlib.h> Cause a core dump */
void abort(void)
{
}

/* <stdlib.h> Return the absolute value of an int */
int abs(int num)
{
}

/* <math.h> Return the angle whose cosine is cosvalue */
double acos(double cosvalue)
{
}

/* <time.h> Return a date/time string, e.g., "Wed Jun 30 21:49:08 1993\n" */
char *asctime(const struct tm *currtime)
{
}

/* <math.h> Return the angle whose sine is sinvalue */
double asin(double sinvalue)
{
}

/* <assert.h> If expression is false, abort */
void assert(int expression)
{
}

/* <math.h> Return the angle whose tangent is tanvalue */
double atan(double tanvalue)
{
}

/* <math.h> Return the angle from (0,0) to (x,y) */
double atan2(double y, double x)
{
}

/* <stdlib.h> Arrange for func() to be called when the program exits */
int atexit(void(*func)(void))
{
}

/* <stdlib.h> Convert a string to a double */
double atof(const char *string)
{
}

/* <stdlib.h> Convert a string to an int */
int atoi(const char *string)
{
}

/* <stdlib.h> Convert a string to a long */
long int atol(const char *string)
{
}

/* <stdlib.h> Seach through an array for a given item.  There are five
 * arguments: the item to find, the array to find it in, the number of
 * items in the array, the size of each item, and a pointer to a comparison
 * function.  It returns a pointer to the found item, or NULL if it isn't
 * found.
 */
void *bsearch(const void *itemx, const void *array, size_t nitems, size_t itemsize, int (*compare) (const void *itemi, const void *itemj))
{
}

/* <stdlib.h> Allocate an array, and initialize it to 0 */
void *calloc(size_t num, size_t length)
{
}

/* <math.h> Round upwards to the next integer value (might not fit in "int") */
double ceil(double upwards)
{
}

/* <stdio.h> Clear the error flag on a FILE */
void clearerr(FILE *fp)
{
}

/* <time.h> Return the approximate CPU time used by this process, in time
 * slices.  To get seconds, divide by CLOCKS_PER_SEC.
 */
clock_t clock(void)
{
}

/* <math.h> Return the cosine for a given angle */
double cos(double angle)
{
}

/* <math.h> Return the hyperbolic cosine of an angle */
double cosh(double angle)
{
}

/* <time.h> Return a date/time string, e.g., "Wed Jun 30 21:49:08 1993\n" */
char *ctime(const time_t *timeval)
{
}

/* <time.h> Compute the time difference, in seconds, between two times */
double difftime(time_t end, time_t begin)
{
}

/* <stdlib.h> Compute the quotient and remainder of integer division */
div_t div(int number, int divider)
{
}

/* <stdlib.h> Terminate this process */
void exit(int exit_code)
{
}

/* <math.h> Return e^num */
double exp(double num)
{
}

/* <math.h> Return the absolute value of num */
double fabs(double num)
{
}

/* <stdio.h> Close a FILE */
int fclose(FILE *fp)
{
}

/* <stdio.h> Return a true (non-zero) value if a previous attempt to read
 * from fp resulted in an end-of-file condition; else return false (0).
 * Note that this function doesn't "look ahead" -- you must try to read
 * past the EOF before this function will know you've hit it.
 */
int feof(FILE *fp)
{
}

/* <stdio.h> Return a true (non-zero) value if a previos I/O operation on fp
 * failed for any reason.
 */
int ferror(FILE *fp)
{
}

/* <stdio.h> Flush any data written to fp, by writing it to the operating
 * system.  Note that this does not necessarily guarantee that the data has
 * been written to disk.
 */
int fflush(FILE *fp)
{
}

/* <stdio.h> Read a character or EOF from fp. */
int fgetc(FILE *fp)
{
}

/* <stdio.h> Store fp's current record number into the position argument */
int fgetpos(FILE *fp, fpos_t *position)
{
}

/* <stdio.h> Read a string from fp.  Returns NULL at end of file. */
char *fgets(char *string, int width, FILE *fp)
{
}

/* <math.h> Round a double to the next lower integer (might not fit an "int") */
double floor(double lower)
{
}

/* <math.h> Return the remainder of integer division, performed on doubles */
double fmod(double first, double second)
{
}

/* <stdio.h> Open a file. */
FILE *fopen(const char *file_name, const char *mode)
{
}

/* <stdio.h> Print formatted text out to fp. */
int fprintf(FILE *fp, const char *format, ...)
{
}

/* <stdio.h> Write a single character to fp */
int fputc(int ch, FILE *fp)
{
}

/* <stdio.h> Write a string to fp, without adding a newline. */
int fputs(const char *string, FILE *fp)
{
}

/* <stdio.h> Read an array of items from fp.  "width" is the size of each
 * item, and "count" is the number of items requested.  It returns the
 * number of items actually read.
 */
size_t fread(void *buffer, size_t width, size_t count, FILE *fp)
{
}

/* <stdlib.h> Release memory allocated by malloc(), calloc(), or realloc() */
void free(void *memory_ptr)
{
}

/* <stdio.h> Force an existing fp to use a different file and/or mode */
FILE *freopen(const char *new_file, const char *mode, FILE *fp)
{
}

/* <math.h> Break a double into its exponent and (normalized) mantissa. */
double frexp(double num, int *exp)
{
}

/* <stdio.h> Scan a formatted text string from fp */
int fscanf(FILE *fp, const char *format, ...)
{
}

/* <stdio.h> Change fp's I/O offset */
int fseek(FILE *fp, long int offset, int whence)
{
}

/* <stdio.h> Restore fp's current record number to a position from a previous
 * fgetpos().  Note that the file must remain open between those two calls.
 */
int fsetpos(FILE *fp, const fpos_t *position)
{
}

/* <stdio.h> Write an array of items to fp.  "width" is the size of each
 * item, and "count" is the number of items to be written.  Returns the
 * number of items actually written.
 */
size_t fwrite(const void *buffer, size_t width, size_t count, FILE *fp)
{
}

/* <stdio.h> Get a single character from fp */
int getc(FILE *fp)
{
}

/* <stdio.h> Get a single character from stdin */
int getchar(void)
{
}

/* <stdlib.h> Return the value (as a '\0'-terminated string) of an environment
 * variable.
 */
char *getenv(const char *name)
{
}

/* <stdio.h> Read a line from stdin, and strip off the terminating '\n'.
 * Return NULL if * the end of the file was encountered.  THIS FUNCTION IS A
 * SECURITY HOLE!  Use the fgets() function instead, and strip off the '\n'
 * from that function's returned value if necessary.
 */
char *gets(char *string)
{
}

/* <stdio.h> Return the I/O offset of fp */
long int ftell(FILE *fp)
{
}

/* <stdlib.h> Return the absolute value of a long */
long int labs(long int num)
{
}

/* <stdlib.h> Convert string to a long int.  This is more versatile than
 * atol(), because it works with various bases (2 through 64, instead of
 * just base 10).  Also, it has the side-effect of setting "end" to point
 * to the character after the number's last digit.
 */
long int strtol(const char *start, char **end, int base)
{
}

/* <ctype.h> True for letters and digits */
int isalnum(int ch)
{
}

/* <ctype.h> True for letters */
int isalpha(int ch)
{
}

/* <ctype.h> True for ASCII control characters */
int iscntrl(int ch)
{
}

/* <ctype.h> True for digits */
int isdigit(int ch)
{
}

/* <ctype.h> True for printable characters other than whitespace */
int isgraph(int ch)
{
}

/* <ctype.h> True for lowercase letters */
int islower(int ch)
{
}

/* <ctype.h> True for printable characters including whitespace */
int isprint(int ch)
{
}

/* <ctype.h> True for punctuation characters */
int ispunct(int ch)
{
}

/* <ctype.h> True for any whitespace character */
int isspace(int ch)
{
}

/* <ctype.h> True for uppercase letters */
int isupper(int ch)
{
}

/* <ctype.h> True for hexadecimal digits -- 0-9, a-f, A-F */
int isxdigit(int ch)
{
}

/* <locale.h> Fetch the locale-specific information for numeric conversions */
struct lconv *localeconv(void)
{
}

/* <math.h> Return num * (2 ^ exp) -- i.e., add exp the num's exponent */
double ldexp(double num, int exp)
{
}

/* <stdlib.h> Return the quotient and remainder of two longs */
ldiv_t ldiv(long int num, long int divisor)
{
}

/* <math.h> Return the natural logarithm of num */
double log(double num)
{
}

/* <math.h> Return the base-10 logarithm of num */
double log10(double num)
{
}

/* <stdlib.h> Convert a string to an unsigned long int, using a given base */
unsigned long int strtoul(const char *start, char **end, int base)
{
}

/* <setjmp.h> Jump to a location set by setjmp().  Note that the function
 * which called setjmp() must not have returned yet.
 */
void longjmp(jmp_buf jmpenv, int value)
{
}

/* The entry point of a C program */
int main(void) or (int argc, char *argv[])
{
}

/* <stdlib.h> Allocate memory.  The returned memory is NOT initialized */
void *malloc(size_t amount)
{
}

/* <string.h> Seach for ch in the first count bytes of string.  String is
 * permitted to contain '\0' bytes.
 */
void *memchr(const void *string, int ch, size_t count)
{
}

/* <string.h> Compare two chunks of memory */
int memcmp(const void *area1, const void *area2, size_t width)
{
}

/* <string.h> Copy one chunk of memory into another (non-overlapping) chunk of
 * memory.  If the regions might overlap, you should use memmove() instead.
 */
void *memcpy(void *to, const void *from, size_t width)
{
}

/* <string.h> Copy one chunk of memory into another.  They are permitted to
 * overlap.
 */
void *memmove(void *to, const void *from, size_t width)
{
}

/* <string.h> Set every byte in a chunk of memory to "ch" */
void *memset(void *buffer, int ch, size_t count)
{
}

/* <time.h> Convert "local" into a time_t value */
time_t mktime(struct tm *local)
{
}

/* <math.h> Return the modulo (remainder of integer division)) of two doubles */
double modf(double num, *double whole)
{
}

/* <stdio.h> Print an error message to stderr.  The string should identify
 * what caused the error (e.g., the name of a file that couldn't be opened).
 * The error's description is produced by examining the "errno" global
 * variable.
 */
void perror(const char *string)
{
}

/* <math.h> Return base^exp */
double pow(double base, double exp)
{
}

/* <stdio.h> Print a formatted string to stdout */
int printf(const char *format, ...)
{
}

/* <stdio.h> Write a single character to fp */
int putc(int ch, FILE *fp)
{
}

/* <stdio.h> write a single character to stdout */
int putchar(int ch)
{
}

/* <stdio.h> Write a string to stdout, and then write a '\n' character */
int puts(const char *string)
{
}

/* <stdlib.h> Sort an array.  There are four arguments: the array to sort,
 * the number of items in the array, the size of each item, and a pointer
 * to a comparison function.
 */
void qsort(void *array, size_t nitems, size_t itemsize, int (*compare)(const void *itemi, const void *itemj))
{
}

/* <signal.h> Send a single to yourself, like "kill(getpid(), signo)" */
int raise(int signo)
{
}

/* <stdlib.h> Return a random number between 0 and RAND_MAX */
int rand(void)
{
}

/* <stdlib.h> Change the size of a previously allocated chunk of memory.
 * If possible, leave it in the same location.  If the surrounding memory
 * isn't free, then allocate a totally new chunk of memory, copy the old
 * data into it, and then free the old memory.  Either way, return a
 * pointer to the reallocated chunk.
 */
void *realloc(void *mem, size_t new_size)
{
}

/* <stdio.h> Delete a file */
int remove(const char *filename)
{
}

/* <stdio.h> Rename a file */
int rename(const char *oldname, const char *newname)
{
}

/* <stdio.h> Reset fp to read from the beginning of a file */
void rewind(FILE *fp)
{
}

/* <stdio.h> Scan formatted text from stdin */
int scanf(const char *format , ...)
{
}

/* <stdio.h> Force fp to use a given buffer */
void setbuf(FILE *fp, char buffer[BUFSIZ])
{
}

/* <setjmp.h> Define the destination for a later longjmp() call.  When
 * called initially, setjmp() returns 0.  When longjmp() is called later,
 * setjmp() will appear to return again, this time with the value that was
 * passed into longjmp().
 */
int setjmp(jmp_buf jmpenv)
{
}

/* <locale.h> Set the locale */
char *setlocale(int type, const char *locale)
{
}

/* <stdio.h> "mode" is one _IONBF for unbuffered, _IOLBF for line buffered,
 * or _IOFBF for fully buffered.  This function must be called before the
 * first I/O operation on the FILE.
 */
int setvbuf(FILE *fp, char *buffer, int mode, size_t width)
{
}

/* <signal.h> This one looks trickier than it is.  There are two arguments:
 * a signal number, and a function to call when that signal is received.
 * signal() returns the previous function pointer for that signal.  The
 * catcher functions should accept a signal number argument, and return void.
 */
void (*signal (int signo, void (*fn)(int signo)))(int signo)
{
}

/* <math.h> Return the sine of an angle */
double sin(double angle)
{
}

/* <math.h> Return the hyperbolic sine of an angle */
double sinh(double angle)
{
}

/* <stdio.h> Write formatted text into a buffer. */
int sprintf(char *buffer, const char *format, ...)
{
}

/* <math.h> Return the square root of num */
double sqrt(double num)
{
}

/* <stdlib.h> Set the seed for the random number generator, rand() */
void srand(unsigned int seed)
{
}

/* <stdio.h> Scan formatted text from a buffer */
int sscanf(const char *buffer, const char *format, ...)
{
}

/* <string.h> Append rear onto the end of the front string.  It is assumed
 * that the front string's buffer is large enough to hold the combined string.
 */
char *strcat(char *front, const char *rear)
{
}

/* <string.h> Return a pointer to the first instance of ch in string.  If it
 * doesn't appear there, then return NULL.
 */
char *strchr(const char *string, int ch)
{
}

/* <string.h> Compare two strings, without using the locale */
int strcmp(const char *first, const char *second)
{
}

/* <string.h> Compare two strings using the locale */
int strcoll(const char *first, const char *second)
{
}

/* <string.h> Copy a string */
char *strcpy(char *dest, const char *source)
{
}

/* <string.h> Count the initial chars in `string' which DON'T which appear in
 * the `chars' string.
 */
size_t strcspn(const char *string, const char *chars)
{
}

/* <string.h> Return the string describing an error */
char *strerror(int error_num)
{
}

/* <time.h> Convert a time from struct tm to a string, using a given format */
size_t strftime(char *date_time, size_t maxsize,const char *format, const struct tm *currtime)
{
}

/* <string.h> Return the length of a string */
size_t strlen(const char *string)
{
}

/* <string.h> Append the rear string onto the end of the front string, up
 * to an overall length limit.
 */
char *strncat(char *front, const char *rear, size_t length)
{
}

/* <string.h> Compare first "length" characters of two strings */
int strncmp(const char *first, const char *second, size_t length)
{
}

/* <string.h> Copy the first "length" characters of "from" into "to" */
char *strncpy(char *to, const char *from, size_t length)
{
}

/* <string.h> Locate the first instance of in string, of any character in
 * chars, and return a pointer to it.  If no such character can be found,
 * return NULL.
 */
char *strpbrk(const char *string, const char *chars)
{
}

/* <string.h> Return a pointer to the last instance of ch in string,
 * or NULL if none.
 */
char *strrchr(const char *string, int ch)
{
}

/* <string.h> Count the initial chars in `string' which appear in `chars' */
size_t strspn(const char *string, const char *chars)
{
}

/* Return a pointer to the first instance of `target' in `string', or NULL if
 * `target' doesn't appear anywhere in `string'
 */
char *strstr(const char *string, const char *target)
{
}

/* <stdlib.h> Convert a string to a double.  Also set *end to point to
 * character after the last digit of the number.
 */
double strtod(const char *start, char **end)
{
}

/* <string.h> Return a token from buf.  On the first call, buf should be
 * the start of a buffer; on subsequent calls, it should be NULL.  The
 * delim argument is used as a list of delimiter characters.  Each call
 * will return sequential tokens from buf, until they've all been returned
 * at which point it switches to NULL.  Note that buf is modified as this
 * progresses.
 */
char* strtok(char *buf, const char *delim)
{
}

/* <string.h> Transform a string into a form which can be used for
 * locale-dependent comparisons efficiently.
 */
size_t strxfrm(char *first, const char *second, size_t length)
{
}

/* <stdlib.h> Run a program, and wait for it to complete.  Return the exit
 * status of that program.
 */
int system(const char *command)
{
}

/* <math.h> Return the tangent of an angle */
double tan(double angle)
{
}

/* <math.h> Return the hyperbolic tangent of an angle */
double tanh(double angle)
{
}

/* <time.h> Return the current system time.  If "tptr" is not NULL, then also
 * copy the current system time into the variable that it points to.
 */
time_t time(time_t *tptr)
{
}

/* <time.h> Decompose a time_t value, for the Universal Time zone */
struct tm *gmtime(const time_t *tptr)
{
}

/* <time.h> Decompose a time_t value, for the local time zone */
struct tm *localtime(const time_t *tptr)
{
}

/* <stdio.h> Create a temporary file */
FILE *tmpfile(void)
{
}

/* <stdio.h> Create a name for a temporary file.  If buf is null, then it
 * uses a static variable to store the name.  Note that some other process
 * may create a file with the same name before yours does, so this function
 * is not 100% reliable.
 */
char *tmpnam(char *buf)
{
}

/* <ctype.h> Convert ch to lowercase */
int tolower(int ch)
{
}

/* <ctype.h> Convert ch to uppercase */
int toupper(int ch)
{
}

/* <stdio.h> Stuff ch back into the input stream for fp */
int ungetc(int ch, FILE *fp)
{
}

/* <stdarg.h> Return the next varargs value, for a given type */
type va_arg(va_list arg_ptr, type)
{
}

/* <stdarg.h> End the varargs processing */
void va_end(va_list arg_ptr)
{
}

/* <stdarg.h> Start the varargs processing.  The variable portion of the
 * arguments start immediately after last_fixed_arg.  Note that this is a
 * macro, and its second parameter is simply the identifier of an argument,
 * NOT A REFERENCE TO IT.
 */
void va_start(va_list first_arg, last_fixed_arg)
{
}

/* <stdio.h> and <stdarg.h> Print varargs out to fp */
int vfprintf(FILE *fp, const char *format, va_list arg_ptr)
{
}

/* <stdio.h> and <stdarg.h> Print varargs out to stdout */
int vprintf(const char *format, va_list arg_ptr)
{
}

/* <stdio.h> and <stdarg.h> Print varargs into a buffer */
int vsprintf(char *buffer, const char *format, va_list arg_ptr)
{
}

/* <stdlib.h> Convert a string of multibyte characters into a string of wide
 * characters.  Return the number of wide characters.  If pwcs is NULL, then
 * still returns the count but does not store the converted text.
 */
mbstowcs(wchar_t *pwcs, const char *m_byte, size_t length)
{
}

/* <stdlib.h> Convert a single wide character into a multibyte character
 * string, and return the number of bytes required for the multibyte text
 * (including a terminating NUL byte, if appropriate).  Note that there is
 * no way to predict how large the m_byte buffer must be, but you're
 * responsible for making it large enough anyway.
 * 
 * If m_byte is NULL then it resets its internal shift state and returns 1
 * if that state was complex, or 0 if trivial.
 */
int wctomb(char* m_byte, wchar_t wide_char)
{
}
