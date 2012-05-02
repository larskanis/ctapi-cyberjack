
#include "Platform.h"
#include "Reader.h"
#include "USBUnix.h"
#include "eca_defines.h"

#include <stdio.h>


int readFile(const char *fname, uint8_t *buffer) {
  FILE *f;
  uint8_t *p;
  int len;

  f=fopen(fname, "r");
  if (f==NULL)
    return -1;

  p=buffer;
  len=0;
  while(!feof(f)) {
    int rv;

    rv=fread(p, 1, 1024, f);
    if (rv==0)
      break;
    p+=rv;
    len+=rv;
  }
  fclose(f);
  return len;
}



int main(int argc, char **argv) {
  CReader *r;
  int idx;
  char *devName;
  const char *fname1;
  const char *fname2;
  const char *fname3;
  uint8_t buffer1[64*1024];
  uint8_t buffer2[64*1024];
  uint8_t buffer3[64*1024];
  int len1;
  int len2;
  int len3;
  int rv;
  uint32_t result;
  cj_ModuleInfo ModuleInfo;
  uint32_t EstimatedUpdateTime;

  if (argc<5) {
    fprintf(stderr, "Usage:\n %s READERNR IMAGEFILE SIGNATUREFILE KEYFILE\n", argv[0]);
    return 1;
  }
  idx=atoi(argv[1]);
  fname1=argv[2];
  fname2=argv[3];
  fname3=argv[4];

  fprintf(stderr, "Reading image file\n");
  len1=readFile(fname1, buffer1);
  if (len1<1) {
    fprintf(stderr, "Error reading file \"%s\"\n", fname1);
    return 2;
  }

  fprintf(stderr, "Reading signature file\n");
  len2=readFile(fname2, buffer2);
  if (len2<1) {
    fprintf(stderr, "Error reading file \"%s\"\n", fname2);
    return 2;
  }

  fprintf(stderr, "Reading key file\n");
  len3=readFile(fname3, buffer3);
  if (len3<1) {
    fprintf(stderr, "Error reading file \"%s\"\n", fname3);
    return 2;
  }


  fprintf(stderr, "Opening reader\n");
  devName=CUSBUnix::createDeviceName(idx);
  if (devName==NULL) {
    fprintf(stderr, "Device %d not found\n", idx);
    return 2;
  }

  if (getenv("CJFLASH_DEBUG"))
    Debug.setLevelMask(0xffffffff);


  r=new CReader(devName);
  rv=r->Connect();
  if (rv!=CJ_SUCCESS) {
    fprintf(stderr, "Could not connect to reader (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Updating the keys\n");
  rv=r->CtKeyUpdate(buffer3, len3,&result);
  if (rv!=CJ_SUCCESS) {
	 fprintf(stderr, "Unable to update the keys (%d / %d)\n", rv,result);
	 return 2;
  }

  ModuleInfo.SizeOfStruct=sizeof(ModuleInfo);
  rv=r->CtGetModuleInfoFromFile(buffer1, len1,
				&ModuleInfo,
				&EstimatedUpdateTime);
  if (rv!=CJ_SUCCESS) {
    fprintf(stderr, "Unable to get module info (%d)\n", rv);
    return 2;
  }
  
  if(ModuleInfo.ID==MODULE_ID_KERNEL){
    /* for kernel updates we need the entire flash */
    fprintf(stderr,
	    "Updating the kernel module requires there is no "
	    "other module on the reader.\n"
	    "(please look at the display of "
	    "the reader and press \"OK\")\n");
    rv=r->CtDeleteALLModules(&result);
    if (rv!=CJ_SUCCESS) {
      fprintf(stderr, "Unable to flash reader (%d / %d)\n", rv,result);
      return 2;
    }
  }

  fprintf(stderr,
	  "Flashing image (please look at the "
	  "display of the reader and press \"OK\")\n");
  rv=r->CtLoadModule(buffer1, len1, buffer2, len2, &result);
  if (rv!=CJ_SUCCESS) {
    fprintf(stderr, "Unable to flash reader (%d / %d)\n", rv,result);
    return 2;
  }

  fprintf(stderr,
	  "Now the reader authentication is running.\n"
	  "Please look at the reader for checking all "
	  "versions and variants off the active modules\n");

  rv=r->CtShowAuth();
  if (rv!=CJ_SUCCESS) {
    fprintf(stderr, "Unable to show reader authentication (%d)\n", rv);
    return 2;
  }


  fprintf(stderr,
          "==========================================\n"
	  "Reader sucessfully flashed!\n"
          "\n"
	  "You might need to unplug and replug the reader.\n"
	  "==========================================\n");

  r->Disonnect();
  delete r;
  free(devName);

  return 0;
}



