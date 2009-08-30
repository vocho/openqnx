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





/*
Feb 4, 2000 - Complete rewrite
if run as 'od' this util follows the posix standard for od.
if run as 'hd' it functions similar to the previous version of 'hd'.

At the moment there is no support for 64bit fp dumps... it defaults to 32bit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifdef __QNXNTO__
#include <libgen.h>
#endif
#ifdef __NT__
#include <lib/compat.h>
#endif
#ifdef __MINGW32__
#define fcloseall _fcloseall
#endif

struct options
       {
         int verbose, blocksize, no_formats;
         unsigned long countint, skipint;
         char *ab, *s, *c, *ts;
       };

extern char* optarg;
extern int optind, opterr, optopt;
int default_flag=1, endian_flag=1;
int single_input_file=0;
int bit8=0;

char *names[34]={"nul","soh","stx","etx","eot","enq","ack","bel","bs","ht",
                 "lf","vt","ff","cr","so","si","dle","dc1","dc2","dc3",
                 "dc4","nak","syn","etb","can","em","sub","esc","fs","gs",
                 "rs","us","sp","del"};

char *_pname;

void set_od_defaults( struct options *);
void set_hd_defaults( struct options *);
void parse_opts( struct options *);
char **mk_file_list( char**, int, int);
FILE **open_list( char**, int);
void close_list( FILE**, int );
void read_files( FILE**, int, struct options );
void read_stdin( struct options );
int is_printable(char);
int is_type(char);
void format_out(int, char *, struct options,int );
int od_main(int, char**);
int hd_main(int, char**);
int get_endian(void);
int get_blocksize(char, char);

int main( int argc, char **argv )
{
  int ret;
  _pname=basename(argv[0]);
  endian_flag=get_endian();
  if (!strncmp(_pname, "od", 2))
    ret = od_main(argc, argv);
  else
    if(!strncmp(_pname,"hd", 2))
      ret = hd_main(argc, argv);
    else
      ret = od_main(argc, argv);
  return ret; 
}

int od_main( int argc, char **argv )
{
  int optret, totalfiles;
  struct options opts;
  char **filelist=NULL;
  FILE **inputfiles=NULL;
  set_od_defaults( &opts );
  while( ( optret = getopt( argc, argv, "vA:j:N:t:" ) ) != -1 )
  {
    switch( optret ) 
    {
        case 'v': 
          opts.verbose=1;
          break;
        case 'A':
          opts.ab=optarg;
          break;
        case 'j':
          opts.s=optarg;
          break;
        case 'N':
          opts.c=optarg;
          break;
        case 't':
          opts.ts=optarg;
          default_flag=0;
          break;
        case '?':
          exit(1);
          break;
    }
  }
  parse_opts( &opts );
  
  totalfiles=argc-optind;
  
  if (totalfiles==0)
  {
    read_stdin(opts);    
  }
  else
  {
      if (totalfiles==1)
        single_input_file=1;
      else
        single_input_file=0;
      filelist=mk_file_list( argv, optind, argc );
      inputfiles=open_list( filelist, totalfiles );
      read_files( inputfiles, totalfiles, opts );
      close_list( inputfiles, totalfiles );
  }
  
  free( filelist );
  free( inputfiles );
  return 0;
}

int hd_main( int argc, char **argv )
{
  int optret, totalfiles;
  struct options opts;
  char **filelist=NULL;
  FILE **inputfiles=NULL;
  set_hd_defaults( &opts );
  while( ( optret = getopt( argc, argv, "vA:j:s:n:N:t:8" ) ) != -1 )
  {
    switch( optret ) 
    {
        case 'v': 
          opts.verbose=1;
          break;
        case 'A':
          opts.ab=optarg;
          break;
	case 's':
          opts.s=optarg;
          break;
        case 'n':
          opts.c=optarg;
          break;
        case 't':
          opts.ts=optarg;
          default_flag=0;
          break;
        case '8':
          bit8=1;
          break;
        case '?':
          exit(1);
          break;
    }
  }
  parse_opts( &opts );
  
  totalfiles=argc-optind;
  
  if (totalfiles==0)
  {
    read_stdin(opts);    
  }
  else
  {
      if (totalfiles==1)
        single_input_file=1;
      else
        single_input_file=0;
      filelist=mk_file_list( argv, optind, argc );
      inputfiles=open_list( filelist, totalfiles );
      read_files( inputfiles, totalfiles, opts );
      close_list( inputfiles, totalfiles );
  }
  
  free( filelist );
  free( inputfiles );
  return 0;
}

void set_od_defaults( struct options *opts )
{
  opts->ab="o";
  opts->ts="o";
  opts->verbose=0;
  opts->s="0";
  opts->c="0";
}

void set_hd_defaults( struct options *opts )
{
  opts->ab="x";
  opts->ts="x1";
  opts->verbose=0;
  opts->s="0";
  opts->c="0";
}

void parse_opts( struct options *opts )
{
  int temp=0;
  long multiplier=1;
  char *local;
  if ( strcmp(opts->ab, "d") && strcmp(opts->ab, "o"))
    if ( strcmp(opts->ab, "x") && strcmp(opts->ab, "n"))
    {
      fprintf(stderr, "invalid output radix '%s'; must be one character of [doxn]\n", opts->ab);
      exit(1);
    }
  if (!strcmp(opts->s, "0"))
    opts->skipint=0;
  else
  {
    temp=strlen(opts->s);
    if (( local=(char *)malloc(temp*sizeof(char))) == NULL ) 
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(1);
    }    
    memcpy(local, opts->s, temp);
    if((local[temp-1]=='b')||(local[temp-1]=='B'))
    {
      if ((local[1]!='x')&&(local[1]!='X'))
      {
        multiplier=512;
        local[temp-1]='\0';
      } 
    } 
    else
      if((local[temp-1]=='k')||(local[temp-1]=='K'))
      {
        multiplier=1024;
        local[temp-1]='\0';
      } 
      else
        if((local[temp-1]=='m')||(local[temp-1]=='M'))
        {
          multiplier=1048576;
          local[temp-1]='\0';
        }
    opts->skipint=strtoul(local, NULL, 0)*multiplier;

    if (opts->skipint<=0)
    {
      fprintf(stderr, "invalid skip argument '%s'\n", opts->s);
      exit(1);
    }
    memset(local, 0, temp);
    free(local);
  }
  multiplier=1;
  if ( !strcmp(opts->c, "0"))
    opts->countint=0;
  else
  {
	temp=strlen(opts->c);
	if (( local=(char *)malloc(temp*sizeof(char))) == NULL )
	{
  		fprintf(stderr, "%s\n", strerror(errno));
  		exit(1);
	}
	memcpy(local, opts->c, temp);
	if((local[temp-1]=='b')||(local[temp-1]=='B'))
	{
  		if ((local[1]!='x')&&(local[1]!='X'))
  		{
    			multiplier=512;
    			local[temp-1]='\0';
  		}
	}
	else
  		if((local[temp-1]=='k')||(local[temp-1]=='K'))
  		{
    			multiplier=1024;
    			local[temp-1]='\0';
  		}
  		else
    			if((local[temp-1]=='m')||(local[temp-1]=='M'))
    			{
      				multiplier=1048576;
      				local[temp-1]='\0';
    			}

	opts->countint=strtoul(local, NULL, 0)*multiplier;

	if (opts->countint<=0)
	{
  		fprintf(stderr, "invalid count argument '%s'\n", opts->s);
  		exit(1);
	}
	free(local);
	local=NULL;
    }
}

char **mk_file_list( char **cmdline, int curr_pos, int total_pos )
{
  char **local_temp=NULL;
  int local_count=0, total_files=0;
  total_files=total_pos-curr_pos;
  if (( local_temp=(char **)malloc(total_files*sizeof(char*)) ) == NULL ) 
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }
  for( local_count = 0; local_count < total_files ; local_count++ )
  {
    local_temp[local_count]=cmdline[curr_pos+local_count];
  } 
  return local_temp;
}

FILE **open_list( char **list, int total_files )
{
  int local_count=0;
  FILE **local_list=NULL;
  if( (local_list = (FILE **)malloc(total_files*sizeof(FILE*))) == NULL )
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }
  for (local_count=0;local_count<total_files;local_count++)
  {
    local_list[local_count]=fopen( list[local_count], "rb" );
    if( local_list[local_count] == NULL )
    {
      fprintf(stderr, "%s: %s\n", strerror(errno), list[local_count]);  
      fcloseall();
      exit(1);
    }
  }
  return local_list;
}


void close_list( FILE **list, int total_files )
{
  /*there was more here, hence the parameters*/
  fcloseall();
}


void read_files( FILE **list, int total_files, struct options opts)
{
  char current, buf[17];
  int count=0, off=0, poff=0;
  int local_count=0, tally=0;
  int flag=1;
  poff+=opts.skipint;
  tally=opts.countint;
  for ( local_count=0;local_count<total_files;local_count++)
  {
    if (single_input_file)
    {
          fseek(list[local_count], 0, SEEK_END);
          if (ftell(list[local_count])>opts.skipint)
            if (!( fseek(list[local_count], opts.skipint, SEEK_SET) ))
              off+=opts.skipint;
            else
            {
              fprintf(stderr, "error skipping %ld bytes of input\n", opts.skipint);
              exit(1);
            }
          else
          {
            fprintf(stderr, "attempt to skip past end of combined input\n");
            exit(1);  
          }
      single_input_file=0;
    }
    while (fread(&current, sizeof(char), 1, list[local_count]))
    {
      off++;
      if ( opts.countint )
        if (!tally)
          break;
      if ( off > opts.skipint )
      {
        flag=0;
        buf[count]=current;
        buf[count+1]='\0';
        count++;
        if (opts.countint) tally--;
        if (count > 15) 
        {
          format_out(poff,buf,opts,count);
          poff+=count;
          count=0;
        }
      }
    }
  }
  if (count) format_out(poff,buf,opts, count);
  if (flag)
  {
    fprintf(stderr, "attempt to skip past end of combined input\n");
    exit(1);
  }
  format_out(off,NULL,opts, 0);
}

void read_stdin( struct options opts )
{
  char current, buf[17];
  int count=0, off=0, poff=0;
  int tally=0;
  int flag=1;
  poff+=opts.skipint;
  tally=opts.countint;
  while (fread(&current, sizeof(char), 1, stdin))
  {
    off++;
    if ( opts.countint )
      if (!tally)
        break;
    if ( off > opts.skipint )
    {
      flag=0;
      buf[count]=current;
      buf[count+1]='\0';
      count++;
      if (opts.countint) tally--;
      if (count > 15) 
      {
        format_out(poff,buf,opts,count);
        poff+=count;
        count=0;
      }
    }
  }
  if (count) format_out(poff,buf,opts,count);
  if (flag)
  {
    fprintf(stderr, "attempt to skip past end of combined input\n");
    exit(1);
  }
  format_out(off,NULL,opts,0);
}

int is_printable( char test )
{
  
  if (test<=32)  
    return 0;
  if (bit8)
  {
    if (test==127)
      return 0;
    return 1;
  }
  else
  {
    if (test>=127)
      return 0; 
    return 1;
  }
}

int is_type( char t )
{
  if((t=='a')||(t=='c')||(t=='d')||(t=='f')||(t=='o')||(t=='u')||(t=='x'))
    return 1;
  return 0;
}

int get_endian(void)
{
  int t=1;
  t = (*(char *)&t == 1) ? 0 : 1;
  return t;
}

int get_blocksize( char type, char next )
{
 char nextn[2];
 int blocksize=0;
 nextn[0]=next;
 nextn[1]='\0';
 if((is_type(next))||(next==NULL))
          {
            blocksize=sizeof(int);
          }
          else
          {
            if( (type=='d')||(type=='o')||(type=='u')||(type=='x') )
            {
              if( (next=='1')||(next=='2')||(next=='4') )
                blocksize=atoi(nextn);
              else
                if (next=='C')
                  blocksize=sizeof(char);
                else
                  if (next=='S')
                    blocksize=sizeof(short);
                  else
                    if (next=='I')
                      blocksize=sizeof(int);
                    else
                      if (next=='L')
                        blocksize=sizeof(long);
                      else
                      {
                        fprintf(stderr, "invalid block size '%c' for type '%c'\n", next, type);
                        exit(1);
                      }  
            }
            else
              if( type=='f' )
              {
                if( (next=='4')||(next=='8') )
                  blocksize=atoi(nextn);
                else
                  if (next=='F')
                    blocksize=sizeof(float);
                  else
                    if (next=='D')
                      //blocksize=sizeof(float);
                      blocksize=sizeof(double);
                    else
                      if (next=='L')
                        //blocksize=sizeof(float);
                        blocksize=sizeof(long double);
                      else
                      {
                        fprintf(stderr, "invalid block size '%c' for type '%c'\n", next, type);
                        exit(1);
                      }
              }
              else
                if( (type=='a')||(type=='c') )
                {
                  blocksize=1;
                }
          }
  return blocksize;
}

void format_out( int off, char *buf, struct options opts , int readbytes)
{
  static char lastbuff[17];
  char type;
  int current=0, temp, arg_count=0, blocksize=0, adj=0;
  static int flag;
  long conv;
  //double dbuf;

  lastbuff[16]='\0';
  if (!is_type(opts.ts[0]))
  {
    fprintf(stderr, "invalid type '%c'\n", (char)opts.ts[0]);
    exit(1); 
  }
 
  if (readbytes)
    if (!memcmp(buf, lastbuff, 16)&&(!opts.verbose))
    {
      if (flag) 
        printf("*\n");
      flag=0;
    }
    else
      flag=1;
  else
    flag=1;

  if (flag)
  {
    if (readbytes) 
      memcpy(lastbuff, buf, 16);
    if (strcmp(opts.ab, "n"))
    {
      if (!strcmp(opts.ab, "x"))
        printf("%.7X ", off);
      if (!strcmp(opts.ab, "d"))
        printf("%.9d ", off);
      if (!strcmp(opts.ab, "o"))
        printf("%.10o ", off);
    }
    if ( readbytes)
    {
      for (arg_count=0;arg_count<strlen(opts.ts);arg_count++)
      {
        if (is_type(opts.ts[arg_count]))
        {
          type=opts.ts[arg_count];
          blocksize=get_blocksize(opts.ts[arg_count], opts.ts[arg_count+1]);
          if ((arg_count)&&(strcmp(opts.ab, "n")))
            printf("       ");
          if ((type=='a')||(type=='c')) blocksize=1;
          for( adj=0;adj<readbytes;adj+=blocksize)
          {
            conv=0x00000000;
            for( temp=0;temp<blocksize;temp++)
            {
              conv<<=8;
              conv&=0xffffff00;
              if (endian_flag)
              {
                if (((unsigned char)buf[temp+adj])==NULL) conv&=0xffffff00;
                else conv|=(unsigned char)(buf[temp+adj]&0xff);
              }
              else
              {
                if (((unsigned char)buf[(blocksize-temp-1)+adj])==NULL) conv&=0xffffff00;
                else conv|=(unsigned char)(buf[(blocksize-temp-1)+adj]&0xff);
              } 
            }
            current++;
            if (type=='x') 
            {
              if (blocksize==1) printf("%2.2x ", (unsigned int)(conv&0xff));
              if (blocksize==2) printf("%4.4x ", (unsigned int)(conv&0xffff));
              if (blocksize==4) printf("%8.8x ", (unsigned int)(conv));
            } 
            if (type=='d') 
            {
              if (blocksize==1) printf("%3.3d ", (int)(conv&0xff));
              if (blocksize==2) printf("%5.5d ", (int)(conv&0xffff));
              if (blocksize==4) printf("%10.10d ", (int)(conv));
            }
            if (type=='o')
            {
              if (blocksize==1) printf("%3.3o ", (unsigned int)(conv&0xff));
              if (blocksize==2) printf("%6.6o ", (unsigned int)(conv&0xffff));
              if (blocksize==4) printf("%11.11o ", (unsigned int)(conv));
            }
            if (type=='u') 
            {
              if (blocksize==1) printf("%3.3u ", (unsigned int)(conv&0xff));
              if (blocksize==2) printf("%5.5u ", (unsigned int)(conv&0xffff));
              if (blocksize==4) printf("%10.10u ", (unsigned int)(conv));
            }
            if (type=='f')
            {
              if (blocksize==4) printf("%e ", (double)(conv));
              if (blocksize==8) printf("%e ", (double)(conv));
            }
            if (type=='c')
            {
              if ((char)(conv&0xff)=='\\')
                printf("  \\ ");  
              else
                if ((char)(conv&0xff)=='\a')
                  printf(" \\a ");
                else
                 if ((char)(conv&0xff)=='\b')
                   printf(" \\b ");
                 else
                   if ((char)(conv&0xff)=='\f')
                     printf(" \\f ");
                   else
                     if ((char)(conv&0xff)=='\n')
                       printf(" \\n ");
                     else
                       if ((char)(conv&0xff)=='\r')
                         printf(" \\r ");
                       else
                         if ((char)(conv&0xff)=='\t')
                           printf(" \\t ");
                         else
                           if ((char)(conv&0xff)=='\v')
                             printf(" \\v ");
                           else
                             if ((char)(conv&0xff)=='\0')
                               printf(" \\0 ");
                             else
                                 if(!is_printable((char)(conv&0xff)))
                                   printf("%3.3o ", (unsigned char)(conv&0x7f));
                                 else
                                   printf("%3c ", (char)(conv&0x7f));
            }
            if (type=='a')
            {
              if (is_printable((char)(conv&0xff))) printf("%3c ", (char)(conv&0x7f));
              else 
              {
                if ((char)(conv&0x7f)<=32)  printf("%3.3s ", names[(int)(conv&0x7f)]);
                if ((char)(conv&0x7f)==127) printf("%3.3s ", names[33]);
              }
            }
          }
         if (!default_flag) printf("\n");
        }
      }
      if (default_flag)
      {
        if (!strncmp(_pname, "hd", 2)) 
        {
          //this will work for now.  push the ascii to the end of an incomplete line
          for (temp=0; temp<(16-current) ; temp++)
          {
            printf("   ");
          }
          current=0;
          while (current<readbytes)
          {
            if (!bit8)
            {
              if (is_printable(buf[current]&0xff))
                printf("%c", buf[current]&0x7f);
              else
                printf(".");
            }
            else
            {
              if (is_printable(buf[current]&0x7f))
                printf("%c", buf[current]&0xff);
              else
                printf(".");
            }
            current++;
          }
          printf("\n");
        }
        else
          printf("\n");
      }
    }
    else
      printf("\n");
  }
} 
 
