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


#include "cm_driver.h"
#include "checksuite.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>



typedef void (*CT_VERSION_FN)(uint8_t *vmajor,
			      uint8_t *vminor,
			      uint8_t *vpatchlevel,
			      uint16_t *vbuild);


bool CM_Driver::check(std::string &xmlString,
		      std::string &reportString,
		      std::string &hintString) {
  void *p;
  void *sym;
  CT_VERSION_FN versionFn=NULL;
  char buffer[256];
  const char **pFolder;
  const char *drivers[]={
    "libctapi-cyberjack.so",
    "libctapi-cyberjack.so.2",
    "libctapi-cyberjack.so.2.0",
    "libctapi-cyberjack.so.1",
    "libctapi-cyberjack.so.1.0",
    NULL};
  const char *folders[]={
    "/usr/lib64",
    "/usr/lib64/readers",
    "/usr/lib",
    "/usr/lib/readers",
    NULL};

  assert(_suite);

  p=NULL;
  pFolder=folders;
  while(*pFolder) {
    const char **pDriver;

    pDriver=drivers;
    while(*pDriver) {
      snprintf(buffer, sizeof(buffer)-1, "%s/%s",
	       *pFolder, *pDriver);
      buffer[sizeof(buffer)-1]=0;
      p=dlopen(buffer, RTLD_LAZY | RTLD_LOCAL
#ifdef RTLD_DEEPBIND
               | RTLD_DEEPBIND
#endif
              );
      if (p)
	break;
      pDriver++;
    }
    if (p)
      break;
    pFolder++;
  }

  if (p==NULL) {
    const char *s;

    s=dlerror();
    reportString+="*FEHLER* (";
    reportString+=s;
    reportString+=")\n";

    xmlString+="<check type=\"driver\">\n";
    xmlString+="  <result type=\"error\" function=\"dlopen\">";
    xmlString+=s;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }

  sym=dlsym(p, "rsct_version");
  if (sym) {
    uint8_t vmajor;
    uint8_t vminor;
    uint8_t vpatchlevel;
    uint16_t vbuild;
    char buf[256];

    versionFn=(CT_VERSION_FN) sym;

    versionFn(&vmajor, &vminor, &vpatchlevel, &vbuild);
    snprintf(buf, sizeof(buf)-1,
	     "%d.%d.%d.%d\n",
	     vmajor, vminor, vpatchlevel, vbuild);
    buf[sizeof(buf)-1]=0;
    reportString+="Treiberdatei: ";
    reportString+=buffer;
    reportString+="\n";
    reportString+="Treiberversion: ";
    reportString+=buf;

    xmlString+="<check type=\"driver\">\n";
    xmlString+="  <file>";
    xmlString+=buffer;
    xmlString+="</file>\n";
    xmlString+="  <result type=\"success\">\n";
    xmlString+="    Treiber gefunden und geladen\n";
    xmlString+="  </result>\n";
    xmlString+="  <version>\n";
    snprintf(buf, sizeof(buf)-1,
	     "    <vmajor>%d</vmajor>\n"
	     "    <vminor>%d</vminor>\n"
	     "    <vpatchlevel>%d</vpatchlevel>\n"
	     "    <vbuild>%d</vbuild>\n",
	     vmajor, vminor, vpatchlevel, vbuild);
    buf[sizeof(buf)-1]=0;
    xmlString+=buf;
    xmlString+="  </version>\n";
    xmlString+="</check>\n";
    _suite->setDriverHandle(p);
  }
  else {
    const char *s;

    s=dlerror();
    reportString+="*FEHLER* (";
    reportString+=s;
    reportString+=")\n";

    xmlString+="<check type=\"driver\">\n";
    xmlString+="  <result type=\"error\" function=\"dlsym\">";
    xmlString+=s;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    dlclose(p);

    return false;
  }


  return true;
}





