/***************************************************************************
    begin       : Tue Apr 06 2010
    copyright   : (C) 2010 by Martin Preuss
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


#ifndef FXCY_READERFLASH2_HPP
#define FXCY_READERFLASH2_HPP

#include "libcyberjack/driver.hpp"
#include "libcyberjack/modulestore.hpp"

#include <fx.h>

#include <list>


class FXCY_App;


class FXCY_ReaderFlash2: public FXDialogBox {
  FXDECLARE(FXCY_ReaderFlash2)

protected:
  FXCY_ReaderFlash2();

public:

  enum {
    ID_LISTBOX=FXDialogBox::ID_LAST,
    ID_MODULE_LIST,
    ID_BUTTON_FLASH,

    ID_LAST
  };

  long onCmdReject(FXObject*, FXSelector, void *ptr);
  long onCmdButtonFlash(FXObject*, FXSelector, void *ptr);
  long onCmdListBox(FXObject*, FXSelector, void *ptr);


  FXCY_ReaderFlash2(Cyberjack::Reader *r,
		    FXWindow *owner,
		    const FXString& name,
		    FXuint opts=0,
		    FXint x=0, FXint y=0, FXint w=0, FXint h=0);
  ~FXCY_ReaderFlash2();

  void toGui();

  FXint getDefaultWidth();
  FXint getDefaultHeight();

protected:
  Cyberjack::Reader *m_reader;
  std::list<Cyberjack::ModuleStore> m_moduleStoreList;
  Cyberjack::ModuleStore *m_moduleStore;

  FXListBox *m_listBox;
  FXFoldingList *m_moduleList;
  FXButton *m_flashButton;

  FXString m_lastPath;

  bool selectModuleStore();
  void selectedTemplate(int idx);
  Cyberjack::Module *findModuleByName(const char *s);


};


#endif



