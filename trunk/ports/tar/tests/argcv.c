/* argcv.c - simple functions for parsing input based on whitespace
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA  */

#include <ctype.h>

#include <argcv.h>

/*
 * takes a string and splits it into several strings, breaking at ' '
 * command is the string to split
 * the number of strings is placed into argc
 * the split strings are put into argv
 * returns 0 on success, nonzero on failure
 */

#define isws(c) ((c)==' '||(c)=='\t'||(c)=='\n')
#define isdelim(c,delim) ((c)=='"'||strchr(delim,(c))!=NULL)

static int
argcv_scan (int len, const char *command, const char *delim, const char* cmnt,
	    int *start, int *end, int *save)
{
  int i = 0;

  for (;;)
    {
      i = *save;

      if (i >= len)
	return i + 1;

      /* Skip initial whitespace */
      while (i < len && isws (command[i]))
	i++;
      *start = i;

      switch (command[i])
	{
	case '"':
	case '\'':
	  while (++i < len
		 && (command[i] != command[*start]
		     || command[i-1] == '\\'))
	    ;
	  if (i < len)		/* found matching quote */
	    break;
	 /*FALLTHRU*/ default:
	  if (isdelim (command[i], delim))
	    break;
	  /* Skip until next whitespace character or end of line. Honor
	     escaped whitespace. */
	  while (++i < len &&
		 !((isws (command[i]) && command[i-1] != '\\')
		   || isdelim (command[i], delim)));
	  i--;
	  break;
	}

      *end = i;
      *save = i + 1;

      /* If we have a token, and it starts with a comment character, skip
         to the newline and restart the token search. */
      if (*save <= len)
	{
	  if (cmnt && strchr (cmnt, command[*start]) != NULL)
	    {
	      i = *save;
	      while (i < len && command[i] != '\n')
		i++;

	      *save = i;
	      continue;
	    }
	}
      break;
    }
  return *save;
}

static char escape_transtab[] = "\\\\a\ab\bf\fn\nr\rt\t";

int
argcv_unescape_char (int c)
{
  char *p;

  for (p = escape_transtab; *p; p += 2)
    {
      if (*p == c)
	return p[1];
    }
  return c;
}

int
argcv_escape_char (int c)
{
  char *p;
  
  for (p = escape_transtab + sizeof(escape_transtab) - 2;
       p > escape_transtab; p -= 2)
    {
      if (*p == c)
	return p[-1];
    }
  return -1;
}
  

static int
xtonum (const char *src, int base, size_t cnt)
{
  int val;
  char *p;
  char tmp[4]; /* At most three characters + zero */
  
  /* Notice: No use to check `cnt'. It should be either 2 or 3 */
  memcpy (tmp, src, cnt);
  tmp[cnt] = 0;
  val = strtoul (tmp, &p, base);
  return (*p == 0) ? val : -1;
}

static size_t
escaped_length (const char *str, int *quote)
{
  size_t len = 0;

  for (; *str; str++)
    {
      if (*str == ' ')
	{
	  len++;
	  *quote = 1;
	}
      else if (*str == '"')
	{
	  len += 2;
	  *quote = 1;
	}
      else if (isprint (*str))
	len++;
      else if (argcv_escape_char (*str) != -1)
	len += 2;
      else
	len += 4;
    }
  return len;
}

static void
unescape_copy (char *dst, const char *src, size_t n)
{
  int c;
  
  while (n > 0)
    {
      n--;
      if (*src == '\\')
	{
	  switch (*++src)
	    {
	    case 'x':
	    case 'X':
	      ++src;
	      --n;
	      if (n == 0)
		{
		  *dst++ = '\\';
		  *dst++ = src[-1];
		}
	      else
		{
		  c = xtonum(src, 16, 2);
		  if (c == -1)
		    {
		      *dst++ = '\\';
		      *dst++ = src[-1];
		    }
		  else
		    {
		      *dst++ = c;
		      src += 2;
		      n -= 2;
		    }
		}
	      break;
	      
	    case '0':
	      ++src;
	      --n;
	      if (n == 0)
		{
		  *dst++ = '\\';
		  *dst++ = src[-1];
		}
	      else
		{
		  c = xtonum(src, 8, 3);
		  if (c == -1)
		    {
		      *dst++ = '\\';
		      *dst++ = src[-1];
		    }
		  else
		    {
		      *dst++ = c;
		      src += 3;
		      n -= 3;
		    }
		}
	      break;
	      
	    default:
	      *dst++ = argcv_unescape_char (*src++);
	      n--;
	    }
	}
      else
	{
	  *dst++ = *src++;
	}
    }
  *dst = 0;
}

static void
escape_copy (char *dst, const char *src)
{
  for (; *src; src++)
    {
      if (*src == '"')
	{
	  *dst++ = '\\';
	  *dst++ = '"';
	}
      else if (*src != '\t' && isprint(*src))
	*dst++ = *src;      
      else
	{
	  int c = argcv_escape_char (*src);
	  *dst++ = '\\';
	  if (c != -1)
	    *dst++ = c;
	  else
	    {
	      char tmp[4];
	      snprintf (tmp, sizeof tmp, "%03o", *(unsigned char*)src);
	      memcpy (dst, tmp, 3);
	      dst += 3;
	    }
	}
    }
}

int
argcv_get (const char *command, const char *delim, const char* cmnt,
	   int *argc, char ***argv)
{
  int len = strlen (command);
  int i = 0;
  int start, end, save;

  *argv = NULL;

  /* Count number of arguments */
  *argc = 0;
  save = 0;

  while (argcv_scan (len, command, delim, cmnt, &start, &end, &save) <= len)
      (*argc)++;

  *argv = calloc ((*argc + 1), sizeof (char *));

  i = 0;
  save = 0;
  for (i = 0; i < *argc; i++)
    {
      int n;
      argcv_scan (len, command, delim, cmnt, &start, &end, &save);

      if ((command[start] == '"' || command[end] == '\'')
	  && command[end] == command[start])
	{
	  start++;
	  end--;
	}
      n = end - start + 1;
      (*argv)[i] = calloc (n+1,  sizeof (char));
      if ((*argv)[i] == NULL)
	return 1;
      unescape_copy ((*argv)[i], &command[start], n);
      (*argv)[i][n] = 0;
    }
  (*argv)[i] = NULL;
  return 0;
}

/*
 * frees all elements of an argv array
 * argc is the number of elements
 * argv is the array
 */
int
argcv_free (int argc, char **argv)
{
  while (--argc >= 0)
    if (argv[argc])
      free (argv[argc]);
  free (argv);
  return 1;
}

/* Take a argv an make string separated by ' '.  */

int
argcv_string (int argc, char **argv, char **pstring)
{
  size_t i, j, len;
  char *buffer;

  /* No need.  */
  if (pstring == NULL)
    return 1;

  buffer = malloc (1);
  if (buffer == NULL)
    return 1;
  *buffer = '\0';

  for (len = i = j = 0; i < argc; i++)
    {
      int quote = 0;
      int toklen;

      toklen = escaped_length (argv[i], &quote);
      
      len += toklen + 2;
      if (quote)
	len += 2;
      
      buffer = realloc (buffer, len);
      if (buffer == NULL)
        return 1;

      if (i != 0)
	buffer[j++] = ' ';
      if (quote)
	buffer[j++] = '"';
      escape_copy (buffer + j, argv[i]);
      j += toklen;
      if (quote)
	buffer[j++] = '"';
    }

  for (; j > 0 && isspace (buffer[j-1]); j--)
    ;
  buffer[j] = 0;
  if (pstring)
    *pstring = buffer;
  return 0;
}

#if 0
char *command = "set prompt=\"& \a\\\"\" \\x25\\0145\\098\\ta";

main(int xargc, char **xargv)
{
  int i, argc;
  char **argv;
  char *s;
  
  argcv_get (xargv[1] ? xargv[1]:command, "=", "#", &argc, &argv);
  printf ("%d args:\n", argc);
  for (i = 0; i < argc; i++)
    printf ("%s\n", argv[i]);
  printf ("===\n");
  argcv_string (argc, argv, &s);
  printf ("%s\n", s);
}
#endif
