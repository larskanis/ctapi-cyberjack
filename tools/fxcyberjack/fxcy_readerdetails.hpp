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


#ifndef FXCY_READERDETAILS_HPP
#define FXCY_READERDETAILS_HPP

#include <fx.h>

#include "libcyberjack/driver.hpp"

class FXCY_App;


class FXCY_ReaderDetails: public FXDialogBox {
  FXDECLARE(FXCY_ReaderDetails)

protected:
  FXCY_ReaderDetails();

public:

  long onCmdAccept(FXObject*, FXSelector, void *ptr);
  long onCmdReject(FXObject*, FXSelector, void *ptr);


  FXCY_ReaderDetails(Cyberjack::Reader *r,
                     FXWindow *owner,
                     const FXString& name,
                     FXuint opts=0,
                     FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~FXCY_ReaderDetails();

  void create();

protected:
  Cyberjack::Reader *m_reader;
  FXIcon *m_readerIcon;

};


#endif



