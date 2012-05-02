#include "cjppa.h"
#include "cjpp.h"

#include <string.h>

#define SELECT            0xA4
#define READBINARY        0xB0
#define UPDATEBINARY      0xD6
#define VERIFY            0x20
#define MODIFY            0x24
#define DIRECT            0x60
#define GETKEYSTATUS      0xF2


extern int SyncInterpreter(HANDLE hDevice,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response);

static unsigned short TestSID(unsigned char *capdu,int capdu_len,unsigned char *rapdu,unsigned short *rapdu_len,unsigned short ID)
{
	unsigned short Result=0x6700;
	if(capdu_len>6)
	{
		if(capdu[4]==2)
		{
			if(capdu[5]==(unsigned char)(ID>>8) && capdu[6]==(unsigned char)ID)
				Result=0x9000;
			else
				Result=0x6a82;
		}
	}
	*rapdu_len=2;
	*rapdu++=(unsigned char)(Result>>8);
	*rapdu=(unsigned char)(Result);
	return Result;
}

static int S10SyncCommands(HANDLE hDevice,unsigned char Command,unsigned char addr,unsigned char outlen,unsigned char *outdata,int len,unsigned char *indata)
{
	int Result;
	unsigned char sbuffer[261];
	int rlen;
	sbuffer[0]=Command;
	sbuffer[1]=addr;
	sbuffer[2]=outlen;
	sbuffer[3]=(unsigned char)(len>>8);
	sbuffer[4]=(unsigned char)len;
	if(outlen>0)
		memcpy(sbuffer+5,outdata,outlen);
	rlen=len;
	if((Result=cjccid_XfrBlock(hDevice,0,sbuffer,5+outlen,indata,&rlen,0))!=CJPP_SUCCESS)
	{
		cjccid_iccPowerOff(hDevice);
	}
	return Result;
}

static int S9SyncCommands(HANDLE hDevice,unsigned char Command,unsigned short addr,unsigned char outlen,unsigned char *outdata,int len,unsigned char *indata)
{
	int Result;
	unsigned char sbuffer[261];
	int rlen;
	sbuffer[0]=(unsigned char)(addr>>8);
	sbuffer[1]=(unsigned char)addr;
	sbuffer[2]=outlen;
	sbuffer[3]=(unsigned char)(len>>8);
	sbuffer[4]=(unsigned char)len;
	if(outlen>0)
		memcpy(sbuffer+5,outdata,outlen);
	rlen=len;
	if((Result=cjccid_XfrBlock(hDevice,Command,sbuffer,5+outlen,indata,&rlen,0))!=CJPP_SUCCESS)
	{
		cjccid_iccPowerOff(hDevice);
	}
	return Result;
}

static int S8SyncCommands(HANDLE hDevice,unsigned short outlen,unsigned char *out,unsigned short inlen,unsigned char *in)
{
	int Result;
	int rlen;
	rlen=inlen;
	if((Result=cjccid_XfrBlock(hDevice,0,out,outlen,in,&rlen,(unsigned short)rlen))!=CJPP_SUCCESS)
	{
		cjccid_iccPowerOff(hDevice);
	}
	return Result;
}

static int S8SyncCommand(HANDLE hDevice,unsigned short offset,unsigned short outlen,unsigned char *out,unsigned short inlen,unsigned char *in)
{
	unsigned char buffer[260];
	unsigned char addr;
	
	addr=((cjccidHANDLE)hDevice)->iic_deviceaddr;
	if(((cjccidHANDLE)hDevice)->iic_offset_bytes==1)
	{
		addr|=((offset>>7) & 0x0e);
		buffer[1]=(unsigned char)offset;
		memcpy(buffer+2,out,outlen);
		outlen+=2;
	}
	else
	{
		buffer[1]=(unsigned char)(offset>>8);
		buffer[2]=(unsigned char)offset;
		memcpy(buffer+3,out,outlen);
		outlen+=3;
	}
	buffer[0]=addr;
	
	return S8SyncCommands(hDevice,outlen,buffer,inlen,in);
}


int S7SyncCommand(HANDLE hDevice,unsigned short command,unsigned short outlen,unsigned char *out,unsigned short inlen,unsigned char *in)
{
    int Result;
	unsigned char buffer[260];
	int rlen;
	
	buffer[0] = (unsigned char)command;
	memcpy(buffer+1,out,outlen);
	outlen+=1;
	
	rlen=inlen;
	
	if((Result=cjccid_XfrBlock(hDevice,0,buffer,outlen,in,&rlen,(unsigned short)rlen))!=CJPP_SUCCESS)
		
	{
		cjccid_iccPowerOff(hDevice);
	}
	
	return Result;
	
}



int SyncInterpreter(HANDLE hDevice,unsigned short lenc,unsigned char *cmd,unsigned short *lenr,unsigned char *response)
{
	int Res;
	unsigned char command=0;
	unsigned char buffer[260];
	unsigned char buffer2[4];
	unsigned short Rest;
	unsigned char i;
	unsigned short len;
	unsigned short len2;
	unsigned char cmd3;
	unsigned char Offset;
	unsigned char *ptr;
	unsigned char *ptr2;
	unsigned short glen;
	unsigned char cbyte;
	unsigned short Result=0x6700;
	unsigned short addr=(((unsigned short)cmd[2])<<8)+cmd[3];
	unsigned char byteaddr;
	
	
	switch(((cjccidHANDLE)hDevice)->Protokoll)
	{
		
		
    case 6:   
		if(cmd[0]!=0)									
		{
			response[0]=0x6E;
			response[1]=0x00;
			*lenr=2;
			return CJPP_SUCCESS;
		}
		
		switch(cmd[1])
		{
			
		case MODIFY:
		case VERIFY:
			
			command = cmd[3];
			if(command < 8)
				byteaddr = (command  * 8) + 0x40;
			else
				byteaddr = ((command & 0x07) * 8) + 0x44;
			if((lenc==8 && cmd[4]==3) ||
				(cmd[1]==MODIFY && lenc==11 && cmd[4]==6))
			{
				
				if (cmd[3] > 0x0F) 
				{
					response[0]=0x6A;
					response[1]=0x00;
					*lenr=2;
					break;
				}
				if((cmd[1]==VERIFY && lenc==8) ||
					(cmd[1]==MODIFY && lenc==11))
					
				{
					
					
					
					CJPP_TEST(S7SyncCommand(hDevice,0xB5,1,&byteaddr,1,buffer));
					
					
					if(buffer[0]==0)
					{
						response[0]=0x69;
						response[1]=0x83;
						*lenr=2;
						break;
					}	
					
					else
					{
						buffer2[0] = command;					
						memcpy(buffer2+1,cmd+5,3);
						CJPP_TEST(S7SyncCommand(hDevice,0xB3,4,buffer2,0,buffer));
						
						CJPP_TEST(S7SyncCommand(hDevice,0xB5,1,&byteaddr,1,buffer));
						
						if(buffer[0]==0xFF)						
							Result=0x9000;
						else
						{
							Result=0;
							for(;buffer[0];buffer[0]>>=1)
							{
								if(buffer[0] & 1)
									Result++;
							}
							Result|=0x63C0;
						}
						
						
						
						if (Result != 0x9000)
						{			
							response[0]=(unsigned char)(Result>>8);
							response[1]=(unsigned char)Result;
							*lenr=2;
							break;
						}
					}					
				}
				
				if (cmd[1]==MODIFY)
				{
					buffer2[0] = byteaddr + 1;
					if(lenc==8)
						memcpy(buffer2+1,cmd+5,3);
					else
						memcpy(buffer2+1,cmd+8,3);
					CJPP_TEST(S7SyncCommand(hDevice,0xB4,4,buffer2,0,0));						
					Result = 0x9000;
				}
				
			}
			else
			{
				Result=0x6700;
			}
			response[0]=(unsigned char)(Result>>8);
			response[1]=(unsigned char)Result;
			*lenr=2;
			break;			 
			
        case READBINARY:
			
			command = 0xB2;				
			ptr = cmd+2;
			
			CJPP_TEST(S7SyncCommand(hDevice,command,1,ptr,0,response));
			
			len=cmd[4];
			if(len==0)
				len=256;
			if(len+2>*lenr)
			{
				*lenr=0;
				return CJPP_ERR_RBUFFER_TO_SMALL;
			}
			
			if(cmd[2] > 0x08)
			{
				response[0]=0x6A;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			
			command = 0xB1;
			
			if (cmd[2] == 0x08)
			{
				command = 0xB5;
				
			}
			
			ptr = cmd+3;
			
			CJPP_TEST(S7SyncCommand(hDevice,command,1,ptr,len,response));
			
			*lenr=len+2;
			response[len]=0x90;
			response[len+1]=0;
			break;
			
        case UPDATEBINARY:
			
			if(lenc!=5+cmd[4])
			{
				response[0]=0x67;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			
			command = 0xB2;					
			ptr = cmd+2;
			if(*ptr!=8)
				CJPP_TEST(S7SyncCommand(hDevice,command,1,ptr,0,response));
			
			i = 5;
			command = 0xB0;
			len = cmd[4];
			len2=1+cmd[4];
			
			if(cmd[2] > 0x08)
			{
				response[0]=0x6A;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			
			if (cmd[2] == 0x08)
			{
				command = 0xB4;
			}
			
			if (len < 0x11)
			{	
				memcpy(buffer,cmd+3,1);
				memcpy(buffer+1,cmd+5,len);
				
				CJPP_TEST(S7SyncCommand(hDevice,command,len2,buffer,0,0));						
				*lenr=2;
				response[0]=0x90;
				response[1]=0;	
			}
			else
			{
				cmd3 = cmd[3];
				cmd3 = cmd3 & 0x0f;
				
				Rest = 0x10;
				Result=0x9000;
				
				while (len)
					
				{
					Rest=(Rest>len)?len:Rest;									
					Rest = Rest - cmd3;
					
					memcpy(buffer,cmd+3,1);								
					memcpy(buffer+1,cmd+i,Rest);						
					Rest+=1;
					
					if(S7SyncCommand(hDevice,command,Rest,buffer,0,0))
						
					{									
						*lenr=len+2;
						response[len]=0x6F;
						response[len+1]=0;
						break;
					}
					
					Rest-=1;
					cmd[3]+=Rest;
					len-=Rest;
					i += Rest;																				
					cmd3 = 0;
					
				}
				*lenr=2;
				response[0]=(unsigned char)(Result>>8);
				response[1]=(unsigned char)Result;						  
				break;
			}
			break;
			default:
				response[0]=0x6d;
				response[1]=0;
				*lenr=2;
		}
		break;
			
			
	case 7:   
		if(cmd[0]!=0)				
		{
			response[0]=0x6E;
			response[1]=0x00;
			*lenr=2;
			return CJPP_SUCCESS;
		}
		
		switch(cmd[1])
		{
			
		case MODIFY:
		case VERIFY:
			
			command = cmd[3];
			if(command < 2)
				byteaddr = (command  * 8) + 0x30;
			else
				byteaddr = ((command & 0x07) * 8) + 0x34;
			
			if((lenc==8 && cmd[4]==3) ||
				(cmd[1]==MODIFY && lenc==11 && cmd[4]==6))
			{
				if (cmd[3] > 0x03)		
				{
					response[0]=0x6A;
					response[1]=0x00;
					*lenr=2;
					break;
				}
				ptr=cmd+5;
				if((cmd[1]==VERIFY && lenc==8) ||
					(cmd[1]==MODIFY && lenc==11))
				{
					
					
					CJPP_TEST(S7SyncCommand(hDevice,0xBD,1,&byteaddr,1,buffer));
					
					if((addr=buffer[0])==0x00)
					{
						response[0]=0x69;
						response[1]=0x83;
						*lenr=2;
						break;
					}	
					
					else
					{
						
						command = (unsigned char)(0xB3 | (cmd[3] << 2));
						
						CJPP_TEST(S7SyncCommand(hDevice,command,3,ptr,0,0));						
						CJPP_TEST(S7SyncCommand(hDevice,command,3,ptr,0,0));						
						CJPP_TEST(S7SyncCommand(hDevice,0xBD,1,&byteaddr,1,buffer));
						
						if((addr=buffer[0])==0xFF)
						{
							Result=0x9000;
						}
						else
						{
							Result=0;
							for(;buffer[0];buffer[0]>>=1)
							{
								if(buffer[0] & 1)
									Result++;
							}
							Result|=0x63C0;
						}
						if (Result != 0x9000)
						{			
							response[0]=(unsigned char)(Result>>8);
							response[1]=(unsigned char)Result;
							*lenr=2;
							break;
						}
						
					}
				}
				
				
				if (cmd[1]==MODIFY)
				{
					byteaddr++;
					buffer2[0] = byteaddr;
					if(lenc==8)
						memcpy(buffer2+1,cmd+5,3);
					else
						memcpy(buffer2+1,cmd+8,3);
					CJPP_TEST(S7SyncCommand(hDevice,0xBC,4,buffer2,0,0));						
					Result = 0x9000;
				}
				
			}
			else
			{
				Result=0x6700;
			}
			response[0]=(unsigned char)(Result>>8);
			response[1]=(unsigned char)Result;
			*lenr=2;
			break;
			
			
        case READBINARY:
			
			if(cmd[3]>0x40 && cmd[2]==0)
			{
				cmd[2]=cmd[3]>>6;
				cmd[3]&=0x3f;
			}
			
			if(cmd[2] > 0x03)
			{
				response[0]=0x6E;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			
			
			if (cmd[2] == 0x00)
				cmd[2] = 0xB1;
			else if (cmd[2] == 0x01)
				cmd[2] = 0xB5;
			else if (cmd[2] == 0x02)
				cmd[2] = 0xB9;
			else if (cmd[2] == 0x03)
				cmd[2] = 0xBD;
			
			len=cmd[4];
			if(len==0)
				len=0x40;
			if (len > 0x3F)
			{len=0x40;}
			if(len+2>*lenr)
			{
				*lenr=0;
				return CJPP_ERR_RBUFFER_TO_SMALL;
			}
			
			ptr = cmd+3;				
			CJPP_TEST(S7SyncCommand(hDevice,cmd[2],1,ptr,len,response));
			
			*lenr=len+2;
			response[len]=0x90;
			response[len+1]=0;
			break;
			
		case UPDATEBINARY:
			
			if(cmd[3]>0x40 && cmd[2]==0)
			{
				cmd[2]=cmd[3]>>6;
				cmd[3]&=0x3f;
			}
			
			if(lenc!=5+cmd[4])
			{
				response[0]=0x67;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			
			if (cmd[2] == 0x00)
			{
				cmd[2] =0xB0;
			}
			
			else if (cmd[2] == 0x01)
			{
				cmd[2] =0xB4;
			}
			
			else if (cmd[2] == 0x02)
			{
				cmd[2] =0xB8;
			}
			
			else if (cmd[2] == 0x03)
			{
				cmd[2] =0xBc;
			}
			
			i = 5;
			len = cmd[4];
			len2=1+cmd[4];
			
			if (len < 0x09)
			{	
				memcpy(buffer,cmd+3,1);
				memcpy(buffer+1,cmd+5,len);
				
				CJPP_TEST(S7SyncCommand(hDevice,cmd[2],len2,buffer,0,0));						
				*lenr=2;
				response[0]=0x90;
				response[1]=0;							
			}
			else
			{
				cmd3 = cmd[3];
				cmd3 = cmd3 & 0x0f;
				
				if (cmd3 >0x08 )
				{
					if (cmd3 <0x11)
					{
						cmd3 -=8;
					}
				}
				
				Rest = 0x08;
				Result=0x9000;
				while (len)
					
				{
					Rest=(Rest>len)?len:Rest;									
					Rest = Rest - cmd3;
					
					memcpy(buffer,cmd+3,1);
					memcpy(buffer+1,cmd+i,Rest);
					
					Rest+=1;							
					
					if(S7SyncCommand(hDevice,cmd[2],Rest,buffer,0,0))
						
					{									
						*lenr=len+2;
						response[len]=0x6F;
						response[len+1]=0;
						break;
					}
					
					Rest-=1;
					cmd[3]+=Rest;
					len-=Rest;
					i += Rest;																				
					cmd3 = 0;
					
				}
				*lenr=2;
				response[0]=(unsigned char)(Result>>8);
				response[1]=(unsigned char)Result;						  
				break;
			}
			break;
		default:
			response[0]=0x6d;
			response[1]=0;
			*lenr=2;
			}
			break;
			
			
	case 8:
		if(cmd[0]!=0 && (cmd[0]!=0x80|| cmd[1]!=GETKEYSTATUS))				
		{
			response[0]=0x6E;
			response[1]=0x00;
			*lenr=2;
			return CJPP_SUCCESS;
		}
		switch(cmd[1])
		{
		case SELECT:
			if((Result=TestSID(cmd,lenc,response,lenr,0x3F00))==0x9000)
				((cjccidHANDLE)hDevice)->IFSC=0;
			response[0]=(unsigned char)(Result>>8);
			response[1]=(unsigned char)Result;
			*lenr=2;
			break;
		case READBINARY:
			
			len=cmd[4];
			if(len==0)
				len=256;
			if(len+2>*lenr)
			{
				*lenr=0;
				return CJPP_ERR_RBUFFER_TO_SMALL;
			}
			CJPP_TEST(S8SyncCommand(hDevice,(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]),0,0,len,response));
			*lenr=len+2;
			response[len]=0x90;
			response[len+1]=0;
			break;
		case UPDATEBINARY:
			if(lenc!=5+cmd[4])
			{
				response[0]=0x67;
				response[1]=0;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			len=cmd[4];
			ptr=cmd+5;
			Result=0x9000;
			addr=(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]);
			while(len)
			{
				Rest=((cjccidHANDLE)hDevice)->iic_pagesize-(addr%((cjccidHANDLE)hDevice)->iic_pagesize);
				Rest=(Rest>len)?len:Rest;
				if(S8SyncCommand(hDevice,addr,Rest,ptr,0,0)!=CJPP_SUCCESS)
				{
					Result=0x6F00;
					break;
				}
				addr+=Rest;
				len-=Rest;
				ptr+=Rest;
			}
			response[0]=(unsigned char)(Result>>8);
			response[1]=(unsigned char)Result;
			*lenr=2;
            break;
			
		default:
			response[0]=0x6d;
			response[1]=0;
			*lenr=2;
		}
		break;
		case 9:
			if(cmd[0]!=0 && (cmd[0]!=0x80 || cmd[1]!=GETKEYSTATUS))
			{
				response[0]=0x6E;
				response[1]=0x00;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			if(cmd[2]>3)
			{
				response[0]=0x6A;
				response[1]=0x00;
				*lenr=2;
				return CJPP_SUCCESS;
			}
			switch(cmd[1])
			{
            case SELECT:
				if((Result=TestSID(cmd,lenc,response,lenr,0x3F00))==0x9000)
					((cjccidHANDLE)hDevice)->IFSC=0;
				else if((Result=TestSID(cmd,lenc,response,lenr,0x2F09))==0x9000)
					((cjccidHANDLE)hDevice)->IFSC=1;
				response[0]=(unsigned char)(Result>>8);
				response[1]=(unsigned char)Result;
				*lenr=2;
				break;
            case READBINARY:
				if(((cjccidHANDLE)hDevice)->IFSC==0)
				{
					len=cmd[4];
					if(len==0)
						len=256;
					if(len+2>*lenr)
					{
						*lenr=0;
						return CJPP_ERR_RBUFFER_TO_SMALL;
					}
					CJPP_TEST(S9SyncCommands(hDevice,0x0e,(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]),0,0,len,response));
				}
				else
				{
					len=cmd[4];
					if(len==0)
						len=256;
					if(len+2>*lenr)
					{
						*lenr=0;
						return CJPP_ERR_RBUFFER_TO_SMALL;
					}
					glen=len>>3;
					if((len&7)!=0)
						glen++;
					CJPP_TEST(S9SyncCommands(hDevice,0x0c,(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]),0,0,glen,buffer));
					ptr=response;
					memset(ptr,0,len);
					Rest=len;
					for(command=0;command<glen;command++)
					{
						for(Res=0;Res<8;Res++)
						{
							if(buffer[command] & 1)
								*ptr=0x01;
							ptr++;
							buffer[command] >>= 1;
							if(--Rest==0)
								break;
						}
					}
				}
				*lenr=len+2;
				response[len]=0x90;
				response[len+1]=0;
				break;
            case UPDATEBINARY:
				if(((cjccidHANDLE)hDevice)->IFSC==0)
					command=0x33;
				else
					command=0x30;
				CJPP_TEST(S9SyncCommands(hDevice,command,(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]),cmd[4],cmd+5,0,0));
				if(((cjccidHANDLE)hDevice)->IFSC==0)
				{
					CJPP_TEST(S9SyncCommands(hDevice,0x0e,(unsigned short)((((unsigned short)cmd[2])<<8)+cmd[3]),0,0,cmd[4],buffer));
					if(memcmp(buffer,cmd+5,cmd[4])!=0)
					{
						response[0]=0x65;
						response[1]=0x81;
						*lenr=2;
						return CJPP_SUCCESS;
					}
				}
				response[0]=0x90;
				response[1]=0;
				*lenr=2;
				break;
            case VERIFY:
            case MODIFY:
				if((lenc==7 && cmd[4]==2) ||
					(lenc==9 && cmd[4]==4))
				{
					ptr=cmd+5;
					if(cmd[1]==VERIFY || lenc==9)
					{
						CJPP_TEST(S9SyncCommands(hDevice,0x0e,1021,0,0,1,buffer));
						if((addr=buffer[0])==0)
							Result=0x6983;
						else
						{
							for(Offset=1;(addr & (Offset ^ 0xff))==addr;Offset<<=1);
							addr&= (Offset ^ 0xff);
							cbyte=(unsigned char)addr;
							CJPP_TEST(S9SyncCommands(hDevice,0x32,1021,1,&cbyte,0,0));
							CJPP_TEST(S9SyncCommands(hDevice,0x0d,1022,2,ptr,0,0));
							ptr+=2;
							cbyte=0xff;
							CJPP_TEST(S9SyncCommands(hDevice,0x33,1021,1,&cbyte,0,0));
							CJPP_TEST(S9SyncCommands(hDevice,0x0e,1021,0,0,1,buffer));
							if((Offset=buffer[0])!=0xff)
							{
								len=0;
								for(addr=0;addr<8;addr++)
								{
									if(Offset&1)
										len++;
									Offset>>=1;
								}
								Result=(((unsigned short)len)) | 0x63c0;
							}
						}
					}
					if(Result==0x6700)
					{
						if(cmd[1]==VERIFY)
						{
							Result=0x9000;
						}
						else
						{
							ptr2=ptr;
							cbyte=0xff;
							CJPP_TEST(S9SyncCommands(hDevice,0x33,1021,1,&cbyte,0,0));
							CJPP_TEST(S9SyncCommands(hDevice,0x33,1022,2,ptr,0,0));
							ptr+=2;
							CJPP_TEST(S9SyncCommands(hDevice,0x0e,1021,0,0,1,buffer));
							if((Offset=buffer[0])!=0xff)
							{
								len=0;
								for(addr=0;addr<8;addr++)
								{
									if(Offset&1)
										len++;
									Offset>>=1;
								}
								Result=((unsigned short)len) | 0x63C0;
							}
							else
							{
								Result=0x9000;
							}
						}
					}
					memset(buffer,0,1);
				}
				response[0]=(unsigned char)(Result>>8);
				response[1]=(unsigned char)Result;
				*lenr=2;
				break;
			case GETKEYSTATUS:
				CJPP_TEST(S9SyncCommands(hDevice,0x0e,1021,0,0,1,buffer));
				Offset=buffer[0];
				len=0;
				for(addr=0;addr<8;addr++)
				{
					if(Offset&1)
						len++;
					Offset>>=1;
				}
				response[0]=(unsigned char)len;
				response[1]=0x90;
				response[2]=0x00;
				*lenr=3;
				break;
				
				
            default:
				response[0]=0x6d;
				response[1]=0;
				*lenr=2;
         }
         break;
      case 10:
		  
		  
		  if(cmd[0]!=0 && (cmd[0]!=0x80 || cmd[1]!=GETKEYSTATUS))
		  {
			  response[0]=0x6E;
			  response[1]=0x00;
			  *lenr=2;
			  return CJPP_SUCCESS;
		  }
		  if(cmd[2]!=0)
		  {
			  response[0]=0x6A;
			  response[1]=0x00;
			  *lenr=2;
			  return CJPP_SUCCESS;
		  }
		  switch(cmd[1])
		  {
		  case SELECT:
			  if((Result=TestSID(cmd,lenc,response,lenr,0x3F00))==0x9000)
                  ((cjccidHANDLE)hDevice)->IFSC=0;
			  else if((Result=TestSID(cmd,lenc,response,lenr,0x2F09))==0x9000)
                  ((cjccidHANDLE)hDevice)->IFSC=1;
			  response[0]=(unsigned char)(Result>>8);
			  response[1]=(unsigned char)Result;
			  *lenr=2;
			  break;
		  case READBINARY:
			  if(((cjccidHANDLE)hDevice)->IFSC==0)
			  {
				  len=cmd[4];
				  if(len==0)
					  len=256;
				  if(len+2>*lenr)
				  {
					  *lenr=0;
					  return CJPP_ERR_RBUFFER_TO_SMALL;
				  }
				  CJPP_TEST(S10SyncCommands(hDevice,0x30,cmd[3],0,0,len,response));
			  }
			  else
			  {
				  if(cmd[2]!=0 || cmd[3]>31 || cmd[3]+cmd[4]>32)
				  {
					  response[0]=0x6A;
					  response[1]=0x00;
					  *lenr=2;
					  return CJPP_SUCCESS;
				  }
				  len=cmd[4];
				  if(len==0)
					  len=32;
				  if(len>32-cmd[3])
					  len=32;
				  if(len+2>*lenr)
				  {
					  *lenr=0;
					  return CJPP_ERR_RBUFFER_TO_SMALL;
				  }
				  CJPP_TEST(S10SyncCommands(hDevice,0x34,0,0,0,4,buffer));
				  ptr=buffer+4;
				  memset(ptr,0,32);
				  for(command=0;command<4;command++)
				  {
					  for(Res=0;Res<8;Res++)
					  {
						  if(buffer[command] & 1)
							  *ptr=0x01;
						  ptr++;
						  buffer[command] >>= 1;
					  }
				  }
				  memcpy(response,buffer+4+cmd[3],len);
			  }
			  *lenr=len+2;
			  response[len]=0x90;
			  response[len+1]=0;
			  break;
		  case UPDATEBINARY:
			  if(((cjccidHANDLE)hDevice)->IFSC==0)
                  command=0x38;
			  else
                  command=0x3c;
			  CJPP_TEST(S10SyncCommands(hDevice,command,cmd[3],cmd[4],cmd+5,0,0));
			  if(((cjccidHANDLE)hDevice)->IFSC==0)
			  {
                  CJPP_TEST(S10SyncCommands(hDevice,0x30,cmd[3],0,0,cmd[4],buffer));
				  if(memcmp(buffer,cmd+5,cmd[4])!=0)
				  {
					  response[0]=0x65;
					  response[1]=0x81;
					  *lenr=2;
					  return CJPP_SUCCESS;
				  }
			  }
			  response[0]=0x90;
			  response[1]=0;
			  *lenr=2;
			  break;
		  case VERIFY:
		  case MODIFY:
			  
			  
			  if((lenc==8 && cmd[4]==3) ||
                  (lenc==11 && cmd[4]==6))
				  
				  
			  {
                  ptr=cmd+5;
                  if(cmd[1]==VERIFY || lenc==11)
                  {
					  CJPP_TEST(S10SyncCommands(hDevice,0x31,0xff,0,0,4,buffer));
					  if((addr=buffer[0])==0)
						  Result=0x6983;
					  else
					  {
						  for(Offset=1;(addr & (Offset ^ 0xff))==addr;Offset<<=1);
						  addr&= (Offset ^ 0xff);
						  cbyte=(unsigned char)addr;
						  CJPP_TEST(S10SyncCommands(hDevice,0x39,0,1,&cbyte,0,0));
						  CJPP_TEST(S10SyncCommands(hDevice,0x33,1,3,ptr,0,0));
						  ptr+=3;
						  cbyte=7;
						  CJPP_TEST(S10SyncCommands(hDevice,0x39,0,1,&cbyte,0,0));
						  CJPP_TEST(S10SyncCommands(hDevice,0x31,0xff,0,0,4,buffer));
						  if((Offset=buffer[0])!=7)
						  {
							  len=0;
							  for(addr=0;addr<8;addr++)
							  {
								  if(Offset&1)
									  len++;
								  Offset>>=1;
							  }
							  Result=(((unsigned short)len)) | 0x63c0;
						  }
					  }
                  }
                  if(Result==0x6700)
                  {
					  if(cmd[1]==VERIFY)
					  {
						  Result=0x9000;
					  }
					  else
					  {
						  ptr2=ptr;
						  cbyte=7;
						  CJPP_TEST(S10SyncCommands(hDevice,0x39,0,1,&cbyte,0,0));
						  CJPP_TEST(S10SyncCommands(hDevice,0x39,1,3,ptr,0,0));
						  ptr+=3;
						  CJPP_TEST(S10SyncCommands(hDevice,0x31,0xff,0,0,4,buffer));
						  if((Offset=buffer[0])!=7)
						  {
							  len=0;
							  for(addr=0;addr<8;addr++)
							  {
								  if(Offset&1)
									  len++;
								  Offset>>=1;
							  }
							  Result=((unsigned short)len) | 0x63C0;
						  }
						  else
						  {
							  Result=0x9000;
						  }
					  }
                  }
                  memset(buffer,0,4);
			  }
			  response[0]=(unsigned char)(Result>>8);
			  response[1]=(unsigned char)Result;
			  *lenr=2;
			  break;
		  case GETKEYSTATUS:
			  CJPP_TEST(S10SyncCommands(hDevice,0x31,0xff,0,0,4,buffer));
			  len=0;
			  Offset=buffer[0];
			  for(addr=0;addr<8;addr++)
			  {
                  if(Offset&1)
					  len++;
                  Offset>>=1;
			  }
			  response[0]=(unsigned char)len;
			  response[1]=0x90;
			  response[2]=0x00;
			  *lenr=3;
			  break;
			  
			  
		  default:
			  response[0]=0x6d;
			  response[1]=0;
			  *lenr=2;
         }
         break;
      default:
		  response[0]=0x6f;
		  response[1]=0;
		  *lenr=2;
   }
   return CJPP_SUCCESS;
}
