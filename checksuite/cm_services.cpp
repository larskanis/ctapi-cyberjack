/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "cm_services.h"
#include "checksuite.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>




bool CM_Services::check(std::string &xmlString,
			std::string &reportString,
			std::string &hintString) {
  DIR *d;
  struct dirent *de=NULL;

  assert(_suite);

  d=opendir("/proc");
  if (d==NULL) {
    const char *s;

    s=strerror(errno);
    reportString+="*FEHLER* (";
    reportString+=s;
    reportString+=")\n";

    xmlString+="<check type=\"services\">\n";
    xmlString+="  <result type=\"error\" function=\"opendir\">";
    xmlString+=s;
    xmlString+="  </result>\n";
    xmlString+="</check>\n";

    return false;
  }

  xmlString+="<check type=\"services\">\n";
  xmlString+="  <result type=\"success\"></result>\n";

  while( (de=readdir(d)) ) {
    struct stat st;
    const char *s;

    s=de->d_name;
    while(*s && isdigit(*s))
      s++;
    if (*s==0) {
      std::string fullName;

      /* all digits, so it is most likely a process dir */
      fullName="/proc/";
      fullName+=de->d_name;

      if (stat(fullName.c_str(), &st)==0) {
	if (S_ISDIR(st.st_mode)) {
	  char lbuf[256];
	  struct passwd *pw;
	  const char *ownerName=NULL;
          FILE *f;

	  /* get PW entry for owner of the process */
	  pw=getpwuid(st.st_uid);
	  if (pw && pw->pw_name[0])
	    ownerName=pw->pw_name;
          else
	    ownerName="(unknown)";

	  /* it is a folder, so it definately *is* a process dir */
	  fullName+="/cmdline";
	  /* the folder contains a symbolic link \"exe\" which points to
	   * the running executable. We read this link to get its name
	   * and check it against known executables for problematic
           * services.
           */
	  f=fopen(fullName.c_str(), "r");
	  if (f) {
	    if (1==fscanf(f, "%255s", lbuf)) {
	      lbuf[sizeof(lbuf)-1]=0;

	      if (strstr(lbuf, "pcscd")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_PCSCD)){
		  reportString+="PC/SC Dienst gefunden.\n";
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="pcscd</service>\n";
		  hintString+=
		    "Auf diesem System laeuft der PC/SC Dienst. "
		    "Manche Programme (insbesondere Moneyplex) koennen in "
		    "diesem Fall nicht auf den Leser zugreifen.\n"
		    "Abhilfe: Entweder deinstallieren Sie den PC/SC Dienst "
		    "komplett (das Paket heisst je nach System \"pcscd\""
		    "oder \"pcsc-lite\"), oder sie deinstallieren einfach "
		    "das Cyberjack-Paket mit dem IFD-Treiber (Paket mit "
		    "\"-ifd\" am Ende).\n"
		    "Sie koennen allerdings auch den Dienst mittels "
		    "\"killall pcscd\" (als root) beenden.\n\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_PCSCD);
		}
	      }
	      else if (strstr(lbuf, "chipcardd2")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_LCC2)){
		  reportString+="Libchipcard2 Dienst gefunden.\n";
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="chipcardd2</service>\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_LCC2);
		}
	      }
	      else if (strstr(lbuf, "chipcardd3")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_LCC3)){
		  reportString+="Libchipcard3 Dienst gefunden.\n";
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="chipcardd3</service>\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_LCC3);
		}
	      }
	      else if (strstr(lbuf, "chipcardd4")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_LCC4)){
		  reportString+="Libchipcard4 Dienst gefunden.\n";
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="chipcardd4</service>\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_LCC4);
		}
	      }
	      else if (strstr(lbuf, "chipcardd")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_LCC1)){
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="chipcardd</service>\n";
		  reportString+="Libchipcard1 Dienst gefunden.\n";
		  hintString+=
		    "Auf diesem System laeuft der Libchipcard1 Dienst. "
		    "Manche Programme (insbesondere Moneyplex) koennen in "
		    "diesem Fall nicht auf den Leser zugreifen.\n"
		    "Abhilfe: Deinstallieren Sie Libchipcard1 oder beenden "
		    "Sie den Dienst (mittels \"killall chipcardd\") solange "
		    "Moneyplex laeuft.\n"
		    "Fuer Libchipcard1 gibt es auch KDE-Tools, welche "
		    "ebenfalls zweitweise den Dienst stoppen koennen.\n\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_LCC1);
		}
	      }
	      else if (strstr(lbuf, "ifdhandler")) {
		if (!(_suite->getFlags() & CHECKSUITE_FLAGS_HAVE_OPENCT)){
		  reportString+="OpenCT Dienst gefunden.\n";
		  xmlString+="  <service owner=\"";
		  xmlString+=ownerName;
		  xmlString+="\">";
		  xmlString+="openct</service>\n";
		  hintString+=
		    "Auf diesem System laeuft der OpenCT Dienst. "
		    "Manche Programme (insbesondere Moneyplex) koennen in "
		    "diesem Fall je nach Leser und Version von OpenCT nicht "
		    "auf den Leser zugreifen.\n"
		    "Abhilfe: Entweder deinstallieren Sie den OpenCT Dienst "
		    "komplett (das Paket heisst je nach System \"openct\" "
		    "oder \"libopenct\"), oder Sie stoppen OpenCT mittels "
		    "\"/etc/init.d/openct stop\" als root solange das "
		    "Programm verwendet wird.\n\n";
		  _suite->addFlags(CHECKSUITE_FLAGS_HAVE_OPENCT);
		}
	      }
	    }
	    fclose(f);
	  }
	}
      }
    }
  }

  xmlString+="</check>\n";
  closedir(d);

  return true;
}


