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
#include "driver.hpp"

#include <string>



class CM_Reader: public CheckModule {
public:
  CM_Reader() {};
  ~CM_Reader() {};

  std::string getTitle() { return "ermittle und teste angeschlossene Leser";};
  bool check(std::string &xmlString,
	     std::string &reportString,
	     std::string &hintString);

protected:
  bool _checkReaders(Cyberjack::Driver *dr,
		     std::string &xmlString,
		     std::string &reportString,
		     std::string &hintString);
  bool _checkPcsc(std::string &xmlString,
		  std::string &reportString,
		  std::string &hintString);
  bool _checkCtapi(std::string &xmlString,
		   std::string &reportString,
		   std::string &hintString);

};

