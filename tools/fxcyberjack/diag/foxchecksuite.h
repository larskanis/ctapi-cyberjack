/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOXCHECKSUITE_H
#define FOXCHECKSUITE_H

#include "checksuite.h"
#include "msgqueue.h"



class FoxCheckSuite: public CheckSuite {
protected:
  MessageQueue *messageQueue;

public:
  FoxCheckSuite(MessageQueue *mq);
  virtual ~FoxCheckSuite();

  bool beginCheck(const char *title,
		  int doneChecks,
		  int totalChecks);

  bool endCheck(const char *title,
		int doneChecks,
		int totalChecks,
		bool result);

};


#endif

