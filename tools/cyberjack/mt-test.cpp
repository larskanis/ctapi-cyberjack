/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#ifdef HAVE_PTHREAD_H


#include "config_l.h"


#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>


//#define VERBOUS

//#define NUM_LOOPS 10000000
#define NUM_LOOPS 10000000/2


void printCurrentTime(FILE *f) {
  int rv;
  struct timeval tv;

  rv=gettimeofday(&tv, NULL);
  if (rv) {
    fprintf(f, "--------:-------- ");
  }
  else {
    fprintf(f, "%u08:%0u8 ", (unsigned int)(tv.tv_sec), (unsigned int)(tv.tv_usec));
  }
}



static int readerTest(int ctn) {
  int8_t res;
  uint8_t rsp[256+2];
  uint8_t sad, dad;
  uint16_t lenc, lenr;

  uint8_t apdu_reset_icc[]={0x20, 0x11, 0x01, 0x01, 0x00};

#ifdef VERBOUS
  printCurrentTime(stdout);
  fprintf(stdout, "Thread %d: Started\n", ctn);
#endif
  res=CT_init(ctn, ctn);
  if (res!=CT_API_RV_OK) {
    fprintf(stderr, "===== CTAPI(%d): Error on CT_init (%d)\n", ctn, res);
    return -1;
  }

  lenc=sizeof(apdu_reset_icc);
  lenr=sizeof(rsp);
  sad=2;
  dad=CT_API_AD_CT;
  res=CT_data(ctn, &dad, &sad, lenc, apdu_reset_icc, &lenr, rsp);
  if (res!=CT_API_RV_OK) {
    fprintf(stderr, "===== CTAPI::RESET_ICC(%d): Error on CT_data (%d)\n", ctn, res);
    CT_close(ctn);
    return -1;
  }
  if (lenr<2) {
    fprintf(stderr, "===== CTAPI::RESET_ICC(%d): Too few bytes returned (%d)\n", ctn, lenr);
    CT_close(ctn);
    return -1;
  }

  if (rsp[lenr-2]==0x90) {
    int loop;

#ifdef VERBOUS
    printCurrentTime(stdout);
    fprintf(stdout, "Thread %d: Got a card\n", ctn);
#endif

    for (loop=0; loop<NUM_LOOPS; loop++) {
      long int r;
      uint8_t apdu_select_mf[]={0x00, 0xa4, 0x00, 0x00, 0x02, 0x3f, 0x00, 0x00};

      lenc=sizeof(apdu_select_mf);
      lenr=sizeof(rsp);
      sad=2;
      dad=CT_API_AD_CT;
#ifdef VERBOUS
      printCurrentTime(stdout);
      fprintf(stdout, "Thread %d: -> Selecting MF\n", ctn);
#endif
      res=CT_data(ctn, &dad, &sad, lenc, apdu_select_mf, &lenr, rsp);
#ifdef VERBOUS
      printCurrentTime(stdout);
      fprintf(stdout, "Thread %d: <- Returned from selecting MF\n", ctn);
#endif
      if (res!=CT_API_RV_OK) {
	fprintf(stderr, "===== CTAPI::SELECT_MF(%d): Error on CT_data (%d)\n", ctn, res);
	CT_close(ctn);
	return -1;
      }
      if (lenr<2) {
	fprintf(stderr, "===== CTAPI::SELECT_MF(%d): Too few bytes returned (%d)\n", ctn, lenr);
	CT_close(ctn);
	return -1;
      }

      if (rsp[lenr-2]==0x90) {
#ifdef VERBOUS
	printCurrentTime(stdout);
	fprintf(stdout, "Thread %d: MF selected.\n", ctn);
#endif
      }
      else {
	fprintf(stderr, "===== CTAPI::SELECT_MF(%d): Command error (%02x %02x)\n", ctn,
		rsp[lenr-2], rsp[lenr-1]);
	CT_close(ctn);
	return -1;
      }

      r=(random()% 200);
#ifdef VERBOUS
      usleep(r);
#endif
    }
  }
  else {
    fprintf(stderr, "===== CTAPI::RESET_ICC(%d): No card (%02x %02x)\n", ctn,
	    rsp[lenr-2], rsp[lenr-1]);
    CT_close(ctn);
    return -1;
  }

  sleep(2);

  res=CT_close(ctn);
  if (res!=CT_API_RV_OK) {
    fprintf(stderr, "===== CTAPI(ctn): Error on CT_close (%d)\n", res);
    return -1;
  }

#ifdef VERBOUS
  fprintf(stdout, "Thread %d: Stopped\n", ctn);
#endif
  return 0;
}



static void *thread1(void *ptr) {
  readerTest(1);
  return NULL;
}



static void *thread2(void *ptr) {
  readerTest(2);
  return NULL;
}



static void *thread3(void *ptr) {
  readerTest(3);
  return NULL;
}



static void *thread4(void *ptr) {
  readerTest(4);
  return NULL;
}




int mtTest(int tc) {
  pthread_t tr1;
  pthread_t tr2;
  pthread_t tr3;
  pthread_t tr4;

  srandom(time(0));

  if (tc>0) {
    if (pthread_create(&tr1, NULL, &thread1, NULL)) {
      fprintf(stderr, "pthread_create: %s [%d] (1)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>1) {
    if (pthread_create(&tr2, NULL, thread2, NULL)) {
      fprintf(stderr, "pthread_create: %s [%d] (2)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>2) {
    if (pthread_create(&tr3, NULL, thread3, NULL)) {
      fprintf(stderr, "pthread_create: %s [%d] (3)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>3) {
    if (pthread_create(&tr4, NULL, thread4, NULL)) {
      fprintf(stderr, "pthread_create: %s [%d] (4)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>0) {
    if (pthread_join(tr1, NULL)) {
      fprintf(stderr, "pthread_join: %s [%d] (1)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>1) {
    if (pthread_join(tr2, NULL)) {
      fprintf(stderr, "pthread_join: %s [%d] (2)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>2) {
    if (pthread_join(tr3, NULL)) {
      fprintf(stderr, "pthread_join: %s [%d] (3)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  if (tc>3) {
    if (pthread_join(tr4, NULL)) {
      fprintf(stderr, "pthread_join: %s [%d] (4)\n",
	      strerror(errno), errno);
      return 2;
    }
  }

  return 0;
}


#endif


