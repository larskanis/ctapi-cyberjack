/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "fxcy_diag.h"
#include "checkwidget.h"

#include <string>



FXDEFMAP(FXCY_Diagnosis) FXCY_DiagnosisMap[]= {
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, FXCY_Diagnosis::onCmdAccept),
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, FXCY_Diagnosis::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND, FXCY_Diagnosis::ID_MENU_NEWTEST, FXCY_Diagnosis::onCmdNewTest),
};


FXIMPLEMENT(FXCY_Diagnosis, FXDialogBox,
	    FXCY_DiagnosisMap, ARRAYNUMBER(FXCY_DiagnosisMap));




FXCY_Diagnosis::FXCY_Diagnosis()
:FXDialogBox()
,m_withReaders(false)
,tabbook(NULL)
,tab1(NULL)
,tab2(NULL)
,tab3(NULL)
,tab4(NULL)
,reportWidget(NULL)
,hintWidget(NULL)
,xmlWidget(NULL)
{

}



FXCY_Diagnosis::FXCY_Diagnosis(FXWindow *owner,
                               const FXString& name,
			       bool withReaders,
                               FXuint opt,
                               FXint x,FXint y, FXint w,FXint h)
:FXDialogBox(owner, name, opt, x, y, w, h)
,m_withReaders(withReaders)
,tabbook(NULL)
,tab1(NULL)
,tab2(NULL)
,tab3(NULL)
,tab4(NULL)
,reportWidget(NULL)
,hintWidget(NULL)
,xmlWidget(NULL)
{
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;

  // Tooltip
  new FXToolTip(getApp());

  // Separator
  new FXHorizontalSeparator(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|SEPARATOR_GROOVE);

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  tabbook=new FXTabBook(vf,this,ID_TABBOOK,
			PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|
			LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_LEFT);

  tab1=new FXTabItem(tabbook,"&Einleitung",NULL);
  introWidget=new FXText(tabbook,
			 this, ID_REPORT_TEXT,
			 LAYOUT_FILL_X|LAYOUT_FILL_Y|
			 TEXT_READONLY|TEXT_WORDWRAP);
  tab2=new FXTabItem(tabbook,"&Bericht",NULL);
  reportWidget=new FXText(tabbook,
			  this, ID_REPORT_TEXT,
			  LAYOUT_FILL_X|LAYOUT_FILL_Y|
			  TEXT_READONLY|TEXT_WORDWRAP);

  tab3=new FXTabItem(tabbook,"&Hinweise",NULL);
  hintWidget=new FXText(tabbook,
			this, ID_HINT_TEXT,
			LAYOUT_FILL_X|LAYOUT_FILL_Y|
			TEXT_READONLY|TEXT_WORDWRAP);
  tab4=new FXTabItem(tabbook,"&XML",NULL);
  xmlWidget=new FXText(tabbook,
		       this, ID_XML_TEXT,
		       LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY);


  introWidget->setText
    (
     "Cyberjack Diagnose Programm\n"
     "\n"
     "Dieses Programm ist in der Lage Ihr System auf die haeufigsten "
     "Konfigurationsprobleme hin zu ueberpruefen.\n"
     "\n"
     "Es dient ausserdem dazu fuer den Support benoetigte Informationen "
     "ueber Ihr System und den eventuell angeschlossenen Leser "
     "zusammenzutragen.\n"
     "\n"
     "Bei Problemen bekommen Sie Hinweise, die Ihnen helfen sollen diese "
     "Probleme selbst zu loesen.\n"
     "\n"
     "Sollten Sie dabei nicht weiterkommen, wenden Sie sich bitte an den "
     "Support von Reiner SCT (entweder per Email an support@reiner-sct.de "
     "oder ueber die Support-Seite auf http://www.reiner-sct.de/).\n"
     "\n"
     "Bitte fuegen Sie Ihrer Support-Anfrage die gesammelten Daten der "
     "\"XML\"-Seite bei.\n"
     );

  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
  new FXButton(hf, "Schliessen", NULL, this, ID_ACCEPT,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);

}



FXCY_Diagnosis::~FXCY_Diagnosis() {
}



bool FXCY_Diagnosis::performTest() {
  CheckWidget w(getApp(), getX()+100, getY()+100);
  std::string xmlString;
  std::string reportString;
  std::string hintString;
  bool b;

  w.create();
  w.layout();
  w.show();

  b=w.performChecks(m_withReaders, xmlString, hintString, reportString);
  fprintf(stderr, "Result of tests: %s\n", b?"OK":"ERROR");

  if (!b) {
    std::string s;

    s="Es sind Probleme aufgetreten.\n"
      "\n"
      "Auf der \"Hinweise\"-Seite finden Sie Tips, wie Sie diese "
      "Probleme eventuell selbst loesen koennen.\n"
      "\n"
      "Sollten die Hinweise Ihnen nicht weiterhelfen, wenden Sie sich "
      "bitte an den Support von Reiner SCT.\n"
      "\n"
      "Um Ihnen helfen zu koennen, benoetigt der Support einige "
      "Informationen ueber Ihr System, die dieses Programm bereits fuer "
      "Sie zusammengestellt hat.\n"
      "\n"
      "Bitte oeffnen Sie die \"XML\"-Seite, markieren mit der Maus den "
      "gesamten Text auf der Seite und kopieren Sie ihn in Ihre Email an "
      "den Support von Reiner SCT (support@reiner-sct.de).\n"
      "\n"
      "Die Tastenkombination \"STRG-C\" kopiert dabei den markierten "
      "Text in die Zwischenablage. In Ihrem Email-Programm (oder auf der "
      "Support-Seite) koennen Sie dann den so markierten Text mit der "
      "Kombination \"STRG-V\" aus der Zwischenablage abrufen.\n"
      "\n"
      "Test-Bericht folgt:\n"
      "-------------------\n";
    reportString=s+reportString;
  }
  else {
    std::string s;

    s="Auf Ihrem System scheint alles in Ordnung zu sein.\n";
    if (!hintString.empty())
      s+="Schauen Sie sich bitte dennoch die \"Hinweise\"-Seite an um "
	"zu erfahren, wie Sie den Zugriff auf den Leser verbessern "
	"koennen.\n";
    s+=
      "\n"
      "Test-Bericht folgt:\n"
      "-------------------\n";
    reportString=s+reportString;
  }

  if (hintString.empty())
    hintString+="Es liegen keine Hinweise vor.\n";

  reportWidget->setText(reportString.c_str());
  hintWidget->setText(hintString.c_str());
  xmlWidget->setText(xmlString.c_str());

  w.hide();

  tabbook->setCurrent(1);

  return b;
}


long FXCY_Diagnosis::onCmdNewTest(FXObject*,FXSelector sel,void*){
  performTest();
  return 1;
}



long FXCY_Diagnosis::onCmdAccept(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,TRUE);
  hide();
  return 1;
}



long FXCY_Diagnosis::onCmdReject(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,FALSE);
  hide();
  return 1;
}



