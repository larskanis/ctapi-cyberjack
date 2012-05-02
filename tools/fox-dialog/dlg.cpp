/***************************************************************************
    begin       : Fri Jul 01 2010
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "dlg.hpp"
#include "libdialog/base/network.h"

#include <stdio.h>
#include <assert.h>
#include <unistd.h>


#include "icons/icons.cpp"


#define I18N(msg) msg


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



int CyberJackDialog::m_lastDialogId=0;



FXDEFMAP(CyberJackDialog) CyberJackDialogMap[]={
  FXMAPFUNC(SEL_IO_READ, CyberJackDialog::ID_SOCKET, CyberJackDialog::onIoRead),
  FXMAPFUNC(SEL_TIMEOUT, CyberJackDialog::ID_TIMER, CyberJackDialog::onTimeout),
  FXMAPFUNC(SEL_TIMEOUT, CyberJackDialog::ID_TIMER, CyberJackDialog::onTimeout),
  FXMAPFUNC(SEL_KEYPRESS, 0, CyberJackDialog::onSelKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE, 0, CyberJackDialog::onSelKeyRelease)
};


FXIMPLEMENT(CyberJackDialog, FXDialogBox, CyberJackDialogMap, ARRAYNUMBER(CyberJackDialogMap))




CyberJackDialog::CyberJackDialog()
:FXDialogBox()
,m_currentStage(-1)
,m_stages(-1)
,m_numChars(0) {
}



CyberJackDialog::CyberJackDialog(int sock,
				 FXApp* a,
				 FXuint opts,
				 FXint x, FXint y, FXint w, FXint h,
				 FXint pl, FXint pr, FXint pt, FXint pb,
				 FXint hs, FXint vs)
:FXDialogBox(a, "Reiner SCT cyberJack", opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
,m_socket(sock)
,m_currentStage(-1)
,m_stages(0)
,m_numChars(0)
,m_keyTimeout(15)
,m_stageLabel(NULL)
,m_progressBar(NULL)
,m_image(NULL)
,m_fontSmall(NULL)
,m_fontBig(NULL) {
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;
  FXVerticalFrame *vf2;
  FXIconSource iconSource(a);
  FXLabel *label;
  FXFont *fnt;

  m_image=iconSource.loadIconData(fxcj_icon_cy_ecom_plus_n, "jpg");
  fnt=getApp()->getNormalFont();
  if (fnt) {
    int fsize;
    int nsize;

    fsize=fnt->getSize();
    nsize=(fsize-(fsize/3))/10;
    m_fontSmall=new FXFont(getApp(), fnt->getName(), nsize,
			   fnt->getWeight(), fnt->getSlant(), fnt->getEncoding());

    nsize=(fsize*2)/10;
    m_fontBig=new FXFont(getApp(), fnt->getName(), nsize,
			 fnt->getWeight(), fnt->getSlant(), fnt->getEncoding());
  }

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X);
  label=new FXLabel(hf, "Reiner SCT cyberJack", NULL, LAYOUT_LEFT);
  if (m_fontSmall)
    label->setFont(m_fontSmall);
  m_stageLabel=new FXLabel(hf, "", NULL, LAYOUT_RIGHT);
  if (m_fontSmall)
    m_stageLabel->setFont(m_fontSmall);
  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);

  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  new FXImageFrame(hf, m_image, FRAME_SUNKEN|FRAME_THICK);

  vf2=new FXVerticalFrame(hf, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  m_textLabel=new FXLabel(vf2, "", NULL, LABEL_NORMAL);
  m_enteredLabel=new FXLabel(vf2, "", NULL, LABEL_NORMAL|LAYOUT_CENTER_X);
  if (m_fontBig)
    m_enteredLabel->setFont(m_fontBig);

  m_progressBar=new FXProgressBar(vf, NULL, 0, PROGRESSBAR_NORMAL|PROGRESSBAR_HORIZONTAL|LAYOUT_FILL_X);
  m_progressBar->setTotal(m_keyTimeout);
  m_progressBar->setProgress(m_keyTimeout);

}



CyberJackDialog::~CyberJackDialog() {
}



void CyberJackDialog::create() {
  FXDialogBox::create();
  if (m_image)
    m_image->create();
  if (m_fontSmall)
    m_fontSmall->create();
  if (m_fontBig)
    m_fontBig->create();
}



void CyberJackDialog::setStage(int stage) {
  FXString str;

  switch(stage) {
  case 0: m_textLabel->setText(m_textStage0); break;
  case 1: m_textLabel->setText(m_textStage1); break;
  case 2: m_textLabel->setText(m_textStage2); break;
  default:
    break;
  }

  m_currentStage=stage;
  if (m_stages>1)
    str=FXStringVal(m_currentStage+1, 10)+"/"+FXStringVal(m_stages, 10);
  m_stageLabel->setText(str);
  setNumDigits(0);

  m_progressBar->setTotal(m_keyTimeout);
  m_progressBar->setProgress(m_keyTimeout);

  m_startTime=time(NULL);
  getApp()->addTimeout(this, ID_TIMER, 1000);
}



void CyberJackDialog::setNumDigits(int num) {
  FXString str;
  int i;

  for (i=0; i<num; i++)
    str+="*";
  m_enteredLabel->setText(str);
}



int CyberJackDialog::handleOpenDialog(RSCT_MESSAGE *msg) {
  int rv;

  setTitle(msg->openDialog.title);
  m_stages=msg->openDialog.stages;
  m_keyTimeout=msg->openDialog.keyTimeout;

  m_progressBar->setTotal(m_keyTimeout);
  m_progressBar->setProgress(m_keyTimeout);

  m_startTime=time(NULL);
  getApp()->addTimeout(this, ID_TIMER, 1000);

  /* prepare response */
  msg->openDialog.result=0;
  msg->openDialog.dialogId=++m_lastDialogId;

  switch(m_stages) {
  case 1:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      m_textStage0=FXString(msg->openDialog.textStage0);
    else
      m_textStage0=FXString(I18N("Please enter your PIN into the reader's keypad"));
    break;

  case 2:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      m_textStage0=FXString(msg->openDialog.textStage0);
    else
      m_textStage0=FXString(I18N("Please enter the new PIN into the reader's keypad"));

    /* text 3 */
    if (msg->openDialog.textStage1[0])
      m_textStage1=FXString(msg->openDialog.textStage1);
    else
      m_textStage1=FXString(I18N("Please repeat entering the new PIN into the reader's keypad"));
    break;

  case 3:
    /* text 1 */
    if (msg->openDialog.textStage0[0])
      m_textStage0=FXString(msg->openDialog.textStage0);
    else
      m_textStage0=FXString(I18N("Please enter the current PIN into the reader's keypad"));

    /* text 2 */
    if (msg->openDialog.textStage1[0])
      m_textStage1=FXString(msg->openDialog.textStage1);
    else
      m_textStage1=FXString(I18N("Please enter the new PIN into the reader's keypad"));

    /* text 3 */
    if (msg->openDialog.textStage2[0])
      m_textStage2=FXString(msg->openDialog.textStage2);
    else
      m_textStage2=FXString(I18N("Please repeat entering the new PIN into the reader's keypad"));
    break;
  }

  setStage(0);

  msg->header.type=RSCT_Message_Command_OpenDialog;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_OPENDIALOG);

  /* send response */
  rv=rsct_net_send(m_socket, msg);
  if (rv<0) {
    fprintf(stderr, "Error sending response (%d)\n", rv);
    return rv;
  }

  return 0;
}



int CyberJackDialog::handleCloseDialog(RSCT_MESSAGE *msg) {
  getApp()->removeTimeout(this, ID_TIMER);
  m_socket=-1;
  getApp()->stopModal(this, FALSE);
  hide();

  return 0;
}



int CyberJackDialog::handleSetStage(RSCT_MESSAGE *msg) {
  int rv;

  setStage(msg->setStage.stage);

  /* prepare response */
  msg->setStage.result=0;

  msg->header.type=RSCT_Message_Command_SetStage;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETSTAGE);

  /* send response */
  rv=rsct_net_send(m_socket, msg);
  if (rv<0) {
    fprintf(stderr, "Error sending response (%d)\n", rv);
    return rv;
  }

  return 0;
}


int CyberJackDialog::handleSetNumChars(RSCT_MESSAGE *msg) {
  int rv;

  setNumDigits(msg->setCharNum.charNum);
  if (msg->setCharNum.beep>0) {
    getApp()->beep();
  }

  /* prepare response */
  msg->setCharNum.result=0;

  msg->header.type=RSCT_Message_Command_SetCharNum;
  msg->header.len=RSCT_MSG_SIZE(RSCT_MESSAGE_SETCHARNUM);

  /* send response */
  rv=rsct_net_send(m_socket, msg);
  if (rv<0) {
    fprintf(stderr, "RSCT: Error sending response (%d)\n", rv);
    return rv;
  }

  return 0;
}



int CyberJackDialog::handleMessage(RSCT_MESSAGE *msg) {
  assert(msg);

  fprintf(stderr, "Received message %d\n", msg->header.type);
  switch(msg->header.type) {
  case RSCT_Message_Command_OpenDialog:
    return handleOpenDialog(msg);
  case RSCT_Message_Command_CloseDialog:
    return handleCloseDialog(msg);
  case RSCT_Message_Command_SetStage:
    return handleSetStage(msg);
  case RSCT_Message_Command_SetCharNum:
    return handleSetNumChars(msg);
  default:
    break;
  }

  return 0;
}



int CyberJackDialog::getAndHandleMessage() {
  int rv;
  union {
    char buffer[RSCT_MAX_MESSAGE_LEN];
    RSCT_MESSAGE msg;
  } m;

  /* readable, read message */
  memset(m.buffer, 0, RSCT_MAX_MESSAGE_LEN);
  rv=rsct_net_recv(m_socket, &m.msg);
  if (rv<0) {
    return rv;
  }
  else {
    /* handle message */
    rv=handleMessage(&(m.msg));
    if (rv<0) {
      return rv;
    }
  }

  return m.msg.header.type;
}



long CyberJackDialog::onIoRead(FXObject*, FXSelector, void *ptr) {
  int rv;

  rv=getAndHandleMessage();
  if (rv<0) {
    m_socket=-1;
    getApp()->stopModal(this,FALSE);
    hide();
  }

  return 1;
}



long CyberJackDialog::onTimeout(FXObject*, FXSelector, void *ptr) {
  time_t now;
  int diff;
  int newVal=0;

  now=time(NULL);
  diff=(int)difftime(now, m_startTime);
  if (diff<(int)m_keyTimeout)
    newVal=m_keyTimeout-diff;
  m_progressBar->setProgress(newVal);
  getApp()->addTimeout(this, ID_TIMER, 1000);

  return 1;
}



long CyberJackDialog::onSelKeyPress(FXObject*, FXSelector, void *ptr) {
  return 1;
}



long CyberJackDialog::onSelKeyRelease(FXObject*, FXSelector, void *ptr) {
  return 1;
}




