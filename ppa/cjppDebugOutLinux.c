/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2004  REINER SCT
 * Author: Harald Welte
 * Support: support@reiner-sct.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * File: cjppDebugOutLinux.c
 * CVS: $Id: cjppDebugOutLinux.c 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "cjppa.h"
#include "cjpp.h"
#include "cjppa_linux.h"


  static unsigned int debug_level = 0;

void cjppDebugSetLevel(int level){
  debug_level = level;
}



void cjppDebugCommand(CCID_DEVICE hDevice, unsigned char *dad,
                      unsigned char *sad, unsigned short lenc,
		      const unsigned char *cmd,
                      unsigned short *lenr,
                      unsigned char *response){
#if 0
  unsigned char *ptr=cmd;

  if (debug_level == 0)
    return;

  rsct_log(CT_FLAGS_DEBUG_TRANSFER,
	   __FILE__, __LINE__, __FUNCTION__,
	   "PC->CYBJCK: SAD=%02x, DAD=%02x\n",
	   *sad, *dad);
  rsct_log_bytes(CT_FLAGS_DEBUG_TRANSFER,
		 __FILE__, __LINE__, __FUNCTION__,
		 "PC->CYBJCK", lenc, ptr);
#endif
}



void cjppDebugResponse(CCID_DEVICE hDevice, unsigned char *dad,
		       unsigned char *sad, unsigned short lenc,
                       const unsigned char *cmd, unsigned short *lenr,
		       unsigned char *response, char res){
#if 0
  unsigned char *ptr = response;

  if (debug_level == 0)
    return;

  rsct_log(CT_FLAGS_DEBUG_TRANSFER,
	   __FILE__, __LINE__, __FUNCTION__,
	   "CYBJCK->PC: res=%02x, SAD=%02x, DAD=%02x\n",
	   res, *sad, *dad);
  if (res==0) {
    rsct_log_bytes(CT_FLAGS_DEBUG_TRANSFER,
		   __FILE__, __LINE__, __FUNCTION__,
		   "CYBJCK->PC", *lenr, ptr);
  }
#endif
}



void cjppDebugOut(cjppHANDLE hDevice, unsigned char *Caption,
		  unsigned short lenc, unsigned char *cmd)
{
#if 0
   if(DebugOut!=NULL)
   {
      HWND pcWin;
	   pcWin=FindWindow (NULL,"SiiDebugMon2");
      if(pcWin)
      {
         int i=0;
         unsigned char *ptr=cmd;
         DWORD cl=GetTickCount();
         ULONG			nBytesToCopy=0;
         COPYDATASTRUCT	cds;
         CWinMsg		 	botschaft;
         memset(&botschaft,0,sizeof(botschaft));
         sprintf(botschaft.m_aText,"%s\n",Caption);
         botschaft.m_aText[sizeof (botschaft.m_aText)-1]=0x00;
         botschaft.m_nTime=cl;
         sprintf(botschaft.m_aSource,"CJPPA_DOUT_%#02d",(int)(hDevice->reader_id));
         cds.dwData = 0;
         cds.cbData = sizeof (botschaft);
         cds.lpData = &botschaft;
         SendMessage (pcWin,WM_COPYDATA,0, (LPARAM) &cds);
         strcpy(botschaft.m_aText,"        DATA: ");
         for(i=0;i<lenc;i++)
         {
            sprintf(botschaft.m_aText+14+(i%16)*3,"%02X ",(int)*ptr++);
            if((i%16)==15)
            {
               strcat(botschaft.m_aText,"\n");
               SendMessage (pcWin,WM_COPYDATA,0, (LPARAM) &cds);
               strcpy(botschaft.m_aText,"              ");
            }
         }
         if((lenc%16)!=0)
         {
            strcat(botschaft.m_aText,"\n");
            SendMessage (pcWin,WM_COPYDATA,0, (LPARAM) &cds);
         }
		}
   }
#endif
}


