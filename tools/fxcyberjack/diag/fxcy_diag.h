/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef FXCY_DIAG_H
#define FXCY_DIAG_H


#include <fx.h>




class FXCY_Diagnosis: public FXDialogBox {
  FXDECLARE(FXCY_Diagnosis)

protected:
  bool m_withReaders;
  FXTabBook *tabbook;
  FXTabItem *tab1;
  FXTabItem *tab2;
  FXTabItem *tab3;
  FXTabItem *tab4;

  FXText *introWidget;
  FXText *reportWidget;
  FXText *hintWidget;
  FXText *xmlWidget;

  FXCY_Diagnosis();

public:
  enum {
    ID_TABBOOK=FXDialogBox::ID_LAST,
    ID_REPORT_TEXT,
    ID_HINT_TEXT,
    ID_XML_TEXT,
    ID_MENU_NEWTEST,
    ID_QUIT
  };

  long onCmdAccept(FXObject*, FXSelector, void *ptr);
  long onCmdReject(FXObject*, FXSelector, void *ptr);


public:
  FXCY_Diagnosis(FXWindow *owner,
                 const FXString& name,
                 bool withReaders,
                 FXuint opts=DECOR_ALL,
                 FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~FXCY_Diagnosis();

  bool performTest();

  long onCmdNewTest(FXObject*,FXSelector sel,void*);
  long onCmdQuit(FXObject*,FXSelector sel,void*);
};


#endif


