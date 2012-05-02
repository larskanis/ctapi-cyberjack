/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "cm_uname.h"

#include <sys/utsname.h>
#include <errno.h>
#include <string.h>



bool CM_Uname::check(std::string &xmlString,
		     std::string &reportString,
		     std::string &hintString) {
  struct utsname un;

  if (uname(&un)) {
    const char *s;

    s=strerror(errno);
    reportString+="*FEHLER* (";
    reportString+=s;
    reportString+=")\n";

    xmlString+="<check type=\"uname\">\n";
    xmlString+="  <result type=\"error\" function=\"uname\">";
    xmlString+=s;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }

  if (strncmp(un.release, "2.4.", 4)==0 ||
      strncmp(un.release, "2.3.", 4)==0 ||
      strncmp(un.release, "2.2.", 4)==0 ||
      strncmp(un.release, "2.1.", 4)==0 ||
      strncmp(un.release, "2.0.", 4)==0) {
    hintString+=
      "Dieser Treiber wurde fuer einen 2.6er Kernel entwickelt, "
      "es wurde aber ein aelterer Kernel gefunden.\n"
      "Daher duerfte der Treiber wahrscheinlich auf diesem System "
      "nicht funktionieren.\n";

    /* create XML string */
    xmlString+="<check type=\"uname\">\n";
    xmlString+="  <result type=\"error\">Kernel too old</result>\n";
    xmlString+="  <systemname>";
    xmlString+=un.sysname;
    xmlString+="</systemname>\n";
    xmlString+="  <release>";
    xmlString+=un.release;
    xmlString+="</release>\n";
    xmlString+="  <version>";
    xmlString+=un.version;
    xmlString+="</version>\n";
    xmlString+="  <machine>";
    xmlString+=un.machine;
    xmlString+="</machine>\n";
    xmlString+="</check>\n";

    return false;
  }
  else {
    /* create XML string */
    xmlString+="<check type=\"uname\">\n";
    xmlString+="  <result type=\"success\"></result>\n";
    xmlString+="  <systemname>";
    xmlString+=un.sysname;
    xmlString+="</systemname>\n";
    xmlString+="  <release>";
    xmlString+=un.release;
    xmlString+="</release>\n";
    xmlString+="  <version>";
    xmlString+=un.version;
    xmlString+="</version>\n";
    xmlString+="  <machine>";
    xmlString+=un.machine;
    xmlString+="</machine>\n";
    xmlString+="</check>\n";

    /* create report string */
    reportString+="System: ";
    reportString+=un.sysname;
    reportString+=", ";
    reportString+=un.release;
    reportString+=", ";
    reportString+=un.version;
    reportString+=", ";
    reportString+=un.machine;
    reportString+="\n";

    return true;
  }
}





