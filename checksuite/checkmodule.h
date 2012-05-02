/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHECKMODULE_H
#define CHECKMODULE_H


class CheckSuite;
class CheckModule;


#include <string>


class CheckModule {
  friend class CheckSuite;
protected:
  CheckSuite *_suite;

protected:
  void setSuite(CheckSuite *cs) { _suite=cs;};

public:
  CheckModule();
  virtual ~CheckModule();

  virtual std::string getTitle();
  virtual bool check(std::string &xmlString,
		     std::string &reportString,
		     std::string &hintString);

  CheckSuite *getCheckSuite() const { return _suite;};
};



#endif

