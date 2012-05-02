/***************************************************************************
    begin       : Wed Mar 25 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "fxcy_readerdetails.hpp"
#include "fxcy_app.hpp"

#define RDETAILS_TEXTFIELD_WIDTH 40


FXDEFMAP(FXCY_ReaderDetails) FXCY_ReaderDetailsMap[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, FXCY_ReaderDetails::onCmdAccept),
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, FXCY_ReaderDetails::onCmdCancel)
};


FXIMPLEMENT(FXCY_ReaderDetails, FXDialogBox, FXCY_ReaderDetailsMap, ARRAYNUMBER(FXCY_ReaderDetailsMap))




FXCY_ReaderDetails::FXCY_ReaderDetails(Cyberjack::Reader *r,
                                       FXWindow *owner,
                                       const FXString& name,
                                       FXuint opts,
                                       FXint x, FXint y, FXint w, FXint h)
:FXDialogBox(owner, name, opts, x, y, w, h)
,m_reader(r)
,m_readerIcon(NULL) {
  FXCY_App *a;
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;
  FXIcon *ic=NULL;
  const char *s;
  FXLabel *ll;
  FXString str;
  const cj_ReaderInfo &ri=r->getReaderInfo();

  a=dynamic_cast<FXCY_App*>(getApp());

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  /* title (name) */
  new FXLabel(vf, FXString(r->getName().c_str()).trim(), NULL, LAYOUT_TOP | LAYOUT_CENTER_X);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  /* big icon and product string */
  s=r->getProductString().c_str();
  if (strcasecmp(s, "cyberJack e-com(a)")==0)
    ic=a->getIcon(FXCY_App::IconId_EcomABig);
  else if (strcasecmp(s, "cyberJack pinpad(a)")==0)
    ic=a->getIcon(FXCY_App::IconId_PinpadABig);
  else if (strcasecmp(s, "cyberJack e-com/pinpad")==0)
    ic=a->getIcon(FXCY_App::IconId_PinpadBig);
  else if (strcasecmp(s, "cyberJack e-com plus")==0)
    ic=a->getIcon(FXCY_App::IconId_EcomPlusBig);
  else if (strcasecmp(s, "cyberJack e-com plus RFID")==0)
    ic=a->getIcon(FXCY_App::IconId_EcomPlusBig);
  else if (strcasecmp(s, "cyberJack Secoder")==0)
    ic=a->getIcon(FXCY_App::IconId_SecoderBig);
  if (ic==NULL)
    ic=a->getIcon(FXCY_App::IconId_EcomABig);
  m_readerIcon=ic;

  new FXLabel(hf, FXString(r->getProductString().c_str()).trim(), ic, TEXT_BELOW_ICON);

  /* reader info */
  if (ri.ContentsMask==0) {
    new FXLabel(hf,
                "Keine Details verfuegbar.\n"
                "Aeltere Leser (wie cyberJack Pinpad und cyberJack e-com) stellen\n"
                "keine Informationen ueber installierte Module etc zur Verfuegung.",
                NULL, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  }
  else {
    FXMatrix *m;

    m=new FXMatrix(hf, 2, MATRIX_BY_COLUMNS | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    if (ri.ContentsMask & RSCT_READER_MASK_PID) {
      new FXLabel(m, "PID", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, FXStringVal(ri.PID, 16), NULL,
                     LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_VENDOR_STRING) {
      new FXLabel(m, "Hersteller", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.VendorString).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_PRODUCT_STRING) {
      new FXLabel(m, "Produkt", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.ProductString).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
      new FXLabel(m, "Seriennummer", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.SeriaNumber).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE) {
      new FXLabel(m, "Herstellungsdatum", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.ProductionDate).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE) {
      new FXLabel(m, "Testdatum", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.TestDate).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE) {
      new FXLabel(m, "Freigabedatum", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.CommissioningDate).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_COM_TYPE) {
      new FXLabel(m, "Anschlussart", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.CommunicationString).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE) {
      new FXLabel(m, "Eigenschaften", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      str="";
  
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC1)
        str+=" ICC1";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC2)
        str+=" ICC2";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC3)
        str+=" ICC3";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC4)
        str+=" ICC4";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC5)
        str+=" ICC5";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC6)
        str+=" ICC6";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC7)
        str+=" ICC7";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC8)
        str+=" ICC8";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_KEYPAD)
        str+=" Keypad";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_DISPLAY)
        str+=" Display";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_BIOMETRIC)
        str+=" Fingersensor";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_UPDATEABLE)
        str+=" Firmwareupdate";
      if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_MODULES)
        str+=" Sicherheitsmodule";
  
      str.trim();
  
      ll->setText(str);
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_VERSION) {
      new FXLabel(m, "Version", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXStringVal(ri.Version, 16));
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION) {
      new FXLabel(m, "Hardware-Version", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXStringVal(ri.HardwareVersion, 16));
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_IFD_BRIDGE) {
      new FXLabel(m, "IFD-Bridge", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.IFDNameOfIfdBridgeDevice).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_HW_STRING) {
      new FXLabel(m, "Hardware-String", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXString((const char*)ri.HardwareString).trim());
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_FLASH_SIZE) {
      new FXLabel(m, "Flashgroesse", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXStringVal(ri.FlashSize, 10));
    }
  
    if (ri.ContentsMask & RSCT_READER_MASK_HEAP_SIZE) {
      new FXLabel(m, "Heapgroesse", NULL, LABEL_NORMAL);
      ll=new FXLabel(m, "", NULL, LABEL_NORMAL | JUSTIFY_LEFT | FRAME_SUNKEN | LAYOUT_FILL_X);
      ll->setBackColor(getApp()->getBackColor());
      ll->setText(FXStringVal(ri.HeapSize, 10));
    }
  }

  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
  new FXButton(hf, "Schliessen", NULL, this, ID_ACCEPT,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);

}



FXCY_ReaderDetails::FXCY_ReaderDetails()
:FXDialogBox()
,m_reader(NULL)
,m_readerIcon(NULL) {
}



FXCY_ReaderDetails::~FXCY_ReaderDetails() {
}



void FXCY_ReaderDetails::create() {
  FXDialogBox::create();
  if (m_readerIcon)
    m_readerIcon->create();
}



long FXCY_ReaderDetails::onCmdAccept(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,TRUE);
  hide();
  return 1;
}



long FXCY_ReaderDetails::onCmdReject(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,FALSE);
  hide();
  return 1;
}





