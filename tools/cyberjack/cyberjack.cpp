/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "Platform.h"

#include "config_l.h"

#include "checksuite.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef OS_LINUX
# include <linux/kd.h>
# include <sys/ioctl.h>
#endif
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>



typedef void* (*XOPENDISPLAY_FN)(char *display_name);
typedef int (*XCLOSEDISPLAY_FN)(void *display);
typedef int (*XBELL_FN)(void *display, int percent);
typedef int (*XFLUSH_FN)(void *display);


/* forward declarations */
int mtTest(int tc);



int check() {
  CheckSuite *cs;
  std::string xmlString;
  std::string reportString;
  std::string hintString;
  bool b;
  FILE *f;

  cs=new CheckSuite();
  cs->addStandardModules(true);
  b=cs->performChecks(xmlString, reportString, hintString);

  f=fopen("cyberjack.xml", "w+");
  if (f) {
    fprintf(f, "%s", xmlString.c_str());
    fclose(f);
  }
  else {
    fprintf(stderr, "fopen(\"cyberjack.xml\"): %s\n", strerror(errno));
  }

  f=fopen("cyberjack-report.log", "w+");
  if (f) {
    fprintf(f, "%s", reportString.c_str());
    fclose(f);
  }
  else {
    fprintf(stderr, "fopen(\"cyberjack-report.log\"): %s\n", strerror(errno));
  }

  f=fopen("cyberjack-hints.log", "w+");
  if (f) {
    fprintf(f, "%s", hintString.c_str());
    fclose(f);
  }
  else {
    fprintf(stderr, "fopen(\"cyberjack-hints.log\"): %s\n", strerror(errno));
  }

  fprintf(stderr,
	  "\n"
	  "Es wurden 3 Dateien im aktuellen Verzeichnis angelegt:\n"
	  "- cyberjack-report.log: Enthaelt die Ergebnisse der Tests\n"
	  "- cyberjack-hints.log : Enthaelt moeglicherweise Hinweise\n"
	  "                        zu gefundenen Problemen und deren\n"
	  "                        Behebung.\n"
	  "- cyberjack.xml       : Enthaelt die Testergebnisse in fuer\n"
	  "                        den Support aufbereiteter Form.\n"
	  "Bitte senden Sie bei Problemen die Datei \"cyberjack.xml\"\n"
	  "an den Linux-Support von Reiner SCT.\n"
	  "\n");

  return 0;
}




int addFlags(unsigned int fl) {
  unsigned int oldFlags;

  if (rsct_config_init()) {
    fprintf(stderr, "ERROR: Could not init configuration.\n");
    return 2;
  }

  if (fl & CT_FLAGS_ECOM_KERNEL_OLD) {
    /* this flag is now ignored by the driver in order to turn off the use of
     * the kernel module for new drivers. However, if this flag is given here
     * (some documentation will refer to that old flag) we use the correct one.
     */
    fl|=CT_FLAGS_ECOM_KERNEL;
  }

  oldFlags=rsct_config_get_flags();
  oldFlags|=fl;
  rsct_config_set_flags(oldFlags);

  if (rsct_config_save()) {
    fprintf(stderr,
	    "ERROR: Could not save configuration, "
	    "maybe you need to be root?\n");
    rsct_config_fini();
    return 3;
  }
  rsct_config_fini();

  fprintf(stderr, "INFO: Added flags %08x (now: %08x)\n", fl, oldFlags);

  return 0;
}



int delFlags(unsigned int fl) {
  unsigned int oldFlags;

  if (rsct_config_init()) {
    fprintf(stderr, "ERROR: Could not init configuration.\n");
    return 2;
  }

  if (fl & CT_FLAGS_ECOM_KERNEL_OLD) {
    /* this flag is now ignored by the driver in order to turn off the use of
     * the kernel module for new drivers. However, if this flag is given here
     * (some documentation will refer to that old flag) we use the correct one.
     */
    fl|=CT_FLAGS_ECOM_KERNEL;
  }

  oldFlags=rsct_config_get_flags();
  oldFlags&=~fl;
  rsct_config_set_flags(oldFlags);

  if (rsct_config_save()) {
    fprintf(stderr,
	    "ERROR: Could not save configuration, "
	    "maybe you need to be root?\n");
    rsct_config_fini();
    return 3;
  }
  rsct_config_fini();

  fprintf(stderr, "INFO: Cleared flags %08x (now: %08x)\n", fl, oldFlags);

  return 0;
}



int getFlags() {
  unsigned int fl;

  if (rsct_config_init()) {
    fprintf(stderr, "ERROR: Could not init configuration.\n");
    return 2;
  }

  fl=rsct_config_get_flags();
  fprintf(stderr, "INFO: Flags: %08x\n", fl);

  rsct_config_fini();

  return 0;
}



int beepConsole1() {
#ifdef OS_LINUX
  int fd;

  fd=open("/dev/console", O_WRONLY);
  if (fd!=-1) {
    int arg;
    int rv;

    arg=(150<<16)+440; /* "A" for 150ms */
    rv=ioctl(fd, KDMKTONE, arg);
    close(fd);
    if (rv) {
      fprintf(stderr, "Error on IOCTL(KDMTONE) with /dev/console: %d [%s]\n",
	      errno, strerror(errno));
      return 2;
    }
  }
  else {
    fprintf(stderr, "Error opening /dev/console: %d [%s]\n",
	   errno, strerror(errno));
    return 2;
  }
  return 0;
#else
  fprintf(stderr, "Beep using Linux IOCTL not supported\n");
  return 2;
#endif
}



int beepConsole2() {
  printf("\a");
  fflush(stdout);
  return 0;
}



int beepX11() {
  void *xhdl;
  void *display;
  void *fn;
  XOPENDISPLAY_FN xOpenDisplay;
  XCLOSEDISPLAY_FN xCloseDisplay;
  XBELL_FN xBell;
  XFLUSH_FN xFlush;
  int rv;

  xhdl = dlopen("libX11.so", RTLD_NOW);
  if (xhdl==NULL) {
    fprintf(stderr, "Error loading LibX11.so: %d [%s]\n",
	    errno, strerror(errno));
    return 2;
  }

  fn=dlsym(xhdl, "XOpenDisplay");
  if (fn==NULL) {
    fprintf(stderr, "Symbol XOpenDisplay not found: %d [%s]\n",
	    errno, strerror(errno));
    return 2;
  }
  xOpenDisplay=(XOPENDISPLAY_FN) fn;

  fn=dlsym(xhdl, "XCloseDisplay");
  if (fn==NULL) {
    fprintf(stderr, "Symbol XCloseDisplay not found: %d [%s]\n",
	    errno, strerror(errno));
    return 2;
  }
  xCloseDisplay=(XCLOSEDISPLAY_FN) fn;

  fn=dlsym(xhdl, "XBell");
  if (fn==NULL) {
    fprintf(stderr, "Symbol XBell not found: %d [%s]\n",
	    errno, strerror(errno));
    return 2;
  }
  xBell=(XBELL_FN) fn;

  fn=dlsym(xhdl, "XFlush");
  if (fn==NULL) {
    fprintf(stderr, "Symbol XFlush not found: %d [%s]\n",
	    errno, strerror(errno));
    return 2;
  }
  xFlush=(XFLUSH_FN) fn;

  /* connect to X11 server */
  display = (void *) xOpenDisplay(NULL);
  if (display==NULL) {
    fprintf(stderr, "Error connecting to X11 server\n");
    return 2;
  }

  xBell(display, 100);
  xFlush(display);

  rv=xCloseDisplay(display);
  if (rv) {
    fprintf(stderr, "XCloseDisplay returned: %d\n", rv);
    return 2;
  }
  display=NULL;
  dlclose(xhdl);

  return 0;
}



int setVar(const char *name, const char *val) {
  if (rsct_config_init()) {
    fprintf(stderr, "ERROR: Could not init configuration.\n");
    return 2;
  }

  rsct_config_set_var(name, val);

  if (rsct_config_save()) {
    fprintf(stderr,
	    "ERROR: Could not save configuration, "
	    "maybe you need to be root?\n");
    rsct_config_fini();
    return 3;
  }
  rsct_config_fini();

  fprintf(stderr, "INFO: Variable \"%s\" set to \"%s\"\n", name, val);

  return 0;
}



int getVar(const char *name) {
  const char *s;

  if (rsct_config_init()) {
    fprintf(stderr, "ERROR: Could not init configuration.\n");
    return 2;
  }

  s=rsct_config_get_var(name);
  if (s && *s)
    fprintf(stderr, "Value of variable \"%s\": \"%s\"\n", name, s);
  else
    fprintf(stderr, "Value of variable \"%s\" is empty\n", name);
  rsct_config_fini();

  return 0;
}




int main(int argc, char **argv) {
  if (argc>1) {
    if (strcmp(argv[1], "check")==0)
      return check();
    else if (strcmp(argv[1], "addflags")==0) {
      if (argc>2) {
	int fl;

	if (sscanf(argv[2], "%i", &fl)!=1) {
	  fprintf(stderr, "ERROR: Not an integer\n");
          return 1;
	}
	return addFlags(fl);
      }
      else {
	fprintf(stderr, "ERROR: Flags missing.\n");
        return 1;
      }
    }
    else if (strcmp(argv[1], "delflags")==0) {
      if (argc>2) {
	int fl;

	if (sscanf(argv[2], "%i", &fl)!=1) {
	  fprintf(stderr, "ERROR: Not an integer\n");
          return 1;
	}
        return delFlags(fl);
      }
      else {
	fprintf(stderr, "ERROR: Flags missing.\n");
        return 1;
      }
    }
    else if (strcmp(argv[1], "getflags")==0) {
      return getFlags();
    }
    else if (strcmp(argv[1], "setvar")==0) {
      if (argc>3) {
        /* set variable */
        return setVar(argv[2], argv[3]);
      }
      else {
	fprintf(stderr, "ERROR: Name and/or value missing.\n");
        return 1;
      }
    }
    else if (strcmp(argv[1], "getvar")==0) {
      if (argc>2) {
        /* set variable */
        return getVar(argv[2]);
      }
      else {
        fprintf(stderr, "ERROR: Name missing.\n");
        return 1;
      }
    }
    else if (strcmp(argv[1], "beep1")==0) {
      return beepConsole1();
    }
    else if (strcmp(argv[1], "beep2")==0) {
      return beepConsole2();
    }
    else if (strcmp(argv[1], "beep3")==0) {
      return beepX11();
    }
    else if (strcmp(argv[1], "mttest")==0) {
#ifdef HAVE_PTHREAD_H
      int tc=4;

      if (argc>2)
        tc=atoi(argv[2]);
      return mtTest(tc);
#else
      fprintf(stderr, "This tool was not compiled with PTHREAD support.\n");
      return 1;
#endif
    }
    else {
      fprintf(stderr,
	      "Usage: %s [COMMAND] [ARGUMENTS]\n"
              "\n"
	      "COMMAND can be:\n"
	      " check:    performs system checks\n"
	      " addflags: adds flags in the CTAPI configuration file\n"
	      "           (on this system %s)\n"
              "           This command needs an argument (the flags to add)\n"
	      " delflags: clears flags in the CTAPI configuration file\n"
	      "           This command needs an argument (the flags to clear)\n"
	      " beep1:    Beeps using /dev/console\n"
	      " beep2:    Beeps by outputting sending a control character to stdout\n"
	      " beep3:    Beeps using a running X11 server\n"
              "\n"
	      "Examples:\n"
	      " %s check\n"
	      " %s addflags 0xffff\n"
	      " %s delflags 0xff00\n",
	      argv[0],
	      CYBERJACK_CONFIG_FILE,
	      argv[0],
	      argv[0],
	      argv[0]);
      return 1;
    }
  }
  else {
    return check();
  }
}





