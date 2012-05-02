
#include "Platform.h"


#include "cjppa.h"
#include "cjpp.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

extern int SyncInterpreter(HANDLE hDevice,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response);


static int GetBits(unsigned char Value)
{
   int Res=0;
   while(Value)
   {
      if(Value & 1)
        Res++;
      Value>>=1;
   }
   return Res;
}

static unsigned char *GetTag(unsigned char *start,int len,unsigned char tagvalue,int *taglen)
{
   unsigned char tag;
   unsigned char tlen;
   *taglen=0;
   while(len>2)
   {
      tag=*start++;
      tlen=*start++;
      if(tag==tagvalue)
      {
         *taglen=tlen;
         return start;
      }
      start+=tlen;
      len-=tlen+2;
   }
   return NULL;
}

static int check_len(unsigned char *atr,unsigned short buf_len,unsigned char **historical,unsigned char *hist_len)
{
   unsigned char *ptr;
   unsigned char len;
   unsigned char len1;
   unsigned char v;
   int t1=0;

   ptr=atr+1;
   v=*ptr;
   len1=(unsigned char)((*hist_len=(unsigned char)(v & 0x0f))+2);
   len=0;
   do
   {
      v=(unsigned char)GetBits((unsigned char)((*ptr) & 0xf0));
      len+=v;
      if(buf_len>=len && ((*ptr)&0x80))
      {
         ptr+=v;
         if(!t1 && ((*ptr) & 0x0f)!=0)
         {
            len1++;
            t1=1;
         }
      }
      else
      {
         *historical=ptr+v+1;
         break;
      }
   }while(buf_len>len);
   if(t1)
   {
     int i;
     unsigned char lrc=0;
     for(i=1;i<buf_len;i++)
     {
       lrc^=atr[i];
     }
     if(lrc)
        return 0;
   }
   else if(buf_len!=len+len1)
      return 2;
   return 1;
}

static int ctBcsReset(HANDLE hDevice,unsigned char *ATR,unsigned char *ATR_len,unsigned char *historical,unsigned char *hist_len)
{
  unsigned char atr[260];
  int len=sizeof(atr);
  unsigned char TA1;
  unsigned char TC1;
//  unsigned char TA2;
  unsigned char TC2;
  unsigned char TA3;
  unsigned char TB3;
  unsigned char TC3;
  unsigned char Dx;
  unsigned char *ptr;
  unsigned char *hist;
  int specific;
  int hasTA1;
  int error;
  int warm;
  int protocol=0;
  *hist_len=0;
  hist=ATR;
  warm=0;
  do
  {
     TA1=0x11;
     TC1=0;
     specific=0;
     hasTA1=0;
     TC2=10;
     protocol=0x01;
     error=0;
     TA3=0x20;
     TB3=0x45;
     TC3=0;

     switch(cjccid_iccPowerOn(hDevice,CCID_VOLTAGE_50,atr,&len))
     {
       case CJPP_SUCCESS:

          if(len > 0 && (atr[0]==0x3b || atr[0]==0x3f))
          {
              if(check_len(atr,(unsigned short)len,&hist,hist_len)==1)
              {
                 ptr=atr+1;
                 Dx=*ptr++;
                 if(Dx & 0x10)
                 {
                    TA1=*ptr++;
                    hasTA1=1;
                 }
                 if(Dx & 0x20)
                 {
                    if(*ptr!=0 && !warm)
                    {
                       error=1;
                    }
                    ptr++;
                 }
                 else
                 {
                    if(!warm)
                        error=1;
                 }
                 if(Dx & 0x40)
                 {
                    TC1=*ptr++;
                 }
                 if(Dx & 0x80)
                 {
                    Dx=*ptr++;
                    protocol=1<<(Dx & 0x0f);
                    if((Dx & 0x0f)>1)
                       error=1;
                    if(Dx & 0x10)
                    {
                       if(*ptr++ & 0x10)
                          error=1;
                       specific=1;
                    }
                    if(Dx & 0x20)
                    {
                       error=1;
                       ptr++;
                    }
                    if(Dx & 0x40)
                    {
                       if((TC2=*ptr++)==0)
                          error=1;
                    }
                    if(Dx & 0x80)
                    {
                       Dx=*ptr++;
                       if((Dx & 0x0f)!=1 && (protocol!=1 || (Dx & 0x0e)!=0x0e))
                          error=1;
                       protocol|=1<<(Dx & 0x0f);
                       if((Dx & 0x0f)==1)
                       {
                          if(Dx & 0x10)
                          {
                             if((TA3=*ptr++)<0x10 || TA3==0xff)
                                error=1;
                          }
                          if(Dx & 0x20)
                          {
                             if((TB3=*ptr++)>0x45 || (TB3&0x0f)>0x05 || ((1<<(TB3&0x0f))<=TC1+1 && TC1!=0xff))
      /*                             if(Card->norm==EMV)
                                   error=1;*/;
                          }
                          else
                             error=1;
                          if(Dx & 0x40)
                          {
                             TC3=*ptr;
                             if(*ptr++!=0)
                                error=1;
                          }
                       }
                    }
						  else if(protocol&2)
							  error=1;
                 }
              }
              else if(check_len(atr,(unsigned short)len,&hist,hist_len)==2)
              {
                 error=1;
              }
              else
              {
                 error=2;
              }
              if(error==2 || (error==1 && warm))
              {
                 cjccid_iccPowerOff(hDevice);
              }
              else if(error==1)
              {
                 warm++;
              }
          }
          else
          {
              /*Synchron*/
                 ((cjccidHANDLE)hDevice)->IFSC=0;
				 error=3;
				 if(len!=4)
				    error=2;
			    else
				 {
					 if(atr[0]==0xa2)
						 ((cjccidHANDLE)hDevice)->Protokoll=10;
					 if(atr[0]==0x92)
						 ((cjccidHANDLE)hDevice)->Protokoll=9;
					 if((atr[0] & 0xf0)==0x80)
					 {
						 ((cjccidHANDLE)hDevice)->Protokoll=8;
						 ((cjccidHANDLE)hDevice)->iic_pagesize=1;
						 ((cjccidHANDLE)hDevice)->iic_deviceaddr=0xa0;
						 ((cjccidHANDLE)hDevice)->iic_offset_bytes=1;
					 }

				 }
          }
          break;
       default:;
          error=2;
     }
  }while(warm==1 && error==1);
  if(error==0)
  {
     int in_len;
     unsigned char rbuffer[5];
	  if((TA1 & 0x0f)==0x06) // D factor of 32 is to fast
	  {
		  if(TA1!=0x96 || (protocol&1) || TC1==0xff)
		     TA1=(TA1 & 0xf0) | 0x05;
	  }
     CJPP_TEST(cjccid_SetParameters(hDevice,(unsigned char)((protocol&1)?0:1),TA1,0,TC1,(unsigned char)((protocol&1)?TC2:TB3)));
     if(specific==0 && hasTA1)
     {
        unsigned char sbuffer[4];
        in_len=sizeof(rbuffer);

        sbuffer[0]=0xff;
        if(protocol&1)
           sbuffer[1]=0x10;
        else
           sbuffer[1]=0x11;
        sbuffer[2]=TA1;
        sbuffer[3]=(unsigned char)(0xff ^ TA1 ^ sbuffer[1]);
        if(cjccid_XfrBlock(hDevice,0,sbuffer,4,rbuffer,&in_len,0)!=CJPP_SUCCESS)
        {
           CJPP_TEST(cjccid_iccPowerOff(hDevice));
           return CJPP_ERR_TIMEOUT;
        }
        if(in_len<3 || rbuffer[0]!=0xff || (rbuffer[1] & 0x0f)!=(sbuffer[1] & 0x0f) ||
          (in_len==3 && ((rbuffer[1] & 0xf0)!=0 || (0xff ^ rbuffer[1])!=rbuffer[2])) ||
          (in_len==4 && ((rbuffer[1] & 0xf0)!=0x10 || (0xff ^ rbuffer[1] ^ rbuffer[2])!=rbuffer[3])))
        {
           CJPP_TEST(cjccid_iccPowerOff(hDevice));
           return CJPP_ERR_PROT;
        }
        else
        {
          if(in_len==3)
          {
             TA1=0x11;
             CJPP_TEST(cjccid_SetParameters(hDevice,(unsigned char)((protocol&1)?0:1),TA1,0,TC1,(unsigned char)((protocol&1)?TC2:TB3)));
          }
        }
     }
     if(protocol&1)
        ((cjccidHANDLE)hDevice)->Protokoll=0;
     else
     {
        ((cjccidHANDLE)hDevice)->Protokoll=1;
        for(error=0;error<3;error++)
        {
          in_len=sizeof(rbuffer);
          switch(cjccid_XfrBlock(hDevice,0,(unsigned char *)"\x00\xC1\x01\xfe\x3e",5,rbuffer,&in_len,0))
          {
             case CJPP_SUCCESS:
                if(in_len==5 && memcmp("\x00\xE1\x01\xfe\x1e",rbuffer,5)==0)
                   error=4;
                break;
             case CJPP_ERR_PARITY:
             case CJPP_ERR_TIMEOUT:
             case CJPP_ERR_LEN:
             case CJPP_ERR_RBUFFER_TO_SMALL:
                break;
             default:
                return CJPP_ERR_DEVICE_LOST;

          }
        }
        if(error==3)
        {
           CJPP_TEST(cjccid_iccPowerOff(hDevice));
           return CJPP_ERR_PROT;
        }
        ((cjccidHANDLE)hDevice)->IFSC=TA3;
        ((cjccidHANDLE)hDevice)->PCB_seq=0;
        ((cjccidHANDLE)hDevice)->EDC=(unsigned char)(TC3&1);
     }
  }
  else if(error!=3)
  {
     CJPP_TEST(cjccid_iccPowerOff(hDevice));
     return CJPP_ERR_TIMEOUT;
  }

  if(*ATR_len<len)
     return CJPP_ERR_RBUFFER_TO_SMALL;
  memcpy(ATR,atr,(*ATR_len<len)?*ATR_len:len);
  memcpy(historical,hist,*hist_len);
  *ATR_len=len;
  return CJPP_SUCCESS;
}


static int APDU2TPDU_T0(HANDLE hDevice,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response)
{
   int Result;
   unsigned char rbuffer[258];
   int rlen=sizeof(rbuffer);
   if(lenc==4)
   {
      unsigned char sbuffer[5];
      memcpy(sbuffer,cmd,4);
      sbuffer[4]=0;
      if((Result=cjccid_XfrBlock(hDevice,0,sbuffer,5,rbuffer,&rlen,0))!=CJPP_SUCCESS)
      {
         cjccid_iccPowerOff(hDevice);
         *lenr=0;
         return Result;
      }
      if(rlen>*lenr)
      {
         *lenr=0;
         return CJPP_ERR_RBUFFER_TO_SMALL;
      }
      memcpy(response,rbuffer,rlen);
      *lenr=rlen;
      return CJPP_SUCCESS;
   }
   else if(lenc==5)
   {
      if((Result=cjccid_XfrBlock(hDevice,0,cmd,5,rbuffer,&rlen,0))!=CJPP_SUCCESS)
      {
         cjccid_iccPowerOff(hDevice);
         *lenr=0;
         return Result;
      }
      if(rlen==2 && rbuffer[0]==0x6C)
      {
         unsigned char sbuffer[5];
         memcpy(sbuffer,cmd,4);
         sbuffer[4]=rbuffer[1];
         rlen=sizeof(rbuffer);
         if((Result=cjccid_XfrBlock(hDevice,0,sbuffer,5,rbuffer,&rlen,0))!=CJPP_SUCCESS)
         {
            cjccid_iccPowerOff(hDevice);
            *lenr=0;
            return Result;
         }
         if(rlen==2)
         {
            memcpy(response,rbuffer,2);
            *lenr=2;
         }
         else if((cmd[4]!=0 && sbuffer[4]>cmd[4]))
         {
            if(*lenr<2+cmd[4])
            {
               *lenr=0;
               return CJPP_ERR_RBUFFER_TO_SMALL;
            }
            memcpy(response,rbuffer,cmd[4]);
            memcpy(response,rbuffer+sbuffer[4],2);
            *lenr=cmd[4]+2;
            return CJPP_SUCCESS;
         }
         else
         {
            if(*lenr<2+sbuffer[4])
            {
               *lenr=0;
               return CJPP_ERR_RBUFFER_TO_SMALL;
            }
            memcpy(response,rbuffer,2+sbuffer[4]);
            *lenr=sbuffer[4]+2;
            return CJPP_SUCCESS;
         }
      }
      if(rlen>*lenr)
      {
         *lenr=0;
         return CJPP_ERR_RBUFFER_TO_SMALL;
      }
      memcpy(response,rbuffer,rlen);
      *lenr=rlen;
      return CJPP_SUCCESS;
   }
   else if(lenc==5+cmd[4] && cmd[4]!=0)
   {
      if((Result=cjccid_XfrBlock(hDevice,0,cmd,lenc,rbuffer,&rlen,0))!=CJPP_SUCCESS)
      {
         cjccid_iccPowerOff(hDevice);
         *lenr=0;
         return Result;
      }
      if(rlen>*lenr)
      {
         *lenr=0;
         return CJPP_ERR_RBUFFER_TO_SMALL;
      }
      memcpy(response,rbuffer,rlen);
      *lenr=rlen;
      return CJPP_SUCCESS;
   }
   else if(lenc==6+cmd[4] && cmd[4]!=0)
   {
      unsigned char sbuffer[5];
      unsigned int tot_size=0;
      unsigned int rest_size=sizeof(rbuffer);
      unsigned char *rptr=rbuffer;
      if((Result=cjccid_XfrBlock(hDevice,0,cmd,lenc-1,rbuffer,&rlen,0))!=CJPP_SUCCESS)
      {
         cjccid_iccPowerOff(hDevice);
         *lenr=0;
         return Result;
      }
      sbuffer[0]=cmd[0];
      memcpy(sbuffer+1,"\xC0\x00\x00",3);
      rptr+=rlen-2;
      rest_size-=rlen-2;
      tot_size+=rlen-2;
      if(rlen==2 && (((rbuffer[0] & 0xf0)==0x90 && (rbuffer[0]!=0x90 || rbuffer[1]!=0x00)) || rbuffer[0]==0x62 || rbuffer[0]==0x63))
      {
         sbuffer[4]=cmd[lenc-1];
         rlen=rest_size;
         if((Result=cjccid_XfrBlock(hDevice,0,sbuffer,5,rbuffer,&rlen,0))!=CJPP_SUCCESS)
         {
            cjccid_iccPowerOff(hDevice);
            *lenr=0;
            return Result;
         }
         rest_size-=rlen-2;
         tot_size+=rlen-2;
      }
      if(rlen >=2 && (rptr[rlen-2]==0x61 || rptr[rlen-2]==0x6C))
      {
         while(rlen >=2 && (rptr[rlen-2]==0x61 || rptr[rlen-2]==0x6C))
         {
            rptr+=rlen-2;
            if(cmd[lenc-1]<rptr[rlen-1] && cmd[lenc-1]!=0)
               sbuffer[4]=cmd[lenc-1];
            else
               sbuffer[4]=rptr[1];
            rlen=rest_size;
            if((Result=cjccid_XfrBlock(hDevice,0,sbuffer,5,rptr,&rlen,0))!=CJPP_SUCCESS)
            {
               cjccid_iccPowerOff(hDevice);
               *lenr=0;
               return Result;
            }
            rest_size-=rlen-2;
            tot_size+=rlen-2;
         }
      }
      if(tot_size+2>*lenr)
      {
         *lenr=0;
         return CJPP_ERR_RBUFFER_TO_SMALL;
      }
      memcpy(response,rbuffer,tot_size+2);
      *lenr=tot_size+2;
      return CJPP_SUCCESS;
   }
/*   else if(lenc==7 && cmd[4]==0)
   {
   }
   else if(lenc==7+(((unsigned short)cmd[5])<<8)+cmd[6] && cmd[4]==0 && lenc!=7)
   {
   }
   else if(lenc==9+(((unsigned short)cmd[5])<<8)+cmd[6] && cmd[4]==0 && lenc!=9)
   {
   }*/
   else
      return CJPP_ERR_LEN;
}

static int APDU2TPDU_T1(HANDLE hDevice,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response)
{
   unsigned char lrc;
   unsigned short crc;
   int i;
   int error;
   unsigned char *sblock;
   unsigned char rblock[6];
   unsigned short slen;
   unsigned char sbuffer[259];
   unsigned char rbuffer[259];
   unsigned char xwt=0;
   int rlen;
   unsigned char INF;
   int maxresp;
   int Result=CJPP_SUCCESS;
   error=0;
   maxresp=*lenr;
   *lenr=0;
   while(lenc)
   {
      INF=(lenc>((cjccidHANDLE)hDevice)->IFSC)?((cjccidHANDLE)hDevice)->IFSC:lenc;
      sbuffer[0]=0;
      if(((cjccidHANDLE)hDevice)->PCB_seq&0x01)
         sbuffer[1]=0x40;
      else
         sbuffer[1]=0;
      if(lenc>INF)
         sbuffer[1]|=0x20;
      sbuffer[2]=INF;
      memcpy(sbuffer+3,cmd,INF);
      sblock=sbuffer;
      slen=(unsigned short)(INF+3);
      for(;;)
      {
         if(((cjccidHANDLE)hDevice)->EDC==0)
         {
            lrc=0;
            for(i=0;i<slen;i++)
            {
               lrc^=sblock[i];
            }
            sblock[slen++]=lrc;
         }
         else
         {
            /*crc berechnung*/
            crc=0;
            sblock[slen++]=(unsigned char)(crc>>8);
            sblock[slen++]=(unsigned char)crc;
         }
         rlen=sizeof(rbuffer);
         Result=cjccid_XfrBlock(hDevice,xwt,sblock,slen,rbuffer,&rlen,0);
         if(Result==CJPP_SUCCESS || Result==CJPP_ERR_PARITY || Result==CJPP_ERR_TIMEOUT)
         {
            if(Result==CJPP_SUCCESS)
            {
               if(((cjccidHANDLE)hDevice)->EDC==0)
               {
                  lrc=0;
                  for(i=0;i<rlen;i++)
                  {
                     lrc^=rbuffer[i];
                  }
                  if(lrc)
                     Result=CJPP_ERR_PARITY;
               }
               else
               {
                  /*crc berechnung*/
               }
            }
            if((sblock[1]&0xe0)==0xc0)
            {
               if(Result==CJPP_ERR_PARITY || rbuffer[0]!=0 || (rbuffer[1]&0xe0)!=0xe0 || (rbuffer[1]&0xdf)!=sblock[1] || rbuffer[2]!=sblock[2] || memcmp(rbuffer+3,sblock+3,sblock[2])!=0)
               {
                  if(++error>2)
                  {
                     cjccid_iccPowerOff(hDevice);
                     Result=CJPP_ERR_PROT;
                     break;
                  }
               }
               else
               {
                  sblock=sbuffer;
                  slen=(unsigned short)(INF+3);
                  error=0;
               }
            }
            else
            {
               if(Result==CJPP_SUCCESS)
	       {

                  if (rbuffer[0]!=0 ||  //Falsches NAD
                     ((rbuffer[1]&0xc0)==0x80 && (rbuffer[2]!=0 || (rbuffer[1]&0x20)==0x20)) ||  //Unsinniger R-Block
                     (rbuffer[1]&0xe0)==0xe0 || //SResponse ohne Request
                     ((rbuffer[1]&0xe0)==0xc0 &&
                      ((((rbuffer[1]& 0x1f)==0 || (rbuffer[1]& 0x1f)==2) && rbuffer[2]!=0) ||
                      ((rbuffer[1]& 0x1f)==1 && (rbuffer[2]!=1 || rbuffer[3]<0x10 || rbuffer[3]==255)) ||
                      ((rbuffer[1]& 0x1f)==3 && rbuffer[2]!=1) ||
                      (rbuffer[1]& 0x1f)>3 )))
                  {
                     if(++error>2)
                     {
                        cjccid_iccPowerOff(hDevice);
                        Result=CJPP_ERR_PROT;
                        break;
                     }
                     if((sblock[1]&0xc0)!=0x80)
                     {
                        rblock[0]=0x00;
                        rblock[1]=(unsigned char)(0x82 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                        rblock[2]=0;
                        sblock=rblock;
                     }
                     slen=3;
                  }
                  else if((rbuffer[1]&0x80)==0) /*I-Block*/
                  {
                     if((rbuffer[1]&0x40)!=((((cjccidHANDLE)hDevice)->PCB_seq&0x10)<<2) ||
                         (sbuffer[1]&0x20)==0x20 || rbuffer[2]==0 || rbuffer[2]==255)
                     {
                        if(++error>2)
                        {
                           cjccid_iccPowerOff(hDevice);
                           Result=CJPP_ERR_PROT;
                           break;
                        }
                        if((sblock[1]&0xc0)!=0x80)
                        {
                           rblock[0]=0x00;
                           rblock[1]=(unsigned char)(0x82 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                           rblock[2]=0;
                           sblock=rblock;
                        }
                        slen=3;
                     }
                     else
                     {
                        error=0;
                        ((cjccidHANDLE)hDevice)->PCB_seq^=0x10;
                        if(lenc)
                        {
                           ((cjccidHANDLE)hDevice)->PCB_seq^=0x01;
                           lenc-=INF;
                           cmd+=INF;
                        }
                        if(maxresp>=rlen-4-((cjccidHANDLE)hDevice)->EDC)
                        {
                           memcpy(response+(*lenr),rbuffer+3,rlen-4-((cjccidHANDLE)hDevice)->EDC);
                           maxresp-=rlen-4-((cjccidHANDLE)hDevice)->EDC;
                        }
                        *lenr+=(unsigned short)(rlen-4-((cjccidHANDLE)hDevice)->EDC);
                        if(rbuffer[1] & 0x20)
                        {
                           rblock[0]=0x00;
                           rblock[1]=(unsigned char)(0x80 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                           rblock[2]=0;
                           sblock=rblock;
                           slen=3;
                        }
                        else
                        {
                           break;
                        }
                     }
                  }
                  else if((rbuffer[1]&0xE0)==0xC0) /*S-Block*/
                  {
                     error=0;
                     memcpy(rblock,rbuffer,rlen);
                     rblock[1]|=0x20;
                     sblock=rblock;
                     slen=(unsigned short)(rlen-1);
                     if((rbuffer[1]& 0x1f)==0)
                     {
                        ((cjccidHANDLE)hDevice)->PCB_seq=0;
                     }
                     else if((rbuffer[1]& 0x1f)==1)
                     {
                        ((cjccidHANDLE)hDevice)->IFSC=rbuffer[3];
                     }
                     else if((rbuffer[1]& 0x1f)==2)
                     {
                        cjccid_iccPowerOff(hDevice);
                        Result=CJPP_ERR_PROT;
                        break;
                     }
                     else if((rbuffer[1]& 0x1f)==3)
                     {
                        xwt=rbuffer[3];
                     }
                  }
                  else   /*R-Block*/
                  {
//                     if((rbuffer[1]&0x10)==((((cjccidHANDLE)hDevice)->PCB_seq<<4) & 0x10))
                     if((rbuffer[1]&0x10)==((sbuffer[1]&0x40)>>2))
                     {
                        if(++error>2)
                        {
                           cjccid_iccPowerOff(hDevice);
                           Result=CJPP_ERR_PROT;
                        }
                        sblock=sbuffer;
                        slen=3+sbuffer[2];
                     }
                     else if(sbuffer[1] & 0x20)
                     {
                        error=0;
                        ((cjccidHANDLE)hDevice)->PCB_seq^=0x01;
                        cmd+=INF;
                        lenc-=INF;
                        break;
                     }
                     else if((sblock[1] & 0xC0)==0x80)
                     {
                        if(++error>2)
                        {
                           cjccid_iccPowerOff(hDevice);
                           Result=CJPP_ERR_PROT;
                           break;
                        }
                        slen=3;
                     }
                     else
                     {
                        if(++error>2)
                        {
                           cjccid_iccPowerOff(hDevice);
                           Result=CJPP_ERR_PROT;
                           break;
                        }
                        if((sblock[1]&0xc0)!=0x80)
                        {
                           rblock[0]=0x00;
                           rblock[1]=(unsigned char)(0x82 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                           rblock[2]=0;
                           sblock=rblock;
                        }
                        slen=3;
                     }
                  }
               }
               if(Result==CJPP_ERR_PARITY)
               {
                  if(++error>2)
                  {
                     cjccid_iccPowerOff(hDevice);
                     Result=CJPP_ERR_PROT;
                     break;
                  }
                  if((sblock[1]&0xc0)!=0x80)
                  {
                     rblock[0]=0x00;
                     rblock[1]=(unsigned char)(0x81 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                     rblock[2]=0;
/*                     rblock[0]=0x00;
                     rblock[1]=0xE3;
                     rblock[2]=01;
                     rblock[3]=01;*/
                     sblock=rblock;
                  }
                  slen=3;
//                  slen=4;

               }
               if(Result==CJPP_ERR_TIMEOUT)
               {
//                  if(++error>4)
                  {
//                     cjccid_iccPowerOff(hDevice);
                     Result=CJPP_ERR_PROT;
                     break;
                  }
                  rblock[0]=0x00;
                  rblock[1]=(unsigned char)(0x82 | (((cjccidHANDLE)hDevice)->PCB_seq & 0x10));
                  rblock[2]=0;
                  sblock=rblock;
                  slen=3;
               }
            }
         }
         else
            return Result;
      }
      if(Result==CJPP_ERR_PROT || (rbuffer[1] & 0xA0)==0)
         break;
   }
   return Result;
}

static int PVMVT1(HANDLE hDevice,int Result,unsigned char *rbuffer,int rlen,int *lenr)
{
   unsigned char lrc;
   int i;
   if(Result!=CJPP_SUCCESS)
      return Result;
   if(((cjccidHANDLE)hDevice)->EDC==0)
   {
      lrc=0;
      for(i=0;i<rlen;i++)
      {
         lrc^=rbuffer[i];
      }
      if(lrc)
      {
         cjccid_iccPowerOff(hDevice);
         return CJPP_ERR_PARITY;
      }
   }
   else
   {
      //crc berechnung
   }
   if((rbuffer[1]&0x80)!=0) //I-Block
   {
      cjccid_iccPowerOff(hDevice);
      return CJPP_ERR_PROT;
   }
   if((rbuffer[1]&0x40)!=((((cjccidHANDLE)hDevice)->PCB_seq&0x10)<<2))
   {
      cjccid_iccPowerOff(hDevice);
      return CJPP_ERR_PROT;
   }
   else
   {
      ((cjccidHANDLE)hDevice)->PCB_seq^=0x11;
      memmove(rbuffer,rbuffer+3,rlen-4-((cjccidHANDLE)hDevice)->EDC);
      *lenr=(unsigned short)(rlen-4-((cjccidHANDLE)hDevice)->EDC);
      return CJPP_SUCCESS;
   }
}


static char ctapiData_inner(CCID_DEVICE hDevice,unsigned char *dad,unsigned char *sad,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response)
{
   int Slot;
   ((cjccidHANDLE)hDevice)->hasCanceled=0;
   if(*dad==1 && *sad==2)
   {
      if(lenc<4)
      {
         *lenr=0;
         return -1;
      }
      if(*lenr<2)
      {
         *lenr=0;
         return -11;
      }
      *dad=2;
      *sad=1;
      if((lenc!=4 &&
          lenc!=5 &&
          (lenc<=5 || (lenc>5 && lenc!=5+cmd[4])) &&
          (lenc<=6 || (lenc>6 && lenc!=6+cmd[4]))) ||
         (lenc==6 && cmd[4]==0) || lenc<4)
      {
         response[0]=0x67;
         response[1]=0x00;
         *lenr=2;
         return 0;
      }
      if(cmd[0]==0x00)
      {
         if(cmd[1]==0xa4)
         {
            if(cmd[2]!=0 || cmd[3]!=0)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(cmd[4]!=2)
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(memcmp(cmd+5,"\x3f\x00",2)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_path,cmd+5,2);
               ((cjccidHANDLE)hDevice)->reader_path_len=2;
               memset(((cjccidHANDLE)hDevice)->reader_file,0,2);
               memcpy(response,"\x00\x2d\x00\x2d\x88\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x00\x20",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==2 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00",2)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x0e\x00\x0e\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x7f\x60",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==2 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00",2)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_path+2,cmd+5,2);
               ((cjccidHANDLE)hDevice)->reader_path_len=4;
               memset(((cjccidHANDLE)hDevice)->reader_file,0,2);
               memcpy(response,"\x00\x05\x00\x05\x88\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x60\x20",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x60",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x10\x00\x10\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x60\x21",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x60",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x10\x00\x10\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x60\x30",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x60",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x04\x00\x04\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x60\x31",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x60",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x03\x00\x03\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x7f\x70",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==2 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00",2)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_path+2,cmd+5,2);
               ((cjccidHANDLE)hDevice)->reader_path_len=4;
               memset(((cjccidHANDLE)hDevice)->reader_file,0,2);
               memcpy(response,"\x00\x19\x00\x19\x88\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x70\x20",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x70",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x0f\x00\x0f\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            if(memcmp(cmd+5,"\x70\x21",2)==0 && ((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7F\x70",4)==0)
            {
               if(*lenr<12)
               {
                  return -11;
               }
               memcpy(((cjccidHANDLE)hDevice)->reader_file,cmd+5,2);
               memcpy(response,"\x00\x10\x00\x10\x08\x00\x00\x00\x00\x00\x90\x00",12);
               *lenr=12;
               return 0;
            }
            response[0]=0x6A;
            response[1]=0x82;
            *lenr=2;
            return 0;
         }
         else if(cmd[1]==0xb0)
         {
            if(((cjccidHANDLE)hDevice)->reader_path_len==2 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00",2)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x00\x20",2)==0)
            {
               if(*lenr<16)
                  return -11;
               memcpy(response,"\x01\x05\x43\x4a\x50\x50\x41\x02\01\00\x03\x02\x20\x00\x90\x00",16);
               *lenr=16;
               return 0;
            }

            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x60",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x60\x20",2)==0)
            {
               if(*lenr<18)
                  return -11;
               memcpy(response,"\x03\x02\x20\x00\x10\x01\x00\x11\x01\x00\x12\x01\x00\x13\x01\x00\x90\x00",18);
               *lenr=18;
               return 0;
            }

            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x60",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x60\x21",2)==0)
            {
               if(*lenr<18)
                  return -11;
               memcpy(response,"\x03\x02\x20\x00\x10\x01\x00\x11\x01\x00\x12\x01\x00\x13\x01\x00\x90\x00",18);
               *lenr=18;
               return 0;
            }

            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x60",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x60\x30",2)==0)
            {
               if(*lenr<6)
                  return -11;
               memcpy(response,"\x30\x02\x01\x03\x90\x00",6);
               *lenr=6;
               return 0;
            }
            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x60",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x60\x31",2)==0)
            {
               if(*lenr<5)
                  return -11;
               memcpy(response,"\x30\x01\xff\x90\x00",5);
               *lenr=5;
               return 0;
            }
            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x70",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x70\x20",2)==0)
            {
               if(*lenr<17)
                  return -11;
               memcpy(response,"\x03\x02\x20\x00\x20\x01\x81\x22\x06\01\x02\x03\x80\x81\x82\x90\x00",17);
               *lenr=17;
               return 0;
            }
            if(((cjccidHANDLE)hDevice)->reader_path_len==4 && memcmp(((cjccidHANDLE)hDevice)->reader_path,"\x3f\x00\x7f\x70",4)==0 && memcmp(((cjccidHANDLE)hDevice)->reader_file,"\x70\x21",2)==0)
            {
               if(*lenr<18)
                  return -11;
               memcpy(response,"\x03\x02\x20\x00\x21\x02\x01\x89\x22\x06\01\x02\x03\x80\x81\x82\x90\x00",17);
               *lenr=18;
               return 0;
            }
            response[0]=0x69;
            response[1]=0x82;
            *lenr=2;
            return 0;
         }
         else
         {
            response[0]=0x6D;
            response[1]=0x00;
            *lenr=2;
            return 0;
         }
      }
      else if(cmd[0]==0x20)
      {
         if(cmd[1]==0x10)  /*B1*/
         {
           if(cmd[2]>1 || (cmd[2]==0 && cmd[3]!=0) || (cmd[2]==1 && cmd[3]>2))
           {
              response[0]=0x6A;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           if((cmd[2]==1 && cmd[3]>0 && (lenc!=5 || cmd[4]!=0)) || lenc!=5 || cmd[4]!=0)
           {
              response[0]=0x67;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           cjccid_iccPowerOff(hDevice);

           if(cmd[2]==0x01)
           {
              unsigned char atr[250];
              unsigned char historical[15];
              unsigned char hist_len=sizeof(historical);
              unsigned char atr_len=(unsigned char)sizeof(atr);
              switch(ctBcsReset(hDevice,atr,&atr_len,historical,&hist_len))
              {
                 case CJPP_SUCCESS:
                    if(cmd[3]==0)
                    {
                       if(atr[0]==0x3b || atr[0]==0x3f)
                       {
                         response[0]=0x90;
                         response[1]=0x01;
                       }
                       else
                       {
                         response[0]=0x90;
                         response[1]=0x00;
                       }
                       *lenr=2;
                    }
                    else if(cmd[3]==1)
                    {
                       if(atr_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           return -11;
                       }
                       else
                       {
                          memcpy(response,atr,atr_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x01;
                          }
                          else
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(atr_len+2);
                       }
                    }
                    else
                    {
                       if(hist_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           *lenr=0;
                           return -11;
                       }
                       else
                       {
                          memcpy(response,historical,hist_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x01;
                          }
                          else
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(hist_len+2);
                       }
                    }
                    return 0;
                 case CJPP_ERR_PARITY:
                 case CJPP_ERR_TIMEOUT:
                 case CJPP_ERR_PROT:

                     response[0]=0x64;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                 case CJPP_ERR_NO_ICC:
                     response[0]=0x64;
                     response[1]=0xA1;
                     *lenr=2;
                     return 0;
                 default:
                    cjccid_iccPowerOff(hDevice);
                    return -128;
              }
           }
           else
           {
              response[0]=0x90;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
         }
         else if(cmd[1]==0x11)
         {
           if(cmd[2]>1 || (cmd[2]==0 && cmd[3]!=0) || (cmd[2]==1 && cmd[3]>2))
           {
              response[0]=0x6A;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           if(cmd[2]==1 && cmd[3]>0 && (lenc!=5 || cmd[4]!=0))
           {
              response[0]=0x67;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           cjccid_iccPowerOff(hDevice);

           if(cmd[2]==0x01)
           {
              unsigned char atr[250];
              unsigned char historical[15];
              unsigned char hist_len=sizeof(historical);
              unsigned char atr_len=(unsigned char)sizeof(atr);
              switch(ctBcsReset(hDevice,atr,&atr_len,historical,&hist_len))
              {
                 case CJPP_SUCCESS:
                    if(cmd[3]==0)
                    {
                       if(atr[0]==0x3b || atr[0]==0x3f)
                       {
                         response[0]=0x90;
                         response[1]=0x01;
                       }
                       else
                       {
                         response[0]=0x90;
                         response[1]=0x00;
                       }
                       *lenr=2;
                    }
                    else if(cmd[3]==1)
                    {
                       if(atr_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           *lenr=0;
                           return -11;
                       }
                       else
                       {
                          memcpy(response,atr,atr_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x01;
                          }
                          else
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(atr_len+2);
                       }
                    }
                    else
                    {
                       if(hist_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           *lenr=0;
                           return -11;
                       }
                       else
                       {
                          memcpy(response,historical,hist_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x01;
                          }
                          else
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(hist_len+2);
                       }
                    }
                    return 0;
                 case CJPP_ERR_PARITY:
                 case CJPP_ERR_TIMEOUT:
                 case CJPP_ERR_PROT:

                     response[0]=0x64;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                 case CJPP_ERR_NO_ICC:
                     response[0]=0x64;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                 default:
                    cjccid_iccPowerOff(hDevice);
                    return -128;
              }
           }
           else
           {
              response[0]=0x90;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
         }

         else if(cmd[1]==0x12)
         {
           cmd[3]&=0x0f;
           if(cmd[2]!=1 || cmd[3]>2)
           {
              response[0]=0x6A;
              response[1]=0x00;
	      *lenr=2;
              return 0;
           }
           if((cmd[3]>0 && (lenc<5 || cmd[lenc-1]!=0)) ||
              (lenc!=4 && cmd[lenc-1]!=0))
           {
              response[0]=0x67;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           if((Slot=cjccid_GetSlotStatus(hDevice))==CJPP_SUCCESS)
           {
              response[0]=0x62;
              response[1]=0x01;
              *lenr=2;
              return 0;
           }
           if(lenc>5)
           {
              unsigned char *tag80;
              int taglen;
              if((tag80=GetTag(cmd+5,cmd[4],0x80,&taglen))!=NULL || cmd[4]==1)
              {
                 if(cmd[4]==1)
                    taglen=cmd[5];
                 else
                 {
                    if(taglen!=1)
                    {
                        response[0]=0x67;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                    }
                    taglen=*tag80;
                 }
                 taglen*=10;

                 while(((cjccidHANDLE)hDevice)->Status==0 && ((cjccidHANDLE)hDevice)->hasCanceled==0 && taglen--)
                 {
                    cjppSleep(100);
                 }
                 if(((cjccidHANDLE)hDevice)->Status==0)
                 {
                     if(((cjccidHANDLE)hDevice)->hasCanceled==0)
                     {
                        response[0]=0x62;
                        response[1]=0x00;
                     }
                     else
                     {
                        response[0]=0x64;
                        response[1]=0x01;
                     }
                     *lenr=2;
                     return 0;
                 }
              }
           }
           else if(Slot==CJPP_ERR_NO_ICC)
           {
              response[0]=0x62;
              response[1]=0x00;
              *lenr=2;
              return 0;
           }
           {
              unsigned char atr[250];
              unsigned char historical[15];
              unsigned char hist_len=sizeof(historical);
              unsigned char atr_len=(unsigned char)sizeof(atr);
              switch(ctBcsReset(hDevice,atr,&atr_len,historical,&hist_len))
              {
                 case CJPP_SUCCESS:
                    if(cmd[3]==0)
                    {
                       if(atr[0]==0x3b || atr[0]==0x3f)
                       {
                         response[0]=0x90;
                         response[1]=0x01;
                       }
                       else
                       {
                         response[0]=0x90;
                         response[1]=0x00;
                       }
                       *lenr=2;
                    }
                    else if(cmd[3]==1)
                    {
                       if(atr_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           return -11;
                       }
                       else
                       {
                          memcpy(response,atr,atr_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x01;
                          }
                          else
                          {
                            response[atr_len]=0x90;
                            response[atr_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(atr_len+2);
                       }
                    }
                    else
                    {
                       if(hist_len+2>*lenr)
                       {
                           cjccid_iccPowerOff(hDevice);
                           return -11;
                       }
                       else
                       {
                          memcpy(response,historical,hist_len);
                          if(atr[0]==0x3b || atr[0]==0x3f)
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x01;
                          }
                          else
                          {
                            response[hist_len]=0x90;
                            response[hist_len+1]=0x00;
                          }
                          *lenr=(unsigned short)(hist_len+2);
                       }
                    }
                    return 0;
                 case CJPP_ERR_PARITY:
                 case CJPP_ERR_TIMEOUT:
                 case CJPP_ERR_PROT:

                     response[0]=0x64;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                 case CJPP_ERR_NO_ICC:
                     response[0]=0x62;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                 default:
                    cjccid_iccPowerOff(hDevice);
                    *lenr=0;
                    return -128;
              }
           }
         }
         else if(cmd[1]==0x13)
         {
            if(lenc==4 && cmd[2]==0x01 && cmd[3]==0x00)
            {
               if(*lenr<8)
               {
                  *lenr=0;
                  return -11;
               }
               response[0] = 0x71;
               response[1] = 0x01;
               response[2] = 0x00;
               response[3] = 0x72;
               response[4] = 0x01;
               response[5] = (unsigned char)((cjccidHANDLE)hDevice)->Protokoll;
               response[6] = 0x90;
               response[7] = 0x00;
               *lenr=8;
               return 0;
            }
            else if(lenc!=5 || cmd[4]!=0)
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(cmd[2]>1 || (cmd[2]==0 && cmd[3]!=0x46 && cmd[3]!=0x80 && cmd[3]!=0x81) ||
               (cmd[2]==1 && cmd[3]!=0x80))
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(cmd[3]==0x80)
            {
              if(*lenr<5)
              {
                 *lenr=0;
                 return -11;
              }
              response[0]=0x80;
              response[1]=1;
              switch(cjccid_GetSlotStatus(hDevice))
              {
                 case CJPP_SUCCESS:
                   response[2]=0x05;
                   break;
                 case CJPP_ERR_NO_ACTIVE_ICC:
                   response[2]=0x03;
                   break;
                 case CJPP_ERR_NO_ICC:
                   response[2]=0x00;
                   break;
                 default:
                   *lenr=0;
                   return -128;
              }
              response[3]=0x90;
              response[4]=0x00;
              *lenr=5;
            }
            else if(cmd[3]==0x81)
            {
              if(*lenr<6)
              {
                 *lenr=0;
                 return -11;
              }
              response[0]=0x81;
              response[1]=2;
              response[2]=1;
              response[3]=0x50;
              response[4]=0x90;
              response[5]=0x00;
              *lenr=6;
            }
            else
            {
              if(*lenr<19)
              {
                 *lenr=0;
                 return -11;
              }
              response[0]=0x46;
              response[1]=15;
              if(*lenr>=19+sizeof(cjpp_Info))
                 response[1]+=(unsigned char)sizeof(cjpp_Info);
              memcpy(response+2,"DESCTCJPPA",10);
              sprintf((char *)(response+12)," V%1d.%1d",(int)(((cjccidHANDLE)hDevice)->Info.ApplicationVersion>>12),(int)((((cjccidHANDLE)hDevice)->Info.ApplicationVersion>>8) & 0x0f));
              if(*lenr>=19+sizeof(cjpp_Info))
              {
                 memcpy(response+17,&(((cjccidHANDLE)hDevice)->Info),sizeof(cjpp_Info));
                 response[17+sizeof(cjpp_Info)]=0x90;
                 response[18+sizeof(cjpp_Info)]=0x00;
                 *lenr=19+sizeof(cjpp_Info);
              }
              else
              {
                 response[17]=0x90;
                 response[18]=0x00;
                 *lenr=19;
              }
            }
            return 0;
         }
         else if(cmd[1]==0x14)
         {
            if(lenc!=5 || cmd[4]!=0)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(cmd[2]!=1 || cmd[3]!=0)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }

            if(cjccid_iccPowerOff(hDevice)!=CJPP_SUCCESS)
               return -10;
            if(((cjccidHANDLE)hDevice)->Status!=0)
            {
               response[0]=0x90;
               response[1]=0x00;
            }
            else
            {
               response[0]=0x64;
               response[1]=0xA1;
            }
            *lenr=2;
            return 0;
         }
         else if(cmd[1]==0x15)
         {
            if(cmd[2]!=1)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(lenc!=4 && (lenc<=6 || lenc!=5+cmd[4]))
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }

            if(cjccid_iccPowerOff(hDevice)!=CJPP_SUCCESS)
               return -10;
            if((cmd[3]&0x04)!=0x04 && lenc>4)
            {
              unsigned char *tag80;
              int taglen;
              if((tag80=GetTag(cmd+5,cmd[4],0x80,&taglen))!=NULL || cmd[4]==1)
              {
                 if(cmd[4]==1)
                    taglen=cmd[5];
                 else
                 {
                    if(taglen!=1)
                    {
                        response[0]=0x67;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                    }
                    taglen=*tag80;
                 }
                 taglen*=10;

                 while(((cjccidHANDLE)hDevice)->Status!=0 && taglen--)
                 {
                    cjppSleep(100);
                 }
                 if(((cjccidHANDLE)hDevice)->Status==0)
                 {
                    response[0]=0x90;
                    response[1]=0x01;
                 }
                 else
                 {
                    response[0]=0x62;
                    response[1]=0x00;
                 }
              }
              else
              {
                 response[0]=0x90;
                 response[1]=0x00;
              }
            }
            else
            {
               response[0]=0x90;
               response[1]=0x00;
            }
            *lenr=2;
            return 0;
         }
         else if(cmd[1]==0x16)
         {
            int charlen;
            unsigned char timeout=15;
            unsigned char *tag80;
            int taglen;
            int i;
            unsigned char buffer[256];
            unsigned char key;
            void (CJPP_CALLBACK_TYPE *merkcallback)(HANDLE,unsigned char);

            merkcallback=((cjccidHANDLE)hDevice)->backs.CallbackKey;
            ((cjccidHANDLE)hDevice)->backs.CallbackKey=NULL;
            if(lenc<5 || (lenc>5 && lenc!=6+cmd[4]))
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
               return 0;
            }
            if(cmd[2]!=0x50 || (cmd[3]&0x0f)>0x02 || (cmd[3]&0xf0)>0x10)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
               return 0;
            }
            if(lenc>5 && ((tag80=GetTag(cmd+5,cmd[4],0x80,&taglen))!=NULL || cmd[4]==1))
            {
              if(cmd[4]==1)
                 timeout=cmd[5];
              else
              {
                 if(taglen!=1)
                 {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
                     return 0;
                 }
                 timeout=*tag80;
              }
            }
            if(lenc==5)
               charlen=cmd[4];
            else
               charlen=cmd[5+cmd[4]];
            if(charlen==0)
               charlen=256;
            for(i=0;i<charlen;i++)
            {
               if(cjppInput(hDevice,&key,timeout)!=CJPP_SUCCESS)
               {
                  *lenr=0;
                  ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
                  return -128;
               }
               if(key==0xff)
               {
                  response[0]=0x64;
                  response[1]=0x00;
                  *lenr=2;
                  ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
                  return 0;
               }
               timeout=5;
               if((cmd[3]&0xf0)==0x10)
               {
                  if((key!=10 && key!=11) || charlen==1)
                  {
                     buffer[i]=key;
                  }
                  else if(key==10)
                  {
                     response[0]=0x64;
                     response[1]=0x01;
                     *lenr=2;
                    ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
                     return 0;
                  }
                  else if(key==11)
                  {
                     break;
                  }
               }
               else
               {
                  if(key<10)
                  {
                     key+=(unsigned char)'0';
                     buffer[i]=key;
                  }
                  else if(key==10)
                  {
                     response[0]=0x64;
                     response[1]=0x01;
                     *lenr=2;
                     ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
                     return 0;
                  }
                  else if(key==11)
                  {
                     break;
                  }
                  else
                  {
                     i--;
                  }
               }
            }
            if(*lenr<i+2)
            {
               *lenr=0;
               ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
               return -11;
            }
            memcpy(response,buffer,i);
            response[i]=0x90;
            response[i+1]=0x00;
            *lenr=i+2;
            ((cjccidHANDLE)hDevice)->backs.CallbackKey=merkcallback;
            return 0;
         }
         else if(cmd[1]==0x18)
         {
            unsigned char timeout=15;
            if(cmd[2]!=1 || cmd[3]!=0)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(lenc<13 || lenc!=cmd[4]+5)
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            else
            {
               unsigned char *tag80;
               int taglen;
               if((tag80=GetTag(cmd+5,cmd[4],0x80,&taglen))!=NULL)
               {
                  if(taglen!=1)
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  timeout=*tag80;
               }
	       /* MP: changed 0x52 to 0x80 (bug?) */
	       /* MP: changed it back, 0x52 is correct, just the varname is
                * misleading!
		*/
               if((tag80=GetTag(cmd+5,cmd[4],0x52,&taglen))!=NULL)
               {
                  unsigned char PinLength;
                  unsigned char Min;
                  unsigned char Max;
                  unsigned char PinType;
                  unsigned char Condition;
                  unsigned char PinPosition;
                  unsigned char PinLengthPosition=0;
                  unsigned char PinLengthSize=0;
                  int outlen;
                  int inlen;
                  unsigned char buffer[260];
                  unsigned char rbuffer[260];
                  unsigned char Prologue[3];
                  int Res;
                  if(taglen<6)
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  memcpy(buffer,tag80+2,taglen-2);
                  PinLength=tag80[0]>>4;
                  if(tag80[1]>5)
                     PinPosition=tag80[1]-6;
                  else
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  if(taglen==6)
                  {
                     buffer[4]=0;
                     taglen++;
                  }
                  else
                  {
                     if(taglen!=buffer[4]+7 || PinPosition>buffer[4])
                     {
                        response[0]=0x67;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                     }
                  }
                  switch(tag80[0] & 3)
                  {
                     case 0:
                        PinType=1;
                        if(taglen==7)
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=15;
                              Condition=2;
                              PinLength=8;
                              memset(buffer+5,0xff,8);
                              buffer[4]=0;
                              outlen=13;
                           }
                           else
                           {
                              Min=PinLength;
                              Max=PinLength;
                              PinLength=PinLength/2+(PinLength & 1);
                              Condition=1;
                              memset(buffer+5,0xff,PinLength);
                              outlen=5+PinLength;
                           }
                        }
                        else
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=(taglen-7-PinPosition)*2;
                              Condition=2;
                              PinLength=(taglen-7-PinPosition);
                              outlen=(taglen-2);
                           }
                           else
                           {
                              if(PinLength/2+(PinLength & 1)+PinPosition>buffer[4])
                              {
                                 response[0]=0x67;
                                 response[1]=0x00;
                                 *lenr=2;
                                 return 0;
                              }
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=taglen-2;
                           }
                        }
                        break;
                     case 1:
                        PinType=2;
                        if(taglen==7)
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=15;
                              Condition=2;
                              PinLength=15;
                              outlen=5;
                           }
                           else
                           {
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=5;
                           }
                        }
                        else
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=taglen-7-PinPosition;
                              Condition=2;
                              PinLength=(taglen-7-PinPosition);
                              outlen=(taglen-2);
                           }
                           else
                           {
                              if(PinLength+PinPosition>buffer[4])
                              {
                                 response[0]=0x67;
                                 response[1]=0x00;
                                 *lenr=2;
                                 return 0;
                              }
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=taglen-2;
                           }
                        }
                        break;
                     case 2:
                        PinType=1;
                        PinLengthPosition=4;
                        PinLengthSize=4;
                        PinPosition++;
								if(PinPosition>240)
                        {
                           response[0]=0x67;
                           response[1]=0x00;
                           *lenr=2;
                           return 0;
                        }
								if(buffer[4]<PinPosition+7)
									buffer[4]=PinPosition+7;
                        buffer[4+PinPosition]=0x2f;
                        memset(buffer+5+PinPosition,0xff,7);
                        outlen=buffer[4]+5;
                        if(PinLength==0)
                        {
                           Min=4;
                           Max=12;
                           Condition=2;
                           PinLength=7;
                        }
                        else
                        {
                           Min=PinLength;
                           Max=PinLength;
                           if(PinLength<4 || PinLength>12)
                           {
                              response[0]=0x67;
                              response[1]=0x00;
                              *lenr=2;
                              return 0;
                           }
                           PinLength=7;
                           Condition=1;
                        }
                        break;
                     default:
                        response[0]=0x6A;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                  }
                  Prologue[0]=0;
                  if(((cjccidHANDLE)hDevice)->PCB_seq&0x01)
                     Prologue[1]=0x40;
                  else
                     Prologue[1]=0;
                  Prologue[2]=outlen;
                  inlen=sizeof(rbuffer);
                  switch(Res=cjccid_SecurePV(hDevice,timeout,PinPosition,PinType,
                                  PinLengthSize,PinLength,
                                  PinLengthPosition,
                                  Min,Max,
                                  Condition,Prologue,
                                  buffer,outlen,rbuffer,&inlen))
                  {
                     case CJPP_SUCCESS:
                        if(((cjccidHANDLE)hDevice)->Protokoll==1)
                        {
                           Res=PVMVT1(hDevice,Res,rbuffer,inlen,&inlen);
                        }
                        if(Res==CJPP_SUCCESS)
                        {
                           if(*lenr<inlen)
                           {
                              *lenr=0;
                              return -11;
                           }
                           memcpy(response,rbuffer,inlen);
                           *lenr=inlen;
                           *sad=0;
                           return 0;
                        }
                        else
                        {
                           response[0]=0x6f;
                           response[1]=0x00;
                           *lenr=2;
                           return 0;
                        }
                        break;
                     case CJPP_ERR_PIN_CANCELED:
                        response[0]=0x64;
                        response[1]=0x01;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_PIN_TIMEOUT:
                        response[0]=0x64;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_NO_ICC:
                        response[0]=0x64;
                        response[1]=0xA1;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_NO_ACTIVE_ICC:
                        response[0]=0x64;
                        response[1]=0xA2;
                        *lenr=2;
                        return 0;
							case CJPP_ERR_WRITE_DEVICE:
                     case CJPP_ERR_DEVICE_LOST:
								*lenr=0;
								return -128;

                     default:
                        response[0]=0x6f;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                  }
               }
               else
               {
                  response[0]=0x67;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               }
            }
         }
         else if(cmd[1]==0x19)
         {
            unsigned char timeout=15;
            if(cmd[2]!=1 || cmd[3]!=0)
            {
               response[0]=0x6A;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            if(lenc<14 || lenc!=cmd[4]+5)
            {
               response[0]=0x67;
               response[1]=0x00;
               *lenr=2;
               return 0;
            }
            else
            {
               unsigned char *tag80;
               int taglen;
               if((tag80=GetTag(cmd+5,cmd[4],0x80,&taglen))!=NULL)
               {
                  if(taglen!=1)
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  timeout=*tag80;
               }
               if((tag80=GetTag(cmd+5,cmd[4],0x52,&taglen))!=NULL)
               {
                  unsigned char PinLength;
                  unsigned char Min;
                  unsigned char Max;
                  unsigned char PinType;
                  unsigned char Condition;
                  unsigned char PinPositionOld;
                  unsigned char PinPositionNew;
                  unsigned char PinPosition=0;
                  unsigned char PinLengthPosition=0;
                  unsigned char PinLengthSize=0;
                  int outlen;
                  int inlen;
                  unsigned char buffer[260];
                  unsigned char rbuffer[260];
                  unsigned char Prologue[3];
                  unsigned char bConfirmPIN;
                  int Res;
                  if(taglen<7)
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  memcpy(buffer,tag80+3,taglen-3);
                  PinLength=tag80[0]>>4;
                  if(tag80[1]>5 && tag80[1]!=tag80[2])
                  {
                     PinPositionOld=tag80[1]-6;
                     bConfirmPIN=3; /* 2 */
                  }
                  else if(tag80[2]>5 && tag80[1]==tag80[2])
                  {
                     PinPositionOld=0;
                     bConfirmPIN=0;
                  }
                  else
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  if(tag80[2]>5)
                  {
                     PinPositionNew=tag80[2]-6;
                  }
                  else if(tag80[2]==0)
                  {
                     PinPositionNew=0;
                  }
                  else
                  {
                     response[0]=0x67;
                     response[1]=0x00;
                     *lenr=2;
                     return 0;
                  }
                  if(taglen==7)
                  {
                     buffer[4]=0;
                     taglen++;
                  }
                  else
                  {
                     if(taglen!=buffer[4]+8 || PinPositionOld>buffer[4] || PinPositionNew>buffer[4])
                     {
                        response[0]=0x67;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                     }
                  }
                  switch(tag80[0] & 3)
                  {
                     case 0:
                        PinType=1;
                        if(taglen==8)
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=15;
                              Condition=2;
                              PinLength=8;
                              memset(buffer+5,0xff,16);
                              outlen=21;
                           }
                           else
                           {
                              Min=PinLength;
                              Max=PinLength;
                              PinLength=PinLength/2+(PinLength & 1);
                              Condition=1;
                              memset(buffer+5,0xff,PinLength*2);
                              outlen=5+PinLength*2;
                           }
                        }
                        else
                        {
                           if(PinLength==0)
                           {
                              if(PinPositionOld)
                              {
                                 Min=1;
                                 Max=taglen-8;
                                 if(PinPositionOld<PinPositionNew)
                                 {
                                    Max-=PinPositionNew-PinPositionOld;
                                 }
                                 else
                                 {
                                    Max-=PinPositionOld-PinPositionNew;
                                 }
                                 PinLength=Max;
                                 Max<<=1;
                                 Condition=2;
                                 outlen=(taglen-3);
                              }
                              else
                              {
                                 Min=1;
                                 Max=(taglen-8-PinPositionNew)*2;
                                 Condition=2;
                                 PinLength=(taglen-8-PinPositionNew);
                                 outlen=(taglen-3);
                              }
                           }
                           else
                           {
                              if(PinLength/2+(PinLength & 1)+PinPositionOld>buffer[4] || PinLength/2+(PinLength & 1)+PinPositionNew>buffer[4])
                              {
                                 response[0]=0x67;
                                 response[1]=0x00;
                                 *lenr=2;
                                 return 0;
                              }
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=taglen-2;
                           }
                        }
                        break;
                     case 1:
                        PinType=2;
                        if(taglen==8)
                        {
                           if(PinLength==0)
                           {
                              Min=1;
                              Max=15;
                              Condition=2;
                              PinLength=15;
                              outlen=5;
                           }
                           else
                           {
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=5;
                           }
                        }
                        else
                        {
                           if(PinLength==0)
                           {
                              if(PinPositionOld)
                              {
                                 Min=1;
                                 Max=taglen-8;
                                 if(PinPositionOld<PinPositionNew)
                                 {
                                    Max-=PinPositionNew-PinPositionOld;
                                 }
                                 else
                                 {
                                    Max-=PinPositionOld-PinPositionNew;
                                 }
                                 PinLength=Max;
                                 Condition=2;
                                 outlen=(taglen-3);
                              }
                              else
                              {
                                 Min=1;
                                 Max=(taglen-8-PinPositionNew);
                                 Condition=2;
                                 PinLength=(taglen-8-PinPositionNew);
                                 outlen=(taglen-3);
                              }
                           }
                           else
                           {
                              if(PinLength+PinPositionOld>buffer[4] || PinLength+PinPositionNew>buffer[4])
                              {
                                 response[0]=0x67;
                                 response[1]=0x00;
                                 *lenr=2;
                                 return 0;
                              }
                              Min=PinLength;
                              Max=PinLength;
                              Condition=1;
                              outlen=taglen-3;
                           }
                        }
                        break;
                     case 2:
                        PinType=1;
                        PinPositionNew++;
                        PinPositionOld++;
                        PinLengthPosition=4;
                        PinLengthSize=4;

								if(PinPositionNew>240 || PinPositionOld>240)
                        {
                           response[0]=0x67;
                           response[1]=0x00;
                           *lenr=2;
                           return 0;
                        }
								if(buffer[4]<PinPositionOld+7)
									buffer[4]=PinPositionOld+7;
								if(buffer[4]<PinPositionNew+7)
									buffer[4]=PinPositionNew+7;
                        buffer[4+PinPositionOld]=0x2f;
                        memset(buffer+5+PinPositionOld,0xff,7);
                        buffer[4+PinPositionNew]=0x2f;
                        memset(buffer+5+PinPositionNew,0xff,7);

                        outlen=buffer[4]+5;

                        if(PinLength==0)
                        {
                           Min=4;
                           Max=12;
                           Condition=2;
                           PinLength=7;
                        }
                        else
                        {
                           Min=PinLength;
                           Max=PinLength;
                           if(PinLength<4 || PinLength>12)
                           {
                              response[0]=0x67;
                              response[1]=0x00;
                              *lenr=2;
                              return 0;
                           }
                           PinLength=7;
                           Condition=1;
                        }
                        break;
                     default:
                        response[0]=0x6A;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                  }
                  Prologue[0]=0;
                  if(((cjccidHANDLE)hDevice)->PCB_seq&0x01)
                     Prologue[1]=0x40;
                  else
                     Prologue[1]=0;
                  Prologue[2]=outlen;
                  inlen=sizeof(rbuffer);
                  switch(Res=cjccid_SecureMV(hDevice,timeout,PinPosition,PinType,
                                  PinLengthSize,PinLength,
                                  PinLengthPosition,
                                  Min,Max,
                                  bConfirmPIN,
                                  Condition,Prologue,
                                  PinPositionOld,PinPositionNew,
                                  buffer,outlen,rbuffer,&inlen))
                  {
                     case CJPP_SUCCESS:
                        if(((cjccidHANDLE)hDevice)->Protokoll==1)
                        {
                           Res=PVMVT1(hDevice,Res,rbuffer,inlen,&inlen);
                        }
                        if(Res==CJPP_SUCCESS)
                        {
                           if(*lenr<inlen)
                           {
                              *lenr=0;
                              return -11;
                           }
                           memcpy(response,rbuffer,inlen);
                           *lenr=inlen;
                           *sad=0;
                           return 0;
                        }
                        else
                        {
                           response[0]=0x6f;
                           response[1]=0x00;
                           *lenr=2;
                           return 0;
                        }
                        break;
                     case CJPP_ERR_PIN_CANCELED:
                        response[0]=0x64;
                        response[1]=0x01;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_PIN_TIMEOUT:
                        response[0]=0x64;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_PIN_DIFFERENT:
                        response[0]=0x64;
                        response[1]=0x02;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_NO_ICC:
                        response[0]=0x64;
                        response[1]=0xA1;
                        *lenr=2;
                        return 0;
                     case CJPP_ERR_NO_ACTIVE_ICC:
                        response[0]=0x64;
                        response[1]=0xA2;
                        *lenr=2;
                        return 0;

							case CJPP_ERR_WRITE_DEVICE:
                     case CJPP_ERR_DEVICE_LOST:
								*lenr=0;
								return -128;

                     default:
                        response[0]=0x6f;
                        response[1]=0x00;
                        *lenr=2;
                        return 0;
                  }
               }
               else
               {
                  response[0]=0x67;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               }
            }
         }
         else
         {
            response[0]=0x6D;
            response[1]=0x00;
            *lenr=2;
            return 0;
         }
      }
	  else if(cmd[0]==0x80)
	  {
		  if(cmd[1]==0x60)
		  {
              unsigned char *tag44;
              int taglen;
			  if(cmd[2]!=0x01 || cmd[3]!=0x00)
			  {
				 response[0]=0x6A;
				 response[1]=0x00;
				 *lenr=2;
				 return 0;
			  }
			  if(cmd[4]<3)
			  {
				 response[0]=0x67;
				 response[1]=0x00;
				 *lenr=2;
				 return 0;
			  }
              if((tag44=GetTag(cmd+5,cmd[4],0x44,&taglen))!=NULL)
			  {
				  if(taglen!=1)
				  {
					 response[0]=0x67;
					 response[1]=0x00;
					 *lenr=2;
					 return 0;
				  }
				  if((*tag44 & 0xf0)==0x90)
				  {
                     ((cjccidHANDLE)hDevice)->iic_offset_bytes=2;
				  }
				  else
				  {
                     ((cjccidHANDLE)hDevice)->iic_offset_bytes=1;
				  }
				  ((cjccidHANDLE)hDevice)->iic_pagesize=1<<(*tag44 & 0x0f);
			  }
              if((tag44=GetTag(cmd+5,cmd[4],0x45,&taglen))!=NULL)
			  {
				  if(taglen!=1)
				  {
					 response[0]=0x67;
					 response[1]=0x00;
					 *lenr=2;
					 return 0;
				  }
				  ((cjccidHANDLE)hDevice)->iic_deviceaddr=*tag44;
			  }
        	  response[0]=0x90;
			  response[1]=0x00;
			  *lenr=2;
			  return 0;
		  }
		  else
		  {
            response[0]=0x6D;
            response[1]=0x00;
            *lenr=2;
	    return 0;
		  }

	  }
      else
      {
         response[0]=0x6E;
         response[1]=0x00;
         *lenr=2;
         return 0;
      }
   }
   else if(*dad==0 && *sad==2)
   {
      unsigned short merklenr=*lenr;
      *dad=2;
      *sad=0;
      if(*lenr<2)
         return -11;
      switch(((cjccidHANDLE)hDevice)->Protokoll)
      {
         case 0:   /*T=0*/
            switch(APDU2TPDU_T0(hDevice,lenc,cmd,lenr,response))
            {
               case CJPP_ERR_RBUFFER_TO_SMALL:
                  *lenr=0;
                  return -11;
               case CJPP_SUCCESS:
                  return 0;
               case CJPP_ERR_PROT:
               case CJPP_ERR_TIMEOUT:
               case CJPP_ERR_PARITY:
                  *sad=1;
                  response[0]=0x6f;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_LEN:
                  *sad=1;
                  response[0]=0x67;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA1;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ACTIVE_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA2;
                  *lenr=2;
                  return 0;
					case CJPP_ERR_WRITE_DEVICE:
               case CJPP_ERR_DEVICE_LOST:
						*lenr=0;
						return -128;

               default:
                  return -1;
               }
            break;
         case 1:   /*T=1*/
            switch(APDU2TPDU_T1(hDevice,lenc,cmd,lenr,response))
            {
               case CJPP_SUCCESS:
                  if(*lenr>merklenr)
                  {
                     *lenr=0;
                     return -11;
                  }
                  return 0;
               case CJPP_ERR_PROT:
                  *sad=1;
                  response[0]=0x6f;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA1;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ACTIVE_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA2;
                  *lenr=2;
                  return 0;
					case CJPP_ERR_WRITE_DEVICE:
               case CJPP_ERR_DEVICE_LOST:
						*lenr=0;
						return -128;

               default:
                  return -1;
            }
            break;
			case 8:
			case 9:
			case 10:
            switch(SyncInterpreter(hDevice,lenc,cmd,lenr,response))
				{
               case CJPP_ERR_RBUFFER_TO_SMALL:
                  *lenr=0;
                  return -11;
               case CJPP_SUCCESS:
                  return 0;
               case CJPP_ERR_PROT:
                  *sad=1;
                  response[0]=0x6f;
                  response[1]=0x00;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA1;
                  *lenr=2;
                  return 0;
               case CJPP_ERR_NO_ACTIVE_ICC:
                  *sad=1;
                  response[0]=0x64;
                  response[1]=0xA2;
                  *lenr=2;
                  return 0;
					case CJPP_ERR_WRITE_DEVICE:
               case CJPP_ERR_DEVICE_LOST:
						*lenr=0;
						return -128;

               default:
						*lenr=0;
                  return -1;
    
				}
			default:
   			*lenr=0;
            return -1;

      }
   }
   else
   {
		*lenr=0;
      return -1;
   }
}


CJPP_EXP_TYPE char CJPP_CALL_CONV ctapiData(CCID_DEVICE hDevice,
					    unsigned char *dad,
					    unsigned char *sad,
					    unsigned short lenc,
					    const unsigned char *cmd,
					    unsigned short *lenr,
					    unsigned char *response){
  char res;

  cjppDebugCommand(hDevice,dad,sad,lenc,cmd,lenr,response);
  if (cmd && lenc) {
    unsigned char *nc_cmd;

    nc_cmd=malloc(lenc);
    if (nc_cmd==NULL)
      return -11; /* MEMORY */
    memmove(nc_cmd, cmd, lenc);
    res=ctapiData_inner(hDevice,dad,sad,lenc,nc_cmd,lenr,response);
    free(nc_cmd);
    if(res!=0)
      *lenr=0;
  }
  else
    return -1; /* invalid */

  cjppDebugResponse(hDevice,dad,sad,lenc,cmd,lenr,response,res);
  return res;
}


