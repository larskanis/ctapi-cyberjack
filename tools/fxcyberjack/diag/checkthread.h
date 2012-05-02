/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <fx.h>

#include "msgqueue.h"

#include <string>



class CheckThread: public FXThread {
private:
  bool m_withReaders;
  MessageQueue *_messageQueue;
  std::string _xmlString;
  std::string _hintString;
  std::string _reportString;

protected:
  FXint run();

public:
  CheckThread(MessageQueue *mq, bool withReaders);
  ~CheckThread();

  std::string getXmlString();
  std::string getHintString();
  std::string getReportString();

};


