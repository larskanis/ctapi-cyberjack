

#define ENABLE_DEBUGPI


#include "Platform.h"
#include "libdialog/base/network.h"
#include "dlg.hpp"

#include <fx.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <utmp.h>

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <stdlib.h>

#include <signal.h>
#include <sys/wait.h>


#ifdef ENABLE_DEBUGPE
# define DEBUGPE(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPE(format, args...)
#endif


#ifdef ENABLE_DEBUGPI
# define DEBUGPI(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPI(format, args...)
#endif


#ifdef ENABLE_DEBUGPD
# define DEBUGPD(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPD(format, args...)
#endif



static int daemon_abort=0;

static char daemon_sock_buf[512];



/* Signal handler */

struct sigaction saINT,saTERM, saINFO, saHUP, saCHLD;

void signalHandler(int s) {
  switch(s) {
  case SIGINT:
  case SIGTERM:
#ifdef SIGHUP
  case SIGHUP:
#endif
    daemon_abort=1;
    DEBUGPI("INFO: Terminating daemon.\n");
    break;

#ifdef SIGCHLD
  case SIGCHLD:
    for (;;) {
      pid_t pid;
      int stat_loc;

      pid=waitpid((pid_t)-1, &stat_loc, WNOHANG);
      if (pid==-1 || pid==0)
	break;
      else {
	DEBUGPI("INFO: Service %d finished.\n", (int)pid);
      }
    }
    break;
#endif

  default:
    DEBUGPI("INFO: Unhandled signal %d\n", s);
    break;
  } /* switch */
}



int setSingleSignalHandler(struct sigaction *sa, int sig) {
  sa->sa_handler=signalHandler;
  sigemptyset(&sa->sa_mask);
  sa->sa_flags=0;
  if (sigaction(sig, sa,0)) {
    DEBUGPE("ERROR: sigaction(%d): %d=%s",
	   sig, errno, strerror(errno));
    return -1;
  }
  return 0;
}



int setSignalHandler() {
  int rv;

  rv=setSingleSignalHandler(&saINT, SIGINT);
  if (rv)
    return rv;
#ifdef SIGCHLD
  rv=setSingleSignalHandler(&saCHLD, SIGCHLD);
  if (rv)
    return rv;
#endif
  rv=setSingleSignalHandler(&saTERM, SIGTERM);
  if (rv)
    return rv;
#ifdef SIGHUP
  rv=setSingleSignalHandler(&saHUP, SIGHUP);
  if (rv)
    return rv;
#endif

  return 0;
}





int mkSockName() {
  struct passwd *p;

  p=getpwuid(geteuid());
  if (!p) {
    fprintf(stderr, "ERROR: %s at getpwuid\n", strerror(errno));
    endpwent();
    return -1;
  }
  if (sizeof(daemon_sock_buf)<strlen(p->pw_dir)+1) {
    fprintf(stderr, "Internal: Buffer too small (need %d bytes)\n",
	    (int)(strlen(p->pw_dir)+1));
    endpwent();
    return -1;
  }
  strcpy(daemon_sock_buf, p->pw_dir);
  endpwent();

  strncat(daemon_sock_buf, "/.cyberJack_gui_sock", sizeof(daemon_sock_buf)-1);
  daemon_sock_buf[sizeof(daemon_sock_buf)-1]=0;

  return 0;
}



int prepareListen() {
  int sock;

  sock=rsct_net_listen_by_path(daemon_sock_buf);
  if (sock==-1) {
    fprintf(stderr, "Error on rsct_net_listen_by_path(%s): %s\n",
	    daemon_sock_buf, strerror(errno));
    return -1;
  }

  return sock;
}



int handleConnection(int argc, char **argv, int sock) {
  FXApp a("cyberJackDialog", "ReinerSCT");
  int rv;

  a.init(argc,argv);
  a.create();

  CyberJackDialog dlg(sock, &a);
  dlg.create();
  dlg.layout();

  rv=dlg.getAndHandleMessage();
  if (rv<0)
    return rv;
  else if (rv!=RSCT_Message_Command_OpenDialog) {
    fprintf(stderr, "ERROR: First message is not OpenDialog\n");
    return -1;
  }

  a.addInput(sock, INPUT_READ, &dlg, CyberJackDialog::ID_SOCKET);
  dlg.show(PLACEMENT_SCREEN);
  dlg.execute();
  a.removeInput(sock, INPUT_READ);
  close(sock);

  return 0;
}



int main(int argc, char **argv) {
  int rv;
  int sk;

  rv=setSignalHandler();
  if (rv) {
    DEBUGPE("ERROR: Could not setup signal handler\n");
    return 2;
  }

  rv=mkSockName();
  if (rv<0) {
    fprintf(stderr, "RSCT: Could not determine sockat path\n");
    return 2;
  }
  unlink(daemon_sock_buf);

  sk=rsct_net_listen_by_path(daemon_sock_buf);
  if (sk==-1) {
    fprintf(stderr, "Error on rsct_net_listen_by_path(%s): %s\n",
	    daemon_sock_buf, strerror(errno));
    return 2;
  }

  DEBUGPI("INFO: cyberJack GUI started\n");

  while(!daemon_abort) {
    int newS;

    newS=rsct_net_accept(sk);
    if (newS!=-1) {
      pid_t pid;

      pid=fork();
      if (pid<0) {
        /* error */
      }
      else if (pid==0) {
	int rv;

	/* child */
	close(sk);
	rv=setSignalHandler();
	if (rv) {
	  DEBUGPE("ERROR: Could not setup child's signal handler\n");
	  exit(2);
	}

	fprintf(stderr, "Received a connection.\n");
        rv=handleConnection(argc, argv, newS);
	fprintf(stderr, "Connection closed.\n");
	if (rv)
	  exit(3);
	exit(0);
      }
      else {
	/* parent */
	DEBUGPI("INFO: cyberJack GUI service spawned (%d)\n", (int)pid);
	close(newS);
      }
    }
  }

  DEBUGPI("INFO: cyberJack GUI going down\n");

  unlink(daemon_sock_buf);
  return 0;
}


