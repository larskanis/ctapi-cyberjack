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


#ifndef RSCT_DIALOG_H
#define RSCT_DIALOG_H


#include "message.h"



#ifdef __cplusplus
extern "C" {
#endif



typedef struct RSCT_DIALOG RSCT_DIALOG;

#define RSCT_DIALOG_MAX_STAGES 3


RSCT_DIALOG *rsct_dialog_new(const char *title, int stages,
			     uint8_t cla, uint8_t ins,
			     int keyTimeout,
			     const char *textStage0,
			     const char *textStage1,
			     const char *textStage2);
void rsct_dialog_free(RSCT_DIALOG *dlg);

uint8_t rsct_dialog_get_cla(const RSCT_DIALOG *dlg);
uint8_t rsct_dialog_get_ins(const RSCT_DIALOG *dlg);

int rsct_dialog_open(RSCT_DIALOG *dlg);
void rsct_dialog_close(RSCT_DIALOG *dlg);

int rsct_dialog_set_stage(RSCT_DIALOG *dlg, int stage);
int rsct_dialog_get_stage(const RSCT_DIALOG *dlg);

int rsct_dialog_get_stages(const RSCT_DIALOG *dlg);

int rsct_dialog_set_char_num(RSCT_DIALOG *dlg, int charNum, int beep);
int rsct_dialog_get_char_num(const RSCT_DIALOG *dlg);



int rsct_dialog_get_socket(const RSCT_DIALOG *dlg);

#ifdef __cplusplus
}
#endif

#endif




