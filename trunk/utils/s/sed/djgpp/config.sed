/for ac_dir in \$PATH; do/,/IFS="\$ac_save_IFS"/ {
  s|test -f \$ac_dir/|test -x $ac_dir/|
}

/ac_given_INSTALL=/,/^CEOF/ {
  /^s%@LIBOBJS@/a\
/TEXINPUTS=/s,:,\\\\\\\\\\\\\\;,g
}

/\*) srcdir=/s,/\*,/*|[A-z]:/*,
/\$]\*) INSTALL=/s,\[/\$\]\*,&|[A-z]:/*,
/ac_file_inputs=/s,\( -e "s%\^%\$ac_given_srcdir/%"\)\( -e "s%:% $ac_given_srcdir/%g"\),\2\1,

/ -\*=\*)/ s,\*=,*:,g
s,=\*,:*,g
s,=\.\*,:.*,
s,\( --..*\)=,\1:,
