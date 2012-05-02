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


#include "fxcy_mainwindow.hpp"
#include "fxcy_readerview.hpp"

#include "diag/fxcy_diag.h"
#include "libcyberjack/driver_ctapi.hpp"

#ifdef HAVE_PCSC
# include "libcyberjack/driver_pcsc.hpp"
#endif



FXDEFMAP(FXCY_MainWindow) FXCY_MainWindowMap[]={
  FXMAPFUNC(SEL_COMMAND, FXCY_MainWindow::ID_CMD_REFRESH, FXCY_MainWindow::onCmdRefresh),
  FXMAPFUNC(SEL_COMMAND, FXCY_MainWindow::ID_CMD_CHECK, FXCY_MainWindow::onCmdDiag),
  FXMAPFUNC(SEL_COMMAND, FXCY_MainWindow::ID_CMD_CHECK2, FXCY_MainWindow::onCmdDiagWithReaders),
};


FXIMPLEMENT(FXCY_MainWindow, FXMainWindow, FXCY_MainWindowMap, ARRAYNUMBER(FXCY_MainWindowMap))




FXCY_MainWindow::FXCY_MainWindow(FXApp* a,
                                 const FXString& name,
                                 FXIcon *ic, FXIcon *mi,
                                 FXuint opts,
                                 FXint x, FXint y, FXint w, FXint h,
                                 FXint pl, FXint pr, FXint pt, FXint pb,
                                 FXint hs, FXint vs)
:FXMainWindow(a, name, ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
,m_readerView(NULL)
,m_driver(NULL)
,m_menubar(NULL)
,m_filemenu(NULL)
,m_viewmenu(NULL)
,m_diagmenu(NULL)
{
  FXCY_ReaderList *rl;

  /* Menu bar */
  m_menubar=new FXMenuBar(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);

  /* File menu */
  m_filemenu=new FXMenuPane(this);
  new FXMenuCommand(m_filemenu, "&Beenden\tCtl-Q", NULL, getApp(), FXApp::ID_QUIT);
  new FXMenuTitle(m_menubar, "&Datei", NULL, m_filemenu);

#if 0
  /* Status bar */
  m_statusbar=new FXStatusBar(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | STATUSBAR_WITH_DRAGCORNER);
#endif

  /* main view */
  m_readerView=new FXCY_ReaderView(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  rl=m_readerView->getReaderList();

  /* diagnostic menu */
  m_diagmenu=new FXMenuPane(this);
  new FXMenuCommand(m_diagmenu, "&System pruefen", NULL, this, ID_CMD_CHECK);
  new FXMenuCommand(m_diagmenu, "&System und Leser pruefen", NULL, this, ID_CMD_CHECK2);
  new FXMenuTitle(m_menubar, "&Diagnose", NULL, m_diagmenu);

  /* Arrange menu */
  m_viewmenu=new FXMenuPane(this);
  new FXMenuCommand(m_viewmenu,"Auff&rischen\tF5", NULL, this, ID_CMD_REFRESH);
  new FXMenuSeparator(m_viewmenu);
  new FXMenuRadio(m_viewmenu,"&Details",rl, FXIconList::ID_SHOW_DETAILS);
  new FXMenuRadio(m_viewmenu,"&Kleine Icons",rl, FXIconList::ID_SHOW_MINI_ICONS);
  new FXMenuRadio(m_viewmenu,"&Grosse Icons",rl, FXIconList::ID_SHOW_BIG_ICONS);
  new FXMenuSeparator(m_viewmenu);
  new FXMenuRadio(m_viewmenu,"&Zeilenweise",rl, FXIconList::ID_ARRANGE_BY_ROWS);
  new FXMenuRadio(m_viewmenu,"&Spaltenweise",rl, FXIconList::ID_ARRANGE_BY_COLUMNS);
  new FXMenuTitle(m_menubar,"&Ansicht", NULL, m_viewmenu);
}



FXCY_MainWindow::FXCY_MainWindow()
:FXMainWindow()
,m_readerView(NULL)
,m_menubar(NULL)
,m_filemenu(NULL)
,m_viewmenu(NULL)
,m_diagmenu(NULL) {
}



FXCY_MainWindow::~FXCY_MainWindow() {
  fprintf(stderr, "Destroying MainWindow\n");
  if (m_driver) {
    m_readerView->clearReaderList();
    m_driver->clearReaderList();
    m_driver->close();
    delete m_driver;
    m_driver=NULL;
  }
  delete m_filemenu;
  delete m_diagmenu;
  delete m_viewmenu;
  delete m_menubar;
}



void FXCY_MainWindow::create() {
  FXMainWindow::create();
  m_readerView->create();
}



int FXCY_MainWindow::updateReaderList(bool force) {
  if (m_driver) {
    int rv;

    if (force) {
      m_readerView->clearReaderList();
      m_driver->clearReaderList();
    }

    rv=m_driver->enumReaders();
    if (rv<0) {
      fprintf(stderr, "Error in enumReaders: %d\n", rv);
      m_readerView->clearReaderList();
      m_driver->clearReaderList();
      return rv;
    }

#if 0
    /* get extended reader info */
    std::list<Cyberjack::Reader*>::const_iterator it;
    for (it=m_driver->getReaders().begin(); it!=m_driver->getReaders().end(); it++) {
      if ((*it)->getUpdateCounter()==0) {
        rv=(*it)->gatherInfo(true);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          fprintf(stderr, "Error gathering info: %d\n", rv);
        }
      }
    }
#endif

    m_readerView->applyReaderList(m_driver->getUpdateCounter(), m_driver->getReaders());
    m_driver->removeOldReaders();
    return Cyberjack::ErrorCode_Ok;
  }
  else {
    return Cyberjack::ErrorCode_Invalid;
  }
}



int FXCY_MainWindow::openDriverCtapi() {
  Cyberjack::Driver *dr;
  int rv;

  dr=new Cyberjack::DriverCtapi();

  rv=dr->open();
  if (rv<0) {
    fprintf(stderr, "Error in open: %d\n", rv);
    delete dr;
    return rv;
  }

  m_driver=dr;
  return Cyberjack::ErrorCode_Ok;
}



int FXCY_MainWindow::openDriverPcsc() {
#ifdef HAVE_PCSC
  Cyberjack::Driver *dr;
  int rv;

  dr=Cyberjack::NewDriverPcsc();

  rv=dr->open();
  if (rv<0) {
    fprintf(stderr, "Error in open: %d\n", rv);
    delete dr;
    return rv;
  }

  m_driver=dr;
  return Cyberjack::ErrorCode_Ok;
#else
  fprintf(stderr, "No support for PC/SC\n");
  return Cyberjack::ErrorCode_Generic;
#endif
}



int FXCY_MainWindow::closeDriver() {
  if (m_driver) {
    int rv;

    m_readerView->clearReaderList();
    m_driver->clearReaderList();
    rv=m_driver->close();
    delete m_driver;
    m_driver=NULL;
    if (rv<0) {
      fprintf(stderr, "Error in close: %d\n", rv);
      return rv;
    }
  }

  return Cyberjack::ErrorCode_Ok;
}



long FXCY_MainWindow::onCmdRefresh(FXObject*, FXSelector, void *ptr) {
  if (m_driver) {
    fprintf(stderr, "Refresh\n");
    updateReaderList(true);
  }
  return 1;
}



long FXCY_MainWindow::onCmdDiag(FXObject*, FXSelector, void *ptr) {
  FXCY_Diagnosis dlg(this, "System ueberpruefen", false, DECOR_ALL);
  dlg.create();
  dlg.resize(500, 400);
  dlg.show();
  dlg.performTest();
  dlg.execute();

  return 1;
}



long FXCY_MainWindow::onCmdDiagWithReaders(FXObject*, FXSelector, void *ptr) {
  FXCY_Diagnosis dlg(this, "System ueberpruefen", true, DECOR_ALL);
  dlg.create();
  dlg.resize(500, 400);
  dlg.show();
  dlg.performTest();
  dlg.execute();

  return 1;
}





