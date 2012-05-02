
#include "stdafx.h"
#include "Reader.h"
#include "eca_defines.h"

#include <stdio.h>
#include <time.h>

#include "cjBingo.h"



static RNDG_RESULT CheckResult(RNDG_RESULT Value)
{
	switch(Value)
	{
	case RNDG_TIMEOUT:
		printf("User timeout\n");
		break;
	case RNDG_ABORT:
		printf("Abort by user\n");
		break;
	case RNDG_COMMUNICATION_ERROR:
		printf("Communication error\n");
		exit(1);
		break;
	case RNDG_EXT_ERROR:
		printf("Error code: %08X\n",(unsigned int)RNDGetLastError());
		break;
	case RNDG_PROTOCOL_ERROR:
		printf("Protocol error\n");
		exit(2);
		break;
	case RNDG_READER_NOT_FOUND:
		printf("Reader not found\n");
		exit(3);
		break;
	case RNDG_READER_BUSY:
		printf("Reader busy\n");
		exit(4);
		break;
	case RNDG_READER_NOT_CONNECTED:
		printf("Reader is not connected\n");
		exit(5);
		break;
	default:;

	}
	return Value;
}


int main(int argc, char **argv) {
  int idx;
  uint8_t Signature[128];
  time_t t;
  int input;
  int i;
  uint64_t RNDs[64];


  if (argc<2) {
    fprintf(stderr, "Usage:\n %s READERNR\n", argv[0]);
    return 1;
  }
  
  idx=atoi(argv[1]);
  CheckResult(RNDGeneratorInit((uint16_t)idx));

  t=time(NULL);
  if(CheckResult(RNDGeneratorInternalAutheticate((uint64_t)t,Signature))!=RNDG_SUCCESS)
  {
	  printf("Error internal authentication\n");
	  exit(6);
  }

  for(;;)
  {
	  printf("[Count] Generate --- [-1] Show last --- [-2] Exit ? ");
	  scanf("%d",&input);
	  if(input==-2)
		  break;
	  else if(input==-1)
		  CheckResult(RNDGeneratorShowLastRNDs());
	  else
	  {
		  if(CheckResult(RNDGeneratorGenerateRND(input,RNDs))==RNDG_SUCCESS)
		  {
			  printf("\n");
			  for(i=0;i<input;i++)
			  {
				  printf("%2d: %05X-%05X    ",i+1,(unsigned int)((RNDs[i]>>20) & 0x000fffff),(unsigned int)(RNDs[i] & 0x000fffff));
				  if(((i+1)%4)==0)
					  printf("\n");
			  }
			  printf("\n");
		  }
	  }
  }
  return 0;
}




