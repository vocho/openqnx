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





#include <errno.h>
#include <hw/sysinfo.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/rsrcdbmgr.h>
#include <sys/syspage.h>
#include <unistd.h>


//*****************************************************************************
/* *INDENT-OFF* */
#ifdef __USAGE
%C [Options] *
- Seed Hardware Specific Resources from the syspage HWINFO section.

Options:
	-d 			- display the hwi info syspage
	-m name		- seed a memory reference for name (DISABLES AutoSeeding)
	-v 			- verbose 
#endif
/* *INDENT-ON* */
//*****************************************************************************


#define NEXT_ITEM(t)	((hwi_tag *)((uint32_t *)(t) + (t)->item.itemsize))
#define NEXT_TAG(t)		((hwi_tag *)((uint32_t *)(t) + (t)->prefix.size))
void
dump_tags (void)
{
	hwi_tag *item;
	hwi_tag *tag;

	item = __hwi_base ();
	for (; item->prefix.size != 0; item = NEXT_ITEM (item))
	{
		printf ("item=%p [%s] \n", item, __hwi_find_string (item->item.itemname));

		for (tag = NEXT_TAG (item); tag->prefix.size != 0 && tag <= NEXT_ITEM (item);
			tag = NEXT_TAG (tag))
		{
			printf ("\ttag=%p [%s] \n", tag, __hwi_find_string (tag->prefix.name));

			if (strcmp (__hwi_find_string (tag->prefix.name), HWI_TAG_NAME_location) == 0)
				printf ("\t\t len=%x base=%llx regshift=%x addrspace=%x \n",
					tag->location.len, tag->location.base,
					tag->location.regshift, tag->location.addrspace);
		}
	}
}


void
add_memory_ref (int verbose, char *name)
{
	unsigned offset;
	hwi_tag *tag;
	rsrc_alloc_t ralloc;

	if ((offset = hwi_find_item (HWI_NULL_OFF, name, NULL)) == HWI_NULL_OFF)
	{
		fprintf (stderr, "Device [%s] Not found is syspage \n", name);
		return;
	}

	if ((offset = hwi_find_tag (offset, 0, HWI_TAG_NAME_location)) == HWI_NULL_OFF)
	{
		fprintf (stderr, "Device [%s] does not have a location tag \n", name);
		return;
	}

	tag = hwi_off2tag (offset);

	if( verbose )
		printf ("Seeding Memory Reference %s @ %llx size %x \n", name,
			tag->location.base, tag->location.len);

	memset (&ralloc, 0, sizeof (ralloc));
	ralloc.start = tag->location.base;
	ralloc.end = tag->location.base + tag->location.len - 1;
	ralloc.flags = RSRCDBMGR_FLAG_NAME | RSRCDBMGR_FLAG_NOREMOVE;
	ralloc.name = name;

	if (rsrcdbmgr_create (&ralloc, 1) == -1)
	{
		perror ("Unable to seed resource memory: ");
		fprintf(stderr, "Unable to seed resource memory: errno=%d \n", errno );
		return;
	}

}

void
add_all_memory_refs (int verbose)
{
	hwi_tag *item;
	char   *name;

	item = __hwi_base ();
	for (; item->prefix.size != 0; item = NEXT_ITEM (item))
	{
		name = strdup(__hwi_find_string (item->item.itemname));
		if (strncmp (name, "memory/", 7) == 0)
			add_memory_ref (verbose, name);
		free(name);
	}
}


int
main (int argc, char *argv[])
{
	int     c;
	int     autoseed = 1;
	int     verbose = 0;

	while ((c = getopt (argc, argv, "dm:v")) != EOF)
	{
		switch (c)
		{
		case 'd':
			dump_tags ();
			break;

		case 'm':
			autoseed = 0;
			add_memory_ref (verbose, optarg);
			break;

		case 'v':
			verbose = 1;
			break;
		}
	}

	if (autoseed)
	{
		add_all_memory_refs (verbose);
	}

	return (EXIT_SUCCESS);
}
