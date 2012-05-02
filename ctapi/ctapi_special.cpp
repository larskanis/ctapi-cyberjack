/***************************************************************************
    begin       : Tue Mar 24 2009
    copyright   : (C) 2009 by Martin Preuss
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

/* this file is included from ctapi.cpp */



static int8_t _specialKeyUpdate(Ctapi_Context *ctx,
                                uint8_t *dad,
                                uint8_t *sad,
                                uint16_t cmd_len,
                                const uint8_t *cmd,
                                uint16_t *response_len,
                                uint8_t *response) {
  uint8_t dataLen;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Key update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  if (cmd[2] & 0x20)
    /* first block */
    ctx->dataToFlash.erase();
  if (cmd[2] & 0x40) {
    /* abort */
    ctx->dataToFlash.erase();
    response[0]=0x90;
    response[1]=0x00;
    *response_len=2;
    return CT_API_RV_OK;
  }

  /* determine length of data */
  if (cmd_len<5) {
    DEBUGP(ctx->ctn, "APDU too short");
    return CT_API_RV_ERR_INVALID;
  }

  /* add data */
  dataLen=cmd[4];
  if (dataLen)
    ctx->dataToFlash+=std::string((const char*) (cmd+5), dataLen);

  if (cmd[2] & 0x80) {
    uint32_t result;
    int rv;

    /* finished */
    DEBUGP(ctx->ctn, "Updating key (%d bytes)", ctx->dataToFlash.size());
    rv=ctx->reader->CtKeyUpdate((uint8_t*) ctx->dataToFlash.data(), ctx->dataToFlash.size(), &result);
    if (rv!=CJ_SUCCESS) {
      DEBUGP(ctx->ctn, "Unable to update the keys (%d / %d)\n", rv, result);
      return CT_API_RV_ERR_CT;
    }
  }

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialUploadMod(Ctapi_Context *ctx,
                                uint8_t *dad,
                                uint8_t *sad,
                                uint16_t cmd_len,
                                const uint8_t *cmd,
                                uint16_t *response_len,
                                uint8_t *response) {
  uint8_t dataLen;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  DEBUGP(ctx->ctn, "Module Upload");

  if (cmd[2] & 0x20)
    /* first block */
    ctx->dataToFlash.erase();
  if (cmd[2] & 0x40) {
    /* abort */
    ctx->dataToFlash.erase();
    response[0]=0x90;
    response[1]=0x00;
    *response_len=2;
    return CT_API_RV_OK;
  }

  /* determine length of data */
  if (cmd_len<5) {
    DEBUGP(ctx->ctn, "APDU too short");
    return CT_API_RV_ERR_INVALID;
  }

  /* add data */
  dataLen=cmd[4];
  if (dataLen)
    ctx->dataToFlash+=std::string((const char*) (cmd+5), dataLen);

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialUploadSig(Ctapi_Context *ctx,
                                uint8_t *dad,
                                uint8_t *sad,
                                uint16_t cmd_len,
                                const uint8_t *cmd,
                                uint16_t *response_len,
                                uint8_t *response) {
  uint8_t dataLen;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  DEBUGP(ctx->ctn, "Signature Upload");

  if (cmd[2] & 0x20)
    /* first block */
    ctx->signatureToFlash.erase();
  if (cmd[2] & 0x40) {
    /* abort */
    ctx->signatureToFlash.erase();
    response[0]=0x90;
    response[1]=0x00;
    *response_len=2;
    return CT_API_RV_OK;
  }

  /* determine length of data */
  if (cmd_len<5) {
    DEBUGP(ctx->ctn, "APDU too short");
    return CT_API_RV_ERR_INVALID;
  }

  /* add data */
  dataLen=cmd[4];
  if (dataLen)
    ctx->signatureToFlash+=std::string((const char*) (cmd+5), dataLen);

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialUploadFlash(Ctapi_Context *ctx,
                                  uint8_t *dad,
                                  uint8_t *sad,
                                  uint16_t cmd_len,
                                  const uint8_t *cmd,
                                  uint16_t *response_len,
                                  uint8_t *response) {
  uint32_t result;
  int rv;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  if (ctx->dataToFlash.size()<1 || ctx->signatureToFlash.size()<1) {
    DEBUGP(ctx->ctn, "Please upload module and signature first");
    return CT_API_RV_ERR_INVALID;
  }

  /* flash data */
  DEBUGP(ctx->ctn, "Flashing module (%d bytes)\n", ctx->dataToFlash.size());
  rv=ctx->reader->CtLoadModule((uint8_t*) ctx->dataToFlash.data(), ctx->dataToFlash.size(),
                               (uint8_t*) ctx->signatureToFlash.data(), ctx->signatureToFlash.size(),
                               &result);
  if (rv!=CJ_SUCCESS) {
    DEBUGP(ctx->ctn, "Unable to flash the module (%d / %d)\n", rv, result);
    return CT_API_RV_ERR_CT;
  }

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialUploadInfo(Ctapi_Context *ctx,
                                 uint8_t *dad,
                                 uint8_t *sad,
                                 uint16_t cmd_len,
                                 const uint8_t *cmd,
                                 uint16_t *response_len,
                                 uint8_t *response) {
  int rv;
  int lr;
  uint32_t estimatedUpdateTime=0;
  cj_ModuleInfo mi;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module info only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  if (ctx->dataToFlash.size()<1) {
    DEBUGP(ctx->ctn, "Please upload module first");
    return CT_API_RV_ERR_INVALID;
  }

  mi.SizeOfStruct=sizeof(cj_ModuleInfo);
  rv=ctx->reader->CtGetModuleInfoFromFile((uint8_t*) ctx->dataToFlash.data(), ctx->dataToFlash.size(),
                                          &mi, &estimatedUpdateTime);
  if (rv!=CJ_SUCCESS) {
    DEBUGP(ctx->ctn, "Unable to extract module info (%d)\n", rv);
    return CT_API_RV_ERR_CT;
  }

  if (*response_len<(2+sizeof(cj_ModuleInfo))) {
    DEBUGP(ctx->ctn, "Response buffer too short");
    return CT_API_RV_ERR_MEMORY;
  }

  memmove(response, (const void*) &mi, sizeof(cj_ModuleInfo));
  lr=sizeof(cj_ModuleInfo);
  response[lr++]=0x90;
  response[lr++]=0x00;
  *response_len=lr;
  return CT_API_RV_OK;
}



static int8_t _specialDeleteAllMods(Ctapi_Context *ctx,
                                    uint8_t *dad,
                                    uint8_t *sad,
                                    uint16_t cmd_len,
                                    const uint8_t *cmd,
                                    uint16_t *response_len,
                                    uint8_t *response) {
  uint32_t result;
  int rv;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  /* delete all modules */
  DEBUGP(ctx->ctn, "Deleting all modules");
  rv=ctx->reader->CtDeleteALLModules(&result);
  if (rv!=CJ_SUCCESS) {
    DEBUGP(ctx->ctn, "Unable to delete all modules (%d / %d)\n", rv, result);
    return CT_API_RV_ERR_CT;
  }

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialShowAuth(Ctapi_Context *ctx,
                               uint8_t *dad,
                               uint8_t *sad,
                               uint16_t cmd_len,
                               const uint8_t *cmd,
                               uint16_t *response_len,
                               uint8_t *response) {
  int rv;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module update only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  /* delete all modules */
  rv=ctx->reader->CtShowAuth();
  if (rv!=CJ_SUCCESS) {
    DEBUGP(ctx->ctn, "Unable to show auth info (%d)\n", rv);
    return CT_API_RV_ERR_CT;
  }

  response[0]=0x90;
  response[1]=0x00;
  *response_len=2;
  return CT_API_RV_OK;
}



static int8_t _specialGetModuleCount(Ctapi_Context *ctx,
                                     uint8_t *dad,
                                     uint8_t *sad,
                                     uint16_t cmd_len,
                                     const uint8_t *cmd,
                                     uint16_t *response_len,
                                     uint8_t *response) {
  int rv;

  if (*response_len<3) {
    DEBUGP(ctx->ctn, "Response buffer too short");
    return CT_API_RV_ERR_MEMORY;
  }

  if (ctx->reader!=NULL) {
    if (ctx->moduleCount==SCARD_AUTOALLOCATE) {
      if (ctx->moduleList)
	free(ctx->moduleList);
      ctx->moduleList=NULL;
      rv=ctx->reader->CtListModules(&(ctx->moduleCount), (cj_ModuleInfo*) &(ctx->moduleList));
      if (rv!=CJ_SUCCESS) {
	DEBUGP(ctx->ctn, "Unable to list module infos (%d)\n", rv);
	return CT_API_RV_ERR_CT;
      }
    }
    response[0]=(ctx->moduleCount<256)?ctx->moduleCount:255;
  }
  else
    /* no module information with older readers */
    response[0]=0;

  response[1]=0x90;
  response[2]=0x00;
  *response_len=3;
  return CT_API_RV_OK;
}



static int8_t _specialGetModuleInfo(Ctapi_Context *ctx,
                                    uint8_t *dad,
                                    uint8_t *sad,
                                    uint16_t cmd_len,
                                    const uint8_t *cmd,
                                    uint16_t *response_len,
                                    uint8_t *response) {
  int rv;
  unsigned int idx;
  int lr;

  if (ctx->reader==NULL) {
    DEBUGP(ctx->ctn, "Module info only available for ECA and newer");
    return CT_API_RV_ERR_INVALID;
  }

  if (ctx->moduleCount==SCARD_AUTOALLOCATE) {
    if (ctx->moduleList)
      free(ctx->moduleList);
    ctx->moduleList=NULL;
    /* this is really ugly */
    rv=ctx->reader->CtListModules(&(ctx->moduleCount), (cj_ModuleInfo*) &(ctx->moduleList));
    if (rv!=CJ_SUCCESS) {
      DEBUGP(ctx->ctn, "Unable to list module infos (%d)\n", rv);
      return CT_API_RV_ERR_CT;
    }
  }

  lr=0;

  idx=cmd[2]; /* p1 */
  if (idx>=ctx->moduleCount) {
    /* EOF met */
    response[lr++]=0x62;
    response[lr++]=0x82;
    *response_len=lr;
    return CT_API_RV_OK;
  }

  if (*response_len<(2+sizeof(cj_ModuleInfo))) {
    DEBUGP(ctx->ctn, "Response buffer too short");
    return CT_API_RV_ERR_MEMORY;
  }

  memmove(response, (const void*) &(ctx->moduleList[idx]), sizeof(cj_ModuleInfo));
  lr+=sizeof(cj_ModuleInfo);
  response[lr++]=0x90;
  response[lr++]=0x00;
  *response_len=lr;
  return CT_API_RV_OK;
}



static int8_t _specialGetReaderInfo(Ctapi_Context *ctx,
                                    uint8_t *dad,
                                    uint8_t *sad,
                                    uint16_t cmd_len,
                                    const uint8_t *cmd,
                                    uint16_t *response_len,
                                    uint8_t *response) {
  int rv;
  int lr;
  cj_ReaderInfo ri;

  lr=0;

  if (*response_len<(2+sizeof(cj_ReaderInfo))) {
    DEBUGP(ctx->ctn, "Response buffer too short");
    return CT_API_RV_ERR_MEMORY;
  }

  memset(&ri, 0, sizeof(cj_ReaderInfo));
  ri.SizeOfStruct=sizeof(cj_ReaderInfo);

  if (ctx->reader!=NULL) {
    rv=ctx->reader->CtGetReaderInfo(&ri);
    if (rv!=CJ_SUCCESS) {
      DEBUGP(ctx->ctn, "Unable to get reader info (%d)\n", rv);
      return CT_API_RV_ERR_CT;
    }
  }
#ifdef ENABLE_NONSERIAL
  else if (ctx->oldEcom!=NULL) {
    /* manufacture some reader info */
    DEBUGP(ctx->ctn, "Manufacturing reader info for e-com/pinpad");
    ri.PID=0x100;
    ri.ContentsMask|=RSCT_READER_MASK_PID;

    strncpy((char*) ri.ProductString, "cyberJack e-com/pinpad", sizeof(ri.ProductString)-1);
    ri.ContentsMask|=RSCT_READER_MASK_PRODUCT_STRING;

    ri.HardwareMask|=
      RSCT_READER_HARDWARE_MASK_ICC1 |
      RSCT_READER_HARDWARE_MASK_KEYPAD;
    ri.ContentsMask|=RSCT_READER_MASK_HARDWARE;
  }
  else if (ctx->ppa!=NULL) {
    /* manufacture some reader info */
    DEBUGP(ctx->ctn, "Manufacturing reader info for pinpad(a)");
    ri.PID=0x300;
    ri.ContentsMask|=RSCT_READER_MASK_PID;

    strncpy((char*) ri.ProductString, "cyberJack pinpad(a)", sizeof(ri.ProductString)-1);
    ri.ContentsMask|=RSCT_READER_MASK_PRODUCT_STRING;

    ri.HardwareMask|=
      RSCT_READER_HARDWARE_MASK_ICC1 |
      RSCT_READER_HARDWARE_MASK_KEYPAD;
    ri.ContentsMask|=RSCT_READER_MASK_HARDWARE;
  }
#endif
  else {
    DEBUGP(ctx->ctn, "No reader info");
    return CT_API_RV_ERR_CT;
  }

  memmove(response, (const void*) &ri, sizeof(cj_ReaderInfo));
  lr+=sizeof(cj_ReaderInfo);
  response[lr++]=0x90;
  response[lr++]=0x00;
  *response_len=lr;
  return CT_API_RV_OK;
}










int8_t CT_special(Ctapi_Context *ctx,
                  uint8_t *dad,
                  uint8_t *sad,
                  uint16_t cmd_len,
                  const uint8_t *cmd,
                  uint16_t *response_len,
                  uint8_t *response) {
  int8_t rv;
  uint8_t tmp;

  DEBUGP(ctx->ctn, "Received special command %02x %02x %02x %02x",
         cmd[0], cmd[1], cmd[2], cmd[3]);

  if (cmd[0]!=CJ_SPECIAL_CLA) {
    DEBUGP(ctx->ctn, "Special command but no special CLA byte (%02x)", cmd[0]);
    return CT_API_RV_ERR_INVALID;
  }

  switch(cmd[1]) {
  case CJ_SPECIAL_INS_KEYUPDATE:
    rv=_specialKeyUpdate(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_DELETEALLMODS:
    rv=_specialDeleteAllMods(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_UPLOADMOD:
    rv=_specialUploadMod(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_UPLOADSIG:
    rv=_specialUploadSig(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_UPLOADFLASH:
    rv=_specialUploadFlash(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_UPLOADINFO:
    rv=_specialUploadInfo(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_SHOWAUTH:
    rv=_specialShowAuth(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_GETMODCOUNT:
    rv=_specialGetModuleCount(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_GETMODINFO:
    rv=_specialGetModuleInfo(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;
  case CJ_SPECIAL_INS_GETREADERINFO:
    rv=_specialGetReaderInfo(ctx, dad, sad, cmd_len, cmd, response_len, response);
    break;

  default:
    DEBUGP(ctx->ctn, "Invalid special command (%02x)", cmd[1]);
    rv=CT_API_RV_ERR_INVALID;
  }

  /* switch sad and dad */
  tmp=*dad;
  *dad=*sad;
  *sad=tmp;

  /* done */
  return rv;
}




