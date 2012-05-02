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


#include "checksuite.h"

#include <stdio.h>
#include <list>
#include <dlfcn.h>

#include "checkmodule.h"

#include "cm_distri.h"
#include "cm_driver.h"
#include "cm_group.h"
#include "cm_reader.h"
#include "cm_services.h"
#include "cm_uname.h"




CheckSuite::CheckSuite()
:_driverHandle(NULL)
,_ctInitNameFn(NULL)
,_ctDataFn(NULL)
,_ctCloseFn(NULL)
,_flags(0)
{

}


CheckSuite::~CheckSuite() {
  if (_driverHandle) {
    dlclose(_driverHandle);
    _driverHandle=NULL;
  }
}



void CheckSuite::setFlags(uint32_t f) {
  _flags=f;
}



void CheckSuite::addFlags(uint32_t f) {
  _flags|=f;
}



void CheckSuite::delFlags(uint32_t f) {
  _flags&=~f;
}



void CheckSuite::setDist(const std::string &dname, const std::string &dver) {
  _distName=dname;
  _distVersion=dver;
}



void CheckSuite::setDriverHandle(void *p) {
  _driverHandle=p;
  if (_driverHandle) {
    void *sym;

    sym=dlsym(p, "rsct_init_name");
    if (sym)
      _ctInitNameFn=(CT_INITNAME_FN)sym;
    sym=dlsym(p, "CT_data");
    if (sym)
      _ctDataFn=(CT_DATA_FN)sym;
    sym=dlsym(p, "CT_close");
    if (sym)
      _ctCloseFn=(CT_CLOSE_FN)sym;
  }
  else {
    _ctInitNameFn=NULL;
    _ctDataFn=NULL;
    _ctCloseFn=NULL;
  }
}



void *CheckSuite::getDriverHandle() {
  return _driverHandle;
}



void CheckSuite::addCheckModule(CheckModule *m) {
  m->setSuite(this);
  _moduleList.push_back(m);
}



int8_t CheckSuite::ctInitName(uint16_t ctn, const char *devName) {
  if (_driverHandle==NULL || _ctInitNameFn==NULL) {
    return CHECKSUITE_SPECIAL_ERRORCODE;
  }

  return _ctInitNameFn(ctn, devName);
}



int8_t CheckSuite::ctData(uint16_t ctn,
			  uint8_t *dad,
			  uint8_t *sad,
			  uint16_t cmd_len,
			  const uint8_t *cmd,
			  uint16_t *response_len,
			  uint8_t *response) {
  if (_driverHandle==NULL || _ctDataFn==NULL) {
    return CHECKSUITE_SPECIAL_ERRORCODE;
  }

  return _ctDataFn(ctn, dad, sad, cmd_len, cmd, response_len, response);
}



int8_t CheckSuite::ctClose(uint16_t ctn) {
  if (_driverHandle==NULL || _ctCloseFn==NULL) {
    return CHECKSUITE_SPECIAL_ERRORCODE;
  }

  return _ctCloseFn(ctn);
}



void CheckSuite::addStandardModules(bool withReaderCheck) {
  CheckModule *m;

  /* create and add modules */
  m=new CM_Distri();
  addCheckModule(m);

  m=new CM_Uname();
  addCheckModule(m);

  m=new CM_Group();
  addCheckModule(m);

  m=new CM_Services();
  addCheckModule(m);

  m=new CM_Driver();
  addCheckModule(m);

  if (withReaderCheck) {
    m=new CM_Reader();
    addCheckModule(m);
  }
}



bool CheckSuite::beginCheck(const char *title,
			    int doneChecks,
			    int totalChecks) {
  fprintf(stderr, "BEGIN: %s (%d/%d)\n",
          title, doneChecks, totalChecks);
  return true;
}



bool CheckSuite::endCheck(const char *title,
			  int doneChecks,
			  int totalChecks,
			  bool result) {
  fprintf(stderr, "END  : %s (%d/%d) [%s]\n",
	  title, doneChecks, totalChecks, result?"OK":"ERROR");
  return true;
}



bool CheckSuite::performChecks(std::string &xmlString,
			       std::string &reportString,
			       std::string &hintString) {
  std::list<CheckModule*>::iterator it;
  bool b=true;
  int i=0;
  char buf[256];

  xmlString="<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
  xmlString+="<cyberjack>\n";

  xmlString+="  <check-version>\n";
  snprintf(buf, sizeof(buf)-1,
	   "    <vmajor>%d</vmajor>\n"
	   "    <vminor>%d</vminor>\n"
	   "    <vpatchlevel>%d</vpatchlevel>\n"
	   "    <vbuild>%d</vbuild>\n"
	   "    <vtag>%s</vtag>\n",
	   CYBERJACK_VERSION_MAJOR,
	   CYBERJACK_VERSION_MINOR,
	   CYBERJACK_VERSION_PATCHLEVEL,
	   CYBERJACK_VERSION_BUILD,
	   CYBERJACK_VERSION_TAG);
  buf[sizeof(buf)-1]=0;
  xmlString+=buf;
  xmlString+="  </check-version>\n";

  for (it=_moduleList.begin();
       it!=_moduleList.end();
       it++) {
    bool lb;

    /* update GUI */
    if (!beginCheck((*it)->getTitle().c_str(),
		    i,
		    _moduleList.size()))
      return false;

    /* perform check */
    lb=(*it)->check(xmlString, reportString, hintString);
    if (!lb) {
      b=false;
    }

    /* increment number of done checks */
    i++;

    /* update GUI again */
    if (!endCheck((*it)->getTitle().c_str(),
		  i,
		  _moduleList.size(),
		  lb))
      return false;
  }

  xmlString+="</cyberjack>\n";

  return b;
}














