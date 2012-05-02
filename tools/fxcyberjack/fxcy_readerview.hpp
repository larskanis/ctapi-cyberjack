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


#ifndef FXCY_READERVIEW_HPP
#define FXCY_READERVIEW_HPP

#include <fx.h>

#include "libcyberjack/driver.hpp"
#include "fxcy_readerlist.hpp"



class FXCY_ReaderView: public FXHorizontalFrame {
  FXDECLARE(FXCY_ReaderView)

protected:
  FXCY_ReaderView();

public:
  enum {
    ID_READER_LIST=FXHorizontalFrame::ID_LAST,
    ID_BUTTON_DETAILS,
    ID_BUTTON_TEST,
    ID_BUTTON_FIRMWARE,
    ID_BUTTON_UPD_KEYS,
    ID_BUTTON_DEL_MODS,
    ID_BUTTON_FLASH,
    ID_LAST
  };

  long onCmdDetails(FXObject*, FXSelector, void *ptr);
  long onCmdTest(FXObject*, FXSelector, void *ptr);
  long onCmdFirmware(FXObject*, FXSelector, void *ptr);
  long onCmdUpdateKeys(FXObject*, FXSelector, void *ptr);
  long onCmdDelModules(FXObject*, FXSelector, void *ptr);
  long onCmdFlash(FXObject*, FXSelector, void *ptr);

  FXCY_ReaderView(FXComposite *p,
                  FXuint opts=0,
                  FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~FXCY_ReaderView();

  void applyReaderList(uint32_t currentCounter, std::list<Cyberjack::Reader*> &rl);
  void clearReaderList();

  void create();

  FXCY_ReaderList *getReaderList() { return m_readerList;};

protected:
  FXCY_ReaderList *m_readerList;
};


#endif



