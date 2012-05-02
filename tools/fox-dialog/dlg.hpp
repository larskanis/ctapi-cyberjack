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


#ifndef DLG_HPP
#define DLG_HPP


#include <fx.h>

#include <time.h>

#include "base/message.h"



class CyberJackDialog: public FXDialogBox {
  FXDECLARE(CyberJackDialog)

public:

  enum {
    ID_SOCKET=FXDialogBox::ID_LAST,
    ID_TIMER,
    ID_LAST
  };


  CyberJackDialog(int sock,
		  FXApp* a,
		  FXuint opts=DECOR_TITLE|DECOR_BORDER,
		  FXint x=0, FXint y=0, FXint w=0, FXint h=0,
		  FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0,
		  FXint hs=0, FXint vs=0);
  virtual ~CyberJackDialog();

  void setStage(int stage);
  void setNumDigits(int num);

  int getAndHandleMessage();
  void addSocketToInput();

  long onIoRead(FXObject*, FXSelector, void *ptr);
  long onTimeout(FXObject*, FXSelector, void *ptr);
  long onSelKeyPress(FXObject*, FXSelector, void *ptr);
  long onSelKeyRelease(FXObject*, FXSelector, void *ptr);

  void create();


protected:
  CyberJackDialog();

  int handleMessage(RSCT_MESSAGE *msg);
  int handleOpenDialog(RSCT_MESSAGE *msg);
  int handleSetStage(RSCT_MESSAGE *msg);
  int handleSetNumChars(RSCT_MESSAGE *msg);
  int handleCloseDialog(RSCT_MESSAGE *msg);

  static int m_lastDialogId;

  int m_socket;

  int m_currentStage;
  int m_stages;

  int m_numChars;

  unsigned int m_keyTimeout;
  time_t m_startTime;

  FXString m_textStage0;
  FXString m_textStage1;
  FXString m_textStage2;

  FXLabel *m_textLabel;
  FXLabel *m_enteredLabel;

  FXLabel *m_stageLabel;

  FXProgressBar *m_progressBar;

  FXImage *m_image;
  FXFont *m_fontSmall;
  FXFont *m_fontBig;

};




#endif






