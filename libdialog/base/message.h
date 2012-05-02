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

#ifndef RSCT_MESSAGE_H
#define RSCT_MESSAGE_H


#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


#if defined (__APPLE__) | defined (sun)
# pragma pack(1)
#else
# pragma pack(push, 1)
#endif


#define RSCT_MAX_MESSAGE_LEN (65*1024)
#define RSCT_MAX_BUFFER_LEN  4096




enum RSCT_MESSAGE_COMMANDS {
  RSCT_Message_Command_OpenDialog,
  RSCT_Message_Command_CloseDialog,
  RSCT_Message_Command_SetStage,
  RSCT_Message_Command_SetCharNum
};



struct RSCT_MESSAGE_OPENDIALOG {
  uint32_t dialogId;
  uint32_t stages;
  uint8_t cla;
  uint8_t ins;
  uint8_t keyTimeout;
  char title[128];
  char textStage0[512];
  char textStage1[512];
  char textStage2[512];
  int8_t result;
};
typedef struct RSCT_MESSAGE_OPENDIALOG RSCT_MESSAGE_OPENDIALOG;



struct RSCT_MESSAGE_CLOSEDIALOG {
  uint32_t dialogId;
  int8_t result;
};
typedef struct RSCT_MESSAGE_CLOSEDIALOG RSCT_MESSAGE_CLOSEDIALOG;



struct RSCT_MESSAGE_SETSTAGE {
  uint32_t dialogId;
  uint32_t stage;
  int8_t result;
};
typedef struct RSCT_MESSAGE_SETSTAGE RSCT_MESSAGE_SETSTAGE;



struct RSCT_MESSAGE_SETCHARNUM {
  uint32_t dialogId;
  uint32_t charNum;
  int8_t beep;
  int8_t result;
};
typedef struct RSCT_MESSAGE_SETCHARNUM RSCT_MESSAGE_SETCHARNUM;



struct RSCT_MESSAGE_HEADER {
  uint8_t type;
  uint32_t len;
};
typedef struct RSCT_MESSAGE_HEADER RSCT_MESSAGE_HEADER;



struct RSCT_MESSAGE {
  RSCT_MESSAGE_HEADER header;
  union {
    RSCT_MESSAGE_OPENDIALOG openDialog;
    RSCT_MESSAGE_CLOSEDIALOG closeDialog;
    RSCT_MESSAGE_SETSTAGE setStage;
    RSCT_MESSAGE_SETCHARNUM setCharNum;
  };
};
typedef struct RSCT_MESSAGE RSCT_MESSAGE;


#if defined (__APPLE__) | defined (sun)
# pragma pack()
#else
# pragma pack(pop)
#endif


#define RSCT_MSG_SIZE(tp) (\
  sizeof(struct RSCT_MESSAGE_HEADER)+\
  sizeof(tp))


const char *RSCT_MessageTypeToString(int t);


#ifdef __cplusplus
}
#endif


#endif

