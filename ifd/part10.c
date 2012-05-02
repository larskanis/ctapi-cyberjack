

#include <arpa/inet.h> /* for htonl */


/* uncomment to disable sending commands to the reader */
/*#define PART10_DISABLE_APDU*/


#define CLASS2_IOCTL_MAGIC 0x330000
#define IOCTL_FEATURE_VERIFY_PIN_DIRECT \
  SCARD_CTL_CODE(FEATURE_VERIFY_PIN_DIRECT + CLASS2_IOCTL_MAGIC)
#define IOCTL_FEATURE_MODIFY_PIN_DIRECT \
  SCARD_CTL_CODE(FEATURE_MODIFY_PIN_DIRECT + CLASS2_IOCTL_MAGIC)
#define IOCTL_FEATURE_MCT_READERDIRECT \
  SCARD_CTL_CODE(FEATURE_MCT_READERDIRECT + CLASS2_IOCTL_MAGIC)
#define IOCTL_FEATURE_MCT_READERUNIVERSAL \
  SCARD_CTL_CODE(FEATURE_MCT_UNIVERSAL + CLASS2_IOCTL_MAGIC)

#define WINDOWS_CTL_GET_FEATURE 0x313520
#define WINDOWS_CTL_GET_FEATURE2 0x42000c20




RESPONSECODE Part10GetFeatures(unsigned short ctn,
			       unsigned short slot,
			       PUCHAR TxBuffer,
			       DWORD TxLength,
			       PUCHAR RxBuffer,
			       DWORD RxLength,
			       PDWORD RxReturned){
  PCSC_TLV_STRUCTURE *pcsc_tlv = (PCSC_TLV_STRUCTURE *)RxBuffer;
  unsigned int len = 0;

  DEBUGP2(ctn, "GetFeatures called\n");
  /* WATCHOUT: When supporting a new TLV the size must be adjusted here */
  if (RxLength<4*sizeof(PCSC_TLV_STRUCTURE))
    return IFD_COMMUNICATION_ERROR;

  DEBUGP(ctn, "  Reporting Feature FEATURE_VERIFY_PIN_DIRECT (%08x)", IOCTL_FEATURE_VERIFY_PIN_DIRECT);
  pcsc_tlv->tag = FEATURE_VERIFY_PIN_DIRECT;
  pcsc_tlv->length = 0x04; /* always 0x04 */
  pcsc_tlv->value = htonl(IOCTL_FEATURE_VERIFY_PIN_DIRECT);
  pcsc_tlv++;
  len+=sizeof(PCSC_TLV_STRUCTURE);

  DEBUGP(ctn, "  Reporting Feature FEATURE_MODIFY_PIN_DIRECT (%08x)", IOCTL_FEATURE_MODIFY_PIN_DIRECT);
  pcsc_tlv->tag=FEATURE_MODIFY_PIN_DIRECT;
  pcsc_tlv->length=0x04; /* always 0x04 */
  pcsc_tlv->value=htonl(IOCTL_FEATURE_MODIFY_PIN_DIRECT);
  pcsc_tlv++;
  len+=sizeof(PCSC_TLV_STRUCTURE);

  DEBUGP(ctn, "  Reporting Feature FEATURE_MCT_READERDIRECT (%08x)", IOCTL_FEATURE_MCT_READERDIRECT);
  pcsc_tlv->tag = FEATURE_MCT_READERDIRECT;
  pcsc_tlv->length = 0x04; /* always 0x04 */
  pcsc_tlv->value = htonl(IOCTL_FEATURE_MCT_READERDIRECT);
  pcsc_tlv++;
  len+=sizeof(PCSC_TLV_STRUCTURE);

  DEBUGP(ctn, "  Reporting Feature FEATURE_MCT_UNIVERSAL (%08x)", IOCTL_FEATURE_MCT_READERUNIVERSAL);
  pcsc_tlv->tag = FEATURE_MCT_UNIVERSAL;
  pcsc_tlv->length = 0x04; /* always 0x04 */
  pcsc_tlv->value = htonl(IOCTL_FEATURE_MCT_READERUNIVERSAL);
  pcsc_tlv++;
  len+=sizeof(PCSC_TLV_STRUCTURE);

  *RxReturned=len;

  return IFD_SUCCESS;
}




static RESPONSECODE verifyStructToCtapi(unsigned short ctn,
					PIN_VERIFY_STRUCTURE *ps,
					uint8_t *buffer,
					int *pLen) {
  uint8_t nMktControlByte=0;
  uint8_t nMktPinPositionStart=0;
  uint8_t nMktPinLengthSizeInBits=0;
  /*uint8_t nPinLengthPositionInBits=0;*/
  uint8_t nMktPinBlockSizeInBytes=0;
  uint8_t nPinLenMin=0;
  uint8_t nPinLenMax=0;
  int bIsFormat2=0;
  int idx=0;
  int apduLength;

  nPinLenMin=((ps->wPINMaxExtraDigit)>>8) & 0xff;
  nPinLenMax=(ps->wPINMaxExtraDigit) & 0xff;
  apduLength=ntohl(ps->ulDataLength);
  if (apduLength>ps->ulDataLength)
    apduLength=ps->ulDataLength;

  /* add APDU */
  if (idx+5>=*pLen) {
    DEBUGP2(ctn, "Buffer too small (%d bytes at least needed)\n", idx+5);
    return IFD_COMMUNICATION_ERROR;
  }
  buffer[idx++]=0x20;
  buffer[idx++]=0x18;
  buffer[idx++]=0x01;
  buffer[idx++]=0x00;
  /* datalen, real size is still to be determined */
  buffer[idx++]=0xff;

  if (ps->bmFormatString & 0x04) {
    DEBUGP2(ctn, "Unsupported pin format <right justify>\n");
    return IFD_COMMUNICATION_ERROR;
  }

  switch(ps->bmFormatString & 0x03) {
  case 0x00: /* binary */
    DEBUGP2(ctn, "Unsupported pin format <binary>\n");
    return IFD_COMMUNICATION_ERROR;

  case 0x01: /* BCD */
    DEBUGP2(ctn, "Pin format: BCD\n");

    if (((ps->bmPINLengthFormat & 0x10)==0x00) && /* units in bits */
	((ps->bmPINLengthFormat & 0x0F)==0x04) && /* insert pos */
	/* -> Sysunits Bits -> 1 Nibbel */
	((ps->bmFormatString & 0x83)==0x81) && /* units in bytes */
	/* Pin: Format BCD */
	( apduLength==13)) { /* APDU bei Format 2 always 13 BKU 22.2.2008 */
      int i;

      /* Format 2 hat immer den ersten Nibble eine 0x2 */
      bIsFormat2=((ps->abData[5] & 0xf0) == 0x20)?1:0;
      for (i=6; i<apduLength && bIsFormat2; i++)
	bIsFormat2=((ps->abData[i] & 0xff) == 0xff)?1:0;
      if (bIsFormat2) {
	DEBUGP2(ctn, "Pin format: FPIN2\n");
      }

      if (bIsFormat2)
	nMktControlByte|=0x02;
      else
	nMktControlByte|=0x00;
    }
    else {
      nMktControlByte|=0x00;
    }
    break;

  case 0x02: /* ASCII oder T.50 */
    nMktControlByte|=0x01;
    break;

  case 0x03: /* RFU */
  default:
    DEBUGP2(ctn, "Unsupported pin format <%d>\n",
	   ps->bmFormatString & 0x03);
    return IFD_COMMUNICATION_ERROR;
  }

  nMktPinPositionStart= (ps->bmFormatString & 0x78) >> 3; /* 01111000 */
  if (!(ps->bmFormatString & 0x80) && /* units=bits */
      (nMktPinPositionStart!=0) &&    /* doesn't start at 0 */
      !bIsFormat2) {                  /* & is not format2 */
    /* System Units Bits -> Nicht unterstuetzt???? */
    DEBUGP2(ctn, "Unsupported system unit <bits>\n");
    return IFD_COMMUNICATION_ERROR;
  }

  /* CCID Counts from byte after LC, MKT from beginning of APDU to Card
   * so add  bytes CLA INS P1 P2 LC */
  if (bIsFormat2){
      if (nMktPinPositionStart==0 || nMktPinPositionStart==1) {
	  nMktPinPositionStart=6;
      }
      else {
	  nMktPinPositionStart+=5;
      };
  }
  else {
      nMktPinPositionStart+=6;
  };

  nMktPinLengthSizeInBits=(ps->bmPINBlockString & 0xF0) >> 4;
  nMktPinBlockSizeInBytes=(ps->bmPINBlockString & 0x0F);

  if (nMktPinLengthSizeInBits) {
    /* OK, there is a request to insert the PIN Len in the APDU
     * Dies wird nur im Format 2 unterestuetzt */
    if (!bIsFormat2) {
      /* System Units Bits -> Nicht unterstuetzt???? */
      DEBUGP2(ctn, "Unsupported PIN len position with coding <> format2\n");
      return IFD_COMMUNICATION_ERROR;
    }


    // OK, is format2, der PinBlock muss aber immer 6-7 Bytes lang sein....
    // Da Fehler im SCM u.a unterst?tzen wir auch die 0
    if (nMktPinBlockSizeInBytes!=0 &&
	nMktPinBlockSizeInBytes!=6 &&
	nMktPinBlockSizeInBytes!=7) {
	DEBUGP2(ctn, "FPIN2 but PIN block size is neither 6 nor 7\n");
	return IFD_COMMUNICATION_ERROR;
    }

    if (nMktPinLengthSizeInBits != 4) {
      // for format 2 must be the second nibble
      DEBUGP2(ctn, "FPIN2 pin len must be the second nibble\n");
      return IFD_COMMUNICATION_ERROR;
    }

    if (ps->bmPINBlockString & 0x10) {
      /* for format 2 must be the second nibble, so sysunits must be bits */
      DEBUGP2(ctn, "FPIN2 pin len must have system units=bits\n");
      return IFD_COMMUNICATION_ERROR;
    }
  }

  if  (ps->bEntryValidationCondition & 0x01){
    if (nPinLenMin==nPinLenMax){
      nMktControlByte |= (nPinLenMax << 4);
      DEBUGP2(ctn, "Fix pin len, setting control byte to %02x\n",
	     nMktControlByte);
    }
    else {
      DEBUGP2(ctn, "Warning: Fix pin len defined but PINmin!=PINmax\n");
    }
  }

  /* possibly create and add tag 80 */
  if (ps->bTimerOut) {
    if (idx+3>=*pLen) {
      DEBUGP2(ctn, "Buffer too small (%d bytes at least needed)\n", idx+3);
      return IFD_COMMUNICATION_ERROR;
    }
    buffer[idx++]=0x80;
    buffer[idx++]=0x01;
    buffer[idx++]=ps->bTimerOut;
  }

  /* create and add tag 52 */
  if (idx+apduLength+2>=*pLen) {
    DEBUGP2(ctn, "Buffer too small (%d bytes at least needed)\n",
	   idx+apduLength+2);
    return IFD_COMMUNICATION_ERROR;
  }
  buffer[idx++]=0x52;
  buffer[idx++]=apduLength+2;
  buffer[idx++]=nMktControlByte;
  buffer[idx++]=nMktPinPositionStart;
  memmove(buffer+idx, ps->abData, apduLength);
  idx+=apduLength;

  /* possibly create and add tag 51 */
  if (ifdh_status[ctn]->supportTag51) {
    DEBUGP2(ctn, "Adding tag 51\n");
    if (idx+4>=*pLen) {
      DEBUGP2(ctn, "Buffer too small (%d bytes at least needed)\n", idx+4);
      return IFD_COMMUNICATION_ERROR;
    }
    buffer[idx++]=0x51;
    buffer[idx++]=0x02;
    buffer[idx++]=nPinLenMin;
    buffer[idx++]=nPinLenMax;
  }

  /* set size in APDU */
  buffer[4]=idx-5;

  *pLen=idx;
  return IFD_SUCCESS;
}



static RESPONSECODE modifyStructToCtapi(unsigned short ctn,
					PIN_MODIFY_STRUCTURE *ps,
					uint8_t *buffer,
					int *pLen) {
  uint8_t nMktControlByte=0;
  uint8_t nMktPinPositionStart=0;
  uint8_t nMktOldPinLengthBytePosition=0;
  uint8_t nMktNewPinLengthBytePosition=0;
  uint8_t nMktPinLength=0;
  uint8_t nMktPinLengthBytePosition=0;
  uint8_t nPinLenMin=0;
  uint8_t nPinLenMax=0;
  int bIsFormat2=0;
  int idx=0;
  int apduLength;

  nPinLenMin=((ps->wPINMaxExtraDigit)>>8) & 0xff;
  nPinLenMax=(ps->wPINMaxExtraDigit) & 0xff;
  apduLength=ntohl(ps->ulDataLength);
  if (apduLength>ps->ulDataLength)
    apduLength=ps->ulDataLength;

  /* add APDU */
  if (idx+5>=*pLen)
    return IFD_COMMUNICATION_ERROR;
  buffer[idx++]=0x20;
  buffer[idx++]=0x19;
  buffer[idx++]=0x01;
  buffer[idx++]=0x00;
  /* datalen, real size is still to be determined */
  buffer[idx++]=0xff;

  if (ps->bmFormatString & 0x04) {
    DEBUGP2(ctn, "Unsupported pin format <right justify>\n");
    return IFD_COMMUNICATION_ERROR;
  }

  switch (ps->bmFormatString & 0x03) {
  case 0x00: /* binary */
    DEBUGP2(ctn, "Unsupported pin format <binary>\n");
    return IFD_COMMUNICATION_ERROR;

  case 0x01: /* BCD */
    DEBUGP2(ctn, "Pin format: BCD<%d>\n", ps->bmFormatString & 0x03);

    /* Format 2: Systemunit= BIT
     * Verschiebung 4 (Bit)
     * BCD
     * L?ge einer Format2 APDU ist immer genau 8 Byte */

    if (((ps->bmPINLengthFormat & 0x10) == 0x00) && /* PinLenInfo: System units are bits */
	((ps->bmPINLengthFormat & 0x0F) == 0x04) && /* PinLenInfo: insterted on 4' Position */
	/* -> Sysunits Bits -> 1 Nibble */
	((ps->bmFormatString    & 0x83) == 0x81) && /* Pin: System Units are Bytes */
	/* Pin: Format BCD */
	(( apduLength == 13 ) ||
	 (apduLength == 21))){      /* APDU bei Format 2 gem? BKU 22.2.2007 */
      /* Format 2 hat immer den ersten Nippel eine 0x2 */
      bIsFormat2=((ps->abData[5] & 0xf0) == 0x20)?1:0;
      if (bIsFormat2)
	nMktControlByte|=0x02;
      else
	nMktControlByte|=0x00;
    }
    else {
      nMktControlByte|=0x00;
    }
    break;

  case 0x02: // ASCII oder T.50
    DEBUGP2(ctn, "Pin format: ASCII\n");
    nMktControlByte|=0x01;
    break;

  case 0x03: // RFU
  default:
    DEBUGP2(ctn, "Unsupported pin format <%d>\n",
	   ps->bmFormatString & 0x03);
    return IFD_COMMUNICATION_ERROR;
  }

  nMktPinPositionStart= (ps->bmFormatString & 0x78) >> 3; /* 01111000 */
  nMktPinLength=ps->wPINMaxExtraDigit & 0xff;
  nMktPinLengthBytePosition=(ps->bmPINLengthFormat & 0x0F);

  if (ps->bmPINLengthFormat && 
      ((ps->bmPINLengthFormat & 0x10)==0)) {
    uint8_t nBitUnits=ps->bmPINLengthFormat & 0x0F;

    switch (nBitUnits) {
    case 4:
      if (!bIsFormat2) {
	DEBUGP2(ctn, "Unsupported system units (4 bits) with format other than fpin2\n");
	return IFD_COMMUNICATION_ERROR;
      }
      break;

    case 8:
      nMktPinLengthBytePosition= nMktPinLengthBytePosition / 8;
      break;

    default:
      DEBUGP2(ctn, "Unsupported system units (%d bits not mutliple of 8)\n", nBitUnits);
      return IFD_COMMUNICATION_ERROR;
    }
  }

  nMktPinLengthBytePosition+=6;

  if  (ps->bEntryValidationCondition & 0x01) {
    nMktControlByte |= (nMktPinLength << 4);
    DEBUGP2(ctn, "Fix pin len, setting control byte to %02x\n",
	   nMktControlByte);
  }

  nMktOldPinLengthBytePosition=ps->bInsertionOffsetOld+6;

  if (ps->bInsertionOffsetNew || !(ps->bConfirmPIN & 2)){
    nMktNewPinLengthBytePosition=ps->bInsertionOffsetNew+6;
  }
  else {
    /* Bei flexibler L?ge muss die Pos 0x00 sein. */
    DEBUGP2(ctn, "Flexible pin len detected\n");
    nMktNewPinLengthBytePosition=ps->bInsertionOffsetNew;
  }

  /* Gem?? mail BKU 7.9.2007 */
  if (!(ps->bConfirmPIN & 2)) { /* No Current PIN requested */
    nMktOldPinLengthBytePosition=nMktNewPinLengthBytePosition;
  }

  /* possibly create and add tag 80 */
  if (ps->bTimerOut) {
    if (idx+3>=*pLen)
      return IFD_COMMUNICATION_ERROR;
    buffer[idx++]=0x80;
    buffer[idx++]=0x01;
    buffer[idx++]=ps->bTimerOut;
  }

  /* create and add tag 52 */
  if (idx+apduLength+3>=*pLen)
    return IFD_COMMUNICATION_ERROR;
  buffer[idx++]=0x52;
  buffer[idx++]=apduLength+3;
  buffer[idx++]=nMktControlByte;
  buffer[idx++]=nMktOldPinLengthBytePosition;
  buffer[idx++]=nMktNewPinLengthBytePosition;
  memmove(buffer+idx, ps->abData, apduLength);
  idx+=apduLength;

  /* possibly create and add tags 51, 53 and 54 */
  if (ifdh_status[ctn]->supportTag51) {
    DEBUGP2(ctn, "Adding tag 51\n");
    if (idx+4>=*pLen)
      return IFD_COMMUNICATION_ERROR;
    buffer[idx++]=0x51;
    buffer[idx++]=0x02;
    buffer[idx++]=nPinLenMin;
    buffer[idx++]=nPinLenMax;

    /* Gemaess mail BKU 7.9.2007 */

    /* All Readers supporting Tag 0x51 do not hang with Tag 0x53 */
    buffer[idx++]=0x53;
    buffer[idx++]=0x03;
    buffer[idx++]=ps->bMsgIndex1;
    buffer[idx++]=ps->bMsgIndex2;
    buffer[idx++]=ps->bMsgIndex3;

    /* All Readers supporting Tag 0x51 do not hang with Tag 0x54 */
    buffer[idx++]=0x54;
    buffer[idx++]=0x01;
    buffer[idx++]=ps->bNumberMessage;

  }

  /* set size in APDU */
  buffer[4]=idx-5;

  *pLen=idx;
  return IFD_SUCCESS;
}




static RESPONSECODE Part10ExecCtrlApdu(unsigned short ctn,
                                       uint16_t lenc,
                                       uint8_t *command,
                                       PUCHAR RxBuffer,
                                       DWORD RxLength,
                                       PDWORD RxReturned) {
  int rv;

  DEBUGP2(ctn, "ExexCtrlApdu called\n");

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif
  if (ifdh_status[ctn] != NULL) {
    uint8_t sad, dad;
    char ret;
    uint16_t lenr;

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    dad=0x01;
    sad=0x02;
    if (RxLength>65535)
      lenr=65535;
    else
      lenr=RxLength;

    rsct_log_bytes(ctn, DEBUG_MASK_IFD,
		   __FILE__, __LINE__, __FUNCTION__,
		   "Sending", lenc, command);
#ifdef PART10_DISABLE_APDU
    DEBUGP2(ctn, "Apdu's disabled");
    ret=-127;
#else
    ret=CT_data(ctn,
		&dad, &sad,
		lenc, command,
		&lenr, RxBuffer);
#endif
    if (ret == 0) {
      *RxReturned=lenr;
      rv=IFD_SUCCESS;
      rsct_log_bytes(ctn, DEBUG_MASK_IFD,
		     __FILE__, __LINE__, __FUNCTION__,
		     "Response", lenr, RxBuffer);
    }
    else {
      *RxReturned=0;
      rv=IFD_COMMUNICATION_ERROR;
      DEBUGP2(ctn, "CTAPI returned error (%d)", ret);
    }
  }
  else {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    *RxReturned=0;
    rv=IFD_ICC_NOT_PRESENT;
  }

  return rv;
}



static RESPONSECODE Part10MctUniversal(unsigned short ctn,
                                       MCTUniversal_t *uni,
                                       PUCHAR RxBuffer,
                                       DWORD RxLength,
                                       PDWORD RxReturned) {
  int rv;
  MCTUniversal_t *response;

  DEBUGP2(ctn, "MctUniversal called\n");

  response=(MCTUniversal_t*)RxBuffer;
  /* need at least space for MCTUniversal_t with at least 2 bytes response */
  if (RxLength<(sizeof(MCTUniversal_t)+1)) {
    DEBUGP2(ctn, "Buffer too small");
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif
  if (ifdh_status[ctn] != NULL) {
    uint8_t sad, dad;
    char ret;
    uint16_t lenc;
    uint16_t lenr;

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    dad=uni->DAD;
    sad=uni->SAD;
    lenc=uni->BufferLength; /* TODO: is this really little endian? */
    if (RxLength>65535)
      lenr=65535-(sizeof(MCTUniversal_t)-1);
    else
      lenr=RxLength-(sizeof(MCTUniversal_t)-1);

    rsct_log_bytes(ctn, DEBUG_MASK_IFD,
		   __FILE__, __LINE__, __FUNCTION__,
                   "Sending", lenc, &(uni->buffer));
#ifdef PART10_DISABLE_APDU
    DEBUGP2(ctn, "Apdu's disabled");
    ret=-127;
#else
    ret=CT_data(ctn,
                &dad, &sad,
                lenc, &(uni->buffer),
		&lenr, &(response->buffer));
#endif
    if (ret == 0) {
      response->BufferLength=lenr;
      *RxReturned=(sizeof(MCTUniversal_t)-1)+lenr;
      response->SAD=sad;
      response->DAD=dad;
      rv=IFD_SUCCESS;
      rsct_log_bytes(ctn, DEBUG_MASK_IFD,
		     __FILE__, __LINE__, __FUNCTION__,
		     "Response", lenr, &(response->buffer));
    }
    else {
      *RxReturned=0;
      rv=IFD_COMMUNICATION_ERROR;
      DEBUGP2(ctn, "CTAPI returned error (%d)", ret);
    }
  }
  else {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    *RxReturned=0;
    rv=IFD_ICC_NOT_PRESENT;
  }

  return rv;
}



static RESPONSECODE Part10VerifyPinDirect(unsigned short ctn,
                                          unsigned short slot,
                                          PUCHAR TxBuffer,
                                          DWORD TxLength,
                                          PUCHAR RxBuffer,
                                          DWORD RxLength,
                                          PDWORD RxReturned){
  uint8_t buffer[256];
  int len=sizeof(buffer);
  RESPONSECODE rc;
  PIN_VERIFY_STRUCTURE *ps;

  DEBUGP2(ctn, "VerifyPin called\n");

  if (TxLength<sizeof(PIN_VERIFY_STRUCTURE))
    return IFD_COMMUNICATION_ERROR;
  ps=(PIN_VERIFY_STRUCTURE*) TxBuffer;
  rc=verifyStructToCtapi(ctn, ps, buffer, &len);
  if (rc!=IFD_SUCCESS) {
    DEBUGP2(ctn, "Failed to create APDU for VerifyPin (%08x)\n");
    return rc;
  }

  /* send APDU and retrieve answer */
  rc=Part10ExecCtrlApdu(ctn,
			len,
			buffer,
			RxBuffer,
			RxLength,
			RxReturned);
  if (rc!=IFD_SUCCESS) {
    DEBUGP2(ctn, "Error executing APDU\n");
  }

  return rc;
}



static RESPONSECODE Part10ModifyPinDirect(unsigned short ctn,
                                          unsigned short slot,
                                          PUCHAR TxBuffer,
                                          DWORD TxLength,
                                          PUCHAR RxBuffer,
                                          DWORD RxLength,
                                          PDWORD RxReturned){
  uint8_t buffer[256];
  int len=sizeof(buffer);
  RESPONSECODE rc;
  PIN_MODIFY_STRUCTURE *ps;

  DEBUGP2(ctn, "ModifyPin called\n");
  if (TxLength<sizeof(PIN_MODIFY_STRUCTURE))
    return IFD_COMMUNICATION_ERROR;
  ps=(PIN_MODIFY_STRUCTURE*) TxBuffer;
  rc=modifyStructToCtapi(ctn, ps, buffer, &len);
  if (rc!=IFD_SUCCESS) {
    DEBUGP2(ctn, "Failed to create APDU for ModifyPin\n");
    return rc;
  }

  /* send APDU and retrieve answer */
  rc=Part10ExecCtrlApdu(ctn,
			len,
			buffer,
			RxBuffer,
			RxLength,
			RxReturned);
  if (rc!=IFD_SUCCESS) {
    DEBUGP2(ctn, "Error executing APDU\n");
  }

  return rc;
}



RESPONSECODE Part10Control(unsigned short ctn,
                           unsigned short slot,
			   DWORD controlCode,
			   PUCHAR TxBuffer,
			   DWORD TxLength,
			   PUCHAR RxBuffer,
			   DWORD RxLength,
			   PDWORD RxReturned){

  DEBUGP2(ctn, "Part10Control called for code %08x\n", controlCode);

  switch(controlCode) {
  case CM_IOCTL_GET_FEATURE_REQUEST:
  case WINDOWS_CTL_GET_FEATURE:
  case WINDOWS_CTL_GET_FEATURE2:
    return Part10GetFeatures(ctn, slot,
			     TxBuffer, TxLength,
			     RxBuffer, RxLength, RxReturned);
  case IOCTL_FEATURE_VERIFY_PIN_DIRECT:
    return Part10VerifyPinDirect(ctn, slot,
				 TxBuffer, TxLength,
				 RxBuffer, RxLength, RxReturned);

  case IOCTL_FEATURE_MODIFY_PIN_DIRECT:
    return Part10ModifyPinDirect(ctn, slot,
				 TxBuffer, TxLength,
				 RxBuffer, RxLength, RxReturned);

  case IOCTL_FEATURE_MCT_READERDIRECT:
    /* directly send the command with DAD=01 and SAD=02 */
    DEBUGP2(ctn, "ReaderDirect called\n");
    return Part10ExecCtrlApdu(ctn,
			      TxLength, TxBuffer, 
			      RxBuffer, RxLength, RxReturned);

  case IOCTL_FEATURE_MCT_READERUNIVERSAL:
    DEBUGP2(ctn, "ReaderUniversal called\n");
    /* directly send the command with SAD/DAD in TxBuffer */
    if (TxLength<(sizeof(MCTUniversal_t)-1)) {
      DEBUGP2(ctn, "Too few bytes in TxBuffer (%d bytes)", TxLength);
      return IFD_COMMUNICATION_ERROR;
    }
    else {
      MCTUniversal_t *uni=(MCTUniversal_t*) TxBuffer;

      /* check overall length including data length */
      if (TxLength<(sizeof(MCTUniversal_t)-1+uni->BufferLength)) {
        DEBUGP2(ctn, "Too few bytes in TxBuffer (%d bytes, %d bytes data)",
                TxLength, uni->BufferLength);
        return IFD_COMMUNICATION_ERROR;
      }

      return Part10MctUniversal(ctn, uni,
                                RxBuffer, RxLength, RxReturned);
    }

  default:
    DEBUGP2(ctn, "Unknown ioctl %0X, forwarding to CTAPI", controlCode);
    if (1) {
      uint32_t res;
      uint32_t rl;

      rl=RxLength;
      res=rsct_ifd_ioctl(ctn,
			 controlCode,
			 TxBuffer, TxLength,
			 RxBuffer, &rl);
      *RxReturned=rl;
      return res;
    }
  }
}


