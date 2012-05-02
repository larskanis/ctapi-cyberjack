/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "checkwidget.h"
#include "checkthread.h"
#include "msgqueue.h"

#include <string.h>
#include <errno.h>



FXDEFMAP(CheckWidget) CheckWidgetMap[]= {
  FXMAPFUNC(SEL_COMMAND, CheckWidget::ID_ABORT_BUTTON,
	    CheckWidget::onAbortClicked),

};

FXIMPLEMENT(CheckWidget, FXTopWindow,
	    CheckWidgetMap, ARRAYNUMBER(CheckWidgetMap));



CheckWidget::CheckWidget()
:FXTopWindow()
,progressBar(NULL)
,messageLabel(NULL)
,abortButton(NULL)
,aborted(false)
,errors(0)
{
}



CheckWidget::CheckWidget(FXApp *app,
			 FXint x, FXint y, FXint w, FXint h)
:FXTopWindow(app, "System wird ueberprueft", NULL, NULL,
	     DECOR_TITLE| DECOR_BORDER | LAYOUT_CENTER_X | LAYOUT_CENTER_Y,
	     x, y, w, h, 0, 0, 0, 0, 0, 0)
,progressBar(NULL)
,messageLabel(NULL)
,abortButton(NULL)
,aborted(false)
,errors(0)
{
  FXVerticalFrame *vf;
  FXLabel *l;
  FXHorizontalFrame *hf;
  FXSpring *sp;

  vf=new FXVerticalFrame(this, LAYOUT_FILL_Y);
  l=new FXLabel(vf, "Ihr System wird ueberprueft, bitte warten...",
		NULL,
		LABEL_NORMAL |
                LAYOUT_CENTER_X |
		LAYOUT_FILL_X);
  progressBar=new FXProgressBar(vf,
				this,
				ID_PROGRESS_BAR,
				PROGRESSBAR_PERCENTAGE |
				PROGRESSBAR_NORMAL |
				PROGRESSBAR_HORIZONTAL |
				LAYOUT_FILL_X |
				LAYOUT_FILL_Y);
  progressBar->setBarSize(32);
  progressBar->setTotal(0);
  progressBar->setProgress(0);
  progressBar->show();

  messageLabel=new FXLabel(vf,
			   "",
			   NULL,
			   LABEL_NORMAL |
			   LAYOUT_CENTER_X |
			   LAYOUT_FILL_X);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X);
  sp=new FXSpring(hf, LAYOUT_FILL_X);
  abortButton=new FXButton(hf, "Abbrechen", NULL,
			   this, ID_ABORT_BUTTON,
                           BUTTON_NORMAL | LAYOUT_RIGHT);
}



CheckWidget::~CheckWidget() {
}



long CheckWidget::onAbortClicked(FXObject*,FXSelector,void*) {
  aborted=true;
  return 1;
}



bool CheckWidget::performChecks(bool withReaders,
				std::string &xmlString,
				std::string &hintString,
				std::string &reportString) {
  MessageQueue *mq;
  Message *m;
  CheckThread *ct;
  bool b=true;

  mq=new MessageQueue();
  ct=new CheckThread(mq, withReaders);
  if (!ct->start()) {
    reportString+="*FEHLER* (Konnte Tests nicht starten)\n";
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    return false;
  }

  while(ct->running()) {
    while( (m=mq->pullMessage()) ) {
      messageLabel->setText(m->getTitle().c_str());
      progressBar->setTotal(m->getTotalChecks());
      progressBar->setProgress(m->getDoneChecks());
      if (!m->getResult()) {
	b=false;
      }
      delete m;
    }
    getApp()->runModalWhileEvents(this);
    if (aborted) {
      ct->cancel();
      b=false;
      break;
    }
  }

  /* read all remaining messages */
  while( (m=mq->pullMessage()) ) {
    fprintf(stderr, "Got message: %d %s %d %d %s\n",
	    m->getMessageType(),
	    m->getTitle().c_str(),
	    m->getDoneChecks(),
	    m->getTotalChecks(),
	    (m->getResult())?"OK":"ERROR");
    messageLabel->setText(m->getTitle().c_str());
    progressBar->setTotal(m->getTotalChecks());
    progressBar->setProgress(m->getDoneChecks());
    if (!m->getResult()) {
      b=false;
      fprintf(stderr, "Got a bad result\n");
    }
    delete m;
  }

  xmlString=ct->getXmlString();
  hintString=ct->getHintString();
  reportString=ct->getReportString();

  delete ct;
  delete mq;

  return b;
}





