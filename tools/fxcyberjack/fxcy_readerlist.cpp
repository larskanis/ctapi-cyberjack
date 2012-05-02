/***************************************************************************
    begin       : Tue Mar 24 2009
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


#include "fxcy_readerlist.hpp"
#include "fxcy_app.hpp"


FXIMPLEMENT(FXCY_ReaderList, FXIconList,NULL,0)




FXCY_ReaderList::FXCY_ReaderList(FXComposite *p, FXObject* tgt, FXSelector sel,
                                 FXuint opts,
                                 FXint x, FXint y, FXint w, FXint h)
:FXIconList(p, tgt, sel, opts, x, y, w, h)
,m_iconSmallPinpad(NULL)
,m_iconSmallPinpadA(NULL)
,m_iconSmallEcom(NULL)
,m_iconSmallEcomA(NULL)
,m_iconSmallEcomPlus(NULL)
,m_iconSmallSecoder(NULL)
,m_iconBigPinpad(NULL)
,m_iconBigPinpadA(NULL)
,m_iconBigEcom(NULL)
,m_iconBigEcomA(NULL)
,m_iconBigEcomPlus(NULL)
,m_iconBigSecoder(NULL)
 {
  FXCY_App *a=dynamic_cast<FXCY_App*>(getApp());

  appendHeader("Name", NULL, 200);
  appendHeader("Anschluss", NULL, 200);
  appendHeader("Seriennummer", NULL, 200);

  /* big icons */
  m_iconBigPinpad=a->getIcon(FXCY_App::IconId_PinpadNormal);
  m_iconBigPinpadA=a->getIcon(FXCY_App::IconId_PinpadANormal);
  m_iconBigEcom=a->getIcon(FXCY_App::IconId_EcomNormal);
  m_iconBigEcomA=a->getIcon(FXCY_App::IconId_EcomANormal);
  m_iconBigEcomPlus=a->getIcon(FXCY_App::IconId_EcomPlusNormal);
  m_iconBigSecoder=a->getIcon(FXCY_App::IconId_SecoderNormal);

  m_iconSmallPinpad=a->getIcon(FXCY_App::IconId_PinpadSmall);
  m_iconSmallPinpadA=a->getIcon(FXCY_App::IconId_PinpadASmall);
  m_iconSmallEcom=a->getIcon(FXCY_App::IconId_EcomSmall);
  m_iconSmallEcomA=a->getIcon(FXCY_App::IconId_EcomASmall);
  m_iconSmallEcomPlus=a->getIcon(FXCY_App::IconId_EcomPlusSmall);
  m_iconSmallSecoder=a->getIcon(FXCY_App::IconId_SecoderSmall);
}



FXCY_ReaderList::FXCY_ReaderList()
:FXIconList()
,m_iconSmallPinpad(NULL)
,m_iconSmallPinpadA(NULL)
,m_iconSmallEcom(NULL)
,m_iconSmallEcomA(NULL)
,m_iconSmallEcomPlus(NULL)
,m_iconSmallSecoder(NULL)
,m_iconBigPinpad(NULL)
,m_iconBigPinpadA(NULL)
,m_iconBigEcom(NULL)
,m_iconBigEcomA(NULL)
,m_iconBigEcomPlus(NULL)
,m_iconBigSecoder(NULL)
{
}



FXCY_ReaderList::~FXCY_ReaderList() {
}



void FXCY_ReaderList::create() {
  FXIconList::create();

  /* realize small icons */
  if (m_iconSmallPinpad)
    m_iconSmallPinpad->create();
  if (m_iconSmallPinpadA)
    m_iconSmallPinpadA->create();
  if (m_iconSmallEcom)
    m_iconSmallEcom->create();
  if (m_iconSmallEcomA)
    m_iconSmallEcomA->create();
  if (m_iconSmallEcomPlus)
    m_iconSmallEcomPlus->create();
  if (m_iconSmallSecoder)
    m_iconSmallSecoder->create();

  /* realize big icons */
  if (m_iconBigPinpad)
    m_iconBigPinpad->create();
  if (m_iconBigPinpadA)
    m_iconBigPinpadA->create();
  if (m_iconBigEcom)
    m_iconBigEcom->create();
  if (m_iconBigEcomA)
    m_iconBigEcomA->create();
  if (m_iconBigEcomPlus)
    m_iconBigEcomPlus->create();
  if (m_iconBigSecoder)
    m_iconBigSecoder->create();
}



void FXCY_ReaderList::addReader(Cyberjack::Reader *r) {
  FXIcon *icBig=NULL;
  FXIcon *icSmall=NULL;
  FXString t;
  FXString name;

  if (r->getBusType()==Cyberjack::BusType_Pcsc) {
    name=FXString(r->getName().c_str()).lower();
    if (name.contains("pp_a")) {
      icBig=m_iconBigPinpadA;
      icSmall=m_iconSmallPinpadA;
    }
    else if (name.contains("ecom_a")) {
      icBig=m_iconBigEcomA;
      icSmall=m_iconSmallEcomA;
    }
    else {
      icBig=m_iconBigPinpad;
      icSmall=m_iconSmallPinpad;
    }
  }
  else {
    name=FXString(r->getProductString().c_str()).lower();
    if (name.contains("e-com/pinpad")) {
      icBig=m_iconBigPinpad;
      icSmall=m_iconSmallPinpad;
    }
    else if (name.contains("pinpad(a)")) {
      icBig=m_iconBigPinpadA;
      icSmall=m_iconSmallPinpadA;
    }
    else if (name.contains("e-com(a)")) {
      icBig=m_iconBigEcomA;
      icSmall=m_iconSmallEcomA;
    }
    else if (name.contains("e-com plus")) {
      icBig=m_iconBigEcomPlus;
      icSmall=m_iconSmallEcomPlus;
    }
    else if (name.contains("secoder")) {
      icBig=m_iconBigSecoder;
      icSmall=m_iconSmallSecoder;
    }
  }

  /* reader name */
  t=r->getName().c_str();
  if (t.left(10)=="cyberJack ")
    t.erase(0, 10);
  t+="\t";

  /* COM type */
  switch(r->getBusType()) {
  case Cyberjack::BusType_None:   t+="keiner"; break;
  case Cyberjack::BusType_UsbRaw: t+="USB"; break;
  case Cyberjack::BusType_UsbTty: t+="USBTTY"; break;
  case Cyberjack::BusType_Serial: t+="seriell"; break;
  case Cyberjack::BusType_Pcsc:   t+="PC/SC"; break;
  default:                        t+="unbekannt"; break;
  }
  t+="\t";

  /* serial number */
  t+=r->getSerial().c_str();
  t+="\t";

  /* append item */
  appendItem(t, icBig, icSmall, (void*) r, false);
}



void FXCY_ReaderList::applyReaderList(uint32_t currentCounter, std::list<Cyberjack::Reader*> &rl) {
  std::list<Cyberjack::Reader*>::iterator rit;

  for (rit=rl.begin(); rit!=rl.end(); rit++) {
    Cyberjack::Reader *r=*rit;
    if (r->getUpdateCounter()==0) {
      /* add reader */
      r->setUpdateCounter(currentCounter);
      addReader(r);
    }
    else if (r->getUpdateCounter()<currentCounter) {
      int i;

      /* remove reader */
      i=findItemByData(r);
      if (i!=-1)
        removeItem(i, false);
    }
  }

}


Cyberjack::Reader *FXCY_ReaderList::getCurrentReader() {
  int idx=getCurrentItem();
  if (idx!=-1) {
    FXIconItem *item=getItem(idx);
    if (item)
      return (Cyberjack::Reader*)(item->getData());
  }

  return NULL;
}



