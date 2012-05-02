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


#include "cm_group.h"

#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

#include <errno.h>
#include <string.h>


bool CM_Group::check(std::string &xmlString,
		     std::string &reportString,
		     std::string &hintString) {
  struct group *gr=NULL;
  struct passwd *pw=NULL;
  char **ps;
  uid_t uid;

  /* search for group "cyberjack" */
  while( (gr=getgrent()) ) {
    if (strcasecmp(gr->gr_name, "cyberjack")==0)
      break;
  }

  if (gr==NULL) {
    /* group does not exist */
    reportString+="*HINWEIS* Gruppe \"cyberjack\" existiert nicht\n";
    hintString+=
      "Die Gruppe \"cyberjack\" existiert nicht.\n"
      "Wenn Sie die richtigen Pakete von Reiner SCT fuer Ihr jeweiliges "
      "System installieren, wird diese Gruppe automatisch erzeugt.\n"
      "Es wird empfohlen, nur die Pakete von Reiner SCT zu installieren, "
      "auch wenn von Ihrem Distributor andere Pakete angeboten werden.\n"
      "Nur mit unseren Paketen ist es uns moeglich, Ihnen im Falle von "
      "Problemen schnell zu helfen.\n"
      "Sollten fuer Ihr System keine Pakete verfuegbar sein, muessen "
      "Sie die Gruppe \"cyberjack\" mit Hilfe Ihrer Systemadministrations-"
      "Tools (z.B. yast, KUser etc) selbst erstellen.\n";
    xmlString+="<check type=\"group\">\n";
    xmlString+="  <result type=\"error\">";
    xmlString+="Gruppe cyberjack existiert nicht";
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    endgrent();
    return false;
  }

  /* get name of calling user */
  uid=getuid();
  if (uid==0) {
    reportString+="Benutzer ist Administrator (root), hat daher alle noetigen Rechte.\n";

    hintString+=
      "Sie haben dieses Programm als Administrator (root) aufgerufen. "
      "Dadurch ist keine Aussage darueber moeglich, ob Sie als "
      "normaler Benutzer alle noetigen Rechte haetten, weil der Administrator "
      "immer alle Rechte hat. Sie sollten dieses Programm als der Benutzer "
      "ausfuehren, der spaeter auch mit dem Leser arbeiten soll.\n\n";

    /* create XML string */
    xmlString+="<check type=\"group\">\n";
    xmlString+="  <result type=\"success\">";
    xmlString+="Called as user \"root\"";
    xmlString+="</result>\n";
    xmlString+="</check>\n";
  }
  else {
    pw=getpwuid(uid);
    if (pw==NULL) {
      const char *s;
  
      s=strerror(errno);
      reportString+="*FEHLER* (";
      reportString+=s;
      reportString+=")\n";
  
      xmlString+="<check type=\"group\">\n";
      xmlString+="  <result type=\"error\" function=\"getpwuid\">";
      xmlString+=s;
      xmlString+="  </result>\n";
      xmlString+="</check>\n";
  
      return false;
    }
  
    /* check whether the user is a member of the group "cyberjack" */
    ps=gr->gr_mem;
    while(*ps) {
      if (strcmp(*ps, pw->pw_name)==0)
	break;
      ps++;
    }
  
    if (*ps==NULL) {
      /* group does not exist */
      reportString+="*HINWEIS* Benutzer \"";
      reportString+=pw->pw_name;
      reportString+="\" ist kein Mitglied der Gruppe \"cyberjack\"\n";
  
      hintString+="Fuegen Sie den Benutzer \"";
      hintString+=pw->pw_name;
      hintString+="\" der Gruppe \"cyberjack\" hinzu und starten Sie Ihr ";
      hintString+="System neu.\n";
  
      xmlString+="<check type=\"group\">\n";
      xmlString+="  <result type=\"error\">";
      xmlString+="Benutzer \"";
      xmlString+=pw->pw_name;
      xmlString+="\" ist kein Mitglied der Gruppe \"cyberjack\"\n";
      xmlString+="  </result>\n";
      xmlString+="</check>\n";
  
      endgrent();
      return false;
    }
  
    /* create XML string */
    xmlString+="<check type=\"group\">\n";
    xmlString+="  <result type=\"success\">\n";
    xmlString+="    Benutzer \"";
    xmlString+=pw->pw_name;
    xmlString+="\" ist Mitglied der Gruppe \"cyberjack\"\n";
    xmlString+="  </result>\n";
    xmlString+="</check>\n";
  
    endgrent();
  }
  return true;
}


