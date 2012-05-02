/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "foxchecksuite.h"




FoxCheckSuite::FoxCheckSuite(MessageQueue *mq)
:CheckSuite()
,messageQueue(mq) {

}



FoxCheckSuite::~FoxCheckSuite() {
}



bool FoxCheckSuite::beginCheck(const char *title,
			       int doneChecks,
			       int totalChecks) {
  Message *m;

  m=new Message(Message::MessageType_Begin,
		title, doneChecks, totalChecks, true);
  messageQueue->pushMessage(m);

  return true;
}



bool FoxCheckSuite::endCheck(const char *title,
			     int doneChecks,
			     int totalChecks,
			     bool result) {
  Message *m;

  m=new Message(Message::MessageType_End,
		title, doneChecks, totalChecks, result);
  messageQueue->pushMessage(m);

  return true;
}



