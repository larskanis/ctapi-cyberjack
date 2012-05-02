#ifndef CJPP_H
#define CJPP_H

#include "ccid.h"
#include "cjppa.h"
#include "config_l.h"

#if defined(_LINUX)
#if 0
#define DEBUGP(format, ...) \
  rsct_log(CT_FLAGS_DEBUG_CJPPA, __FILE__, __LINE__, __FUNCTION__, format , ##__VA_ARGS__)
#else
# define DEBUGP(format, args...)
#endif
#endif


#define CJPP_APPLICATION              2

#define CJPP_SUCCESS                  0
#define CJPP_ERR_OPENING_DEVICE      -1
#define CJPP_ERR_WRITE_DEVICE        -2
#define CJPP_ERR_DEVICE_LOST         -3
#define CJPP_ERR_WRONG_ANSWER        -4
#define CJPP_ERR_SEQ                 -5
#define CJPP_ERR_WRONG_LENGTH        -6
#define CJPP_ERR_NO_ICC              -7
#define CJPP_ERR_OPEN_ICC            -8
#define CJPP_ERR_PARITY              -9
#define CJPP_ERR_TIMEOUT            -10
#define CJPP_ERR_LEN                -11
#define CJPP_ERR_RBUFFER_TO_SMALL   -12
#define CJPP_ERR_PROT               -13
#define CJPP_ERR_NO_ACTIVE_ICC      -14
#define CJPP_ERR_SIGN               -15
#define CJPP_ERR_WRONG_SIZE         -16
#define CJPP_ERR_PIN_TIMEOUT        -17
#define CJPP_ERR_PIN_CANCELED       -18
#define CJPP_ERR_PIN_DIFFERENT      -19
#define CJPP_ERR_FIRMWARE_OLD       -20
#define CJPP_NOT_UPDATABLE          -21
#define CJPP_ERR_NO_SIGN            -22
#define CJPP_ERR_WRONG_PARAMETER    -23

#if defined(_LINUX) 
#define CJPP_TEST(a)    do {						\
				int Res;				\
				if ( (Res=(a)) != CJPP_SUCCESS) {	\
					DEBUGP("error %d\n", Res);	\
					return Res;			\
				}					\
			} while (0);
#else
#define CJPP_TEST(a)     {						\
				int Res;				\
				if ( (Res=(a)) != CJPP_SUCCESS) {	\
					return Res;			\
				}					\
			}
#endif
			
#define CJPP_TEST2(a)   if((Res=(a))!=CJPP_SUCCESS){cjppTerminateThread(hThread);ctapiClose(hCt);return Res;}
#define CJPP_TEST3(a)   if((Res=(a))!=CJPP_SUCCESS){ctapiClose(hCt);return Res;}
//#define CJPP_TEST3(a)   if((Res=(a))!=CJPP_SUCCESS){cjppClose(hCtDevice);return Res;}


#pragma pack(1)
typedef struct
{
   unsigned char ActiveModule;
   unsigned short Version;
   unsigned char SignDate[11];
   unsigned char SignTime[6];
   unsigned char Status;
   unsigned char Config;
   unsigned short ApplicationVersion;
   unsigned short LoaderVersion;
   unsigned char Flags;
   unsigned char ProductionDate[11];
   unsigned char ProductionTime[6];
   unsigned char TestDate[11];
   unsigned char TestTime[6];
   unsigned char FirstDate[11];
   unsigned char FirstTime[6];
   unsigned char Seriennummer[20];
   unsigned short BootLoaderVersion;
}cjpp_Info;
#pragma pack()



typedef struct
{
   unsigned long SleepValue;
   void (CJPP_CALLBACK_TYPE *CallbackProgress)(CCID_CTX);
   CCID_CTX hClass;
   int CountProgessCallbacks;
   int Fini;
}ProgressStr;

#ifdef __cplusplus
extern "C" {
#endif

int cjppWriteAndRead(HANDLE cjppDevice,CCID_Message *Message,CCID_Response *Response);
int cjppGetDeviceInfo(HANDLE cjppDevice,cjpp_Info *cjppInfo);
int cjppStartLoader(HANDLE hCtDevice);
int cjppUpdateData(HANDLE hCtDevice,unsigned short addr,unsigned char *Data,unsigned char len);
int cjppVerifyUpdate(HANDLE hCtDevice);
int cjccid_iccPowerOn(HANDLE cjppDevice,unsigned char Voltage,unsigned char *ATR,int *len);
int cjppSetDateTime(HANDLE cjppDevice,unsigned char bOffset);
int cjppSetSerNumber(HANDLE cjppDevice);
int cjppTransfer(HANDLE cjppDevice,CCID_Message *Message,CCID_Response *Response);
int cjccid_iccPowerOff(HANDLE cjDevice);
int cjccid_XfrBlock(HANDLE cjDevice,unsigned char BWI,unsigned char *out,int out_len,unsigned char *in,int *in_len,unsigned short bLevelParam);
int cjccid_SetParameters(HANDLE cjDevice,unsigned char ProtocolNum,unsigned char FI_DI,unsigned char EDC,unsigned char GuardTime,unsigned char WaitingInteger);
int cjppInput(HANDLE cjppDevice,unsigned char *key,unsigned char timeout);
int cjccid_GetSlotStatus(HANDLE cjDevice);
#ifdef IT_TEST
int cjccid_SecurePV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len,unsigned char Slot);
#else
int cjccid_SecurePV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len);
#endif
int cjccid_SecureMV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char bConfirmPIN,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char OffsetOld,unsigned char OffsetNew,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len);

int cjppGetKeyVersion(HANDLE cjppDevice,unsigned char *KeyVersion);


/*Lowlevel Funktions*/

#define cjppWINAPI WINAPI


typedef struct
{
   OVERLAPPED o_transmit;
   OVERLAPPED o_receive;
	HANDLE tx_empty_event;
   unsigned char rbuffer[272];
	unsigned char SerialAckByte;
   HANDLE hDevice;
	HANDLE SeriellReaderWait;
	HANDLE SeriellAckWait;
#ifdef _WINDOWS
	CRITICAL_SECTION CritSectClose;
#endif
   unsigned char bSeq;
   unsigned char reader_id;
   HANDLE hIntThread;
   int is_usb;
	int WaitAck;
	int Connected;
	int isClass2;
	int nSerialExtraWaits;
	int bSerialDoPoll;
	int *Fini;
	int WaitResponse;
	char *ReaderName;
	void *Owner;
}cjppStruct;

typedef cjppStruct* cjppHANDLE;

HANDLE cjppCreate(const char *cDeviceName);
//void cjppClose(HANDLE cjppDevice);
void cjppSleep(unsigned long Value);
unsigned short cjppSWAB_WORD(unsigned short Value);
unsigned long cjppSWAB_DWORD(unsigned long Value);
unsigned short cjppSWAB_WORD_2(unsigned short Value);
unsigned long cjppSWAB_DWORD_2(unsigned long Value);
HANDLE cjppCreateThread(void (*ThreadRoutine)(void *),void *Params);
void cjppTerminateThread(HANDLE thread);
int cjppSeriellWaitInt(cjppHANDLE hDevice,CCID_Interrupt *Intr,int *Fini);
void cjppFillDevice(cjppHANDLE hcjppDevice);
unsigned long cjppGetLocalInfo(void);
unsigned long cjppGetUniqueID(void);
void cjccidClose(HANDLE cjppDevice);
int cjppWrite(HANDLE cjppDevice,CCID_Message *Message);
int cjppRead(HANDLE cjppDevice,CCID_Response *Response);
void cjppDebugCommand(CCID_DEVICE hDevice,unsigned char *dad,unsigned char *sad,unsigned short lenc,const unsigned char *cmd,unsigned short *lenr,unsigned char *response);
void cjppDebugResponse(CCID_DEVICE hDevice,unsigned char *dad,unsigned char *sad,unsigned short lenc,const unsigned char *cmd,unsigned short *lenr,unsigned char *response,char res);
void cjppDebugOut(cjppHANDLE hDevice,unsigned char *Caption,unsigned short lenc,unsigned char *cmd);
void cjppDebugSetLevel(int level);
int cjppEasyUpdate(HANDLE cjppDevice,unsigned char *prg);
void CardDependencyCorrectTA1(unsigned char *TA1);
void CardDependencyCorrectTC1(unsigned char *atr,int len,unsigned char *TC1);
char CJPP_CALL_CONV innerctapiClose(CCID_DEVICE hDevice);
int S7SyncCommand(HANDLE hDevice,unsigned short command,unsigned short outlen,unsigned char *out,unsigned short inlen,unsigned char *in);
int cjccid_GetStackSignCounter(HANDLE cjppDevice,unsigned short *Counter);








typedef enum{NO_DIALOG, COUNT_DIALOG, PIN_DIALOG}eDIALOG_STATE;

typedef struct
{
   void (CJPP_CALLBACK_TYPE *CallbackStatus)(HANDLE,unsigned char);
   void (CJPP_CALLBACK_TYPE *CallbackKey)(HANDLE,unsigned char);
#ifdef _WINDOWS
	CRITICAL_SECTION CritSectKeyEvent;
	eDIALOG_STATE state;
#endif
    
   CCID_CTX hClass;
   int Fini;
}Callbacks;



typedef struct
{
   cjppStruct cjppStr;
   HANDLE hIntThread;
   HANDLE hSerialPollThread;
   Callbacks backs;
   cjpp_Info Info;
   int Protokoll;
   unsigned char reader_path[10];
   unsigned char reader_file[2];
   unsigned char iic_deviceaddr;
   unsigned char iic_offset_bytes;
   unsigned char iic_pagesize;
   int reader_path_len;
   unsigned char IFSC;
   unsigned char PCB_seq;
   unsigned char EDC;
   unsigned char Status;
   int hasCanceled;
/*   int offset_dir;
   int length_dir;
   int offset_application_data;
   int length_application_data;*/
} cjccidStruct;

typedef cjccidStruct* cjccidHANDLE;

void HandleCyberJackInterruptData(cjccidHANDLE hDevice, CCID_Interrupt* Intr);

#ifdef _WINDOWS
	DWORD WINAPI IntThread(cjccidHANDLE hDevice);
   DWORD WINAPI SerialPollThread(cjccidHANDLE hDevice);
   void TestForOtherPID(unsigned char *ptr);
#endif

#ifdef _Macintosh
   void MacProcessInterruptEvents(OSType inEventType,void* inDataBuffer,UInt16 inDatalen,UInt32 inUserData);
	void InitCTAPIFramework(void);
#endif


DWORD WINAPI IntThread(cjccidHANDLE hDevice);
int cjppDoPoll(cjppHANDLE hDevice, int *Fini);

#define IOCTL_CCID_INT        0x80722216L
#define IOCTL_CCID_ABORT      0x80722218L


#ifdef __cplusplus
}
#endif
extern cjccidHANDLE AllHandles[];

#endif

