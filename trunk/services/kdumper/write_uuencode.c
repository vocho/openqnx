#include "kdumper.h"

#define ENC(c) (((c) & 077) + ' ')

#define SEQMAX 'z'
#define SEQMIN 'a'
char seqc = SEQMAX;


static	unsigned	saved_flag;
static	unsigned	idx;
static	char		buff[45];

static void
outdec(char *p) {
	int c1, c2, c3, c4;

	c1 = p[0] >> 2;
	c2 = (p[0] << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	kprintf("%c%c%c%c", ENC(c1), ENC(c2), ENC(c3), ENC(c4));
}


static void
outblock(char *p, unsigned n) {
	unsigned	i;

	kprintf("%c", ENC(n));
	for(i = 0; i < n; i += 3) {
		outdec(&p[i]);
	}
	kprintf("%c\n", seqc);
	seqc--;
	if(seqc < SEQMIN) seqc = SEQMAX;
}

extern struct kdump	*dip;

void
write_uuencode(void *p, unsigned len) {
	int 				n;

	if(p == WRITE_CMD_ADDR) {
		if(len == WRITE_INIT) {
			// init
			idx = 0;
			kprintf("begin %o %s%s\n", 0644, "kdump.elf", dip->compress == 1 ? ".gz":"" );
			// Since we're outputting the dump to the kprintf device,
			// don't print any debugging messages.
			saved_flag = debug_flag;
			debug_flag = 0;
		} else {
			// fini
			if(idx != 0) {
				outblock(buff, idx);
			}
			/* line of length 0 then end */
			kprintf("`\nend\n");
			// Re-enable debugging messages.
			debug_flag = saved_flag;
		}
		return;
	}
	for(;;) {
		n = sizeof(buff) - idx;
		if(len < n) n = len;
		memcpy(&buff[idx], p, n);
		idx += n;
		if(idx < sizeof(buff)) break;
		outblock(buff, idx);
		p = (char *)p + n;
		len -= n;
		idx = 0;
	}
}
