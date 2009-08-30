/*- $Id: strip.c 153052 2008-08-13 01:17:50Z coreos $
 * strip: remove extraneous information from load files.
 * The source is a bit larger than need be -- it is derived
 * from another lmf-utility.  Should really have a library
 * for this stuff.
 *
 * $Log$
 * Revision 1.3  1999/06/07 21:12:02  steve
 * Attempt #3 to get an initial working setup....
 *
 * Revision 1.1  1996/07/24 20:04:01  steve
 * Initial revision
 *
 */

#include <libc.h>
#include <sys/stat.h>
#include <sys/lmf.h>

char verbose;
char keepsym;
char *pname = "strip";


void
print(char *f, ...)
{
	va_list    v;
	va_start(v, f);
	fputs("\nltrim: error ", stderr);
	vfprintf(stderr, f, v);
}

void
fatal(char *f, ...)
{
	va_list    v;

	va_start(v, f);
	fputs("\nltrim: error ", stderr);
	vfprintf(stderr, f, v);
	exit(1);
}


typedef struct _lmf_header     Lmfhdr;
typedef struct _lmf_definition Lmfdefn;
typedef union  lmfrec          Lrec;

union lmfrec {
	struct _lmf_definition    *ldef;
	struct _lmf_data          *ldat;
	struct _lmf_seg_fixup     *lsfix;
	struct _lmf_linear_fixup  *llfix;
	struct _lmf_eof           *leof;
	struct _lmf_resource      *lres;
};


typedef struct LMF LMF;
struct LMF {
	FILE          *fp;
	ulong_t        foffs;
	Lmfdefn        defrec;       /* definition record */
	unsigned       nsegs;        /* number of segments */
	ulong_t       *segs;         /* segments themselves */
	int            isvalid;
	unsigned       nalloc;
	Lmfhdr         chdr;
	union {
		char         *currec;
		Lrec          rec;
		Lrec;
	};
};
#define UNKNOWN_REC       0x7f


L_next(LMF *p)
{
	if (p->isvalid) {
		p->foffs += sizeof p->chdr + p->chdr.data_nbytes;
		p->isvalid = 0;
	}
	if (fseek(p->fp, p->foffs, SEEK_SET) == -1 ||
	    fread(&p->chdr, sizeof p->chdr, 1, p->fp) != 1) {
	    	p->isvalid = 0;
		return -1;
	}
	switch (p->chdr.rec_type) {
	case _LMF_DEFINITION_REC:
	case _LMF_COMMENT_REC:
	case _LMF_DATA_REC:
	case _LMF_FIXUP_SEG_REC:
	case _LMF_FIXUP_80X87_REC:
	case _LMF_EOF_REC:
	case _LMF_RESOURCE_REC:
	case _LMF_ENDDATA_REC:
	case _LMF_FIXUP_LINEAR_REC:
	case _LMF_PHRESOURCE:
		break;
	default:
		fseek(p->fp, p->foffs, SEEK_SET);
		p->isvalid = 0;
		return UNKNOWN_REC;
	}
	p->isvalid = 1;
	return p->chdr.rec_type;
}

L_rectype(LMF *p)
{
	return p->isvalid ? p->chdr.rec_type : -1;
}

void *
L_readrec(LMF *p)
{
	if (p->isvalid == 0)
		return 0;
	if (!p->currec || p->nalloc < p->chdr.data_nbytes) {
		void *x;
		if ((x=realloc(p->currec, p->chdr.data_nbytes)) == 0) {
			free(p->currec);
			p->currec = 0;
			p->nalloc = 0;
			return 0;
		}
		p->currec = x;
		p->nalloc = p->chdr.data_nbytes;
	}
	fread(p->currec, 1, p->chdr.data_nbytes, p->fp);
	return p->currec;
}

L_done(LMF *p)
{
	fclose(p->fp); p->fp = 0; /* paranoid */
	if (p->currec) {
		free(p->currec);
		p->currec = 0;
		p->nalloc = 0;
	}
	p->foffs = 0;
	p->isvalid = 0;
}

L_init(LMF *p)
{
	if (L_next(p) == -1 ||
	    L_rectype(p) != _LMF_DEFINITION_REC ||
	    L_readrec(p) == 0) {
		L_done(p);
		return -1;
	}
	p->nsegs = (p->chdr.data_nbytes - sizeof p->defrec)/sizeof *p->segs;
	if ((p->segs=calloc(sizeof *p->segs, p->nsegs)) == 0) {
		L_done(p);
		return -1;
	}
	memcpy(p->segs, p->ldef+1, sizeof *p->segs *p->nsegs);
	return 0;
}

L_rewind(LMF *p)
{
	p->foffs   = 0;
	p->isvalid = 0;
	if (p->currec) {
		free(p->currec);
		p->currec  = 0;
	}
	p->nalloc = 0;
	if (p->segs) {
		free(p->segs);
		p->segs = 0;
	}
	p->nsegs = 0;
	fseek(p->fp, 0, SEEK_SET);
	return L_init(p);
}


L_open(char *name, LMF *p)
{
	if ((p->fp = fopen(name, "r")) == 0)
		return -1;
	p->foffs   = 0;
	p->isvalid = 0;
	p->currec  = 0;
	p->nalloc  = 0;
	p->segs    = 0;
	p->nsegs   = 0;
	return L_init(p);
}

iszero(void *p, unsigned nb)
{
	char   *x = p;
	unsigned     i;
	for (i=0; i < nb && x[i]==0; i++);
	return i == nb;
}

LPUT(FILE *fp, LMF *src)
{
	if (verbose) {
		printf("writing %u bytes\n",
			sizeof src->chdr + src->chdr.data_nbytes);
	}
	fwrite(&src->chdr, sizeof src->chdr, 1, fp);
	fwrite(src->currec, src->chdr.data_nbytes, 1, fp);
}

lmf_strip(LMF *ldf, FILE *fp)
{
#define BIG 2000000
	unsigned      last_zrec = BIG;
	int           nzrecs = 0;
	unsigned      rcount;
	unsigned      t;
	char          b[4096];
	memset(b, 0, sizeof b);

	if (L_rewind(ldf)) {
		print("couldn't reset file");
		return -1;
	}
	for (rcount = 0; (t=L_next(ldf)) != -1 && t != UNKNOWN_REC;
	     rcount++) {
		if (verbose)
			printf("%d = %u\n", L_rectype(ldf), ldf->chdr.data_nbytes);
		if (L_readrec(ldf) == 0) {
			print("corrupt ldf\n");
			return -1;
		}
		if (L_rectype(ldf) != _LMF_DATA_REC) {
			continue;
		}
		if (ldf->ldat->segment_index != 0)
			continue;
		if (!iszero(ldf->ldat+1,
			ldf->chdr.data_nbytes-sizeof *ldf->ldat)) {

			last_zrec = BIG; /* kill it */
			nzrecs = 0;
		} else {
			b[rcount] = 1;
		}
	}
	if (verbose) {
		printf("# records = %u, remove records %d..%u\n", rcount, last_zrec, nzrecs);
	}
	L_rewind(ldf);

	LPUT(fp, ldf); /* get defn' record */
	for (rcount = 0; (t=L_next(ldf)) != -1 && t != UNKNOWN_REC;
	     rcount++) {
		if (verbose)
			printf("%d = %u\n", L_rectype(ldf), ldf->chdr.data_nbytes);
		if (L_readrec(ldf) == 0)  {
			print("weird end of file!\n");
			return -1;
		}
		if (rcount >= last_zrec && rcount < last_zrec+nzrecs)
			continue;
		if (!b[rcount])
			LPUT(fp, ldf);
		else if (verbose)
			printf("skipping %u\n", rcount);
	}
	if (t != -1 && keepsym) { /* watcom debugging info */
		int         c;
		while ((c=getc(ldf->fp)) != EOF)
			putc(c, fp);
	}
	return 0;
}

main(int argc, char **argv)
{
	int         c;
	int         errs = 0;
	int         i;
	pname = basename(argv[0]);
	while ((c=getopt(argc, argv, "vsd:")) != -1) {
		switch (c) {
		case 's': keepsym = 1; break;
		case 'v': verbose = 1; break;
		default:
			exit(1);
		}
	}
	if (optind == argc) {
		fatal("need a filename\n");
	}
	for (i=optind; i < argc; i++) {
		char       *fn = argv[i];
		struct stat st;
		LMF      ldf;
		FILE       *fp;
		if (stat(fn, &st) == -1) {
			print("cannot access '%s' (%s)\n", fn, strerror(errno));
			errs++;
		} else if (access(fn, W_OK) == -1) {
			print("cannot write '%s' (%s)\n", fn, strerror(errno));
			errs++;
		} else if (L_open(fn, &ldf) == -1) {
			print("'%s' is not an executable\n", fn);
			errs++;
		} else if (unlink(fn) == -1) {
			print("cannot remove '%s'\n", fn);
			errs++;
		} else if ((fp=fopen(fn, "w")) == 0) {
			print("can't create '%s' (%s)\n", fn, strerror(errno));
			L_done(&ldf);
			errs++;
		} else if (lmf_strip(&ldf, fp) == -1) {
			fclose(fp);
			unlink(fn);
			errs++;
		} else {
			umask(0777);
			fchmod(fileno(fp), st.st_mode);
			fchown(fileno(fp), st.st_uid, st.st_gid);
			fclose(fp);
		}
		L_done(&ldf);
	}
	exit(errs ? 1 : 0);
}
