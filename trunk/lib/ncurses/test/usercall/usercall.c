/*
 * $Id: usercall.c 153052 2008-08-13 01:17:50Z coreos $
 */
#include <test.priv.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/proxy.h>
#include <time.h>
#include <signal.h>
#include <string.h>

static void my_callback(pid_t pid);
static pid_t timer_proxy;
static time_t now;

static void my_callback(pid_t pid)
{
	char *p;

	now = time (NULL);
	p = ctime(&now);
	p[strlen(p)-1] = '\0';

	if (pid != timer_proxy)
		printw("%s: Unknown pid (%d)\n", p, pid);
	else
		printw("%s: timer proxy has been triggled !\n", p);

	refresh();
}

void main( int argc GCC_UNUSED, char *argv[] GCC_UNUSED)
{
	int c;
	int tmout = -1;
	timer_t timer;
	struct sigevent ev;
	struct itimerspec t;
	MEVENT  event;
	char *p;

	while ((c = getopt(argc, argv, "t:")) != EOF) {
		switch (c) {
		case 't':
			tmout = atoi(optarg);
			break;
		}
	}

	/* set up a 5 second timer to triggle us */
	if ((timer_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1) {
		perror("qnx_proxy_attach");
		exit(-1);
	}
	ev.sigev_signo = -1 * timer_proxy;

	if ((timer = timer_create(CLOCK_REALTIME, &ev)) == -1) {
		perror("timer_create");
		exit(-1);
	}

	t.it_value.tv_sec  = 5L;
	t.it_value.tv_nsec = 0L;
	t.it_interval.tv_sec  = 5L;
	t.it_interval.tv_nsec = 0L;

	timer_settime(timer, 0, &t, NULL);

    initscr();
	raw();
	noecho();
//	mousemask(ALL_MOUSE_EVENTS, (mmask_t *)0);
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);

	if (tmout != -1)
		timeout(tmout * 1000);

//	set_qnx_callback(my_callback, TRUE);
	
	for (;;) {
		c = getch();
		now = time(NULL);
		p = ctime(&now);
		p[strlen(p)-1] = '\0';
		switch(c)
		{
			case ERR:
				printw("%s: input timed out\n", p);
				break;
			case('q'):
			case('Q'):
	    		noraw();
				nl();
	    		endwin();
				exit(EXIT_SUCCESS);
				break;
			case KEY_MOUSE:
				getmouse(&event);
				printw("%s: KEY_MOUSE, %s\n", p, _tracemouse(&event));
				break;
			default:
				printw("%s: key = %x\n", p, c);
				break;
		}	
    }
}
