/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2004  REINER SCT
 * Author: Harald Welte
 *         fixed and extended by Martin Preuss
 * Support: support@reiner-sct.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * CVS: $Id: cjctapi_beep.c 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef OS_LINUX
# include <linux/kd.h>
# include <sys/ioctl.h>
#endif

#include "config_l.h"
#include "cyberjack_l.h"

#define DEBUGP(format, ...) \
  rsct_log(CT_INVALID_CTN, CT_FLAGS_DEBUG_CTAPI, __FILE__, __LINE__, __FUNCTION__, format , ##__VA_ARGS__ )



typedef void* (*XOPENDISPLAY_FN)(char *display_name);
typedef int (*XCLOSEDISPLAY_FN)(void *display);
typedef int (*XBELL_FN)(void *display, int percent);
typedef int (*XFLUSH_FN)(void *display);


struct beep_struct {
  void *xhdl;
  void *display;
  XOPENDISPLAY_FN xOpenDisplay;
  XCLOSEDISPLAY_FN xCloseDisplay;
  XBELL_FN xBell;
  XFLUSH_FN xFlush;
};





static void beep_console(struct beep_struct *bs){
#ifdef OS_LINUX
  int fd;

  DEBUGP("using console\n");
  fd=open("/dev/console", O_WRONLY);
  if (fd!=-1) {
    int arg;
    int rv;

    arg=(150<<16)+440; /* "A" for 150ms */
    rv=ioctl(fd, KDMKTONE, arg);
    close(fd);
    if (!rv)
      return;
    DEBUGP("error %d [%s]\n",
	   errno, strerror(errno));
    printf("\a"); /* fallback */
    fflush(stdout);
  }
  else {
    DEBUGP("error %d [%s]\n",
	   errno, strerror(errno));
    printf("\a"); /* fallback */
    fflush(stdout);
  }
#else
    printf("\a"); /* fallback */
    fflush(stdout);
#endif
}



static void beep_x11(struct beep_struct *bs){
  DEBUGP("using X11\n");
  bs->xBell(bs->display, 100);
  bs->xFlush(bs->display);
}



void beep_whatever(struct beep_struct *bs){
  DEBUGP("beeping\n");
  if (bs->display)
    beep_x11(bs);
  else
    beep_console(bs);
}



struct beep_struct *beep_init(void){
  struct beep_struct *bs;

  bs=(struct beep_struct*)malloc(sizeof(*bs));
  if (!bs)
    return NULL;
  memset(bs, 0, sizeof(*bs));

  if (!(rsct_config_get_flags() & CT_FLAGS_BEEP_NO_X11)) {
    if ((bs->xhdl = dlopen("libX11.so", RTLD_NOW))) {
      bs->xOpenDisplay=(XOPENDISPLAY_FN)dlsym(bs->xhdl, "XOpenDisplay");
      bs->xCloseDisplay = (XCLOSEDISPLAY_FN)dlsym(bs->xhdl, "XCloseDisplay");
      bs->xBell = (XBELL_FN)dlsym(bs->xhdl, "XBell");
      bs->xFlush = (XFLUSH_FN)dlsym(bs->xhdl, "XFlush");
      if (bs->xOpenDisplay &&
	  bs->xCloseDisplay &&
	  bs->xBell &&
	  bs->xFlush) {
	bs->display = (void *) bs->xOpenDisplay(NULL);
	if (bs->display) {
          DEBUGP("Connected to X11 server for beeping");
	}
	else {
	  DEBUGP("Could not connect to X11 server for beeping, will use console");
	}
      }
    }
  }
  return bs;
}



void beep_fini(struct beep_struct *bs){
  if (bs->display) {
    int rv;

    rv=bs->xCloseDisplay(bs->display);
    DEBUGP("XCloseDisplay returned: %d", rv);
    bs->display=NULL;
  }

  if (bs->xhdl)
    dlclose(bs->xhdl);
  free(bs);
}



#ifdef BEEP_STANDALONE
int main(int argc, char **argv){
  struct beep_struct *bs;

  bs = beep_init();
  beep_whatever(bs);
  beep_fini(bs);

  exit(0);
}
#endif
