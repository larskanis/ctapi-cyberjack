/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "msgqueue.h"

#include <fx.h>

#include <assert.h>






Message::Message(MessageType t,
		 const char *title,
		 int doneChecks,
		 int totalChecks,
		 bool result)
:_messageType(t)
,_doneChecks(doneChecks)
,_totalChecks(totalChecks)
,_result(result)
{
  assert(title);
  _title=title;
}



Message::~Message() {
}








MessageQueue::MessageQueue()
{
}



MessageQueue::~MessageQueue() {
}



void MessageQueue::pushMessage(Message *m) {
  FXMutexLock mlock(_mutex);

  _messages.push_back(m);
}



Message *MessageQueue::pullMessage() {
  FXMutexLock mlock(_mutex);
  Message *m;

  if (_messages.empty())
    return NULL;

  m=_messages.front();
  _messages.pop_front();

  return m;
}




