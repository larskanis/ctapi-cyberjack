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


#ifndef FXCY_APP_HPP
#define FXCY_APP_HPP

#include <fx.h>


class FXCY_App: public FXApp {
public:
  typedef enum {
    IconId_Unknown=0,
    IconId_PinpadSmall=1,
    IconId_PinpadNormal,
    IconId_PinpadBig,
    IconId_PinpadASmall,
    IconId_PinpadANormal,
    IconId_PinpadABig,
    IconId_EcomSmall,
    IconId_EcomNormal,
    IconId_EcomBig,
    IconId_EcomASmall,
    IconId_EcomANormal,
    IconId_EcomABig,
    IconId_EcomPlusSmall,
    IconId_EcomPlusNormal,
    IconId_EcomPlusBig,
    IconId_SecoderSmall,
    IconId_SecoderNormal,
    IconId_SecoderBig
  } IconId;

  FXCY_App(const FXString& name="Application",const FXString& vendor="FoxDefault");
  virtual ~FXCY_App();

  FXIcon *getIcon(IconId id);

protected:
  FXIconSource *m_iconSource;

};


#endif


