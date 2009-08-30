/* optglob.h */
/* Copyright 1995 by Steve Kirkendall */


/* This function declares global options, and macros for accessing them */

extern OPTVAL optglob[];
#ifdef FEATURE_LPR
extern OPTVAL lpval[];
#endif

#define o_blksize		optglob[0].value.number
#define o_blkhash		optglob[1].value.number
#define o_blkcache		optglob[2].value.number
#define o_blkgrow		optglob[3].value.number
#define o_blkfill		optglob[4].value.number
#define o_blkhit		optglob[5].value.number
#define o_blkmiss		optglob[6].value.number
#define o_blkwrite		optglob[7].value.number
#define o_version		optglob[8].value.string
#define o_bitsperchar		optglob[9].value.number
#define o_gui			optglob[10].value.string
#define o_os			optglob[11].value.string
#define o_session		optglob[12].value.string
#define o_recovering		optglob[13].value.boolean
#define o_digraph		optglob[14].value.boolean
#define o_exrc			optglob[15].value.boolean
#define o_modeline		optglob[16].value.boolean
#define o_modelines		optglob[17].value.number
#define o_ignorecase		optglob[18].value.boolean
#define o_magic			optglob[19].value.boolean
#define o_magicchar		optglob[20].value.string
#define o_magicname		optglob[21].value.boolean
#define o_magicperl		optglob[22].value.boolean
#define o_novice		optglob[23].value.boolean
#define o_prompt		optglob[24].value.boolean
#define o_remap			optglob[25].value.boolean
#define o_report		optglob[26].value.number
#define o_shell			optglob[27].value.string
#define o_sync			optglob[28].value.boolean
#define o_taglength		optglob[29].value.number
#define o_tagkind		optglob[30].value.boolean
#define o_taglibrary		optglob[31].value.boolean
#define o_tags			optglob[32].value.string
#define o_tagstack		optglob[33].value.boolean
#define o_tagprg		optglob[34].value.string
#define o_autoprint		optglob[35].value.boolean
#define o_autowrite		optglob[36].value.boolean
#define o_autoselect		optglob[37].value.boolean
#define o_warn			optglob[38].value.boolean
#define o_window		optglob[39].value.number
#define o_wrapscan		optglob[40].value.boolean
#define o_writeany		optglob[41].value.boolean
#define o_defaultreadonly	optglob[42].value.boolean
#define o_initialstate		optglob[43].value.character
#define o_exitcode		optglob[44].value.number
#define o_keytime		optglob[45].value.number
#define o_usertime		optglob[46].value.number
#define o_security		optglob[47].value.character
#define o_tempsession		optglob[48].value.boolean
#define o_newsession		optglob[49].value.boolean
#define o_exrefresh		optglob[50].value.boolean
#define o_home			optglob[51].value.string
#define o_elvispath		optglob[52].value.string
#define o_terse			optglob[53].value.boolean
#define o_previousdir		optglob[54].value.string
#define o_previousfile		optglob[55].value.string
#define o_previousfileline	optglob[56].value.number
#define o_previouscommand	optglob[57].value.string
#define o_previoustag		optglob[58].value.string
#define o_nearscroll		optglob[59].value.number
#define o_optimize		optglob[60].value.boolean
#define o_edcompatible		optglob[61].value.boolean
#define o_pollfrequency		optglob[62].value.number
#define o_sentenceend		optglob[63].value.string
#define o_sentencequote		optglob[64].value.string
#define o_sentencegap		optglob[65].value.number
#define o_verbose		optglob[66].value.number
#define o_anyerror		optglob[67].value.boolean
#define o_directory		optglob[68].value.string
#define o_errorbells		optglob[69].value.boolean
#define o_warningbells		optglob[70].value.boolean
#define o_flash			optglob[71].value.boolean
#define o_program		optglob[72].value.string
#define o_backup		optglob[73].value.boolean
#define o_showmarkups		optglob[74].value.boolean
#define o_nonascii		optglob[75].value.character
#define o_beautify		optglob[76].value.boolean
#define o_mesg			optglob[77].value.boolean
#define o_sessionpath		optglob[78].value.string
#define o_maptrace		optglob[79].value.character
#define o_maplog		optglob[80].value.character
#define o_gdefault		optglob[81].value.boolean
#define o_matchchar		optglob[82].value.string
#define o_show			optglob[83].value.string
#define o_writeeol		optglob[84].value.character
#define o_binary		optglob[85].value.boolean
#define o_saveregexp		optglob[86].value.boolean
#define o_true			optglob[87].value.string
#define o_false			optglob[88].value.string
#define o_animation		optglob[89].value.number
#define o_completebinary	optglob[90].value.boolean
#define o_optionwidth		optglob[91].value.number
#define o_smarttab		optglob[92].value.boolean
#define o_smartcase		optglob[93].value.boolean
#define o_hlsearch		optglob[94].value.boolean
#define o_background		optglob[95].value.character
#define o_incsearch		optglob[96].value.boolean
#define o_spelldict		optglob[97].value.string
#define o_spellautoload		optglob[98].value.boolean
#define o_spellsuffix		optglob[99].value.string
#define o_locale		optglob[100].value.string
#define o_mkexrcfile		optglob[101].value.string
#define o_prefersyntax		optglob[102].value.character
#define o_eventignore		optglob[103].value.string
#define o_eventerrors		optglob[104].value.boolean
#define o_tweaksection		optglob[105].value.boolean
#define o_timeout 		optglob[106].value.boolean
#define o_listchars		optglob[107].value.string
#define o_cleantext		optglob[108].value.string
#define o_filenamerules		optglob[109].value.string

/* For backward compatibility with older releases of elvis : */
#define o_more    		optglob[110].value.boolean
#define o_hardtabs		optglob[111].value.number
#define o_redraw		optglob[112].value.boolean
#define QTY_GLOBAL_OPTS			113

#ifdef FEATURE_LPR
# define o_lptype		lpval[0].value.string
# define o_lpcrlf		lpval[1].value.boolean
# define o_lpout		lpval[2].value.string
# define o_lpcolumns		lpval[3].value.number
# define o_lpwrap		lpval[4].value.boolean
# define o_lplines		lpval[5].value.number
# define o_lpconvert		lpval[6].value.boolean
# define o_lpformfeed		lpval[7].value.boolean
# define o_lpoptions		lpval[8].value.string
# define o_lpnumber		lpval[9].value.boolean
# define o_lpheader		lpval[10].value.boolean
# define o_lpcolor		lpval[11].value.boolean
# define o_lpcontrast		lpval[12].value.number
# define QTY_LP_OPTS		      13
#endif

BEGIN_EXTERNC
extern void optglobinit P_((void));
extern void optprevfile P_((CHAR *name, long line));
END_EXTERNC
