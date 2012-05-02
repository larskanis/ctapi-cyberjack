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


#include "checkmodule.h"





CheckModule::CheckModule() {
}



CheckModule::~CheckModule() {
}



std::string CheckModule::getTitle() {
  return "Keine Ueberschrift";
}



bool CheckModule::check(std::string &xmlString,
			std::string &reportString,
			std::string &hintString) {
  return false;
}



