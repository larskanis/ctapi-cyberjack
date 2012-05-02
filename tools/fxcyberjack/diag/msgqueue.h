/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef MSGQUEUE_H
#define MSGQUEUE_H


#include <string>
#include <list>


#include <fx.h>


//class FXMutex;




class Message {
public:
  typedef enum {
    MessageType_Unknown=0,
    MessageType_Begin,
    MessageType_End
  } MessageType;

private:
  MessageType _messageType;
  std::string _title;
  int _doneChecks;
  int _totalChecks;
  bool _result;

public:
  Message(MessageType t,
	  const char *title,
	  int doneChecks,
	  int totalChecks,
	  bool result);
  ~Message();

  MessageType getMessageType() const { return _messageType;};
  const std::string &getTitle() const { return _title;};
  int getDoneChecks() const { return _doneChecks;};
  int getTotalChecks() const { return _totalChecks;};
  bool getResult() const { return _result;};

};




class MessageQueue {
private:
  std::list<Message*> _messages;
  FXMutex _mutex;

public:
  MessageQueue();
  ~MessageQueue();

  void pushMessage(Message *m);
  Message *pullMessage();

};



#endif

