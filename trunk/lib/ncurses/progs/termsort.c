/*
 * termsort.c --- sort order arrays for use by infocmp.
 *
 * Note: this file is generated using termsort.sh, do not edit by hand.
 */
static const int bool_terminfo_sort[] = {
	 40 ,	/*  OTMT  */
	 41 ,	/*  OTNL  */
	 37 ,	/*  OTbs  */
	 39 ,	/*  OTnc  */
	 38 ,	/*  OTns  */
	 42 ,	/*  OTpt  */
	 43 ,	/*  OTxr  */
	 1 ,	/*  am  */
	 28 ,	/*  bce  */
	 0 ,	/*  bw  */
	 27 ,	/*  ccc  */
	 23 ,	/*  chts  */
	 35 ,	/*  cpix  */
	 31 ,	/*  crxm  */
	 11 ,	/*  da  */
	 32 ,	/*  daisy  */
	 12 ,	/*  db  */
	 5 ,	/*  eo  */
	 16 ,	/*  eslok  */
	 6 ,	/*  gn  */
	 7 ,	/*  hc  */
	 29 ,	/*  hls  */
	 9 ,	/*  hs  */
	 18 ,	/*  hz  */
	 10 ,	/*  in  */
	 8 ,	/*  km  */
	 36 ,	/*  lpix  */
	 22 ,	/*  mc5i  */
	 13 ,	/*  mir  */
	 14 ,	/*  msgr  */
	 26 ,	/*  ndscr  */
	 25 ,	/*  npc  */
	 24 ,	/*  nrrmc  */
	 21 ,	/*  nxon  */
	 15 ,	/*  os  */
	 34 ,	/*  sam  */
	 19 ,	/*  ul  */
	 4 ,	/*  xenl  */
	 3 ,	/*  xhp  */
	 30 ,	/*  xhpa  */
	 20 ,	/*  xon  */
	 2 ,	/*  xsb  */
	 17 ,	/*  xt  */
	 33 ,	/*  xvpa  */
};

static const int num_terminfo_sort[] = {
	 36 ,	/*  OTdB  */
	 34 ,	/*  OTdC  */
	 35 ,	/*  OTdN  */
	 37 ,	/*  OTdT  */
	 38 ,	/*  OTkn  */
	 33 ,	/*  OTug  */
	 31 ,	/*  bitwin  */
	 32 ,	/*  bitype  */
	 30 ,	/*  btns  */
	 16 ,	/*  bufsz  */
	 13 ,	/*  colors  */
	 0 ,	/*  cols  */
	 28 ,	/*  cps  */
	 1 ,	/*  it  */
	 9 ,	/*  lh  */
	 2 ,	/*  lines  */
	 3 ,	/*  lm  */
	 10 ,	/*  lw  */
	 11 ,	/*  ma  */
	 19 ,	/*  maddr  */
	 21 ,	/*  mcs  */
	 20 ,	/*  mjump  */
	 22 ,	/*  mls  */
	 15 ,	/*  ncv  */
	 8 ,	/*  nlab  */
	 23 ,	/*  npins  */
	 24 ,	/*  orc  */
	 26 ,	/*  orhi  */
	 25 ,	/*  orl  */
	 27 ,	/*  orvi  */
	 14 ,	/*  pairs  */
	 5 ,	/*  pb  */
	 18 ,	/*  spinh  */
	 17 ,	/*  spinv  */
	 6 ,	/*  vt  */
	 29 ,	/*  widcs  */
	 12 ,	/*  wnum  */
	 7 ,	/*  wsl  */
	 4 ,	/*  xmc  */
};

static const int str_terminfo_sort[] = {
	 400 ,	/*  OTG1  */
	 398 ,	/*  OTG2  */
	 399 ,	/*  OTG3  */
	 401 ,	/*  OTG4  */
	 408 ,	/*  OTGC  */
	 405 ,	/*  OTGD  */
	 406 ,	/*  OTGH  */
	 403 ,	/*  OTGL  */
	 402 ,	/*  OTGR  */
	 404 ,	/*  OTGU  */
	 407 ,	/*  OTGV  */
	 395 ,	/*  OTbc  */
	 392 ,	/*  OTi2  */
	 396 ,	/*  OTko  */
	 397 ,	/*  OTma  */
	 394 ,	/*  OTnl  */
	 393 ,	/*  OTrs  */
	 146 ,	/*  acsc  */
	 1 ,	/*  bel  */
	 372 ,	/*  bicr  */
	 371 ,	/*  binel  */
	 370 ,	/*  birep  */
	 26 ,	/*  blink  */
	 27 ,	/*  bold  */
	 411 ,	/*  box1  */
	 0 ,	/*  cbt  */
	 306 ,	/*  chr  */
	 13 ,	/*  civis  */
	 5 ,	/*  clear  */
	 9 ,	/*  cmdch  */
	 16 ,	/*  cnorm  */
	 373 ,	/*  colornm  */
	 304 ,	/*  cpi  */
	 2 ,	/*  cr  */
	 363 ,	/*  csin  */
	 354 ,	/*  csnm  */
	 3 ,	/*  csr  */
	 111 ,	/*  cub  */
	 14 ,	/*  cub1  */
	 107 ,	/*  cud  */
	 11 ,	/*  cud1  */
	 112 ,	/*  cuf  */
	 17 ,	/*  cuf1  */
	 10 ,	/*  cup  */
	 114 ,	/*  cuu  */
	 19 ,	/*  cuu1  */
	 307 ,	/*  cvr  */
	 20 ,	/*  cvvis  */
	 277 ,	/*  cwin  */
	 105 ,	/*  dch  */
	 21 ,	/*  dch1  */
	 275 ,	/*  dclk  */
	 374 ,	/*  defbi  */
	 308 ,	/*  defc  */
	 362 ,	/*  devt  */
	 280 ,	/*  dial  */
	 30 ,	/*  dim  */
	 378 ,	/*  dispc  */
	 106 ,	/*  dl  */
	 22 ,	/*  dl1  */
	 352 ,	/*  docr  */
	 23 ,	/*  dsl  */
	 37 ,	/*  ech  */
	 7 ,	/*  ed  */
	 386 ,	/*  ehhlm  */
	 6 ,	/*  el  */
	 269 ,	/*  el1  */
	 387 ,	/*  elhlm  */
	 388 ,	/*  elohlm  */
	 155 ,	/*  enacs  */
	 375 ,	/*  endbi  */
	 389 ,	/*  erhlm  */
	 390 ,	/*  ethlm  */
	 391 ,	/*  evhlm  */
	 46 ,	/*  ff  */
	 45 ,	/*  flash  */
	 273 ,	/*  fln  */
	 47 ,	/*  fsl  */
	 358 ,	/*  getm  */
	 24 ,	/*  hd  */
	 12 ,	/*  home  */
	 284 ,	/*  hook  */
	 8 ,	/*  hpa  */
	 134 ,	/*  ht  */
	 132 ,	/*  hts  */
	 137 ,	/*  hu  */
	 279 ,	/*  hup  */
	 108 ,	/*  ich  */
	 52 ,	/*  ich1  */
	 51 ,	/*  if  */
	 110 ,	/*  il  */
	 53 ,	/*  il1  */
	 129 ,	/*  ind  */
	 109 ,	/*  indn  */
	 299 ,	/*  initc  */
	 300 ,	/*  initp  */
	 32 ,	/*  invis  */
	 54 ,	/*  ip  */
	 138 ,	/*  iprog  */
	 48 ,	/*  is1  */
	 49 ,	/*  is2  */
	 50 ,	/*  is3  */
	 186 ,	/*  kBEG  */
	 187 ,	/*  kCAN  */
	 188 ,	/*  kCMD  */
	 189 ,	/*  kCPY  */
	 190 ,	/*  kCRT  */
	 191 ,	/*  kDC  */
	 192 ,	/*  kDL  */
	 194 ,	/*  kEND  */
	 195 ,	/*  kEOL  */
	 196 ,	/*  kEXT  */
	 197 ,	/*  kFND  */
	 198 ,	/*  kHLP  */
	 199 ,	/*  kHOM  */
	 200 ,	/*  kIC  */
	 201 ,	/*  kLFT  */
	 203 ,	/*  kMOV  */
	 202 ,	/*  kMSG  */
	 204 ,	/*  kNXT  */
	 205 ,	/*  kOPT  */
	 207 ,	/*  kPRT  */
	 206 ,	/*  kPRV  */
	 208 ,	/*  kRDO  */
	 211 ,	/*  kRES  */
	 210 ,	/*  kRIT  */
	 209 ,	/*  kRPL  */
	 212 ,	/*  kSAV  */
	 213 ,	/*  kSPD  */
	 214 ,	/*  kUND  */
	 139 ,	/*  ka1  */
	 140 ,	/*  ka3  */
	 141 ,	/*  kb2  */
	 158 ,	/*  kbeg  */
	 55 ,	/*  kbs  */
	 142 ,	/*  kc1  */
	 143 ,	/*  kc3  */
	 159 ,	/*  kcan  */
	 148 ,	/*  kcbt  */
	 160 ,	/*  kclo  */
	 57 ,	/*  kclr  */
	 161 ,	/*  kcmd  */
	 162 ,	/*  kcpy  */
	 163 ,	/*  kcrt  */
	 58 ,	/*  kctab  */
	 79 ,	/*  kcub1  */
	 61 ,	/*  kcud1  */
	 83 ,	/*  kcuf1  */
	 87 ,	/*  kcuu1  */
	 59 ,	/*  kdch1  */
	 60 ,	/*  kdl1  */
	 64 ,	/*  ked  */
	 63 ,	/*  kel  */
	 164 ,	/*  kend  */
	 165 ,	/*  kent  */
	 166 ,	/*  kext  */
	 65 ,	/*  kf0  */
	 66 ,	/*  kf1  */
	 67 ,	/*  kf10  */
	 216 ,	/*  kf11  */
	 217 ,	/*  kf12  */
	 218 ,	/*  kf13  */
	 219 ,	/*  kf14  */
	 220 ,	/*  kf15  */
	 221 ,	/*  kf16  */
	 222 ,	/*  kf17  */
	 223 ,	/*  kf18  */
	 224 ,	/*  kf19  */
	 68 ,	/*  kf2  */
	 225 ,	/*  kf20  */
	 226 ,	/*  kf21  */
	 227 ,	/*  kf22  */
	 228 ,	/*  kf23  */
	 229 ,	/*  kf24  */
	 230 ,	/*  kf25  */
	 231 ,	/*  kf26  */
	 232 ,	/*  kf27  */
	 233 ,	/*  kf28  */
	 234 ,	/*  kf29  */
	 69 ,	/*  kf3  */
	 235 ,	/*  kf30  */
	 236 ,	/*  kf31  */
	 237 ,	/*  kf32  */
	 238 ,	/*  kf33  */
	 239 ,	/*  kf34  */
	 240 ,	/*  kf35  */
	 241 ,	/*  kf36  */
	 242 ,	/*  kf37  */
	 243 ,	/*  kf38  */
	 244 ,	/*  kf39  */
	 70 ,	/*  kf4  */
	 245 ,	/*  kf40  */
	 246 ,	/*  kf41  */
	 247 ,	/*  kf42  */
	 248 ,	/*  kf43  */
	 249 ,	/*  kf44  */
	 250 ,	/*  kf45  */
	 251 ,	/*  kf46  */
	 252 ,	/*  kf47  */
	 253 ,	/*  kf48  */
	 254 ,	/*  kf49  */
	 71 ,	/*  kf5  */
	 255 ,	/*  kf50  */
	 256 ,	/*  kf51  */
	 257 ,	/*  kf52  */
	 258 ,	/*  kf53  */
	 259 ,	/*  kf54  */
	 260 ,	/*  kf55  */
	 261 ,	/*  kf56  */
	 262 ,	/*  kf57  */
	 263 ,	/*  kf58  */
	 264 ,	/*  kf59  */
	 72 ,	/*  kf6  */
	 265 ,	/*  kf60  */
	 266 ,	/*  kf61  */
	 267 ,	/*  kf62  */
	 268 ,	/*  kf63  */
	 73 ,	/*  kf7  */
	 74 ,	/*  kf8  */
	 75 ,	/*  kf9  */
	 167 ,	/*  kfnd  */
	 168 ,	/*  khlp  */
	 76 ,	/*  khome  */
	 86 ,	/*  khts  */
	 77 ,	/*  kich1  */
	 78 ,	/*  kil1  */
	 84 ,	/*  kind  */
	 80 ,	/*  kll  */
	 355 ,	/*  kmous  */
	 171 ,	/*  kmov  */
	 169 ,	/*  kmrk  */
	 170 ,	/*  kmsg  */
	 81 ,	/*  knp  */
	 172 ,	/*  knxt  */
	 173 ,	/*  kopn  */
	 174 ,	/*  kopt  */
	 82 ,	/*  kpp  */
	 176 ,	/*  kprt  */
	 175 ,	/*  kprv  */
	 177 ,	/*  krdo  */
	 178 ,	/*  kref  */
	 182 ,	/*  kres  */
	 179 ,	/*  krfr  */
	 85 ,	/*  kri  */
	 62 ,	/*  krmir  */
	 180 ,	/*  krpl  */
	 181 ,	/*  krst  */
	 183 ,	/*  ksav  */
	 193 ,	/*  kslt  */
	 184 ,	/*  kspd  */
	 56 ,	/*  ktbc  */
	 185 ,	/*  kund  */
	 90 ,	/*  lf0  */
	 91 ,	/*  lf1  */
	 92 ,	/*  lf10  */
	 93 ,	/*  lf2  */
	 94 ,	/*  lf3  */
	 95 ,	/*  lf4  */
	 96 ,	/*  lf5  */
	 97 ,	/*  lf6  */
	 98 ,	/*  lf7  */
	 99 ,	/*  lf8  */
	 100 ,	/*  lf9  */
	 18 ,	/*  ll  */
	 305 ,	/*  lpi  */
	 118 ,	/*  mc0  */
	 119 ,	/*  mc4  */
	 120 ,	/*  mc5  */
	 144 ,	/*  mc5p  */
	 336 ,	/*  mcub  */
	 330 ,	/*  mcub1  */
	 335 ,	/*  mcud  */
	 329 ,	/*  mcud1  */
	 337 ,	/*  mcuf  */
	 331 ,	/*  mcuf1  */
	 338 ,	/*  mcuu  */
	 333 ,	/*  mcuu1  */
	 409 ,	/*  meml  */
	 410 ,	/*  memu  */
	 270 ,	/*  mgc  */
	 328 ,	/*  mhpa  */
	 356 ,	/*  minfo  */
	 15 ,	/*  mrcup  */
	 332 ,	/*  mvpa  */
	 103 ,	/*  nel  */
	 298 ,	/*  oc  */
	 297 ,	/*  op  */
	 104 ,	/*  pad  */
	 285 ,	/*  pause  */
	 383 ,	/*  pctrm  */
	 115 ,	/*  pfkey  */
	 116 ,	/*  pfloc  */
	 117 ,	/*  pfx  */
	 361 ,	/*  pfxl  */
	 147 ,	/*  pln  */
	 334 ,	/*  porder  */
	 33 ,	/*  prot  */
	 283 ,	/*  pulse  */
	 281 ,	/*  qdial  */
	 348 ,	/*  rbim  */
	 126 ,	/*  rc  */
	 349 ,	/*  rcsd  */
	 121 ,	/*  rep  */
	 357 ,	/*  reqmp  */
	 34 ,	/*  rev  */
	 125 ,	/*  rf  */
	 215 ,	/*  rfi  */
	 130 ,	/*  ri  */
	 113 ,	/*  rin  */
	 321 ,	/*  ritm  */
	 322 ,	/*  rlm  */
	 38 ,	/*  rmacs  */
	 152 ,	/*  rmam  */
	 276 ,	/*  rmclk  */
	 40 ,	/*  rmcup  */
	 41 ,	/*  rmdc  */
	 323 ,	/*  rmicm  */
	 42 ,	/*  rmir  */
	 88 ,	/*  rmkx  */
	 157 ,	/*  rmln  */
	 101 ,	/*  rmm  */
	 145 ,	/*  rmp  */
	 380 ,	/*  rmpch  */
	 382 ,	/*  rmsc  */
	 43 ,	/*  rmso  */
	 44 ,	/*  rmul  */
	 150 ,	/*  rmxon  */
	 122 ,	/*  rs1  */
	 123 ,	/*  rs2  */
	 124 ,	/*  rs3  */
	 324 ,	/*  rshm  */
	 325 ,	/*  rsubm  */
	 326 ,	/*  rsupm  */
	 327 ,	/*  rum  */
	 320 ,	/*  rwidm  */
	 364 ,	/*  s0ds  */
	 365 ,	/*  s1ds  */
	 366 ,	/*  s2ds  */
	 367 ,	/*  s3ds  */
	 346 ,	/*  sbim  */
	 128 ,	/*  sc  */
	 385 ,	/*  scesa  */
	 384 ,	/*  scesc  */
	 274 ,	/*  sclk  */
	 301 ,	/*  scp  */
	 339 ,	/*  scs  */
	 347 ,	/*  scsd  */
	 310 ,	/*  sdrfq  */
	 360 ,	/*  setab  */
	 359 ,	/*  setaf  */
	 303 ,	/*  setb  */
	 376 ,	/*  setcolor  */
	 302 ,	/*  setf  */
	 131 ,	/*  sgr  */
	 39 ,	/*  sgr0  */
	 311 ,	/*  sitm  */
	 377 ,	/*  slines  */
	 312 ,	/*  slm  */
	 25 ,	/*  smacs  */
	 151 ,	/*  smam  */
	 28 ,	/*  smcup  */
	 29 ,	/*  smdc  */
	 340 ,	/*  smgb  */
	 341 ,	/*  smgbp  */
	 271 ,	/*  smgl  */
	 342 ,	/*  smglp  */
	 368 ,	/*  smglr  */
	 272 ,	/*  smgr  */
	 343 ,	/*  smgrp  */
	 344 ,	/*  smgt  */
	 369 ,	/*  smgtb  */
	 345 ,	/*  smgtp  */
	 313 ,	/*  smicm  */
	 31 ,	/*  smir  */
	 89 ,	/*  smkx  */
	 156 ,	/*  smln  */
	 102 ,	/*  smm  */
	 379 ,	/*  smpch  */
	 381 ,	/*  smsc  */
	 35 ,	/*  smso  */
	 36 ,	/*  smul  */
	 149 ,	/*  smxon  */
	 314 ,	/*  snlq  */
	 315 ,	/*  snrmq  */
	 316 ,	/*  sshm  */
	 317 ,	/*  ssubm  */
	 318 ,	/*  ssupm  */
	 350 ,	/*  subcs  */
	 319 ,	/*  sum  */
	 351 ,	/*  supcs  */
	 309 ,	/*  swidm  */
	 4 ,	/*  tbc  */
	 282 ,	/*  tone  */
	 135 ,	/*  tsl  */
	 287 ,	/*  u0  */
	 288 ,	/*  u1  */
	 289 ,	/*  u2  */
	 290 ,	/*  u3  */
	 291 ,	/*  u4  */
	 292 ,	/*  u5  */
	 293 ,	/*  u6  */
	 294 ,	/*  u7  */
	 295 ,	/*  u8  */
	 296 ,	/*  u9  */
	 136 ,	/*  uc  */
	 127 ,	/*  vpa  */
	 286 ,	/*  wait  */
	 133 ,	/*  wind  */
	 278 ,	/*  wingo  */
	 154 ,	/*  xoffc  */
	 153 ,	/*  xonc  */
	 353 ,	/*  zerom  */
};

static const int bool_variable_sort[] = {
	 0 ,	/*  auto_left_margin  */
	 1 ,	/*  auto_right_margin  */
	 28 ,	/*  back_color_erase  */
	 37 ,	/*  backspaces_with_bs  */
	 27 ,	/*  can_change  */
	 3 ,	/*  ceol_standout_glitch  */
	 30 ,	/*  col_addr_glitch  */
	 35 ,	/*  cpi_changes_res  */
	 31 ,	/*  cr_cancels_micro_mode  */
	 38 ,	/*  crt_no_scrolling  */
	 17 ,	/*  dest_tabs_magic_smso  */
	 4 ,	/*  eat_newline_glitch  */
	 5 ,	/*  erase_overstrike  */
	 6 ,	/*  generic_type  */
	 40 ,	/*  gnu_has_meta_key  */
	 7 ,	/*  hard_copy  */
	 23 ,	/*  hard_cursor  */
	 42 ,	/*  has_hardware_tabs  */
	 8 ,	/*  has_meta_key  */
	 32 ,	/*  has_print_wheel  */
	 9 ,	/*  has_status_line  */
	 29 ,	/*  hue_lightness_saturation  */
	 10 ,	/*  insert_null_glitch  */
	 41 ,	/*  linefeed_is_newline  */
	 36 ,	/*  lpi_changes_res  */
	 11 ,	/*  memory_above  */
	 12 ,	/*  memory_below  */
	 13 ,	/*  move_insert_mode  */
	 14 ,	/*  move_standout_mode  */
	 21 ,	/*  needs_xon_xoff  */
	 39 ,	/*  no_correctly_working_cr  */
	 2 ,	/*  no_esc_ctlc  */
	 25 ,	/*  no_pad_char  */
	 26 ,	/*  non_dest_scroll_region  */
	 24 ,	/*  non_rev_rmcup  */
	 15 ,	/*  over_strike  */
	 22 ,	/*  prtr_silent  */
	 43 ,	/*  return_does_clr_eol  */
	 33 ,	/*  row_addr_glitch  */
	 34 ,	/*  semi_auto_right_margin  */
	 16 ,	/*  status_line_esc_ok  */
	 18 ,	/*  tilde_glitch  */
	 19 ,	/*  transparent_underline  */
	 20 ,	/*  xon_xoff  */
};

static const int num_variable_sort[] = {
	 36 ,	/*  backspace_delay  */
	 31 ,	/*  bit_image_entwining  */
	 32 ,	/*  bit_image_type  */
	 16 ,	/*  buffer_capacity  */
	 30 ,	/*  buttons  */
	 34 ,	/*  carriage_return_delay  */
	 0 ,	/*  columns  */
	 18 ,	/*  dot_horz_spacing  */
	 17 ,	/*  dot_vert_spacing  */
	 37 ,	/*  horizontal_tab_delay  */
	 1 ,	/*  init_tabs  */
	 9 ,	/*  label_height  */
	 10 ,	/*  label_width  */
	 2 ,	/*  lines  */
	 3 ,	/*  lines_of_memory  */
	 4 ,	/*  magic_cookie_glitch  */
	 33 ,	/*  magic_cookie_glitch_ul  */
	 11 ,	/*  max_attributes  */
	 13 ,	/*  max_colors  */
	 19 ,	/*  max_micro_address  */
	 20 ,	/*  max_micro_jump  */
	 14 ,	/*  max_pairs  */
	 12 ,	/*  maximum_windows  */
	 21 ,	/*  micro_char_size  */
	 22 ,	/*  micro_line_size  */
	 35 ,	/*  new_line_delay  */
	 15 ,	/*  no_color_video  */
	 8 ,	/*  num_labels  */
	 38 ,	/*  number_of_function_keys  */
	 23 ,	/*  number_of_pins  */
	 24 ,	/*  output_res_char  */
	 26 ,	/*  output_res_horz_inch  */
	 25 ,	/*  output_res_line  */
	 27 ,	/*  output_res_vert_inch  */
	 5 ,	/*  padding_baud_rate  */
	 28 ,	/*  print_rate  */
	 6 ,	/*  virtual_terminal  */
	 29 ,	/*  wide_char_size  */
	 7 ,	/*  width_status_line  */
};

static const int str_variable_sort[] = {
	 404 ,	/*  acs_btee  */
	 146 ,	/*  acs_chars  */
	 406 ,	/*  acs_hline  */
	 399 ,	/*  acs_llcorner  */
	 401 ,	/*  acs_lrcorner  */
	 402 ,	/*  acs_ltee  */
	 408 ,	/*  acs_plus  */
	 403 ,	/*  acs_rtee  */
	 405 ,	/*  acs_ttee  */
	 398 ,	/*  acs_ulcorner  */
	 400 ,	/*  acs_urcorner  */
	 407 ,	/*  acs_vline  */
	 385 ,	/*  alt_scancode_esc  */
	 397 ,	/*  arrow_key_map  */
	 0 ,	/*  back_tab  */
	 395 ,	/*  backspace_if_not_bs  */
	 1 ,	/*  bell  */
	 372 ,	/*  bit_image_carriage_return  */
	 371 ,	/*  bit_image_newline  */
	 370 ,	/*  bit_image_repeat  */
	 411 ,	/*  box_chars_1  */
	 2 ,	/*  carriage_return  */
	 304 ,	/*  change_char_pitch  */
	 305 ,	/*  change_line_pitch  */
	 306 ,	/*  change_res_horz  */
	 307 ,	/*  change_res_vert  */
	 3 ,	/*  change_scroll_region  */
	 145 ,	/*  char_padding  */
	 354 ,	/*  char_set_names  */
	 4 ,	/*  clear_all_tabs  */
	 270 ,	/*  clear_margins  */
	 5 ,	/*  clear_screen  */
	 269 ,	/*  clr_bol  */
	 6 ,	/*  clr_eol  */
	 7 ,	/*  clr_eos  */
	 363 ,	/*  code_set_init  */
	 373 ,	/*  color_names  */
	 8 ,	/*  column_address  */
	 9 ,	/*  command_character  */
	 277 ,	/*  create_window  */
	 10 ,	/*  cursor_address  */
	 11 ,	/*  cursor_down  */
	 12 ,	/*  cursor_home  */
	 13 ,	/*  cursor_invisible  */
	 14 ,	/*  cursor_left  */
	 15 ,	/*  cursor_mem_address  */
	 16 ,	/*  cursor_normal  */
	 17 ,	/*  cursor_right  */
	 18 ,	/*  cursor_to_ll  */
	 19 ,	/*  cursor_up  */
	 20 ,	/*  cursor_visible  */
	 374 ,	/*  define_bit_image_region  */
	 308 ,	/*  define_char  */
	 21 ,	/*  delete_character  */
	 22 ,	/*  delete_line  */
	 362 ,	/*  device_type  */
	 280 ,	/*  dial_phone  */
	 23 ,	/*  dis_status_line  */
	 275 ,	/*  display_clock  */
	 378 ,	/*  display_pc_char  */
	 24 ,	/*  down_half_line  */
	 155 ,	/*  ena_acs  */
	 375 ,	/*  end_bit_image_region  */
	 25 ,	/*  enter_alt_charset_mode  */
	 151 ,	/*  enter_am_mode  */
	 26 ,	/*  enter_blink_mode  */
	 27 ,	/*  enter_bold_mode  */
	 28 ,	/*  enter_ca_mode  */
	 29 ,	/*  enter_delete_mode  */
	 30 ,	/*  enter_dim_mode  */
	 309 ,	/*  enter_doublewide_mode  */
	 310 ,	/*  enter_draft_quality  */
	 386 ,	/*  enter_horizontal_hl_mode  */
	 31 ,	/*  enter_insert_mode  */
	 311 ,	/*  enter_italics_mode  */
	 387 ,	/*  enter_left_hl_mode  */
	 312 ,	/*  enter_leftward_mode  */
	 388 ,	/*  enter_low_hl_mode  */
	 313 ,	/*  enter_micro_mode  */
	 314 ,	/*  enter_near_letter_quality  */
	 315 ,	/*  enter_normal_quality  */
	 379 ,	/*  enter_pc_charset_mode  */
	 33 ,	/*  enter_protected_mode  */
	 34 ,	/*  enter_reverse_mode  */
	 389 ,	/*  enter_right_hl_mode  */
	 381 ,	/*  enter_scancode_mode  */
	 32 ,	/*  enter_secure_mode  */
	 316 ,	/*  enter_shadow_mode  */
	 35 ,	/*  enter_standout_mode  */
	 317 ,	/*  enter_subscript_mode  */
	 318 ,	/*  enter_superscript_mode  */
	 390 ,	/*  enter_top_hl_mode  */
	 36 ,	/*  enter_underline_mode  */
	 319 ,	/*  enter_upward_mode  */
	 391 ,	/*  enter_vertical_hl_mode  */
	 149 ,	/*  enter_xon_mode  */
	 37 ,	/*  erase_chars  */
	 38 ,	/*  exit_alt_charset_mode  */
	 152 ,	/*  exit_am_mode  */
	 39 ,	/*  exit_attribute_mode  */
	 40 ,	/*  exit_ca_mode  */
	 41 ,	/*  exit_delete_mode  */
	 320 ,	/*  exit_doublewide_mode  */
	 42 ,	/*  exit_insert_mode  */
	 321 ,	/*  exit_italics_mode  */
	 322 ,	/*  exit_leftward_mode  */
	 323 ,	/*  exit_micro_mode  */
	 380 ,	/*  exit_pc_charset_mode  */
	 382 ,	/*  exit_scancode_mode  */
	 324 ,	/*  exit_shadow_mode  */
	 43 ,	/*  exit_standout_mode  */
	 325 ,	/*  exit_subscript_mode  */
	 326 ,	/*  exit_superscript_mode  */
	 44 ,	/*  exit_underline_mode  */
	 327 ,	/*  exit_upward_mode  */
	 150 ,	/*  exit_xon_mode  */
	 285 ,	/*  fixed_pause  */
	 284 ,	/*  flash_hook  */
	 45 ,	/*  flash_screen  */
	 46 ,	/*  form_feed  */
	 47 ,	/*  from_status_line  */
	 358 ,	/*  get_mouse  */
	 278 ,	/*  goto_window  */
	 279 ,	/*  hangup  */
	 48 ,	/*  init_1string  */
	 49 ,	/*  init_2string  */
	 50 ,	/*  init_3string  */
	 51 ,	/*  init_file  */
	 138 ,	/*  init_prog  */
	 299 ,	/*  initialize_color  */
	 300 ,	/*  initialize_pair  */
	 52 ,	/*  insert_character  */
	 53 ,	/*  insert_line  */
	 54 ,	/*  insert_padding  */
	 139 ,	/*  key_a1  */
	 140 ,	/*  key_a3  */
	 141 ,	/*  key_b2  */
	 55 ,	/*  key_backspace  */
	 158 ,	/*  key_beg  */
	 148 ,	/*  key_btab  */
	 142 ,	/*  key_c1  */
	 143 ,	/*  key_c3  */
	 159 ,	/*  key_cancel  */
	 56 ,	/*  key_catab  */
	 57 ,	/*  key_clear  */
	 160 ,	/*  key_close  */
	 161 ,	/*  key_command  */
	 162 ,	/*  key_copy  */
	 163 ,	/*  key_create  */
	 58 ,	/*  key_ctab  */
	 59 ,	/*  key_dc  */
	 60 ,	/*  key_dl  */
	 61 ,	/*  key_down  */
	 62 ,	/*  key_eic  */
	 164 ,	/*  key_end  */
	 165 ,	/*  key_enter  */
	 63 ,	/*  key_eol  */
	 64 ,	/*  key_eos  */
	 166 ,	/*  key_exit  */
	 65 ,	/*  key_f0  */
	 66 ,	/*  key_f1  */
	 67 ,	/*  key_f10  */
	 216 ,	/*  key_f11  */
	 217 ,	/*  key_f12  */
	 218 ,	/*  key_f13  */
	 219 ,	/*  key_f14  */
	 220 ,	/*  key_f15  */
	 221 ,	/*  key_f16  */
	 222 ,	/*  key_f17  */
	 223 ,	/*  key_f18  */
	 224 ,	/*  key_f19  */
	 68 ,	/*  key_f2  */
	 225 ,	/*  key_f20  */
	 226 ,	/*  key_f21  */
	 227 ,	/*  key_f22  */
	 228 ,	/*  key_f23  */
	 229 ,	/*  key_f24  */
	 230 ,	/*  key_f25  */
	 231 ,	/*  key_f26  */
	 232 ,	/*  key_f27  */
	 233 ,	/*  key_f28  */
	 234 ,	/*  key_f29  */
	 69 ,	/*  key_f3  */
	 235 ,	/*  key_f30  */
	 236 ,	/*  key_f31  */
	 237 ,	/*  key_f32  */
	 238 ,	/*  key_f33  */
	 239 ,	/*  key_f34  */
	 240 ,	/*  key_f35  */
	 241 ,	/*  key_f36  */
	 242 ,	/*  key_f37  */
	 243 ,	/*  key_f38  */
	 244 ,	/*  key_f39  */
	 70 ,	/*  key_f4  */
	 245 ,	/*  key_f40  */
	 246 ,	/*  key_f41  */
	 247 ,	/*  key_f42  */
	 248 ,	/*  key_f43  */
	 249 ,	/*  key_f44  */
	 250 ,	/*  key_f45  */
	 251 ,	/*  key_f46  */
	 252 ,	/*  key_f47  */
	 253 ,	/*  key_f48  */
	 254 ,	/*  key_f49  */
	 71 ,	/*  key_f5  */
	 255 ,	/*  key_f50  */
	 256 ,	/*  key_f51  */
	 257 ,	/*  key_f52  */
	 258 ,	/*  key_f53  */
	 259 ,	/*  key_f54  */
	 260 ,	/*  key_f55  */
	 261 ,	/*  key_f56  */
	 262 ,	/*  key_f57  */
	 263 ,	/*  key_f58  */
	 264 ,	/*  key_f59  */
	 72 ,	/*  key_f6  */
	 265 ,	/*  key_f60  */
	 266 ,	/*  key_f61  */
	 267 ,	/*  key_f62  */
	 268 ,	/*  key_f63  */
	 73 ,	/*  key_f7  */
	 74 ,	/*  key_f8  */
	 75 ,	/*  key_f9  */
	 167 ,	/*  key_find  */
	 168 ,	/*  key_help  */
	 76 ,	/*  key_home  */
	 77 ,	/*  key_ic  */
	 78 ,	/*  key_il  */
	 79 ,	/*  key_left  */
	 80 ,	/*  key_ll  */
	 169 ,	/*  key_mark  */
	 170 ,	/*  key_message  */
	 355 ,	/*  key_mouse  */
	 171 ,	/*  key_move  */
	 172 ,	/*  key_next  */
	 81 ,	/*  key_npage  */
	 173 ,	/*  key_open  */
	 174 ,	/*  key_options  */
	 82 ,	/*  key_ppage  */
	 175 ,	/*  key_previous  */
	 176 ,	/*  key_print  */
	 177 ,	/*  key_redo  */
	 178 ,	/*  key_reference  */
	 179 ,	/*  key_refresh  */
	 180 ,	/*  key_replace  */
	 181 ,	/*  key_restart  */
	 182 ,	/*  key_resume  */
	 83 ,	/*  key_right  */
	 183 ,	/*  key_save  */
	 186 ,	/*  key_sbeg  */
	 187 ,	/*  key_scancel  */
	 188 ,	/*  key_scommand  */
	 189 ,	/*  key_scopy  */
	 190 ,	/*  key_screate  */
	 191 ,	/*  key_sdc  */
	 192 ,	/*  key_sdl  */
	 193 ,	/*  key_select  */
	 194 ,	/*  key_send  */
	 195 ,	/*  key_seol  */
	 196 ,	/*  key_sexit  */
	 84 ,	/*  key_sf  */
	 197 ,	/*  key_sfind  */
	 198 ,	/*  key_shelp  */
	 199 ,	/*  key_shome  */
	 200 ,	/*  key_sic  */
	 201 ,	/*  key_sleft  */
	 202 ,	/*  key_smessage  */
	 203 ,	/*  key_smove  */
	 204 ,	/*  key_snext  */
	 205 ,	/*  key_soptions  */
	 206 ,	/*  key_sprevious  */
	 207 ,	/*  key_sprint  */
	 85 ,	/*  key_sr  */
	 208 ,	/*  key_sredo  */
	 209 ,	/*  key_sreplace  */
	 210 ,	/*  key_sright  */
	 211 ,	/*  key_srsume  */
	 212 ,	/*  key_ssave  */
	 213 ,	/*  key_ssuspend  */
	 86 ,	/*  key_stab  */
	 214 ,	/*  key_sundo  */
	 184 ,	/*  key_suspend  */
	 185 ,	/*  key_undo  */
	 87 ,	/*  key_up  */
	 88 ,	/*  keypad_local  */
	 89 ,	/*  keypad_xmit  */
	 90 ,	/*  lab_f0  */
	 91 ,	/*  lab_f1  */
	 92 ,	/*  lab_f10  */
	 93 ,	/*  lab_f2  */
	 94 ,	/*  lab_f3  */
	 95 ,	/*  lab_f4  */
	 96 ,	/*  lab_f5  */
	 97 ,	/*  lab_f6  */
	 98 ,	/*  lab_f7  */
	 99 ,	/*  lab_f8  */
	 100 ,	/*  lab_f9  */
	 273 ,	/*  label_format  */
	 157 ,	/*  label_off  */
	 156 ,	/*  label_on  */
	 394 ,	/*  linefeed_if_not_lf  */
	 409 ,	/*  memory_lock  */
	 410 ,	/*  memory_unlock  */
	 101 ,	/*  meta_off  */
	 102 ,	/*  meta_on  */
	 328 ,	/*  micro_column_address  */
	 329 ,	/*  micro_down  */
	 330 ,	/*  micro_left  */
	 331 ,	/*  micro_right  */
	 332 ,	/*  micro_row_address  */
	 333 ,	/*  micro_up  */
	 356 ,	/*  mouse_info  */
	 103 ,	/*  newline  */
	 334 ,	/*  order_of_pins  */
	 298 ,	/*  orig_colors  */
	 297 ,	/*  orig_pair  */
	 396 ,	/*  other_non_function_keys  */
	 104 ,	/*  pad_char  */
	 105 ,	/*  parm_dch  */
	 106 ,	/*  parm_delete_line  */
	 107 ,	/*  parm_down_cursor  */
	 335 ,	/*  parm_down_micro  */
	 108 ,	/*  parm_ich  */
	 109 ,	/*  parm_index  */
	 110 ,	/*  parm_insert_line  */
	 111 ,	/*  parm_left_cursor  */
	 336 ,	/*  parm_left_micro  */
	 112 ,	/*  parm_right_cursor  */
	 337 ,	/*  parm_right_micro  */
	 113 ,	/*  parm_rindex  */
	 114 ,	/*  parm_up_cursor  */
	 338 ,	/*  parm_up_micro  */
	 383 ,	/*  pc_term_options  */
	 115 ,	/*  pkey_key  */
	 116 ,	/*  pkey_local  */
	 361 ,	/*  pkey_plab  */
	 117 ,	/*  pkey_xmit  */
	 147 ,	/*  plab_norm  */
	 118 ,	/*  print_screen  */
	 144 ,	/*  prtr_non  */
	 119 ,	/*  prtr_off  */
	 120 ,	/*  prtr_on  */
	 283 ,	/*  pulse  */
	 281 ,	/*  quick_dial  */
	 276 ,	/*  remove_clock  */
	 121 ,	/*  repeat_char  */
	 215 ,	/*  req_for_input  */
	 357 ,	/*  req_mouse_pos  */
	 122 ,	/*  reset_1string  */
	 123 ,	/*  reset_2string  */
	 124 ,	/*  reset_3string  */
	 125 ,	/*  reset_file  */
	 126 ,	/*  restore_cursor  */
	 127 ,	/*  row_address  */
	 128 ,	/*  save_cursor  */
	 384 ,	/*  scancode_escape  */
	 129 ,	/*  scroll_forward  */
	 130 ,	/*  scroll_reverse  */
	 339 ,	/*  select_char_set  */
	 364 ,	/*  set0_des_seq  */
	 365 ,	/*  set1_des_seq  */
	 366 ,	/*  set2_des_seq  */
	 367 ,	/*  set3_des_seq  */
	 360 ,	/*  set_a_background  */
	 359 ,	/*  set_a_foreground  */
	 131 ,	/*  set_attributes  */
	 303 ,	/*  set_background  */
	 340 ,	/*  set_bottom_margin  */
	 341 ,	/*  set_bottom_margin_parm  */
	 274 ,	/*  set_clock  */
	 376 ,	/*  set_color_band  */
	 301 ,	/*  set_color_pair  */
	 302 ,	/*  set_foreground  */
	 271 ,	/*  set_left_margin  */
	 342 ,	/*  set_left_margin_parm  */
	 368 ,	/*  set_lr_margin  */
	 377 ,	/*  set_page_length  */
	 272 ,	/*  set_right_margin  */
	 343 ,	/*  set_right_margin_parm  */
	 132 ,	/*  set_tab  */
	 369 ,	/*  set_tb_margin  */
	 344 ,	/*  set_top_margin  */
	 345 ,	/*  set_top_margin_parm  */
	 133 ,	/*  set_window  */
	 346 ,	/*  start_bit_image  */
	 347 ,	/*  start_char_set_def  */
	 348 ,	/*  stop_bit_image  */
	 349 ,	/*  stop_char_set_def  */
	 350 ,	/*  subscript_characters  */
	 351 ,	/*  superscript_characters  */
	 134 ,	/*  tab  */
	 392 ,	/*  termcap_init2  */
	 393 ,	/*  termcap_reset  */
	 352 ,	/*  these_cause_cr  */
	 135 ,	/*  to_status_line  */
	 282 ,	/*  tone  */
	 136 ,	/*  underline_char  */
	 137 ,	/*  up_half_line  */
	 287 ,	/*  user0  */
	 288 ,	/*  user1  */
	 289 ,	/*  user2  */
	 290 ,	/*  user3  */
	 291 ,	/*  user4  */
	 292 ,	/*  user5  */
	 293 ,	/*  user6  */
	 294 ,	/*  user7  */
	 295 ,	/*  user8  */
	 296 ,	/*  user9  */
	 286 ,	/*  wait_tone  */
	 154 ,	/*  xoff_character  */
	 153 ,	/*  xon_character  */
	 353 ,	/*  zero_motion  */
};

static const int bool_termcap_sort[] = {
	 22 ,	/*  5i  */
	 23 ,	/*  HC  */
	 40 ,	/*  MT  */
	 26 ,	/*  ND  */
	 41 ,	/*  NL  */
	 25 ,	/*  NP  */
	 24 ,	/*  NR  */
	 30 ,	/*  YA  */
	 31 ,	/*  YB  */
	 32 ,	/*  YC  */
	 33 ,	/*  YD  */
	 34 ,	/*  YE  */
	 35 ,	/*  YF  */
	 36 ,	/*  YG  */
	 1 ,	/*  am  */
	 37 ,	/*  bs  */
	 0 ,	/*  bw  */
	 27 ,	/*  cc  */
	 11 ,	/*  da  */
	 12 ,	/*  db  */
	 5 ,	/*  eo  */
	 16 ,	/*  es  */
	 6 ,	/*  gn  */
	 7 ,	/*  hc  */
	 29 ,	/*  hl  */
	 9 ,	/*  hs  */
	 18 ,	/*  hz  */
	 10 ,	/*  in  */
	 8 ,	/*  km  */
	 13 ,	/*  mi  */
	 14 ,	/*  ms  */
	 39 ,	/*  nc  */
	 38 ,	/*  ns  */
	 21 ,	/*  nx  */
	 15 ,	/*  os  */
	 42 ,	/*  pt  */
	 19 ,	/*  ul  */
	 28 ,	/*  ut  */
	 2 ,	/*  xb  */
	 4 ,	/*  xn  */
	 20 ,	/*  xo  */
	 43 ,	/*  xr  */
	 3 ,	/*  xs  */
	 17 ,	/*  xt  */
};

static const int num_termcap_sort[] = {
	 30 ,	/*  BT  */
	 13 ,	/*  Co  */
	 12 ,	/*  MW  */
	 15 ,	/*  NC  */
	 8 ,	/*  Nl  */
	 16 ,	/*  Ya  */
	 17 ,	/*  Yb  */
	 18 ,	/*  Yc  */
	 19 ,	/*  Yd  */
	 20 ,	/*  Ye  */
	 21 ,	/*  Yf  */
	 22 ,	/*  Yg  */
	 23 ,	/*  Yh  */
	 24 ,	/*  Yi  */
	 25 ,	/*  Yj  */
	 26 ,	/*  Yk  */
	 27 ,	/*  Yl  */
	 28 ,	/*  Ym  */
	 29 ,	/*  Yn  */
	 31 ,	/*  Yo  */
	 32 ,	/*  Yp  */
	 0 ,	/*  co  */
	 36 ,	/*  dB  */
	 34 ,	/*  dC  */
	 35 ,	/*  dN  */
	 37 ,	/*  dT  */
	 1 ,	/*  it  */
	 38 ,	/*  kn  */
	 9 ,	/*  lh  */
	 2 ,	/*  li  */
	 3 ,	/*  lm  */
	 10 ,	/*  lw  */
	 11 ,	/*  ma  */
	 14 ,	/*  pa  */
	 5 ,	/*  pb  */
	 4 ,	/*  sg  */
	 33 ,	/*  ug  */
	 6 ,	/*  vt  */
	 7 ,	/*  ws  */
};

static const int str_termcap_sort[] = {
	 212 ,	/*  !1  */
	 213 ,	/*  !2  */
	 214 ,	/*  !3  */
	 198 ,	/*  #1  */
	 199 ,	/*  #2  */
	 200 ,	/*  #3  */
	 201 ,	/*  #4  */
	 177 ,	/*  %0  */
	 168 ,	/*  %1  */
	 169 ,	/*  %2  */
	 170 ,	/*  %3  */
	 171 ,	/*  %4  */
	 172 ,	/*  %5  */
	 173 ,	/*  %6  */
	 174 ,	/*  %7  */
	 175 ,	/*  %8  */
	 176 ,	/*  %9  */
	 202 ,	/*  %a  */
	 203 ,	/*  %b  */
	 204 ,	/*  %c  */
	 205 ,	/*  %d  */
	 206 ,	/*  %e  */
	 207 ,	/*  %f  */
	 208 ,	/*  %g  */
	 209 ,	/*  %h  */
	 210 ,	/*  %i  */
	 211 ,	/*  %j  */
	 187 ,	/*  &0  */
	 178 ,	/*  &1  */
	 179 ,	/*  &2  */
	 180 ,	/*  &3  */
	 181 ,	/*  &4  */
	 182 ,	/*  &5  */
	 183 ,	/*  &6  */
	 184 ,	/*  &7  */
	 185 ,	/*  &8  */
	 186 ,	/*  &9  */
	 197 ,	/*  *0  */
	 188 ,	/*  *1  */
	 189 ,	/*  *2  */
	 190 ,	/*  *3  */
	 191 ,	/*  *4  */
	 192 ,	/*  *5  */
	 193 ,	/*  *6  */
	 194 ,	/*  *7  */
	 195 ,	/*  *8  */
	 196 ,	/*  *9  */
	 167 ,	/*  @0  */
	 158 ,	/*  @1  */
	 159 ,	/*  @2  */
	 160 ,	/*  @3  */
	 161 ,	/*  @4  */
	 162 ,	/*  @5  */
	 163 ,	/*  @6  */
	 164 ,	/*  @7  */
	 165 ,	/*  @8  */
	 166 ,	/*  @9  */
	 360 ,	/*  AB  */
	 359 ,	/*  AF  */
	 110 ,	/*  AL  */
	 9 ,	/*  CC  */
	 15 ,	/*  CM  */
	 277 ,	/*  CW  */
	 105 ,	/*  DC  */
	 280 ,	/*  DI  */
	 275 ,	/*  DK  */
	 106 ,	/*  DL  */
	 107 ,	/*  DO  */
	 216 ,	/*  F1  */
	 217 ,	/*  F2  */
	 218 ,	/*  F3  */
	 219 ,	/*  F4  */
	 220 ,	/*  F5  */
	 221 ,	/*  F6  */
	 222 ,	/*  F7  */
	 223 ,	/*  F8  */
	 224 ,	/*  F9  */
	 225 ,	/*  FA  */
	 226 ,	/*  FB  */
	 227 ,	/*  FC  */
	 228 ,	/*  FD  */
	 229 ,	/*  FE  */
	 230 ,	/*  FF  */
	 231 ,	/*  FG  */
	 232 ,	/*  FH  */
	 233 ,	/*  FI  */
	 234 ,	/*  FJ  */
	 235 ,	/*  FK  */
	 236 ,	/*  FL  */
	 237 ,	/*  FM  */
	 238 ,	/*  FN  */
	 239 ,	/*  FO  */
	 240 ,	/*  FP  */
	 241 ,	/*  FQ  */
	 242 ,	/*  FR  */
	 243 ,	/*  FS  */
	 244 ,	/*  FT  */
	 245 ,	/*  FU  */
	 246 ,	/*  FV  */
	 247 ,	/*  FW  */
	 248 ,	/*  FX  */
	 249 ,	/*  FY  */
	 250 ,	/*  FZ  */
	 251 ,	/*  Fa  */
	 252 ,	/*  Fb  */
	 253 ,	/*  Fc  */
	 254 ,	/*  Fd  */
	 255 ,	/*  Fe  */
	 256 ,	/*  Ff  */
	 257 ,	/*  Fg  */
	 258 ,	/*  Fh  */
	 259 ,	/*  Fi  */
	 260 ,	/*  Fj  */
	 261 ,	/*  Fk  */
	 262 ,	/*  Fl  */
	 263 ,	/*  Fm  */
	 264 ,	/*  Fn  */
	 265 ,	/*  Fo  */
	 266 ,	/*  Fp  */
	 267 ,	/*  Fq  */
	 268 ,	/*  Fr  */
	 400 ,	/*  G1  */
	 398 ,	/*  G2  */
	 399 ,	/*  G3  */
	 401 ,	/*  G4  */
	 408 ,	/*  GC  */
	 405 ,	/*  GD  */
	 406 ,	/*  GH  */
	 403 ,	/*  GL  */
	 402 ,	/*  GR  */
	 404 ,	/*  GU  */
	 407 ,	/*  GV  */
	 358 ,	/*  Gm  */
	 279 ,	/*  HU  */
	 108 ,	/*  IC  */
	 299 ,	/*  Ic  */
	 300 ,	/*  Ip  */
	 139 ,	/*  K1  */
	 141 ,	/*  K2  */
	 140 ,	/*  K3  */
	 142 ,	/*  K4  */
	 143 ,	/*  K5  */
	 355 ,	/*  Km  */
	 111 ,	/*  LE  */
	 157 ,	/*  LF  */
	 156 ,	/*  LO  */
	 273 ,	/*  Lf  */
	 270 ,	/*  MC  */
	 271 ,	/*  ML  */
	 368 ,	/*  ML  */
	 272 ,	/*  MR  */
	 369 ,	/*  MT  */
	 356 ,	/*  Mi  */
	 285 ,	/*  PA  */
	 283 ,	/*  PU  */
	 281 ,	/*  QD  */
	 152 ,	/*  RA  */
	 276 ,	/*  RC  */
	 215 ,	/*  RF  */
	 112 ,	/*  RI  */
	 357 ,	/*  RQ  */
	 150 ,	/*  RX  */
	 378 ,	/*  S1  */
	 379 ,	/*  S2  */
	 380 ,	/*  S3  */
	 381 ,	/*  S4  */
	 382 ,	/*  S5  */
	 383 ,	/*  S6  */
	 384 ,	/*  S7  */
	 385 ,	/*  S8  */
	 151 ,	/*  SA  */
	 274 ,	/*  SC  */
	 109 ,	/*  SF  */
	 113 ,	/*  SR  */
	 149 ,	/*  SX  */
	 303 ,	/*  Sb  */
	 302 ,	/*  Sf  */
	 282 ,	/*  TO  */
	 114 ,	/*  UP  */
	 286 ,	/*  WA  */
	 278 ,	/*  WG  */
	 154 ,	/*  XF  */
	 153 ,	/*  XN  */
	 386 ,	/*  Xh  */
	 387 ,	/*  Xl  */
	 388 ,	/*  Xo  */
	 389 ,	/*  Xr  */
	 390 ,	/*  Xt  */
	 391 ,	/*  Xv  */
	 370 ,	/*  Xy  */
	 377 ,	/*  YZ  */
	 372 ,	/*  Yv  */
	 373 ,	/*  Yw  */
	 374 ,	/*  Yx  */
	 375 ,	/*  Yy  */
	 376 ,	/*  Yz  */
	 304 ,	/*  ZA  */
	 305 ,	/*  ZB  */
	 306 ,	/*  ZC  */
	 307 ,	/*  ZD  */
	 308 ,	/*  ZE  */
	 309 ,	/*  ZF  */
	 310 ,	/*  ZG  */
	 311 ,	/*  ZH  */
	 312 ,	/*  ZI  */
	 313 ,	/*  ZJ  */
	 314 ,	/*  ZK  */
	 315 ,	/*  ZL  */
	 316 ,	/*  ZM  */
	 317 ,	/*  ZN  */
	 318 ,	/*  ZO  */
	 319 ,	/*  ZP  */
	 320 ,	/*  ZQ  */
	 321 ,	/*  ZR  */
	 322 ,	/*  ZS  */
	 323 ,	/*  ZT  */
	 324 ,	/*  ZU  */
	 325 ,	/*  ZV  */
	 326 ,	/*  ZW  */
	 327 ,	/*  ZX  */
	 328 ,	/*  ZY  */
	 329 ,	/*  ZZ  */
	 330 ,	/*  Za  */
	 331 ,	/*  Zb  */
	 332 ,	/*  Zc  */
	 333 ,	/*  Zd  */
	 334 ,	/*  Ze  */
	 335 ,	/*  Zf  */
	 336 ,	/*  Zg  */
	 337 ,	/*  Zh  */
	 338 ,	/*  Zi  */
	 339 ,	/*  Zj  */
	 340 ,	/*  Zk  */
	 341 ,	/*  Zl  */
	 342 ,	/*  Zm  */
	 343 ,	/*  Zn  */
	 344 ,	/*  Zo  */
	 345 ,	/*  Zp  */
	 346 ,	/*  Zq  */
	 347 ,	/*  Zr  */
	 348 ,	/*  Zs  */
	 349 ,	/*  Zt  */
	 350 ,	/*  Zu  */
	 351 ,	/*  Zv  */
	 352 ,	/*  Zw  */
	 353 ,	/*  Zx  */
	 354 ,	/*  Zy  */
	 371 ,	/*  Zz  */
	 146 ,	/*  ac  */
	 38 ,	/*  ae  */
	 53 ,	/*  al  */
	 25 ,	/*  as  */
	 395 ,	/*  bc  */
	 1 ,	/*  bl  */
	 0 ,	/*  bt  */
	 411 ,	/*  bx  */
	 269 ,	/*  cb  */
	 7 ,	/*  cd  */
	 6 ,	/*  ce  */
	 8 ,	/*  ch  */
	 363 ,	/*  ci  */
	 5 ,	/*  cl  */
	 10 ,	/*  cm  */
	 2 ,	/*  cr  */
	 3 ,	/*  cs  */
	 4 ,	/*  ct  */
	 127 ,	/*  cv  */
	 21 ,	/*  dc  */
	 22 ,	/*  dl  */
	 29 ,	/*  dm  */
	 11 ,	/*  do  */
	 23 ,	/*  ds  */
	 362 ,	/*  dv  */
	 155 ,	/*  eA  */
	 37 ,	/*  ec  */
	 41 ,	/*  ed  */
	 42 ,	/*  ei  */
	 46 ,	/*  ff  */
	 284 ,	/*  fh  */
	 47 ,	/*  fs  */
	 24 ,	/*  hd  */
	 12 ,	/*  ho  */
	 137 ,	/*  hu  */
	 48 ,	/*  i1  */
	 392 ,	/*  i2  */
	 50 ,	/*  i3  */
	 138 ,	/*  iP  */
	 52 ,	/*  ic  */
	 51 ,	/*  if  */
	 31 ,	/*  im  */
	 54 ,	/*  ip  */
	 49 ,	/*  is  */
	 65 ,	/*  k0  */
	 66 ,	/*  k1  */
	 68 ,	/*  k2  */
	 69 ,	/*  k3  */
	 70 ,	/*  k4  */
	 71 ,	/*  k5  */
	 72 ,	/*  k6  */
	 73 ,	/*  k7  */
	 74 ,	/*  k8  */
	 75 ,	/*  k9  */
	 67 ,	/*  k;  */
	 78 ,	/*  kA  */
	 148 ,	/*  kB  */
	 57 ,	/*  kC  */
	 59 ,	/*  kD  */
	 63 ,	/*  kE  */
	 84 ,	/*  kF  */
	 80 ,	/*  kH  */
	 77 ,	/*  kI  */
	 60 ,	/*  kL  */
	 62 ,	/*  kM  */
	 81 ,	/*  kN  */
	 82 ,	/*  kP  */
	 85 ,	/*  kR  */
	 64 ,	/*  kS  */
	 86 ,	/*  kT  */
	 56 ,	/*  ka  */
	 55 ,	/*  kb  */
	 61 ,	/*  kd  */
	 88 ,	/*  ke  */
	 76 ,	/*  kh  */
	 79 ,	/*  kl  */
	 396 ,	/*  ko  */
	 83 ,	/*  kr  */
	 89 ,	/*  ks  */
	 58 ,	/*  kt  */
	 87 ,	/*  ku  */
	 90 ,	/*  l0  */
	 91 ,	/*  l1  */
	 93 ,	/*  l2  */
	 94 ,	/*  l3  */
	 95 ,	/*  l4  */
	 96 ,	/*  l5  */
	 97 ,	/*  l6  */
	 98 ,	/*  l7  */
	 99 ,	/*  l8  */
	 100 ,	/*  l9  */
	 92 ,	/*  la  */
	 14 ,	/*  le  */
	 18 ,	/*  ll  */
	 397 ,	/*  ma  */
	 26 ,	/*  mb  */
	 27 ,	/*  md  */
	 39 ,	/*  me  */
	 30 ,	/*  mh  */
	 32 ,	/*  mk  */
	 409 ,	/*  ml  */
	 102 ,	/*  mm  */
	 101 ,	/*  mo  */
	 33 ,	/*  mp  */
	 34 ,	/*  mr  */
	 410 ,	/*  mu  */
	 17 ,	/*  nd  */
	 394 ,	/*  nl  */
	 103 ,	/*  nw  */
	 298 ,	/*  oc  */
	 297 ,	/*  op  */
	 144 ,	/*  pO  */
	 104 ,	/*  pc  */
	 119 ,	/*  pf  */
	 115 ,	/*  pk  */
	 116 ,	/*  pl  */
	 147 ,	/*  pn  */
	 120 ,	/*  po  */
	 118 ,	/*  ps  */
	 117 ,	/*  px  */
	 122 ,	/*  r1  */
	 123 ,	/*  r2  */
	 124 ,	/*  r3  */
	 145 ,	/*  rP  */
	 126 ,	/*  rc  */
	 125 ,	/*  rf  */
	 121 ,	/*  rp  */
	 393 ,	/*  rs  */
	 364 ,	/*  s0  */
	 365 ,	/*  s1  */
	 366 ,	/*  s2  */
	 367 ,	/*  s3  */
	 131 ,	/*  sa  */
	 128 ,	/*  sc  */
	 43 ,	/*  se  */
	 129 ,	/*  sf  */
	 35 ,	/*  so  */
	 301 ,	/*  sp  */
	 130 ,	/*  sr  */
	 132 ,	/*  st  */
	 134 ,	/*  ta  */
	 40 ,	/*  te  */
	 28 ,	/*  ti  */
	 135 ,	/*  ts  */
	 287 ,	/*  u0  */
	 288 ,	/*  u1  */
	 289 ,	/*  u2  */
	 290 ,	/*  u3  */
	 291 ,	/*  u4  */
	 292 ,	/*  u5  */
	 293 ,	/*  u6  */
	 294 ,	/*  u7  */
	 295 ,	/*  u8  */
	 296 ,	/*  u9  */
	 136 ,	/*  uc  */
	 44 ,	/*  ue  */
	 19 ,	/*  up  */
	 36 ,	/*  us  */
	 45 ,	/*  vb  */
	 16 ,	/*  ve  */
	 13 ,	/*  vi  */
	 20 ,	/*  vs  */
	 133 ,	/*  wi  */
	 361 ,	/*  xl  */
};

static const int bool_from_termcap[] = {
1,	/*  bw  */
1,	/*  am  */
1,	/*  xsb  */
1,	/*  xhp  */
1,	/*  xenl  */
1,	/*  eo  */
1,	/*  gn  */
1,	/*  hc  */
1,	/*  km  */
1,	/*  hs  */
1,	/*  in  */
1,	/*  da  */
1,	/*  db  */
1,	/*  mir  */
1,	/*  msgr  */
1,	/*  os  */
1,	/*  eslok  */
1,	/*  xt  */
1,	/*  hz  */
1,	/*  ul  */
1,	/*  xon  */
0,	/*  nxon  */
0,	/*  mc5i  */
0,	/*  chts  */
0,	/*  nrrmc  */
0,	/*  npc  */
0,	/*  ndscr  */
0,	/*  ccc  */
0,	/*  bce  */
0,	/*  hls  */
0,	/*  xhpa  */
0,	/*  crxm  */
0,	/*  daisy  */
0,	/*  xvpa  */
0,	/*  sam  */
0,	/*  cpix  */
0,	/*  lpix  */
1,	/*  OTbs  */
1,	/*  OTns  */
1,	/*  OTnc  */
0,	/*  OTMT  */
1,	/*  OTNL  */
1,	/*  OTpt  */
1,	/*  OTxr  */
};

static const int num_from_termcap[] = {
1,	/*  cols  */
1,	/*  it  */
1,	/*  lines  */
1,	/*  lm  */
1,	/*  xmc  */
1,	/*  pb  */
1,	/*  vt  */
1,	/*  wsl  */
0,	/*  nlab  */
0,	/*  lh  */
0,	/*  lw  */
1,	/*  ma  */
0,	/*  wnum  */
0,	/*  colors  */
0,	/*  pairs  */
0,	/*  ncv  */
0,	/*  bufsz  */
0,	/*  spinv  */
0,	/*  spinh  */
0,	/*  maddr  */
0,	/*  mjump  */
0,	/*  mcs  */
0,	/*  mls  */
0,	/*  npins  */
0,	/*  orc  */
0,	/*  orl  */
0,	/*  orhi  */
0,	/*  orvi  */
0,	/*  cps  */
0,	/*  widcs  */
0,	/*  btns  */
0,	/*  bitwin  */
0,	/*  bitype  */
1,	/*  OTug  */
1,	/*  OTdC  */
1,	/*  OTdN  */
1,	/*  OTdB  */
1,	/*  OTdT  */
0,	/*  OTkn  */
};

static const int str_from_termcap[] = {
1,	/*  cbt  */
1,	/*  bel  */
1,	/*  cr  */
1,	/*  csr  */
1,	/*  tbc  */
1,	/*  clear  */
1,	/*  el  */
1,	/*  ed  */
0,	/*  hpa  */
1,	/*  cmdch  */
1,	/*  cup  */
1,	/*  cud1  */
1,	/*  home  */
1,	/*  civis  */
1,	/*  cub1  */
1,	/*  mrcup  */
1,	/*  cnorm  */
1,	/*  cuf1  */
1,	/*  ll  */
1,	/*  cuu1  */
1,	/*  cvvis  */
1,	/*  dch1  */
1,	/*  dl1  */
1,	/*  dsl  */
1,	/*  hd  */
1,	/*  smacs  */
1,	/*  blink  */
1,	/*  bold  */
1,	/*  smcup  */
1,	/*  smdc  */
1,	/*  dim  */
1,	/*  smir  */
0,	/*  invis  */
0,	/*  prot  */
1,	/*  rev  */
1,	/*  smso  */
1,	/*  smul  */
1,	/*  ech  */
1,	/*  rmacs  */
1,	/*  sgr0  */
1,	/*  rmcup  */
1,	/*  rmdc  */
1,	/*  rmir  */
1,	/*  rmso  */
1,	/*  rmul  */
1,	/*  flash  */
1,	/*  ff  */
1,	/*  fsl  */
1,	/*  is1  */
1,	/*  is2  */
1,	/*  is3  */
1,	/*  if  */
1,	/*  ich1  */
1,	/*  il1  */
1,	/*  ip  */
1,	/*  kbs  */
0,	/*  ktbc  */
0,	/*  kclr  */
0,	/*  kctab  */
1,	/*  kdch1  */
0,	/*  kdl1  */
1,	/*  kcud1  */
0,	/*  krmir  */
0,	/*  kel  */
0,	/*  ked  */
1,	/*  kf0  */
1,	/*  kf1  */
0,	/*  kf10  */
1,	/*  kf2  */
1,	/*  kf3  */
1,	/*  kf4  */
1,	/*  kf5  */
1,	/*  kf6  */
1,	/*  kf7  */
1,	/*  kf8  */
1,	/*  kf9  */
1,	/*  khome  */
1,	/*  kich1  */
0,	/*  kil1  */
1,	/*  kcub1  */
1,	/*  kll  */
1,	/*  knp  */
1,	/*  kpp  */
1,	/*  kcuf1  */
0,	/*  kind  */
0,	/*  kri  */
0,	/*  khts  */
1,	/*  kcuu1  */
1,	/*  rmkx  */
1,	/*  smkx  */
0,	/*  lf0  */
0,	/*  lf1  */
0,	/*  lf10  */
0,	/*  lf2  */
0,	/*  lf3  */
0,	/*  lf4  */
0,	/*  lf5  */
0,	/*  lf6  */
0,	/*  lf7  */
0,	/*  lf8  */
0,	/*  lf9  */
1,	/*  rmm  */
1,	/*  smm  */
1,	/*  nel  */
1,	/*  pad  */
1,	/*  dch  */
1,	/*  dl  */
1,	/*  cud  */
1,	/*  ich  */
1,	/*  indn  */
1,	/*  il  */
1,	/*  cub  */
1,	/*  cuf  */
1,	/*  rin  */
1,	/*  cuu  */
0,	/*  pfkey  */
0,	/*  pfloc  */
0,	/*  pfx  */
0,	/*  mc0  */
0,	/*  mc4  */
0,	/*  mc5  */
1,	/*  rep  */
0,	/*  rs1  */
0,	/*  rs2  */
0,	/*  rs3  */
0,	/*  rf  */
1,	/*  rc  */
0,	/*  vpa  */
1,	/*  sc  */
1,	/*  ind  */
1,	/*  ri  */
1,	/*  sgr  */
1,	/*  hts  */
0,	/*  wind  */
1,	/*  ht  */
1,	/*  tsl  */
1,	/*  uc  */
1,	/*  hu  */
0,	/*  iprog  */
1,	/*  ka1  */
1,	/*  ka3  */
1,	/*  kb2  */
1,	/*  kc1  */
1,	/*  kc3  */
0,	/*  mc5p  */
0,	/*  rmp  */
0,	/*  acsc  */
0,	/*  pln  */
0,	/*  kcbt  */
0,	/*  smxon  */
0,	/*  rmxon  */
0,	/*  smam  */
0,	/*  rmam  */
0,	/*  xonc  */
0,	/*  xoffc  */
0,	/*  enacs  */
0,	/*  smln  */
0,	/*  rmln  */
0,	/*  kbeg  */
0,	/*  kcan  */
0,	/*  kclo  */
0,	/*  kcmd  */
0,	/*  kcpy  */
0,	/*  kcrt  */
0,	/*  kend  */
0,	/*  kent  */
0,	/*  kext  */
0,	/*  kfnd  */
0,	/*  khlp  */
0,	/*  kmrk  */
0,	/*  kmsg  */
0,	/*  kmov  */
0,	/*  knxt  */
0,	/*  kopn  */
0,	/*  kopt  */
0,	/*  kprv  */
0,	/*  kprt  */
0,	/*  krdo  */
0,	/*  kref  */
0,	/*  krfr  */
0,	/*  krpl  */
0,	/*  krst  */
0,	/*  kres  */
0,	/*  ksav  */
0,	/*  kspd  */
0,	/*  kund  */
0,	/*  kBEG  */
0,	/*  kCAN  */
0,	/*  kCMD  */
0,	/*  kCPY  */
0,	/*  kCRT  */
0,	/*  kDC  */
0,	/*  kDL  */
0,	/*  kslt  */
0,	/*  kEND  */
0,	/*  kEOL  */
0,	/*  kEXT  */
0,	/*  kFND  */
0,	/*  kHLP  */
0,	/*  kHOM  */
0,	/*  kIC  */
0,	/*  kLFT  */
0,	/*  kMSG  */
0,	/*  kMOV  */
0,	/*  kNXT  */
0,	/*  kOPT  */
0,	/*  kPRV  */
0,	/*  kPRT  */
0,	/*  kRDO  */
0,	/*  kRPL  */
0,	/*  kRIT  */
0,	/*  kRES  */
0,	/*  kSAV  */
0,	/*  kSPD  */
0,	/*  kUND  */
0,	/*  rfi  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf27  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf32  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  el1  */
0,	/*  mgc  */
0,	/*  smgl  */
0,	/*  smgr  */
0,	/*  fln  */
0,	/*  sclk  */
0,	/*  dclk  */
0,	/*  rmclk  */
0,	/*  cwin  */
0,	/*  wingo  */
0,	/*  hup  */
0,	/*  dial  */
0,	/*  qdial  */
0,	/*  tone  */
0,	/*  pulse  */
0,	/*  hook  */
0,	/*  pause  */
0,	/*  wait  */
0,	/*  u0  */
0,	/*  u1  */
0,	/*  u2  */
0,	/*  u3  */
0,	/*  u4  */
0,	/*  u5  */
0,	/*  u6  */
0,	/*  u7  */
0,	/*  u8  */
0,	/*  u9  */
0,	/*  op  */
0,	/*  oc  */
0,	/*  initc  */
0,	/*  initp  */
0,	/*  scp  */
0,	/*  setf  */
0,	/*  setb  */
0,	/*  cpi  */
0,	/*  lpi  */
0,	/*  chr  */
0,	/*  cvr  */
0,	/*  defc  */
0,	/*  swidm  */
0,	/*  sdrfq  */
0,	/*  sitm  */
0,	/*  slm  */
0,	/*  smicm  */
0,	/*  snlq  */
0,	/*  snrmq  */
0,	/*  sshm  */
0,	/*  ssubm  */
0,	/*  ssupm  */
0,	/*  sum  */
0,	/*  rwidm  */
0,	/*  ritm  */
0,	/*  rlm  */
0,	/*  rmicm  */
0,	/*  rshm  */
0,	/*  rsubm  */
0,	/*  rsupm  */
0,	/*  rum  */
0,	/*  mhpa  */
0,	/*  mcud1  */
0,	/*  mcub1  */
0,	/*  mcuf1  */
0,	/*  mvpa  */
0,	/*  mcuu1  */
0,	/*  porder  */
0,	/*  mcud  */
0,	/*  mcub  */
0,	/*  mcuf  */
0,	/*  mcuu  */
0,	/*  scs  */
0,	/*  smgb  */
0,	/*  smgbp  */
0,	/*  smglp  */
0,	/*  smgrp  */
0,	/*  smgt  */
0,	/*  smgtp  */
0,	/*  sbim  */
0,	/*  scsd  */
0,	/*  rbim  */
0,	/*  rcsd  */
0,	/*  subcs  */
0,	/*  supcs  */
0,	/*  docr  */
0,	/*  zerom  */
0,	/*  csnm  */
0,	/*  kmous  */
0,	/*  minfo  */
0,	/*  reqmp  */
0,	/*  getm  */
0,	/*  setaf  */
0,	/*  setab  */
0,	/*  pfxl  */
0,	/*  devt  */
0,	/*  csin  */
0,	/*  s0ds  */
0,	/*  s1ds  */
0,	/*  s2ds  */
0,	/*  s3ds  */
0,	/*  smglr  */
0,	/*  smgtb  */
0,	/*  birep  */
0,	/*  binel  */
0,	/*  bicr  */
0,	/*  colornm  */
0,	/*  defbi  */
0,	/*  endbi  */
0,	/*  setcolor  */
0,	/*  slines  */
0,	/*  dispc  */
0,	/*  smpch  */
0,	/*  rmpch  */
0,	/*  smsc  */
0,	/*  rmsc  */
0,	/*  pctrm  */
0,	/*  scesc  */
0,	/*  scesa  */
0,	/*  ehhlm  */
0,	/*  elhlm  */
0,	/*  elohlm  */
0,	/*  erhlm  */
0,	/*  ethlm  */
0,	/*  evhlm  */
1,	/*  OTi2  */
1,	/*  OTrs  */
1,	/*  OTnl  */
1,	/*  OTbc  */
0,	/*  OTko  */
1,	/*  OTma  */
0,	/*  OTG2  */
0,	/*  OTG3  */
0,	/*  OTG1  */
0,	/*  OTG4  */
0,	/*  OTGR  */
0,	/*  OTGL  */
0,	/*  OTGU  */
0,	/*  OTGD  */
0,	/*  OTGH  */
0,	/*  OTGV  */
0,	/*  OTGC  */
0,	/*  meml  */
0,	/*  memu  */
0,	/*  pln  */
0,	/*  smln  */
0,	/*  rmln  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf27  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf32  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  box1  */
0,	/*  box2  */
0,	/*  batt1  */
0,	/*  batt2  */
0,	/*  colb0  */
0,	/*  colb1  */
0,	/*  colb2  */
0,	/*  colb3  */
0,	/*  colb4  */
0,	/*  colb5  */
0,	/*  colb6  */
0,	/*  colb7  */
0,	/*  colf0  */
0,	/*  colf1  */
0,	/*  colf2  */
0,	/*  colf3  */
0,	/*  colf4  */
0,	/*  colf5  */
0,	/*  colf6  */
0,	/*  colf7  */
0,	/*  font0  */
0,	/*  font1  */
0,	/*  font2  */
0,	/*  font3  */
0,	/*  font4  */
0,	/*  font5  */
0,	/*  font6  */
0,	/*  font7  */
0,	/*  kbtab  */
0,	/*  kdo  */
0,	/*  kcmd  */
0,	/*  kcpn  */
0,	/*  kend  */
0,	/*  khlp  */
0,	/*  knl  */
0,	/*  knpn  */
0,	/*  kppn  */
0,	/*  kppn  */
0,	/*  kquit  */
0,	/*  ksel  */
0,	/*  kscl  */
0,	/*  kscr  */
0,	/*  ktab  */
0,	/*  kmpf1  */
0,	/*  kmpt1  */
0,	/*  kmpf2  */
0,	/*  kmpt2  */
0,	/*  kmpf3  */
0,	/*  kmpt3  */
0,	/*  kmpf4  */
0,	/*  kmpt4  */
0,	/*  kmpf5  */
0,	/*  kmpt5  */
0,	/*  apstr  */
0,	/*  kmpf6  */
0,	/*  kmpt6  */
0,	/*  kmpf7  */
0,	/*  kmpt7  */
0,	/*  kmpf8  */
0,	/*  kmpt8  */
0,	/*  kmpf9  */
0,	/*  kmpt9  */
0,	/*  ksf1  */
0,	/*  ksf2  */
0,	/*  ksf3  */
0,	/*  ksf4  */
0,	/*  ksf5  */
0,	/*  ksf6  */
0,	/*  ksf7  */
0,	/*  ksf8  */
0,	/*  ksf9  */
0,	/*  ksf10  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf26  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf31  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  kact  */
0,	/*  topl  */
0,	/*  btml  */
0,	/*  rvert  */
0,	/*  lvert  */
};

