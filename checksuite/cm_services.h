/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#include "checkmodule.h"



class CM_Services: public CheckModule {
public:
  CM_Services() {};
  ~CM_Services() {};

  std::string getTitle() { return "ermittle laufende Dienste";};
  bool check(std::string &xmlString,
	     std::string &reportString,
	     std::string &hintString);

};

