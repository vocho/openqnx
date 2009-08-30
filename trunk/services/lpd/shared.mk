CFLAGS=    -g1 -w9 -fr=${@:.o=.err} -I common_source  -fi=common_source/protos
LDFLAGS=   -g1 -b -w9 -lsocket_s -lunix 

LPC_OBJS=	lpc.o cmds.o cmdtab.o
LPD_OBJS=	lpd.o lpdchar.o printjob.o recvjob.o daemon.o
LPQ_OBJS=	lpq.o
LPR_OBJS=	lpr.o
LPRM_OBJS=	lprm.o



shr:	bin/lprc_s bin/lpd_s bin/lprq_s bin/lpr_s bin/lprrm_s 


bin/lprc_s:	$(addprefix lprc/,$(LPC_OBJS))  $(addprefix common_source/,startdaemon.o common.o printcap.o)
	cc -o $@ $(LDFLAGS) $^
	usemsg $@ lprc/lpc.use

bin/lpd_s:	$(addprefix lpd/,$(LPD_OBJS))  $(addprefix common_source/,startdaemon.o common.o printcap.o displayq.o rmjob.o)
	cc -o $@ $(LDFLAGS) $^
	usemsg $@ lpd/lpd.use

bin/lprq_s:	$(addprefix lprq/,$(LPQ_OBJS))  $(addprefix common_source/,startdaemon.o common.o printcap.o displayq.o)
	cc -o $@ $(LDFLAGS) $^
	usemsg $@ lprq/lpq.use

bin/lpr_s:	$(addprefix lpr/,$(LPR_OBJS))  $(addprefix common_source/,startdaemon.o common.o printcap.o)
	cc -o $@ $(LDFLAGS) $^
	usemsg $@ lpr/lpr.use

bin/lprrm_s:	$(addprefix lprrm/,$(LPRM_OBJS))  $(addprefix common_source/,startdaemon.o common.o printcap.o rmjob.o)
	cc -o $@ $(LDFLAGS) $^
	usemsg $@ lprrm/lprm.use



common_source/%.o:	common_source/%.c
	cc -o $@ -c $(CFLAGS) $^

lprc/%.o:	lprc/%.c
	cc -o $@ -c $(CFLAGS) $^

lpd/%.o:	lpd/%.c
	cc -o $@ -c $(CFLAGS) $^

lprq/%.o:	lprq/%.c
	cc -o $@ -c $(CFLAGS) $^

lpr/%.o:	lpr/%.c
	cc -o $@ -c $(CFLAGS) $^

lprrm/%.o:	lprrm/%.c
	cc -o $@ -c $(CFLAGS) $^

clean_shr:
	rm -f bin/lp*_s

install_shr: shr
	beirut chown root bin/lpr_s bin/lprrm_s bin/lprq_s
	beirut chmod u+s bin/lpr_s bin/lprrm_s bin/lprq_s
	beirut cp -cnv bin/lpr_s /usr/ucb/lpr
	beirut cp -cnv bin/lprrm_s /usr/ucb/lprrm
	beirut cp -cnv bin/lprq_s /usr/ucb/lprq
	beirut cp -cnv bin/lprc_s /usr/ucb/lprc
	beirut cp -cnv bin/lpd_s /usr/ucb/lpd
	mkdir -p archive/32
	t=`date +"%y%m%d"`; \
	beirut cp -vnf bin/lpr_s archive/32/lpr_s.$$t; \
	beirut cp -vnf bin/lprrm_s archive/32/lprrm_s.$$t; \
	beirut cp -vnf bin/lprq_s archive/32/lprq_s.$$t; \
	beirut cp -vnf bin/lprc_s archive/32/lprc_s.$$t; \
	beirut cp -vnf bin/lpd_s archive/32/lpd_s.%%t


