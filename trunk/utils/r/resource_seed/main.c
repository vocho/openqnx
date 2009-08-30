/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * This program creates a DMA channel resource database manager.  This
 * allows drivers to request and reserve DMA channels. This program should
 * be run when the system boots before loading any drivers that need DMA 
 * (e.g. audio drivers).
 */

#include <sys/rsrcdbmgr.h>
#include <inttypes.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include<stdio.h>

#define BUFF_SIZE (256 + 1)
char   *opts[] = {
	"mem",
	"irq",
	"ioport",
	"dma",
	"pcimem",
	NULL
};

#define RES_TYPES 5

typedef struct resource
{
	int     type;
	uint64_t start;
	uint64_t end;
} resource_t;


static int
parse_opts (char *str, resource_t res[])
{
	int     i;
	char   *p;
	char   *delims = { " =," };

	p = strtok (str, delims);
	for (i = 0; i < RES_TYPES; i++)
	{
		if (!strcmp (p, opts[i]))
		{
			res[i].type = i;
			p = strtok (NULL, delims);
			res[i].start = strtoul (p, NULL, NULL);
			p = strtok (NULL, delims);
			res[i].end = strtoul (p, NULL, NULL);
			return 1;
		}
	}
	return 0;
}



//
//  Private:    creates the rsrcdbmgr
//              returns 0 on success, or -1
//
static int
create_rsrcdbmgr (resource_t res)
{
	rsrc_alloc_t item;

	switch (res.type)
	{
	case RSRCDBMGR_MEMORY:
		break;

	case RSRCDBMGR_IRQ:
		break;

	case RSRCDBMGR_IO_PORT:
		break;

	case RSRCDBMGR_DMA_CHANNEL:
		memset (&item, 0, sizeof (item));
		item.start = res.start;
		item.end = res.end;
		item.flags = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_NOREMOVE;

		slogf (_SLOG_SETCODE (_SLOGC_PROC, 0), _SLOG_INFO, "Seeded DMA channels %lld to %lld",
			item.start, item.end);

		if (rsrcdbmgr_create (&item, 1) == -1)
		{
			slogf (_SLOG_SETCODE (_SLOGC_PROC, 1), _SLOG_ERROR,
				"rsrcdbmgr_create() failed @ %s:%d\n", __FILE__, __LINE__);
			return -1;
		}
		break;
	case RSRCDBMGR_PCI_MEMORY:
		break;
	default:
		break;
	}
	return 0;
}

/*-------------------------------------------------------------------------
	main
-------------------------------------------------------------------------*/
int
main (int argc, char *argv[])
{
	int     i;
	int     machine_unknown = 0;
	char    buf[BUFF_SIZE];
	resource_t res[RES_TYPES];

	memset (res, -1, sizeof (res));

	/* Now check board we are on */
	confstr (_CS_MACHINE, buf, BUFF_SIZE);
	if (strcmp (buf, "EDOSK7780") == 0 
		|| strcmp (buf, "SDK_7785") == 0
		|| strcmp (buf, "Wheat") == 0)
	{
		res[RSRCDBMGR_DMA_CHANNEL].type = RSRCDBMGR_DMA_CHANNEL;
		res[RSRCDBMGR_DMA_CHANNEL].start = 0;
		res[RSRCDBMGR_DMA_CHANNEL].end = 11;
	}
	else if (strcmp (buf, "Sequoia") == 0)
	{
		res[RSRCDBMGR_DMA_CHANNEL].type = RSRCDBMGR_DMA_CHANNEL;
		res[RSRCDBMGR_DMA_CHANNEL].start = 0;
		res[RSRCDBMGR_DMA_CHANNEL].end = 5;
	}
	else
		machine_unknown = 1;


	for (i = 1; i < argc; i++)
		parse_opts (argv[i], res);

	if (machine_unknown && argc == 1)
	{
		slogf (_SLOG_SETCODE (_SLOGC_PROC, 0), _SLOG_INFO,
			"Hardware not recognized, Failed to seed any resources\n");
		fprintf (stderr, "Hardware not recognized, Failed to seed any resources\n");
		return 1;
	}

	for (i = 0; i < RES_TYPES; i++)
	{
		if (res[i].type != -1)
			create_rsrcdbmgr (res[i]);
	}
	return 0;
}
