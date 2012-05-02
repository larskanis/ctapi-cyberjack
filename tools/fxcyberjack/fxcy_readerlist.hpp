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


#ifndef FXCY_READERLIST_HPP
#define FXCY_READERLIST_HPP

#include <fx.h>

#include "libcyberjack/driver.hpp"



class FXCY_ReaderList: public FXIconList {
  FXDECLARE(FXCY_ReaderList)

protected:
  FXCY_ReaderList();

public:
  FXCY_ReaderList(FXComposite *p, FXObject* tgt=NULL, FXSelector sel=0,
                  FXuint opts=ICONLIST_NORMAL,
                  FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~FXCY_ReaderList();

  void addReader(Cyberjack::Reader *r);
  void applyReaderList(uint32_t currentCounter, std::list<Cyberjack::Reader*> &rl);

  Cyberjack::Reader *getCurrentReader();

  void create();

protected:
  FXIcon *m_iconSmallPinpad;   /* 0x100 */
  FXIcon *m_iconSmallPinpadA;  /* 0x300 */
  FXIcon *m_iconSmallEcom;     /* 0x100 */
  FXIcon *m_iconSmallEcomA;    /* 0x400 */
  FXIcon *m_iconSmallEcomPlus; /* 0x400 */
  FXIcon *m_iconSmallSecoder;  /* 0x400 */

  FXIcon *m_iconBigPinpad;     /* 0x100 */
  FXIcon *m_iconBigPinpadA;    /* 0x300 */
  FXIcon *m_iconBigEcom;       /* 0x100 */
  FXIcon *m_iconBigEcomA;      /* 0x400 */
  FXIcon *m_iconBigEcomPlus;   /* 0x400 */
  FXIcon *m_iconBigSecoder;    /* 0x400 */
};


#endif



