/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




#include "cm_distri.h"
#include "checksuite.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



bool CM_Distri::check(std::string &xmlString,
		      std::string &reportString,
		      std::string &hintString) {
  int rv;
  int xs;

  rv=system(CD_SCRIPT_DIR "/getdist.sh");
  if (rv<0) {
    const char *s;

    /* script not found */
    s=strerror(errno);
    reportString+="*FEHLER* (Script \"getdist.sh\" nicht gefunden)\n";

    xmlString+="<check type=\"distri\">\n";
    xmlString+="  <result type=\"error\" function=\"system\">";
    xmlString+=s;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }

  xs=WEXITSTATUS(rv);
  if (xs==127) {
    reportString+="*FEHLER* (\"getdist.sh\" nicht gefunden)\n";
    xmlString+="<check type=\"distri\">\n";
    xmlString+="  <result type=\"error\" function=\"system\">";
    xmlString+="Shell script nicht gefunden";
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }
  else if (xs!=0) {
    char numbuf[16];

    snprintf(numbuf, sizeof(numbuf), "%d", xs);
    reportString+="*FEHLER* (getdist.sh wurde mit code ";
    reportString+=numbuf;
    reportString+=" beendet)\n";
    xmlString+="<check type=\"distri\">\n";
    xmlString+="  <result type=\"error\" function=\"system\">";
    xmlString+="getdist.sh beendet mit code ";
    xmlString+=numbuf;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }
  else {
    FILE *f;
    std::string distName;
    std::string distVer;

    /* read dist name */
    f=fopen("distname", "r");
    if (f) {
      char *p;

      if (1==fscanf(f, "%as", &p)) {
	distName=std::string(p);
	free(p);
	fclose(f);
	unlink("distname");
      }
      else {
	fclose(f);
	reportString+="*FEHLER* (konnte Distributionsname nicht lesen)\n";
	xmlString+="<check type=\"distri\">\n";
	xmlString+="  <result type=\"error\" function=\"system\">";
	xmlString+="Konnte Distributionsname nicht lesen.\n";
	xmlString+="  </result>\n";
	xmlString+="</check>\n";

	return false;
      }
    }
    else {
      const char *s;
  
      s=strerror(errno);
      reportString+="*FEHLER* (";
      reportString+=s;
      reportString+=")\n";
  
      xmlString+="<check type=\"distri\">\n";
      xmlString+="  <result type=\"error\" function=\"fopen\">";
      xmlString+=s;
      xmlString+="  </result>\n";
      xmlString+="</check>\n";
  
      return false;
    }

    /* read dist version */
    f=fopen("distver", "r");
    if (f) {
      char *p;

      if (1==fscanf(f, "%as", &p)) {
	distVer=std::string(p);
	free(p);
	fclose(f);
	unlink("distver");
      }
      else {
	fclose(f);
	reportString+="*FEHLER* (konnte Distributionsversion nicht lesen)\n";
	xmlString+="<check type=\"distri\">\n";
	xmlString+="  <result type=\"error\" function=\"system\">";
	xmlString+="Konnte Distributionsversion nicht lesen.\n";
	xmlString+="  </result>\n";
	xmlString+="</check>\n";

        return false;
      }
    }
    else {
      const char *s;
  
      s=strerror(errno);
      reportString+="*FEHLER* (";
      reportString+=s;
      reportString+=")\n";
  
      xmlString+="<check type=\"distri\">\n";
      xmlString+="  <result type=\"error\" function=\"fopen\">";
      xmlString+=s;
      xmlString+="  </result>\n";
      xmlString+="</check>\n";
  
      return false;
    }

    /* evaluate */
    if (distName.empty())
      distName="(unbekannt)";
    if (distVer.empty())
      distVer="(unbekannt)";

    reportString+="Distribution: ";
    reportString+=distName;
    reportString+=" ";
    reportString+=distVer;
    reportString+=" \n";

    xmlString+="<check type=\"distri\">\n";
    xmlString+="  <result type=\"success\"></result>\n";
    xmlString+="  <distname>";
    xmlString+=distName;
    xmlString+="</distname>\n";
    xmlString+="  <distver>";
    xmlString+=distVer;
    xmlString+="</distver>\n";
    xmlString+="</check>\n";

    _suite->setDist(distName, distVer);

    return true;
  }
}



