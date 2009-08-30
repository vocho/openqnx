SED=$(top_builddir)/sed/sed
#TIME=time
CMP=cmp
RM=rm -f

CLEANFILES = tmp* core *.core
DISTCLEANFILES = version.good

#We're not currently set up for testing the way automake would
#like us to be.
#TESTS = basics othersedcmds cmdlines
check: basics othersedcmds cmdlines

EXTRA_DIST = \
	Makefile.in \
	enable.good enable.inp enable.sed \
	sep.good sep.inp sep.sed \
	inclib.good inclib.inp inclib.sed \
	8bit.good 8bit.inp 8bit.sed \
	newjis.good newjis.inp  newjis.sed \
	manis.inp manis.sed \
	xabcx.good xabcx.inp xabcx.sed \
	dollar.good dollar.inp dollar.sed \
	noeol.good noeol.inp noeol.sed \
	numsub.good numsub.inp numsub.sed \
	allsub.good allsub.sed \
	subwrite.inp subwrite.sed  subwrt1.good subwrt2.good \
	head.good head.inp head.sed \
	madding.good madding.inp madding.sed \
	mac-mf.good mac-mf.inp mac-mf.sed \
	empty.good empty.inp empty.sed \
	8to7.good 8to7.sed \
	cv-vars.good cv-vars.inp cv-vars.sed \
	middle.good middle.sed \
	khadafy.inp khadafy.sed \
	spencer.inp spencer.sh \
	linecnt.good linecnt.inp linecnt.sed \
	distrib.good distrib.inp distrib.sed distrib.sh \
	writeout.inp writeout.sed wrtout1.good wrtout2.good \
	readin.good readin.in2 readin.inp readin.sed \
	dc.good dc.inp \
	help.good version.gin

CORE_BASICS = \
	enable sep inclib 8bit newjis xabcx dollar noeol \
	numsub head madding mac-mf empty


basics: $(CORE_BASICS) manis allsub 8to7 cv-vars middle
othersedcmds: subwrite khadafy linecnt distrib writeout readin dc
cmdlines: help version file quiet

#NOTE: the "spencer" test suite is no longer run by default, because
# it: a) tests EREs instead of BREs; and b) tests using EREs with
# undefined semantics.  One can still explicitly "make spencer" if
# desired, but its use is depreciated.  A replacement BRE tester
# will written for a later revision of sed.



$(CORE_BASICS) linecnt::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/$@.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

manis khadafy::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/$@.inp > tmp.$@
	$(CMP) $(srcdir)/$@.inp tmp.$@
	@$(RM) tmp.$@

allsub::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/numsub.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

8to7::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/8bit.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

cv-vars::
	$(TIME) $(SED) -n -f $(srcdir)/$@.sed < $(srcdir)/$@.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

middle::
	$(TIME) $(SED) -n -f $(srcdir)/$@.sed < $(srcdir)/head.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

subwrite::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/$@.inp > tmp.1$@
	$(CMP) $(srcdir)/subwrt1.good tmp.1$@
	$(CMP) $(srcdir)/subwrt2.good tmp.subwrite-w
	@$(RM) tmp.1$@ tmp.subwrite-w

spencer::
	sh $(srcdir)/$@.sh $(SED) <  $(srcdir)/$@.inp > tmp.$@
	$(TIME) sh tmp.$@
	@$(RM) tmp.$@

distrib::
	$(TIME) sh $(srcdir)/$@.sh $(SED) < $(srcdir)/$@.inp > tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

writeout::
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/$@.inp >tmp.1$@
	$(CMP) $(srcdir)/wrtout1.good tmp.1$@
	$(CMP) $(srcdir)/wrtout2.good tmp.writeout-w
	@$(RM) tmp.1$@ tmp.writeout-w

readin::
	@test -f $@.in2 || cp $(srcdir)/$@.in2 $@.in2
	$(TIME) $(SED) -f $(srcdir)/$@.sed < $(srcdir)/$@.inp >tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

dc::
	@echo "Patience, the dc.sed test is expected to take a while..."
	$(TIME) $(SED) -n -f $(top_srcdir)/dc.sed < $(srcdir)/$@.inp >tmp.$@
	$(CMP) $(srcdir)/$@.good tmp.$@
	@$(RM) tmp.$@

#
# cmdlines targets
#

help::
	$(SED) --help | $(SED) '1s/ [^ ]* / sed /' > tmp.1$@
	$(SED) -h | $(SED) '1s/ [^ ]* / sed /' > tmp.2$@
	-$(SED) 2>&1 | $(SED) '1s/ [^ ]* / sed /' > tmp.3$@ || true
	$(CMP) $(srcdir)/$@.good tmp.1$@
	$(CMP) tmp.1$@ tmp.2$@
	$(CMP) tmp.2$@ tmp.3$@
	@$(RM) tmp.1$@ tmp.2$@ tmp.3$@

version:: version.good
	$(SED) --version  > tmp.1$@ 2>&1
	$(SED) -V > tmp.2$@ 2>&1
	$(CMP) $@.good tmp.1$@
	$(CMP) $@.good tmp.2$@
	@$(RM) tmp.1$@ tmp.2$@

file::
	$(TIME) $(SED) --file=$(srcdir)/newjis.sed < $(srcdir)/newjis.inp > tmp.$@
	$(CMP) $(srcdir)/newjis.good tmp.$@
	@$(RM) tmp.$@

quiet::
	$(TIME) $(SED) --quiet -f $(srcdir)/cv-vars.sed \
		< $(srcdir)/cv-vars.inp > tmp.$@
	$(CMP) $(srcdir)/cv-vars.good tmp.$@
	@$(RM) tmp.$@


# automake won't cooperate with me and let me AC_OUTPUT() this,
# so I need to special-case this:
version.good: $(srcdir)/version.gin Makefile
	$(SED) 's^@'VERSION'@^@VERSION@^' $(srcdir)/version.gin > tmp-$@
	mv tmp-$@ $@
