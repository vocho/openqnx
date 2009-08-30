#include <mips/asm.h>
#include <mips/cpu.h>

	.set mips2

#define SAVE_ONE(reg,bias)	mfc0 t0,$##reg; nop; sw t0,(reg*4)+bias(a0)

#define SAVE_ONE_4K(reg, bias)

#if defined(VARIANT_r3k)
#else
#endif


//
// Generate the routine for the R3K
//	
	
#define VARIANT_r3k
FRAME(r3k_save_extra,sp,0,ra)
#include "save_core.h"
ENDFRAME(r3k_save_extra)

FRAME(r3k_restore_extra,sp,0,ra)
#include "restore_core.h"
ENDFRAME(r3k_restore_extra)
	

//
// Generate the routine for MIPS32	
//	
#undef VARIANT_r3k
#undef SAVE_ONE_4K	
#define SAVE_ONE_4K(reg, bias)	SAVE_ONE(reg,bias)

#define VARIANT_32	
FRAME(mips32_save_extra,sp,0,ra)
#include "save_core.h"
ENDFRAME(mips32_save_extra)

// 
// Generate the routine for R4K/MIPS64	
//	
.set mips3
#undef VARIANT_32	
FRAME(mips64_save_extra,sp,0,ra)
#include "save_core.h"
ENDFRAME(mips64_save_extra)
FRAME(sb1_save_extra,sp,0,ra)
#define VARIANT_sb1
#include "save_core.h"
ENDFRAME(sb1_save_extra)

FRAME(mips64_restore_extra,sp,0,ra)
#include "restore_core.h"
ENDFRAME(mips64_restore_extra)
