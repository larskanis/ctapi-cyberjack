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


#ifndef RSCT_DIALOG_P_H
#define RSCT_DIALOG_P_H


#include "dialog.h"




struct RSCT_DIALOG {
  uint32_t dialogId;
  int socket;
  int stages;
  int currentStage;
  unsigned int keyTimeout;
  char *title;
  char *textStage0;
  char *textStage1;
  char *textStage2;

  uint8_t cla;
  uint8_t ins;

  int numChars;
};




#endif




