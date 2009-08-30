/* 
 These are the trap codes which are referenced
 from traps.S.
 The $ signs are important as they tell GAS/
 AS that the value is an immediate
*/
#define	TRP_ENTRY_CODE	$((SIGCODE_KERNEL+SIGTRAP)+(256*TRAP_TRACE))
//#define	TRP_ENTRY_CODE	$0x4000205
#define MSG_ENTRY_CODE	TRP_ENTRY_CODE
#define BRK_ENTRY_CODE	$((SIGCODE_KERNEL+SIGTRAP)+(256*TRAP_BRKPT))
//#define BRK_ENTRY_CODE	$0x4000105
#define DBG_ENTRY_CODE	TRP_ENTRY_CODE
#define EXC_ENTRY_CODE	TRP_ENTRY_CODE
