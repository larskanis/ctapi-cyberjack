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

#include <string>


class CheckWidget: public FXTopWindow {
  FXDECLARE(CheckWidget)

protected:
  CheckWidget();

  FXProgressBar *progressBar;
  FXLabel *messageLabel;
  FXButton *abortButton;
  bool aborted;
  int errors;

public:
  enum {
    ID_ABORT_BUTTON=FXVerticalFrame::ID_LAST,
    ID_PROGRESS_BAR
  };


  CheckWidget(FXApp *app, FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~CheckWidget();

  long onAbortClicked(FXObject*,FXSelector,void*);

  bool performChecks(bool withReaders,
		     std::string &xmlString,
		     std::string &hintString,
		     std::string &reportString);

};

