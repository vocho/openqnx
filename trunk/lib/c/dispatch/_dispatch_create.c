#include <pthread.h>
#include <sys/dispatch.h>
#include "dispatch.h"

// This file is just to keep available an internal function that we 
// told people about.

dispatch_t *
_dispatch_create(int chid, unsigned flags) {
	return dispatch_create_channel(chid, flags);
}

__SRCVERSION("_dispatch_create.c $Rev: 152112 $");
