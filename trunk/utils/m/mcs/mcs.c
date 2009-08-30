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





#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <utime.h>
#include <ar.h>
#include <libelf.h>
#include <libgen.h>
#include <sys/stat.h>

char *progname;

void panic(const char *str, ...) {
	va_list			ap;

	fprintf(stderr, "%s: ", progname);
	va_start(ap, str);
	vfprintf(stderr, str, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

struct list		{
	struct list			*next;
	char				string[1];
}				*strings, *sections;

void process(Elf *elf, int section) {
	
}

void add_list(struct list **plist, const char *string) {
	struct list			*list;
	int					len;

	len = strlen(string);
	if(!(list = malloc(sizeof *list + len))) {
		panic("Out of memory\n");
	}
	strcpy(list->string, string);
	list->next = 0;
	while(*plist) {
		plist = &(*plist)->next;
	}
	*plist = list;
}

void print(Elf *elf, Elf32_Ehdr *ehdr, struct list *sections, FILE *fp) {
	struct list			*list;
			
	for(list = sections; list; list = list->next) {
		int					section;

		for(section = 0; section < ehdr->e_shnum; section++) {
			Elf_Scn				*scn;
				
			if((scn = elf_getscn(elf, section))) {
				Elf32_Shdr			*shdr;
				
				if((shdr = elf32_getshdr(scn))) {
					char			*p;
				
					if((p = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name))) {
						if(!strcmp(p, list->string)) {
							Elf_Data			*data;

							for(data = 0; (data = elf_getdata(scn, data));) {		// @@@ This should be elf_rawdata() but it doesn't seem to work!!
								if(data->d_buf && data->d_size > 0) {
									int					n;
									char				*str;

									for(n = 0, str = data->d_buf; n < data->d_size; str++, n++) {
										if(*str == '\0') {
											*str = '\n';
										}
									}
									fwrite(data->d_buf, 1, data->d_size, fp);
								}
							}
							break;
						}
					}
				}
			}
		}
	}
}

void append(Elf *elf, Elf32_Ehdr *ehdr, struct list *sections, struct list *strings, const char *fname) {
	char			*file;
	struct list		*list;

	file = tmpnam(0);
	for(list = sections; list; list = list->next) {
		struct list		*string;
		struct list		*tmplist;
		char			buff[4096];
		FILE			*fp;

		if(!(tmplist = alloca(sizeof *tmplist + strlen(list->string)))) {
			unlink(file);
			panic("No memory!");
		}
		strcpy(tmplist->string, list->string);
		tmplist->next = 0;

		if(!(fp = fopen(file, "w"))) {
			unlink(file);
			panic("Unable to open temporary file");
		}

		print(elf, ehdr, tmplist, fp);
		for(string = strings; string; string = string->next) {
			fwrite(string->string, 1, strlen(string->string) + 1, fp);
		}
		fclose(fp);

		snprintf(buff, sizeof buff, "\\ldrel -s%s=%s %s", list->string, file, fname);
		system(buff);
	}
	unlink(file);
}

void compress(Elf *elf, Elf32_Ehdr *ehdr, struct list *sections, const char *fname) {
	char			*file;
	struct list		*list;

	file = tmpnam(0);
	for(list = sections; list; list = list->next) {
		struct list		*tmplist;
		char			buff[4096];
		FILE			*fp;

		if(!(tmplist = alloca(sizeof *tmplist + strlen(list->string)))) {
			unlink(file);
			panic("No memory!");
		}
		strcpy(tmplist->string, list->string);
		tmplist->next = 0;

		if(!(fp = fopen(file, "w"))) {
			unlink(file);
			panic("Unable to open temporary file");
		}

		print(elf, ehdr, tmplist, fp);
		fclose(fp);

unlink(file), panic("Compress not finished\n");

		snprintf(buff, sizeof buff, "\\ldrel -s%s=%s %s", list->string, file, fname);
		system(buff);
	}
	unlink(file);
}

void delete(Elf *elf, Elf32_Ehdr *ehdr, struct list *sections, char **argv) {
	char			buff[4096];
	int				pos;
	struct list		*list;

	pos = snprintf(buff, sizeof buff, "\\strip -p");
	for(list = sections; list; list = list->next) {
		pos += snprintf(buff + pos, sizeof buff - pos, " -R%s", list->string);
	}
	while(*argv) {
		pos += snprintf(buff + pos, sizeof buff - pos, " %s", *argv++);
	}
	system(buff);
}

int main(int argc, char *argv[]) {
	int				i;
	Elf_Cmd			cmd;
	Elf				*arf;
	Elf				*elf;
	int				show_name = 0;
	enum {
		PRINT, APPEND, COMPRESS, DELETE
	}				command = PRINT;

	progname = basename(argv[0]);

	while((i = getopt(argc, argv, "a:cdn:pV")) != -1) {
		switch(i) {
		case 'a':
			command = APPEND;
			add_list(&strings, optarg);
			break;
		case 'c':
			command = COMPRESS;
			break;
		case 'd':
			command = DELETE;
			break;
		case 'p':
			command = PRINT;
			break;
		case 'n':
			add_list(&sections, optarg);
			break;
		case 'V':
			fprintf(stderr, "%s: Version 1\n", progname);
			exit(EXIT_SUCCESS);
		default:
			argc = 0;	// Force help to be printed
			break;
		}
	}
	if(strings && command != APPEND) {
		fprintf(stderr, "%s: -a cannot be used with other options\n", progname);
		exit(EXIT_FAILURE);
	}
	if(argc < 2) {
		fprintf(stderr,
			"%s - Manipulate the comment section of an object file\n"
			"\n"
			"%s [-cdpV] [-a string] [-n name]* file...\n"
	        "\n"
	        "\t-a string Append string to comment section\n"
	        "\t-c        Compress contents of comment section\n"
			"\t-d        Delete comment section\n"
			"\t-n name   Name of comment section (default: .comment)\n"
			"\t-p        Print the contents of the comment section\n"
			"\t-V        Print program version\n",
			progname, progname
		);
		exit(EXIT_SUCCESS);
	}

	if(elf_version(EV_CURRENT) == EV_NONE) {
		panic("Elflib version error version=%d elflibversion=%d\n", EV_CURRENT, elf_version(EV_NONE));
	}

	if(!sections) {
		add_list(&sections, ".comment");
	}

	// If only one file being processed, don't display it.
	if(command == PRINT && argc - optind > 1) {
		show_name = 1;
	};

	--optind;
	while(++optind < argc) {
		int					fd;

		if(!(fd = open(argv[optind], O_RDONLY))) {
			fprintf(stderr, "%s: unable to open %s\n", progname, argv[optind]);
			continue;
		}
			
		cmd = ELF_C_READ;
		if(!(arf = elf_begin(fd, cmd, 0))) {
			fprintf(stderr, "%s: not a known file format %s\n", progname, argv[optind]);
			continue;
		}

		while((elf = elf_begin(fd, cmd, arf))) {
			Elf_Arhdr		*arhdr;

			if((arhdr = elf_getarhdr(elf))) {
				if(command == PRINT) {
					show_name = 1;
				}
				if(show_name && arhdr->ar_name[0] != '/') {
					printf("%s[%s]:\n", argv[optind], arhdr->ar_name);
				}
			} else if (show_name) {
				printf("%s:\n", argv[optind]);
			}

			if(!arhdr || arhdr->ar_name[0] != '/') {
				char		*ident;
				size_t		size;

				if(!(ident = elf_getident(elf, &size)) || size < EI_NIDENT) {
					fprintf(stderr, "%s: not a known file format %s\n", progname, arhdr ? arhdr->ar_name : argv[optind]);
				} else {
					Elf32_Ehdr			*ehdr;

					if((ehdr = elf32_getehdr(elf))) {
						int					section;
						Elf32_Phdr			*phdr;
						Elf32_Off			end = 0;

						// Find end of segments only if we need to modify the file.
						if(command != PRINT && (phdr = elf32_getphdr(elf))) {
							int					segment;

							for(segment = 0; segment < ehdr->e_phnum; segment++, phdr++) {
								Elf32_Off			current;

								current = phdr->p_offset + phdr->p_filesz;
								if(current > end) {
									end = current;
								}
							}
						}
						for(section = 0; section < ehdr->e_shnum; section++) {
							Elf_Scn				*scn;
				
							if((scn = elf_getscn(elf, section))) {
								Elf32_Shdr			*shdr;
				
								if((shdr = elf32_getshdr(scn))) {
									char			*p;
				
									if((p = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name))) {
										struct list			*list;
				
										for(list = sections; list; list = list->next) {
											if(!strcmp(p, list->string)) {
												if(shdr->sh_offset < end) {
													fprintf(stderr, "%s: Section %s is not modifiable\n", progname, p);
													break;
												}
											}
										}
										if(list) {
											break;
										}
									}
								}
							}
						}
						if(section < ehdr->e_shnum) {
							continue;
						}
						switch(command) {
						case PRINT:
							print(elf, ehdr, sections, stdout);
							break;
						case COMPRESS:
							compress(elf, ehdr, sections, argv[optind]);
							break;
						case APPEND:
							append(elf, ehdr, sections, strings, argv[optind]);
							break;
						case DELETE:
							delete(elf, ehdr, sections, &argv[optind]);
							optind = argc;	// Delete handles all the files at once
							break;
						}
					}
				}
			}
		
			cmd = elf_next(elf);
			elf_end(elf);
		}
		elf_end(arf);
	}

	return EXIT_SUCCESS;
}
