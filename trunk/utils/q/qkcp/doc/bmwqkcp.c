/*
 *  File copy customisation callouts for "qkcp -X bmwqkcp.so".
 *  # cc -shared -c bmwqkcp.c -o bmwqkcp.o
 *  # cc -shared -Wl,-Bsymbolic bmwqkcp.o -o bmwqkcp.so
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

/*
 *  Callout made once to allow initial optional configuration; the source
 *  and destination paths are provided as is any optional argument
 *  specified to the '-X' (as in "qkcp -X bmwqkcp.so:limit=128m,name=mp3"
 *  would pass in 'argument' as a copy of the string "limit=128m,name=mp3").
 *  These can be used to determine the direction of the copy if the
 *  transform functions below are not involutions f(f(X))=X, or to load
 *  external transform tables or configuration files or envvars, etc.
 *
 *  For this project, there is nothing to do.
 */
int qkcp_init(const char *src, const char *dst, char *argument)
{
	return(EOK);
}

/*
 *  Callout made for every file, providing the stat details 'st' and
 *  target filename 'name'; the final pathname component may be altered
 *  in-place (total filename buffer is 'maxlen').
 *
 *  For this project, multimedia file extensions are changed, to help
 *  disguise the nature of the backup, and files larger than an arbitrary
 *  value are skipped.
 */
int qkcp_filename(const struct stat64 *st, char *name, int maxlen)
{
static const struct extmap {
	char	from[5], to[5];
}					suffixes[] = {
						{ "aac", "br1" }, { "AAC", "BR1" },
						{ "cda", "br2" }, { "CDA", "BR2" },
						{ "m4a", "br3" }, { "M4A", "BR3" },
						{ "mp3", "br4" }, { "MP3", "BR4" },
						{ "wma", "br5" }, { "WMA", "BR5" },
					};
const struct extmap	*s;
char				*ext;

	if (st->st_size > 0x7FFFFFFF) {
		return(EFBIG);
	}
	else if ((ext = strrchr(((ext = strrchr(name, '/')) != NULL) ? ext + 1 : name, '.')) != NULL) {
		++ext;
		for (s = suffixes; s < &suffixes[sizeof(suffixes) / sizeof(struct extmap)]; ++s) {
			if (!strcmp(ext, s->from))
				strcpy(ext, s->to);
			else if (!strcmp(ext, s->to))
				strcpy(ext, s->from);
			else
				continue;
			return(EOK);
		}
	}
	return(EINVAL);
}

/*
 *  Callout made for every data buffer (at vaddr 'data', 'length' bytes).
 *  The buffer is PROT_NOCACHE DMA-safe memory, typically 64k in size.
 *  As the target file is pre-grown, the data length cannot be changed.
 *
 *  For this project, file content is bit-inverted, to help disguise the
 *  nature of the backup.
 */
int qkcp_filedata(off64_t offset, void *data, int length)
{
unsigned	*word;
int			nwords;

	nwords = (length + sizeof(unsigned) - 1) / sizeof(unsigned);
	for (word = data; --nwords >= 0; ++word) {
		*word = ~*word;
	}
	return(EOK);
}

