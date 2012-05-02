/***************************************************************************
    begin       : Wed Jun 16 2010
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
#include "dialog_p.h"
#include "network.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <utmp.h>

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#ifdef OS_DARWIN
# include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#endif


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


#ifdef OS_DARWIN

static int rsct_dialog_mk_socket() {
  char buffer[256];
  CFStringRef socketPath=NULL;
  CFStringRef currentUser=SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);

  if (currentUser==NULL )
    return -1;

  if ( CFStringGetLength(currentUser) == 0 ) {
    CFRelease(currentUser);
    return -1;
  }

  if (CFStringGetCString(currentUser, buffer, sizeof(buffer)-1, kCFStringEncodingUTF8)) {
    struct passwd * pw = getpwnam(buffer);
    if (pw && pw->pw_dir) {
      socketPath=CFStringCreateWithFormat(NULL, NULL, CFSTR("/%s/.cyberJack_gui_sock"), pw->pw_dir);
    }
  }
  if (socketPath==NULL )
    socketPath=CFStringCreateWithFormat(NULL, NULL, CFSTR("/Users/%@//.cyberJack_gui_sock"), currentUser);

  if (currentUser)
    CFRelease(currentUser);

  if (socketPath==NULL)
    return -1;

  if (CFStringGetFileSystemRepresentation(socketPath, buffer, sizeof(buffer))) {
    int sk;

    CFRelease(socketPath);
    sk=rsct_net_connect_by_path(buffer);
    if (sk==-1) {
      DEBUGPE("RSCT: Could not connect to [%s]: %s (%d)\n",
	      buffer, strerror(errno), errno);
      return -1;
    }

    return sk;
  }
  else {
    CFRelease(socketPath);
    return -1;
  }
}

#else

static int rsct_dialog_mk_socket() {
  char buffer[256];
  struct utmp *u_tmp_p;
  struct passwd *pw;
  int sk;

  while ((u_tmp_p = getutent()) != NULL) {
    DEBUGPI("RSCT: ut_type=%d, ut_line=[%s]\n",
	    u_tmp_p->ut_type,
	    u_tmp_p->ut_line);
    if (u_tmp_p->ut_type==USER_PROCESS &&
	(u_tmp_p->ut_line[0]!=0 && strcasecmp(u_tmp_p->ut_line, ":0")==0) &&
	u_tmp_p->ut_user[0]!=0)
      break;
  }

  if (u_tmp_p==NULL) {
    DEBUGPE("RSCT: No user logged in at XServer :0 (%s (%d))\n", strerror(errno), errno);
    endutent();
    return -1;
  }

  pw=getpwnam(u_tmp_p->ut_user);
  if (pw==NULL) {
    DEBUGPE("RSCT: Could not get home folder for user [%s]: %s (%d)\n",
	    u_tmp_p->ut_user, strerror(errno), errno);
    endutent();
    return -1;
  }

  if (pw->pw_dir==NULL || *(pw->pw_dir)==0) {
    DEBUGPE("RSCT: User [%s] has no home folder\n", u_tmp_p->ut_user);
    endutent();
    return -1;
  }
  strncpy(buffer, pw->pw_dir, sizeof(buffer)-1);
  strncat(buffer, "/.cyberJack_gui_sock", sizeof(buffer)-1);
  endutent();

  sk=rsct_net_connect_by_path(buffer);
  if (sk==-1) {
    DEBUGPE("RSCT: Could not connect to [%s]: %s (%d)\n",
	    buffer, strerror(errno), errno);
    return -1;
  }

  return sk;
}


#endif



RSCT_DIALOG *rsct_dialog_new(const char *title, int stages,
			     uint8_t cla, uint8_t ins,
			     int keyTimeout,
			     const char *textStage0,
			     const char *textStage1,
			     const char *textStage2){
  RSCT_DIALOG *dlg;

  dlg=(RSCT_DIALOG*) malloc(sizeof(RSCT_DIALOG));
  if (dlg) {
    memset(dlg, 0, sizeof(RSCT_DIALOG));
    dlg->socket=-1;

    if (title && *title)
      dlg->title=strdup(title);
    else
      dlg->title=strdup("Reiner SCT cyberJack");

    if (textStage0 && *textStage0)
      dlg->textStage0=strdup(textStage0);
    else
      dlg->textStage0=NULL;

    if (stages>1) {
      if (textStage1 && *textStage1)
	dlg->textStage1=strdup(textStage1);
      else
	dlg->textStage1=NULL;
    }

    if (stages>2) {
      if (textStage2 && *textStage2)
	dlg->textStage2=strdup(textStage2);
      else
	dlg->textStage2=NULL;
    }

    dlg->cla=cla;
    dlg->ins=ins;
    dlg->stages=stages;
    dlg->keyTimeout=keyTimeout;
  }

  return dlg;
}



void rsct_dialog_free(RSCT_DIALOG *dlg) {
  if (dlg) {
    free(dlg->title);
    free(dlg->textStage2);
    free(dlg->textStage1);
    free(dlg->textStage0);
    if (dlg->socket!=-1)
      close(dlg->socket);
    free(dlg);
  }
}



int rsct_dialog_get_socket(const RSCT_DIALOG *dlg) {
  return dlg->socket;
}



int rsct_dialog_get_char_num(const RSCT_DIALOG *dlg) {
  return dlg->numChars;
}



int rsct_dialog_set_stage(RSCT_DIALOG *dlg, int stage) {
  if (dlg && dlg->socket!=-1 &&
      stage<RSCT_DIALOG_MAX_STAGES) {
    int rv;
    union {
      char buffer[RSCT_MAX_MESSAGE_LEN];
      RSCT_MESSAGE msg;
    } m;
  
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    m.msg.header.type=RSCT_Message_Command_SetStage;
    m.msg.header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETSTAGE);
    m.msg.setStage.dialogId=dlg->dialogId;
    m.msg.setStage.stage=stage;
    rv=rsct_net_send(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return rv;
    }

    /* receive response */
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    rv=rsct_net_recv(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return rv;
    }
    if (m.msg.setStage.result<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return m.msg.setStage.result;
    }
  }

  dlg->currentStage=stage;

  return 0;
}



int rsct_dialog_get_stage(const RSCT_DIALOG *dlg) {
  if (dlg)
    return dlg->currentStage;
  return 0;
}



int rsct_dialog_get_stages(const RSCT_DIALOG *dlg) {
  if (dlg)
    return dlg->stages;
  return 0;
}



uint8_t rsct_dialog_get_cla(const RSCT_DIALOG *dlg) {
  if (dlg)
    return dlg->cla;
  return 0;
}



uint8_t rsct_dialog_get_ins(const RSCT_DIALOG *dlg) {
  if (dlg)
    return dlg->ins;
  return 0;
}



int rsct_dialog_open(RSCT_DIALOG *dlg) {
  if (dlg) {
    int rv;
    union {
      char buffer[RSCT_MAX_MESSAGE_LEN];
      RSCT_MESSAGE msg;
    } m;

    dlg->socket=rsct_dialog_mk_socket();
    if (dlg->socket==-1)
      return -1;

    /* send OPEN_DIALOG */
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    m.msg.header.type=RSCT_Message_Command_OpenDialog;
    m.msg.header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_OPENDIALOG);
    m.msg.openDialog.stages=dlg->stages;
    m.msg.openDialog.keyTimeout=dlg->keyTimeout;

    if (dlg->title)
      strncpy(m.msg.openDialog.title, dlg->title, sizeof(m.msg.openDialog.title)-1);
    if (dlg->stages>0 && dlg->textStage0)
      strncpy(m.msg.openDialog.textStage0, dlg->textStage0, sizeof(m.msg.openDialog.textStage0)-1);
    if (dlg->stages>1 && dlg->textStage1)
      strncpy(m.msg.openDialog.textStage1, dlg->textStage1, sizeof(m.msg.openDialog.textStage1)-1);
    if (dlg->stages>2 && dlg->textStage2)
      strncpy(m.msg.openDialog.textStage2, dlg->textStage2, sizeof(m.msg.openDialog.textStage2)-1);

    /* send message */
    rv=rsct_net_send(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return rv;
    }

    /* receive response */
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    rv=rsct_net_recv(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return rv;
    }
    if (m.msg.openDialog.result<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return m.msg.openDialog.result;
    }

    dlg->dialogId=m.msg.openDialog.dialogId;

    return 0;
  }

  return -1;
}



void rsct_dialog_close(RSCT_DIALOG *dlg) {
  if (dlg && dlg->socket!=-1) {
    union {
      char buffer[RSCT_MAX_MESSAGE_LEN];
      RSCT_MESSAGE msg;
    } m;
  
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    m.msg.header.type=RSCT_Message_Command_CloseDialog;
    m.msg.header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_CLOSEDIALOG);
    m.msg.closeDialog.dialogId=dlg->dialogId;
    rsct_net_send(dlg->socket, &m.msg);
    close(dlg->socket);
    dlg->socket=-1;
  }
}



int rsct_dialog_set_char_num(RSCT_DIALOG *dlg, int charNum, int beep) {
  if (dlg && dlg->socket!=-1) {
    int rv;
    union {
      char buffer[RSCT_MAX_MESSAGE_LEN];
      RSCT_MESSAGE msg;
    } m;
  
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    m.msg.header.type=RSCT_Message_Command_SetCharNum;
    m.msg.header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETCHARNUM);
    m.msg.setCharNum.dialogId=dlg->dialogId;
    m.msg.setCharNum.charNum=charNum;
    m.msg.setCharNum.beep=beep;

    rv=rsct_net_send(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
    }

    /* receive response */
    memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
    rv=rsct_net_recv(dlg->socket, &m.msg);
    if (rv<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return rv;
    }
    if (m.msg.setCharNum.result<0) {
      close(dlg->socket);
      dlg->socket=-1;
      return m.msg.setCharNum.result;
    }
  }

  dlg->numChars=charNum;
  return 0;
}







