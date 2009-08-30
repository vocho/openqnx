/*
 * Copyright 2005, QNX Software Systems Ltd. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

// Created for CR 27682

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>

// Non-portable OS function calls for time
#ifdef WIN32
#define get_milliseconds() 	GetTickCount()
#define ONE_SECOND        	1000
typedef tick_type	  		DWORD
#else
#define get_milliseconds() 	clock()
#define ONE_SECOND        	CLOCKS_PER_SEC
#define tick_type	  		clock_t
#endif


typedef int BOOL;
#define FALSE 0
#define TRUE  1



// Maximum number of endian bytes we'll swap
#define MAX_ENDIAN 8

// Structure we allocate for tracking each file
typedef struct file_struct {
    char *filename;
    FILE *fd;
    int64_t    maxcount;
    int64_t    maxsize;
    int64_t    sizeondisk;
    int64_t    currentcount;
    int64_t    currentlocation;
    int32_t    endianswapbytes;

    int32_t    endianarraylocation;
    int32_t    endianarraycount;
    unsigned char endianarray[MAX_ENDIAN];
    
    unsigned char padchar;
    BOOL     ispad;
    BOOL     isdiscard;
    BOOL     isinput;
    BOOL     isfinished;
    struct file_struct *next;
} file_struct;

//
// Our chain of input files
file_struct *input_chain_first;
file_struct *input_chain_last;
file_struct *current_input;

//
// Our chain of output files
file_struct *output_chain_first;
file_struct *output_chain_last;
file_struct *current_output;

//
// Parameters set during argument parsing
int64_t size_parameter   = 0;
int64_t count_parameter  = 0;
int32_t endian_parameter = 1;
uint32_t verbosity       = 0;
BOOL limited_output_size = TRUE;
BOOL limited_input_size  = TRUE;

// ----------------------------------------------------------------------
// Function: PrintUsage
//
// Purpose:  If prog run with no args, prints help.
// ----------------------------------------------------------------------
void print_usage()
{
#if defined(_WIN32)
// Windows doesn't have a separate use message, so embed the message
// directly in the exe
		fprintf(stderr,
"imgmanip - Manipulate image files\n"
"imgmanip is a general purpose utility, used to convert one layout of raw\n"
"binary file into another.  Its primary purpose is to convert boot image\n"
"files into images suitable for flash chips, but has many other uses.\n"
"\n"
"Usage: imgmanip [options]\n"
"\n"
"Input File Options:\n"
" -i inputfile       Input image file\n"
" -p char            Input pseudo-file which only contains pad character\n"
"\n"
"Output File Options:\n"
" -o outputfile      Output image file\n"
" -d                 Output pseudo-file which discards all output\n"
"\n"
"File Control Options (-m & -c apply to all subsequent input/output files):\n"
" -m maxsize         Sets maximum in/out size to <maxsize> bytes\n"
"                      -m0 means no size limit (default)\n"
" -c count           Sets num bytes read/written before cycling to next in/out\n"
"                      -c0 means exhaust current file before cycling (default)\n"
" -e number          Endian: # bytes (1,2,4,8) to swap during read/write\n"
"                      -e1 means do not perform endian swapping (default)\n"
"\n"
" Sizes are of the form <n>[K|M|G] to specify <n> bytes, kilobytes, megabytes,\n"
" or gigabytes.  Use [bits] after any to specify in bits instead of bytes.\n"
"\n"
"General Options:\n"
" -v                 Verbose mode; prints warnings of unconsumed data\n"
" -vv                Extra verbose: reports capacities and consumption\n"
" -vvv               Super verbose; prints all arguments\n"
"\n"
"Examples:\n"
"\n"
" # Split 16MB image file into two 8MB files\n"
" imgmanip -i whole16mb.img -m 8M -o first8mb.img -o second8mb.img\n"
"\n"
" # Combine above two half flash images back into one\n"
" imgmanip -i first8mb.img -i second8mb.img -o whole16mb.img\n"
"\n"
" # Split 16MB image into two 8MB files, the first supplying the low 16-bits\n"
" # and the second the high 16-bits for a 32-bit bus\n"
" imgmanip -i whole16mb.img -m 8M -c 16bits -o low16bits.img -o high16bits.img\n"
"\n"
" # Combine above two low/high flash images back into one\n"
" imgmanip -c 16bits -i low16bits.img -i high16bits.img -o whole16mb.img\n"
"\n"
" # Pad file to 4 megabits with FF characters\n"
" imgmanip -i input.img -p 0xFF -m 4Mbits -o output.img\n"
"\n"
" # Pad a text file to 256 bytes, interleaving zeros to convert ANSI to Unicode\n"
" imgmanip -c 1 -i readme.ansi -p 0 -m 256 -o readme.unicode\n"
"\n"
" # Extract the middle 32K in big endian format from a 96K little-endian file\n"
" imgmanip -i le96k.bin -m 32K -d -e4 -o be32k.bin -d\n"
		);
#else
	// Use message is in imgmanip.use
	
	fprintf(stderr,"imgmanip: invalid arguments. Type \"use imgmanip\" for usage\n");
#endif // _WIN32
}

// ----------------------------------------------------------------------
// Function: human_readable
//
// Purpose:  Returns a 64-bit int as bytes converted into human readable string.
//
// NOTE:    This function uses an internal buffer; subsequent calls to
//          human_readable will reuse that buffer.  Copy out the result
//          if you want to keep it (e.g., don't put multiple calls to
//          human_readable within the same printf).
// ----------------------------------------------------------------------
const char *human_readable(int64_t n, const char *singular, const char *plural)
{
    static char temp[80];

    const int64_t kb = 1024L;
    const int64_t mb = 1024L*1024L;
    const int64_t gb = 1024LL*1024LL*1024LL;

	// If exact number of bytes for Giga, Mega, or Kilo, print that,
	// else print number of bytes.  Always precise and to the highest
	// normally expressable units
    if (n>gb && (n%gb)==0)
        sprintf(temp, "%lldG", n/gb);
    else if (n>mb && (n%mb)==0)
        sprintf(temp, "%lldM", n/mb);
    else if (n>kb && (n%kb)==0)
        sprintf(temp, "%lldK", n/kb);
    else
        sprintf(temp, "%lld", n);

    if (singular && plural)
    {
        strcat(temp, " ");
        strcat(temp, (n==1) ? singular : plural);
    }

    return temp;
}

// ----------------------------------------------------------------------
// Function: get_value
//
// Purpose:  Converts a string of bytes/kbytes/mbytes/gbytes into a number.
// ----------------------------------------------------------------------
int64_t get_value(const char *str)
{
    int64_t total = strtoul(str, 0, 0);
    int len = strlen(str);
    char modifier;
    BOOL bits = FALSE;

    // We allow "bits" to be the last part (so they can
    // specify endianess as "16bits" or a flash part as 8Mbits")
    if (!stricmp(str+len-4,"bits"))
    {
        bits = TRUE;
        len-=4;
    }

    modifier = str[len - 1];

    switch (modifier)
    {
        case 'G':
        case 'g':
            total *= ((int64_t)1024)*((int64_t)1024)*((int64_t)1024);
            break;
        case 'M':
        case 'm':
            total *= ((int64_t)1024)*((int64_t)1024);
            break;
        case 'K':
        case 'k':
            total *= 1024;
            break;
    }

    return (bits) ? (total>>3) : total;
}

// ----------------------------------------------------------------------
// Function: create_new_fs
//
// Purpose:  Creates an empty file structure, fills in all the default
//           fields, and links into the chain.
// ----------------------------------------------------------------------
file_struct *create_new_fs(file_struct **first,file_struct **last, const char *name)
{
    // Create a new file structure
    file_struct *nstruct = (file_struct *)calloc(sizeof(file_struct), 1);

    // Set the filename
    char * newname = NULL;

    // Allocate as a new string
    if (name)
    {
        newname = malloc(strlen(name)+1);
        strcpy(newname, name);
    }
    nstruct->filename = newname;

    // Set rest of default options
    nstruct->maxcount = count_parameter;
    nstruct->maxsize = size_parameter;
    nstruct->endianswapbytes = endian_parameter;

    // Insert into the chain
    if (*first == NULL)
        *first = nstruct;
    else
        (*last)->next = nstruct;

    *last = nstruct;

    if (verbosity>=3)
    {
        fprintf(stderr,"  File      = %s\n",(name) ?
                                                 name :
                                                 "<none>");
        fprintf(stderr,"  Max size  = %s\n",(nstruct->maxsize) ?
                                                 human_readable(nstruct->maxsize, "byte", "bytes") :
                                                 "unlimited");
        fprintf(stderr,"  Max count = %s\n",(nstruct->maxcount) ?
                                                 human_readable(nstruct->maxcount, "byte", "bytes") :
                                                 "all");
        fprintf(stderr,"  Endian    = %s\n", human_readable(nstruct->endianswapbytes, "byte", "bytes"));
    }

    // Reset "dangerous" parameters back to default
    endian_parameter = 1;

    return nstruct;
}

// ----------------------------------------------------------------------
// Function: free_chain
//
// Purpose:  Closes all files and frees all allocated structs in the chain
// ----------------------------------------------------------------------
void free_chain(file_struct *fs)
{
    file_struct *first = fs;
    file_struct *next;

    // If anything at all, loop around the chain to destroy everything
    if (fs)
    {
        do
        {
            next = fs->next;

            if (fs->fd && fclose(fs->fd)!=0)
            {
                fprintf(stderr, "Error: Could not close file %s!\n", fs->filename);
            }

            if (fs->filename)
                free(fs->filename);

            free(fs);

            fs = next;
            // We link the head to the tail to make circular processing
            // easy, so detect when we loop all the way around.
        } while (fs && fs != first);
    }
}

// ----------------------------------------------------------------------
// Function: close_all
//
// Purpose:  Closes all files and frees all allocated data
// ----------------------------------------------------------------------
void close_all()
{
    free_chain(input_chain_first);
    input_chain_first = input_chain_last = NULL;

    free_chain(output_chain_first);
    output_chain_first = output_chain_last = NULL;
}

// ----------------------------------------------------------------------
// Function: parse_args
//
// Purpose:  Parses command line arguments
// ----------------------------------------------------------------------
void parse_args(int argc, char *argv[])
{
    file_struct *fs;
    char c;

	if (argc <= 1)
	{
		print_usage();
		exit(1);
	}

	while ((c = getopt(argc, argv, "i:o:m:c:p:e:dv")) > 0)
    {
        switch (c)
		{
            case 'i':
                if (verbosity>=3)
                    fprintf(stderr, "Input from file:\n");
                
                fs = create_new_fs(&input_chain_first,&input_chain_last, optarg);

                fs->isinput = TRUE;
                fs->fd = fopen(fs->filename, "rb");
                if (!fs->fd)
                {
                    fprintf(stderr, "Error: Unable to open %s for input!\n", fs->filename);
                    close_all();
                    exit(2);
                }

                // Any real file will have a limit, so we never will set 
                // limited_input_size (even if fs-maxsize==0) for this type of input.

                fseek(fs->fd, 0, SEEK_END);
                fs->sizeondisk = ftell(fs->fd);
                fseek(fs->fd, 0, SEEK_SET);

                if (verbosity>=3)
                    fprintf(stderr,"\n");
                break;
            
            case 'p':
                if (verbosity>=3)
                    fprintf(stderr, "Input from pad:\n");

                fs = create_new_fs(&input_chain_first,&input_chain_last, NULL);
                fs->isinput = TRUE;
                fs->ispad = TRUE;
                fs->padchar = (unsigned char)strtoul(optarg, NULL, 0);
 
                // Only a pad has unlimited data; if they don't explicitly cap it, take note of it
                if (fs->maxsize == 0)
                    limited_input_size = FALSE;

                if (verbosity>=3)
                {
                    fprintf(stderr,"  Pad char  = 0x%02x (%d)\n", fs->padchar, fs->padchar);
                    fprintf(stderr,"\n");
                }
                break;
			
            case 'o':
                if (verbosity>=3)
                    fprintf(stderr, "Output to file:\n");

                fs = create_new_fs(&output_chain_first,&output_chain_last, optarg);

                fs->isinput = FALSE;
                fs->fd = fopen(fs->filename, "wb");
                
                if (fs->maxsize == 0)
                    limited_output_size = FALSE;
                
                if (!fs->fd)
                {
                    fprintf(stderr, "Error: Unable to open %s for output!\n", fs->filename);
                    close_all();
                    exit(2);
                }

                if (verbosity>=3)
                    fprintf(stderr,"\n");
                break;

            case 'd':
                if (verbosity>=3)
                    fprintf(stderr, "Output to discard:\n");

                fs = create_new_fs(&output_chain_first,&output_chain_last, NULL);
                fs->isinput = FALSE;
                fs->isdiscard = TRUE;

                if (fs->maxsize == 0)
                    limited_output_size = FALSE;

                if (verbosity>=3)
                    fprintf(stderr,"\n");
                break;

            case 'm':
                size_parameter = get_value(optarg);
                break;

            case 'c':
                count_parameter = get_value(optarg);
                break;

            case 'e':
                endian_parameter = (uint32_t)get_value(optarg);
                if (endian_parameter!=1 && 
                    endian_parameter!=2 &&
                    endian_parameter!=4 &&
                    endian_parameter!=8)
                {
                    fprintf(stderr, "Error: Endian (-e) can only be 1, 2, 4, or 8 bytes.\n");
                    close_all();
                    exit(2);
                }
                break;

            case 'v':
                verbosity++;
                break;

			default:
				fprintf(stderr, "Error: invalid argument.\n");
                close_all();
				exit(2);
		}
    }
}

// ----------------------------------------------------------------------
// Function: validate_args
//
// Purpose:  Make sure that we've got valid args before we start
// ----------------------------------------------------------------------
void validate_args()
{
    // Verify that they have both an input and an output
    if (!input_chain_first)
        fprintf(stderr, "Error: No inputs specified.\n");

    if (!output_chain_first)
        fprintf(stderr, "Error: No outputs specified.\n");

    if (!input_chain_first || !output_chain_first)
    {
        close_all();
        exit(2);
    }

    // If we do have files, link the heads to the tails so we can
    // cycle around the lists in the read/write easily.
    input_chain_last->next = input_chain_first;
    output_chain_last->next = output_chain_first;
}

// ----------------------------------------------------------------------
// Function: are_bytes_to_read
//
// Purpose:  Are there any bytes left to read from the input sources?
// ----------------------------------------------------------------------
BOOL are_bytes_to_read()
{
    file_struct *fs = input_chain_first;

    while (fs)
    {
        if (!fs->isfinished)
            return TRUE;

        fs = fs->next;

        // If we wrap all the way around, we're done.
        if (fs==input_chain_first)  return FALSE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------
// Function: can_write_bytes
//
// Purpose:  Are there any bytes left to write to the output sources?
// ----------------------------------------------------------------------
BOOL can_write_bytes()
{
    file_struct *fs = output_chain_first;

    while (fs)
    {
        if (!fs->isfinished)
            return TRUE;

        fs = fs->next;

        // If we wrap all the way around, we're done.
        if (fs==output_chain_first)  return FALSE;
    }
    return FALSE;
}

// ----------------------------------------------------------------------
// Function: write_data_array
//
// Purpose:  Write a byte to an output
// ----------------------------------------------------------------------
void write_data_array(file_struct *fs, unsigned char ch)
{
    // This destination is easy--to discard, just toss everything
    if (fs->isdiscard)
        return;

    // Otherwise write the data (and do the endian swap)
    if (fs->endianarraycount == 0)
    {
        // Record bytes to be written
        fs->endianarraycount = fs->endianswapbytes;
        fs->endianarraylocation = fs->endianarraycount - 1;
    }

    // Write data to the endian buffer backwards
    fs->endianarray[fs->endianarraylocation--] = ch;

    // If we've filled our endian buffer, flush the data to disk
    if (fs->endianarraylocation < 0)
    {
        // Signify the buffer is empty
        fs->endianarraycount = 0;

        // Actually write the data
        if (fwrite(fs->endianarray, 1, fs->endianswapbytes, fs->fd) != (size_t)fs->endianswapbytes ||
            ferror(fs->fd))
        {
            fprintf(stderr, "Error: unable to write to %s at location %llu!\n", fs->filename, fs->currentlocation);
            close_all();
            exit(3);
        }
    }
}

// ----------------------------------------------------------------------
// Function: write_byte
//
// Purpose:  Write a byte to the outputs
// ----------------------------------------------------------------------
BOOL write_byte(unsigned char ch)
{
    file_struct *beginning = current_output;
    file_struct *fs = current_output;

    do
    {
        // If we are not finished with this file
        if (!fs->isfinished)
        {
            // Okay--we've started to write to this file;
            // keep on this file for the next byte
            current_output = fs;

            // Get the current character and point to the next
            write_data_array(fs, ch);

            // Increment our location in the file.
            fs->currentcount++;
            fs->currentlocation++;

            // We're not filling the entire file and we've hit
            // our byte limit on this particular pass through
            if (fs->maxcount != 0 &&
                fs->currentcount >= fs->maxcount)
            {
                // Reset the count, and move to the next file...
                fs->currentcount = 0;
                current_output = fs->next;
            }

            // If there is a max capacity, and we've written all we're supposed
            // to write to this file, stop outputting to this file.
            if (fs->maxsize && fs->currentlocation>=fs->maxsize)
            {
                fs->isfinished = TRUE;
                fs->endianarraycount = 0;
            }

            // Yes, we wrote data
            return TRUE;
        }

        // Keep looking for a place to write until we make a full circle;
        // if we haven't found one by the time we get back to
        // where we started, we have no more valid outputs.
        fs = fs->next;

    } while (fs != beginning);

    // No, we didn't write data
    return FALSE;
}

// ----------------------------------------------------------------------
// Function: fill_read_arrays
//
// Purpose:  Fill the endian data arrays, which we read out of to get
//           the input data.
// ----------------------------------------------------------------------
void fill_read_arrays(file_struct *fs)
{
    // If we've used all our endian buffered data, clear our data count
    if (fs->endianarraylocation < 0)
        fs->endianarraycount = 0;

    // If there isn't anything left, read more
    if (!fs->endianarraycount)
    {
        if (fs->ispad)
        {
            // For a pad source, we generate the data
            memset(fs->endianarray, fs->padchar, fs->endianswapbytes);
            fs->endianarraycount = fs->endianswapbytes;
        }
        else
        {
            if (feof(fs->fd))
            {
                fs->isfinished = TRUE;
                return;
            }
            else
            {
                fs->endianarraycount = fread(fs->endianarray, 1, fs->endianswapbytes, fs->fd);
                if (ferror(fs->fd))
                {
                    fprintf(stderr, "Error: unable to read from %s at location %ld!\n", fs->filename, ftell(fs->fd));
                    close_all();
                    exit(3);
                }
            }
        }

        // Reset the index to the "first" data byte
        // (since we do endian compensation, we work through the array backwards.)
        fs->endianarraylocation = fs->endianarraycount - 1;
    }
}

// ----------------------------------------------------------------------
// Function: read_byte
//
// Purpose:  Read a byte from the outputs
// ----------------------------------------------------------------------
BOOL read_byte(unsigned char *ch)
{
    file_struct *beginning = current_input;
    file_struct *fs = current_input;

    do
    {
        // If we are not finished with this file
        if (!fs->isfinished)
        {
            // Make sure there is data waiting for us
            fill_read_arrays(fs);

            // As long as there is something, use it;
            // if nothing, we have to cycle to the next file.
            if (fs->endianarraycount && fs->endianarraylocation>=0)
            {
                // Okay--we've started to pull from this file;
                // keep on this file for the next byte
                current_input = fs;

                // Get the current character and point to the next
                *ch = fs->endianarray[fs->endianarraylocation];
                fs->endianarraylocation--;

                // Increment our location in the file.
                fs->currentcount++;
                fs->currentlocation++;

                // We're not consuming this entire file and we've hit
                // our byte limit on this particular pass through
                if (fs->maxcount != 0 &&
                    fs->currentcount >= fs->maxcount)
                {
                    // Reset the count, and move to the next file...
                    fs->currentcount = 0;
                    current_input = fs->next;
                }

                // If there is a max capacity, and we've read all we're supposed
                // to read from this file, stop
                if (fs->maxsize && fs->currentlocation>=fs->maxsize)
                {
                    fs->isfinished = TRUE;
                    fs->endianarraycount = 0;
                }

                // Yes, we have valid data
                return TRUE;
            }
        }

        // Keep looking for data until we make a full circle;
        // if we haven't found it by the time we get back to
        // where we started, we have no more data.
        fs = fs->next;

    } while (fs != beginning);

    // No, we have no valid data
    return FALSE;
}

// ----------------------------------------------------------------------
// Function: main_loop
//
// Purpose:  Does the actual work of reading/writing
// ----------------------------------------------------------------------
void main_loop()
{
    char ch;
    int64_t   bytes = 0;
    tick_type  tick = get_milliseconds();

    current_input = input_chain_first;
    current_output = output_chain_first;

    do
    {
        // Process a single byte
        if (read_byte(&ch))
        {
            write_byte(ch);
        }

        bytes++;

        // If they want progress, every once in a while (every 1K) check
        // to print the status.
        if (verbosity>=1 && (bytes&0x3ff)==0)
        {
            // Limit to once a second
            if (get_milliseconds() - tick > ONE_SECOND)
            {
                tick = get_milliseconds();
                fprintf(stderr,"Processed %s...\r", human_readable(bytes, "byte", "bytes"));
            }
        }
    } while (are_bytes_to_read() && can_write_bytes());

    if (verbosity >= 1)
    {
        // Only complain about having more to read if they aren't using a pad
        if (limited_input_size && are_bytes_to_read())
        {
            fprintf(stderr, "Warning: not all input data was read.\n");
        }

        // Only complain about having more to write if they've constrained
        // the output size.
        if (limited_output_size && can_write_bytes())
        {
            fprintf(stderr, "Warning: output data files are not completely full.\n");
        }

        fprintf(stderr,"Done.                        \n");
    }
}

// ----------------------------------------------------------------------
// Function: print_stats
//
// Purpose:  Prints statistics of how much data there was, and how much
//           was used.
// ----------------------------------------------------------------------
void print_stats()
{
    int64_t total;
    file_struct *fs;

    // Only do this if they asked for it
    if (verbosity < 2)
        return;

    // Okay--they asked for it!
    fs = input_chain_first;
    fprintf(stderr, "\nINPUTS\n");
    fprintf(stderr, "------\n");

    do {
        if (fs->ispad)
            fprintf(stderr, "Pad with 0x%02x (%d)\n", fs->padchar, fs->padchar);
        else
            fprintf(stderr, "File: %s\n", fs->filename);

        fprintf(stderr, "  Max size      = %s\n", (fs->maxsize) ?
                                             human_readable(fs->maxsize, "byte", "bytes") :
                                             "unlimited");
        fprintf(stderr, "  Actual size   = %s\n", (!fs->ispad) ?
                                             human_readable(fs->sizeondisk, "byte", "bytes") :
                                             "unlimited");
        fprintf(stderr, "  Bytes read    = %s\n",  human_readable(fs->currentlocation, "byte", "bytes"));

        if (fs->ispad)
        {
            if (fs->maxsize)
                total = fs->maxsize;
            else
                total = 0;      // If no cap on input, print 0% read (which is TRUE)
        }
        else
        {
            if (fs->maxsize)
                total = min(fs->maxsize, fs->sizeondisk);
            else
                total = fs->sizeondisk;
        }

        if (total)
            fprintf(stderr, "  Percent read  = %.1f %%\n", (100.0 * fs->currentlocation)/total);
        else
            fprintf(stderr, "  Percent read  = 0.0 %%\n");

        fprintf(stderr, "\n");

        fs = fs->next;
    } while (fs!=input_chain_first);

    // Now do the outputs...

    fs = output_chain_first;
    fprintf(stderr, "\nOUTPUTS\n");
    fprintf(stderr, "-------\n");

    do {
        if (fs->isdiscard)
            fprintf(stderr, "Discard output");
        else
            fprintf(stderr, "File: %s\n", fs->filename);

        fprintf(stderr, "  Max size      = %s\n", (fs->maxsize) ?
                                             human_readable(fs->maxsize, "byte", "bytes") :
                                             "unlimited");
        fprintf(stderr, "  Bytes written = %s\n",  human_readable(fs->currentlocation, "byte", "bytes"));

        total = fs->maxsize;

        if (total)
            fprintf(stderr, "  Percent filled= %.1f %%\n", (100.0 * fs->currentlocation)/total);
        else
            fprintf(stderr, "  Percent filled= 0.0 %%\n");

        fprintf(stderr, "\n");

        fs = fs->next;
    } while (fs!=output_chain_first);

}

// ----------------------------------------------------------------------
// Function: main
//
// Purpose:  Program entry point
// ----------------------------------------------------------------------
int main(int argc, char *argv[])
{
	parse_args(argc, argv);
	validate_args();
	main_loop();
	print_stats();
	close_all();
	return(0);
}
