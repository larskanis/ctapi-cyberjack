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


#include "fxcy_app.hpp"
#include "icons/icons.cpp"


FXCY_App::FXCY_App(const FXString& name, const FXString& vendor)
:FXApp(name, vendor)
,m_iconSource(NULL){
  m_iconSource=new FXIconSource(this);
}



FXCY_App::~FXCY_App() {
}



FXIcon *FXCY_App::getIcon(FXCY_App::IconId id) {
  switch(id) {
  case IconId_PinpadSmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_s, "jpg");
  case IconId_PinpadNormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_n, "jpg");
  case IconId_PinpadBig:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_b, "jpg");

  case IconId_PinpadASmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_s, "jpg");
  case IconId_PinpadANormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_n, "jpg");
  case IconId_PinpadABig:
    return m_iconSource->loadIconData(fxcj_icon_cy_pinpad_b, "jpg");

  case IconId_EcomSmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_s, "jpg");
  case IconId_EcomNormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_n, "jpg");
  case IconId_EcomBig:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_b, "jpg");

  case IconId_EcomASmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_s, "jpg");
  case IconId_EcomANormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_n, "jpg");
  case IconId_EcomABig:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_b, "jpg");

  case IconId_EcomPlusSmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_plus_s, "jpg");
  case IconId_EcomPlusNormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_plus_n, "jpg");
  case IconId_EcomPlusBig:
    return m_iconSource->loadIconData(fxcj_icon_cy_ecom_plus_b, "jpg");

  case IconId_SecoderSmall:
    return m_iconSource->loadIconData(fxcj_icon_cy_secoder_s, "jpg");
  case IconId_SecoderNormal:
    return m_iconSource->loadIconData(fxcj_icon_cy_secoder_n, "jpg");
  case IconId_SecoderBig:
    return m_iconSource->loadIconData(fxcj_icon_cy_secoder_b, "jpg");

  case IconId_Unknown:
  default:
    return NULL;
  }
}


