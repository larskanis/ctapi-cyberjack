/***************************************************************************
    begin       : Sat Jul 04 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#include "Platform.h"
#include "libdialog/base/network.h"

#import <Cocoa/Cocoa.h>

#import "PinDialog.h"
#import "Delegate.h"

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


#define I18N(msg) msg




#define ENABLE_DEBUGPI
#define ENABLE_DEBUGPE
#define ENABLE_DEBUGPD

#define DIALOG_WIDTH  500
#define DIALOG_HEIGHT 300


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

static int _lastDialogId=0;



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








@interface AppController : NSObject {
  NSFileHandle *_sockHandle;
  PinDialog *_pinDialog;
  time_t _startTime;
  NSTimer *_abortTimeUpdater;
  int _socket;
  int _keyTimeout;

  int _currentStage;
  int _stages;

  char _stage0Text[512];
  char _stage1Text[512];
  char _stage2Text[512];
}

- (id) init;
- (int) startConnection:(int) sock;

- (void) dataAvailableOnSocket: (NSNotification*) notification;
- (void) updateRemainingTime:(NSTimer*) timer;

- (int) handleOpenDialog: (RSCT_MESSAGE*) msg;
- (int) handleCloseDialog: (RSCT_MESSAGE*) msg;
- (int) handleMessage:(RSCT_MESSAGE*) msg;
- (int) handleSetStage: (RSCT_MESSAGE*) msg;
- (int) handleSetNumChars: (RSCT_MESSAGE*) msg;


- (int) getAndHandleMessage;

- (void) setStages:(int)stages;
- (void) setCurrentStage:(int)stage;

@end


@implementation AppController

- (id) init {
  self=[super init];
  _pinDialog=nil;
  _abortTimeUpdater=nil;
  _socket=-1;
  _stage0Text[0]=0;
  _stage0Text[1]=0;
  _stage0Text[2]=0;

  return self;
}



- (int) startConnection:(int) sock {
  _socket=sock;
  _sockHandle=[[NSFileHandle alloc] initWithFileDescriptor:_socket closeOnDealloc:YES];
  if (_sockHandle==nil) {
    fprintf(stderr, "Could not init _sockHandle\n");
    return -1;
  }

  [ [NSNotificationCenter defaultCenter] addObserver: self
					   selector: @selector(dataAvailableOnSocket:)
					   name: NSFileHandleDataAvailableNotification
					   object: _sockHandle];

  [_sockHandle waitForDataInBackgroundAndNotify];
  return 0;
}



- (void) dataAvailableOnSocket: (NSNotification*) notification {
  fprintf(stderr, "Data available.\n");
  [self getAndHandleMessage];
  [_sockHandle waitForDataInBackgroundAndNotify];
}



- (void) updateRemainingTime:(NSTimer*) timer {
  if (_pinDialog!=nil) {
    time_t now;
    int diff;
    double newVal=0;

    now=time(NULL);
    diff=(int)difftime(now, _startTime);
    if (diff<(int)_keyTimeout)
      newVal=_keyTimeout-diff;
    [_pinDialog setTimeRemaining: newVal of:_keyTimeout];
  }
}



- (void) setStages:(int)stages {
  _stages=stages;
}



- (void) setCurrentStage:(int)stage {
  _currentStage=stage;
  NSString *str=nil;

  if (_pinDialog) {
    [_pinDialog setNumDigits: 0];

    _startTime=time(NULL);
    [_pinDialog setTimeRemaining: _keyTimeout of:_keyTimeout];

    switch(stage) {
    case 0:
      str=[NSString stringWithUTF8String:_stage0Text];
      break;
    case 1:
      str=[NSString stringWithUTF8String:_stage1Text];
      break;
    case 2:
      str=[NSString stringWithUTF8String:_stage2Text];
      break;

    default:
      break;
    }
  }

  if (str!=nil)
    [_pinDialog setCurrentStage:_currentStage of:_stages withText: str];
  DEBUGPE("done\n");
}



- (int) getAndHandleMessage {
  int rv;
  union {
    char buffer[RSCT_MAX_MESSAGE_LEN];
    RSCT_MESSAGE msg;
  } m;

  /* readable, read message */
  memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
  rv=rsct_net_recv(_socket, &m.msg);
  if (rv<0) {
    return rv;
  }
  else {
    /* handle message */
    rv=[self handleMessage: &(m.msg)];
    if (rv<0) {
      return rv;
    }
  }

  return m.msg.header.type;
}



- (int) handleOpenDialog: (RSCT_MESSAGE*) msg {
  int rv;
#if 0
  int style = NSClosableWindowMask | NSResizableWindowMask |
    NSTexturedBackgroundWindowMask | NSTitledWindowMask | NSMiniaturizableWindowMask;
#else
  int style =  NSTexturedBackgroundWindowMask | NSTitledWindowMask;
#endif

  _pinDialog = [[PinDialog alloc] initWithContentRect:NSMakeRect(50, 50, DIALOG_WIDTH, DIALOG_HEIGHT)
	      styleMask:style
	      backing:NSBackingStoreBuffered
	      defer:NO];
  [_pinDialog setFrame:NSMakeRect(50, 50, DIALOG_WIDTH, DIALOG_HEIGHT) display:TRUE];
  [_pinDialog layout];

  [_pinDialog setTitle: [NSString stringWithUTF8String:msg->openDialog.title]];
  _stages=msg->openDialog.stages;
  _keyTimeout=msg->openDialog.keyTimeout;

  /* start timer */
  _startTime=time(NULL);
  [_pinDialog setTimeRemaining: _keyTimeout of:_keyTimeout];

  [_pinDialog display];
  [_pinDialog makeKeyAndOrderFront: self];

  /* prepare response */
  msg->openDialog.result=0;
  msg->openDialog.dialogId=++_lastDialogId;

  switch(_stages) {
  case 1:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      strncpy(_stage0Text, msg->openDialog.textStage0, sizeof(_stage0Text));
    else
      strncpy(_stage0Text, I18N("Please enter your PIN into the reader's keypad"), sizeof(_stage0Text));
    break;

  case 2:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      strncpy(_stage0Text, msg->openDialog.textStage0, sizeof(_stage0Text));
    else
      strncpy(_stage0Text, I18N("Please enter the new PIN into the reader's keypad"), sizeof(_stage0Text));

    /* text 2 */
    if (msg->openDialog.textStage1[0])
      strncpy(_stage1Text, msg->openDialog.textStage1, sizeof(_stage1Text));
    else
      strncpy(_stage1Text, I18N("Please repeat entering the new PIN into the reader's keypad"), sizeof(_stage1Text));
    break;

  case 3:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      strncpy(_stage0Text, msg->openDialog.textStage0, sizeof(_stage0Text));
    else
      strncpy(_stage0Text, I18N("Please enter the current PIN into the reader's keypad"), sizeof(_stage0Text));

    /* text 2 */
    if (msg->openDialog.textStage1[0])
      strncpy(_stage1Text, msg->openDialog.textStage1, sizeof(_stage1Text));
    else
      strncpy(_stage1Text, I18N("Please enter the new PIN into the reader's keypad"), sizeof(_stage1Text));

    /* text 3 */
    if (msg->openDialog.textStage2[0])
      strncpy(_stage2Text, msg->openDialog.textStage2, sizeof(_stage2Text));
    else
      strncpy(_stage2Text, I18N("Please repeat entering the new PIN into the reader's keypad"), sizeof(_stage2Text));
    break;
  }

  [self setCurrentStage: 0];

  msg->header.type=RSCT_Message_Command_OpenDialog;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_OPENDIALOG);

  /* send response */
  rv=rsct_net_send(_socket, msg);
  if (rv<0) {
    fprintf(stderr, "Error sending response (%d)\n", rv);
    return rv;
  }

  _abortTimeUpdater = [[NSTimer scheduledTimerWithTimeInterval:.5
		      target:self selector:@selector(updateRemainingTime:)
		      userInfo:nil repeats:YES] retain];

  return 0;
}



- (int) handleCloseDialog: (RSCT_MESSAGE*) msg {
  DEBUGPI("CloseDialog\n");
  if (_abortTimeUpdater!=nil) {
    [_abortTimeUpdater invalidate];
    [_abortTimeUpdater release];
    _abortTimeUpdater = nil;
  }
  _socket=-1;
  if (_pinDialog != nil) {
    [_pinDialog close];
  }
  return 0;
}



- (int) handleSetStage: (RSCT_MESSAGE*) msg {
  int rv;

  DEBUGPI("SetStage %d\n", msg->setStage.stage);
  [self setCurrentStage:msg->setStage.stage];

  /* prepare response */
  msg->setStage.result=0;

  msg->header.type=RSCT_Message_Command_SetStage;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETSTAGE);

  /* send response */
  rv=rsct_net_send(_socket, msg);
  if (rv<0) {
    fprintf(stderr, "Error sending response (%d)\n", rv);
    return rv;
  }

  return 0;
}


- (int) handleSetNumChars: (RSCT_MESSAGE*) msg {
  int rv;

  DEBUGPI("SetNumChars %d\n", msg->setCharNum.charNum);
  if (_pinDialog) {
    [_pinDialog setNumDigits: msg->setCharNum.charNum];
    if (msg->setCharNum.beep>0) {
      NSBeep();
    }
  }

  /* prepare response */
  msg->setCharNum.result=0;

  msg->header.type=RSCT_Message_Command_SetCharNum;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETCHARNUM);

  /* send response */
  rv=rsct_net_send(_socket, msg);
  if (rv<0) {
    fprintf(stderr, "RSCT: Error sending response (%d)\n", rv);
    return rv;
  }

  return 0;
}



- (int) handleMessage: (RSCT_MESSAGE*) msg {
  fprintf(stderr, "Received message %d\n", msg->header.type);
  switch(msg->header.type) {
  case RSCT_Message_Command_OpenDialog:
    return [self handleOpenDialog: msg];
  case RSCT_Message_Command_CloseDialog:
    return [self handleCloseDialog: msg];
  case RSCT_Message_Command_SetStage:
    return [self handleSetStage: msg];
  case RSCT_Message_Command_SetCharNum:
    return [self handleSetNumChars: msg];
  default:
    break;
  }

  return 0;
}



@end




int handleConnection(int argc, char **argv, int sock) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  int rv;

  NSApplication *app=[NSApplication sharedApplication];
  Delegate *delegate=[[Delegate alloc] init];
  [app setDelegate:delegate];

  AppController *cntrl=[[AppController alloc] init];
  if (cntrl==nil) {
    DEBUGPE("No ApController\n");
    return -1;
  }

  rv=[cntrl startConnection: sock];
  if (rv>0) {
    DEBUGPE("Error on startConnection: %d\n", rv);
    return -1;
  }

  [NSApp run];

  fprintf(stderr, "Done.\n");

  [pool release];

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





