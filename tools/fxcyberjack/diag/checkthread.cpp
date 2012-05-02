/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "checkthread.h"
#include "foxchecksuite.h"




CheckThread::CheckThread(MessageQueue *mq, bool withReaders)
:FXThread()
,m_withReaders(withReaders)
,_messageQueue(mq)
{

}



CheckThread::~CheckThread() {
}



FXint CheckThread::run() {
  FoxCheckSuite *cs;
  bool b;

  fprintf(stderr, "Check thread started.\n");

  cs=new FoxCheckSuite(_messageQueue);

  cs->addStandardModules(m_withReaders);
  b=cs->performChecks(_xmlString, _reportString, _hintString);

  fprintf(stderr, "Check thread ending.\n");
  return 0;
}



std::string CheckThread::getXmlString() {
  if (running())
    return "";
  return _xmlString;
}



std::string CheckThread::getHintString() {
  if (running())
    return "";
  return _hintString;
}



std::string CheckThread::getReportString() {
  if (running())
    return "";
  return _reportString;
}






