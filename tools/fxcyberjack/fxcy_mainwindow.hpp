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


#ifndef FXCY_MAINWINDOW_HPP
#define FXCY_MAINWINDOW_HPP

#include <fx.h>

#include "libcyberjack/driver.hpp"
#include "fxcy_readerview.hpp"



class FXCY_MainWindow: public FXMainWindow {
  FXDECLARE(FXCY_MainWindow)

protected:
  FXCY_MainWindow();

public:
  enum {
    ID_CMD_REFRESH=FXMainWindow::ID_LAST,
    ID_CMD_CHECK,
    ID_CMD_CHECK2,
    ID_LAST
  };

  long onCmdRefresh(FXObject*, FXSelector, void *ptr);
  long onCmdDiag(FXObject*, FXSelector, void *ptr);
  long onCmdDiagWithReaders(FXObject*, FXSelector, void *ptr);

  FXCY_MainWindow(FXApp* a,
                  const FXString& name,
                  FXIcon *ic=NULL, FXIcon *mi=NULL,
                  FXuint opts=DECOR_ALL,
                  FXint x=0, FXint y=0, FXint w=0, FXint h=0,
                  FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0,
                  FXint hs=0, FXint vs=0);
  ~FXCY_MainWindow();

  void create();

  int openDriverCtapi();
  int openDriverPcsc();
  int updateReaderList(bool force);
  int closeDriver();

protected:
  FXCY_ReaderView *m_readerView;
  Cyberjack::Driver *m_driver;

  FXMenuBar *m_menubar;
  FXMenuPane *m_filemenu;
  FXMenuPane *m_viewmenu;
  FXMenuPane *m_diagmenu;

  FXStatusBar *m_statusbar;
};


#endif





