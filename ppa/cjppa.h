#ifndef CJPPA_H
#define CJPPA_H

#ifdef _Windows
   #ifndef _WINDOWS
      #define _WINDOWS
   #endif
#endif

#ifdef _WINDOWS
   #include <windows.h>

   #ifdef BUILD_CJCCID_DLL
      #define CJPP_EXP_TYPE __declspec(dllexport)
   #else
      #define CJPP_EXP_TYPE __declspec(dllimport)
   #endif
   #define CJPP_CALLBACK_TYPE _stdcall
   #define  CJPP_CALL_CONV STDAPICALLTYPE
#else
// JUM
#include <inttypes.h>
	#define CJPP_EXP_TYPE
	#define CJPP_CALLBACK_TYPE
	#define  CJPP_CALL_CONV
	typedef void* HANDLE;
	//typedef uint32_t DWORD;
	typedef int OVERLAPPED;
	#define WINAPI
#endif

typedef void* CCID_CTX;
typedef void* CCID_DEVICE;




#ifdef __cplusplus
   #define CJPP_EXTERN extern "C"
#else
#define CJPP_EXTERN extern
#endif


CJPP_EXTERN CJPP_EXP_TYPE CCID_DEVICE CJPP_CALL_CONV
ctapiInit(const char *cDeviceName,
	  CCID_CTX p_hCtxData,
	  void (CJPP_CALLBACK_TYPE *CallbackStatus)(CCID_CTX p_hCtxData,
						    unsigned char CardStatus),
	  void (CJPP_CALLBACK_TYPE *CallbackKey)(CCID_CTX p_hCtxData,
						 unsigned char KeyStatus));
CJPP_EXTERN CJPP_EXP_TYPE char CJPP_CALL_CONV
ctapiClose(CCID_DEVICE hcjDevice);

CJPP_EXTERN CJPP_EXP_TYPE char CJPP_CALL_CONV
ctapiData(CCID_DEVICE hcjDevice,
	  unsigned char *dad,
	  unsigned char *sad,
	  unsigned short lenc,
	  const unsigned char *cmd,
	  unsigned short *lenr,
	  unsigned char *response);


CJPP_EXTERN CJPP_EXP_TYPE int CJPP_CALL_CONV cJppFirmwareUpdate(char *cDeviceName,              /*Geraetenamen: z.B. \\.\cjccid01*/
                       unsigned long Length,           /*Länge der UpdateDaten*/
                       unsigned char *UpdateData,      /*Daten wie in SGN-Datei*/
                       CCID_CTX p_hCtxData,                  /*this-Pointer der der
                                                         Callbackfunktion
                                                         übergeben wird */
                       void (CJPP_CALLBACK_TYPE *CallbackProgress)(CCID_CTX p_hCtxData),
                       int CountProgessCallbacks);      /*Anzahl der gewünschten
                                                         Callbacks*/



#endif
