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


#include "fxcy_readerview.hpp"
#include "fxcy_readerdetails.hpp"
#include "fxcy_readerflash.hpp"
#include "fxcy_readerkeys.hpp"
#include "fxcy_app.hpp"

#ifdef HAVE_LIBXML2
# include "fxcy_readerflash2.hpp"
#endif


#include <errno.h>
#include <string.h>


#define DISABLE_FLASHING



FXDEFMAP(FXCY_ReaderView) FXCY_ReaderViewMap[]={
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_DETAILS, FXCY_ReaderView::onCmdDetails),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_TEST, FXCY_ReaderView::onCmdTest),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_FIRMWARE, FXCY_ReaderView::onCmdFirmware),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_UPD_KEYS, FXCY_ReaderView::onCmdUpdateKeys),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_DEL_MODS, FXCY_ReaderView::onCmdDelModules),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderView::ID_BUTTON_FLASH, FXCY_ReaderView::onCmdFlash),
};


FXIMPLEMENT(FXCY_ReaderView, FXHorizontalFrame, FXCY_ReaderViewMap, ARRAYNUMBER(FXCY_ReaderViewMap))




FXCY_ReaderView::FXCY_ReaderView(FXComposite *p,
                                 FXuint opts,
                                 FXint x, FXint y, FXint w, FXint h)
:FXHorizontalFrame(p, opts, x, y, w, h)
,m_readerList(NULL){
  FXVerticalFrame *vf;
#ifndef DISABLE_FLASHING
  FXButton *btn;
#endif

  m_readerList=new FXCY_ReaderList(this, this, ID_READER_LIST,
                                   LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                   ICONLIST_BROWSESELECT |
                                   /*ICONLIST_DETAILED |*/
                                   ICONLIST_BIG_ICONS |
                                   ICONLIST_COLUMNS);
  vf=new FXVerticalFrame(this, LAYOUT_FILL_Y | PACK_UNIFORM_WIDTH | PACK_UNIFORM_HEIGHT);
  new FXButton(vf, "Details", NULL, this, ID_BUTTON_DETAILS, BUTTON_NORMAL);
  new FXButton(vf, "Test", NULL, this, ID_BUTTON_TEST, BUTTON_NORMAL);

#ifdef HAVE_LIBXML2
  new FXButton(vf, "Firmware\naktualisieren", NULL, this, ID_BUTTON_FIRMWARE, BUTTON_NORMAL);
#endif

#ifndef DISABLE_FLASHING
  btn=new FXButton(vf, "Schluessel\naktualisieren", NULL, this, ID_BUTTON_UPD_KEYS, BUTTON_NORMAL);
  btn->setTipText("Aktualisiert die Schluessel im Leser (nur neuere cyberJacks)");

  btn=new FXButton(vf, "Alle Module\nentfernen", NULL, this, ID_BUTTON_DEL_MODS, BUTTON_NORMAL);
  btn->setTipText("Entfernt alle Module (nur neuere cyberJacks)");

  btn=new FXButton(vf, "Modul\ninstallieren", NULL, this, ID_BUTTON_FLASH, BUTTON_NORMAL);
  btn->setTipText("Installiert ein neues Modul auf dem Leser (nur neuere cyberJacks)");
#endif
}



FXCY_ReaderView::FXCY_ReaderView()
:FXHorizontalFrame()
,m_readerList(NULL){
}



FXCY_ReaderView::~FXCY_ReaderView() {
}



void FXCY_ReaderView::applyReaderList(uint32_t currentCounter, std::list<Cyberjack::Reader*> &rl) {
  if (m_readerList)
    m_readerList->applyReaderList(currentCounter, rl);
}



void FXCY_ReaderView::clearReaderList() {
  if (m_readerList)
    m_readerList->clearItems();
}



void FXCY_ReaderView::create() {
  FXHorizontalFrame::create();
  if (m_readerList)
    m_readerList->create();
}



long FXCY_ReaderView::onCmdDetails(FXObject*, FXSelector, void *ptr) {
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    int rv;

    rv=r->gatherInfo(true);
    if (rv!=Cyberjack::ErrorCode_Ok) {
      fprintf(stderr, "Error gathering info: %d\n", rv);
      FXMessageBox::error(this, MBOX_OK,
                          "Ein-/Ausgabe-Fehler",
                          "Beim Abrufen der Leser-Details ist ein Fehler aufgetreten.\n"
                          "Sie sollten mit diesem Leser einen Test ausfuehren um Probleme\n"
                          "in der Einrichtung des Lesers auszuschliessen bzw. zu beheben.\n"
                          "(Fehlercode: %d)", rv);
      return 1;
    }

    FXCY_ReaderDetails dlg(r, this,
                           "Details zum Leser",
                           DECOR_ALL);
    dlg.create();
    dlg.show();
    dlg.execute();
  }
  return 1;
}



long FXCY_ReaderView::onCmdTest(FXObject*, FXSelector, void *ptr) {
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    int rv;
    std::string msg;

    rv=r->test(msg);
    if (rv!=Cyberjack::ErrorCode_Ok) {
      FXMessageBox::error(this, MBOX_OK,
                          "Test fehlgeschlagen",
                          "Bei der Ausfuehrung des Lesertests ist ein Fehler aufgetreten.\n"
                          "(Fehlercode: %d)\n"
                          "\n"
                          "Der Bericht folgt:\n"
                          "==================================================\n"
                          "%s\n"
                          "==================================================\n",
                          rv, msg.c_str());
      return 1;
    }
    else {
      FXMessageBox::information(this, MBOX_OK,
                                "Test erfolgreich",
                                "Der Lesertest war erfolgreich.\n"
                                "\n"
                                "Der Bericht folgt:\n"
                                "==================================================\n"
                                "%s\n"
                                "==================================================\n",
                                msg.c_str());
    }
  }
  return 1;
}



long FXCY_ReaderView::onCmdFirmware(FXObject*, FXSelector, void *ptr) {
#ifdef HAVE_LIBXML2
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    int rv;

    rv=r->gatherInfo(true);
    if (rv!=Cyberjack::ErrorCode_Ok) {
      fprintf(stderr, "Error gathering info: %d\n", rv);
      FXMessageBox::error(this, MBOX_OK,
                          "Ein-/Ausgabe-Fehler",
                          "Beim Abrufen der Leser-Details ist ein Fehler aufgetreten.\n"
                          "Sie sollten mit diesem Leser einen Test ausfuehren um Probleme\n"
                          "in der Einrichtung des Lesers auszuschliessen bzw. zu beheben.\n"
                          "(Fehlercode: %d)", rv);
      return 1;
    }


    FXCY_ReaderFlash2 dlg(r, this,
			  "Firmware Aktualisieren",
			  DECOR_ALL);
    dlg.create();
    dlg.toGui();
    dlg.show();
    dlg.execute();
  }
#endif
  return 1;
}



static int readFile(const char *fname, uint8_t *buffer, uint32_t size) {
  FILE *f;
  uint8_t *p;
  int len;

  f=fopen(fname, "r");
  if (f==NULL)
    return -1;

  p=buffer;
  len=0;
  while(!feof(f)) {
    int rv;
    int l;

    l=size;
    if (l<1) {
      fprintf(stderr, "ERROR: Buffer too small\n");
      return -1;
    }
    if (l>1024)
      l=1024;
    rv=fread(p, 1, l, f);
    if (rv==0)
      break;
    p+=rv;
    len+=rv;
    size-=rv;
  }
  fclose(f);
  return len;
}



long FXCY_ReaderView::onCmdUpdateKeys(FXObject*, FXSelector, void *ptr) {
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    FXCY_ReaderKeys dlg(r, this,
                        "Schluessel aktualisieren",
                        DECOR_ALL);
    dlg.create();
    dlg.show();
    for (;;) {
      if (dlg.execute()) {
        FXString keyFileName;
        uint8_t buffer1[64*1024];
        int len1;
        int rv;

        fprintf(stderr, "Accepted\n");
        keyFileName=dlg.getKeyFile();

        len1=readFile(keyFileName.text(), buffer1, sizeof(buffer1));
        if (len1<1) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Lesefehler",
                              "Beim Einlesen der Schluesseldatei ist ein Fehler aufgetreten\n"
                              "Meldung vom System: %s\n",
                              strerror(errno));
          continue;
        }

        rv=r->connect(Cyberjack::Object_Reader);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Leserfehler",
                              "Beim Ansprechen des Lesers ist ein Fehler aufgetreten\n"
                              "(Fehlercode: %d)\n", rv);
          continue;
        }

        /* got both module and signature, flash it */
        rv=r->updateKeys(buffer1, len1);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Fehler",
                              "Beim Senden des Schluessels an den Treiber ist ein Fehler aufgetreten.\n"
                              "(Fehlercode: %d)\n", rv);
          r->disconnect();
          continue;
        }

        rv=r->disconnect();
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Leserfehler",
                              "Beim Schliessen der Verbindung zum Lesers ist ein Fehler aufgetreten\n"
                              "(Fehlercode: %d)\n", rv);
          continue;
        }

        FXMessageBox::information(&dlg, MBOX_OK,
                                  "Update erfolgreich",
                                  "Der Schluessel wurde erfolgreich aktualisiert.\n"
                                  "Eventuell muessen Sie nun den Leser kurz abziehen und\n"
                                  "erneut anschliessen.");
        break;
      }
      else {
        fprintf(stderr, "Rejected\n");
        break;
      }
    }
    dlg.hide();
  }
  return 1;
}



long FXCY_ReaderView::onCmdDelModules(FXObject*, FXSelector, void *ptr) {
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    if (MBOX_CLICKED_YES==FXMessageBox::question(this, MBOX_YES_NO,
						 "Alle Module entfernen",
						 "Diese Aktion entfernt alle Module vom Leser.\n"
						 "Dies funktioniert nur mit neueren cyberJacks\n"
						 "\n"
						 "Wenn Sie \"Ja\" anklicken, werden alle Module ausser dem\n"
						 "Kernel entfernt.\n"
						 "Sie muessen das in diesem Fall auf der Tastatur des Lesers\n"
						 "mit \"Ok\" bestaetigen.\n"
						 "\n"
						 "Moechten Sie alle Module entfernen?")) {

      int rv;

      rv=r->deleteAllModules(true);
      if (rv!=Cyberjack::ErrorCode_Ok) {
	FXMessageBox::error(this, MBOX_OK,
			    "Fehler",
			    "Beim Loeschen aller Module des Lesers ist ein Fehler aufgetreten.\n"
			    "(Fehlercode: %d)\n", rv);
      }
    }
  }
  return 1;
}



long FXCY_ReaderView::onCmdFlash(FXObject*, FXSelector, void *ptr) {
  Cyberjack::Reader *r=m_readerList->getCurrentReader();
  if (r) {
    FXCY_ReaderFlash dlg(r, this,
                         "Modul installieren",
                         DECOR_ALL);
    dlg.create();
    dlg.show();
    for (;;) {
      if (dlg.execute()) {
        FXString modulFileName;
        FXString signatureFileName;
        uint8_t buffer1[64*1024];
        uint8_t buffer2[64*1024];
        int len1;
        int len2;
        int rv;

        fprintf(stderr, "Accepted\n");
        modulFileName=dlg.getModuleFile();
        signatureFileName=dlg.getSignatureFile();

        len1=readFile(modulFileName.text(), buffer1, sizeof(buffer1));
        if (len1<1) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Lesefehler",
                              "Beim Einlesen der Moduldatei ist ein Fehler aufgetreten\n"
                              "Meldung vom System: %s\n",
                              strerror(errno));
          continue;
        }

        len2=readFile(signatureFileName.text(), buffer2, sizeof(buffer2));
        if (len2<1) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Lesefehler",
                              "Beim Einlesen der Signaturdatei ist ein Fehler aufgetreten\n"
                              "Meldung vom System: %s\n",
                              strerror(errno));
          continue;
        }

        rv=r->connect(Cyberjack::Object_Reader);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Leserfehler",
                              "Beim Ansprechen des Lesers ist ein Fehler aufgetreten\n"
                              "(Fehlercode: %d)\n", rv);
          continue;
        }

        /* got both module and signature, flash it */
        rv=r->sendModuleToFlash(buffer1, len1);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Fehler",
                              "Beim Senden des Modules an den Treiber ist ein Fehler aufgetreten.\n"
                              "(Fehlercode: %d)\n", rv);
          r->disconnect();
          continue;
        }

        rv=r->sendSignatureToFlash(buffer2, len2);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Fehler",
                              "Beim Senden der Signatur an den Treiber ist ein Fehler aufgetreten.\n"
                              "(Fehlercode: %d)\n", rv);
          r->disconnect();
          continue;
        }

        rv=r->flash();
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Fehler",
                              "Beim Hochladen des Modules ist ein Fehler aufgetreten.\n"
                              "(Fehlercode: %d)\n", rv);
          r->disconnect();
          continue;
        }

        rv=r->gatherInfo(false);
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Fehler",
                              "Beim Abrufen der Modulinformationen des Lesers ist ein Fehler aufgetreten.\n"
                              "(Fehlercode: %d)\n", rv);
          r->disconnect();
          continue;
        }

        rv=r->disconnect();
        if (rv!=Cyberjack::ErrorCode_Ok) {
          FXMessageBox::error(&dlg, MBOX_OK,
                              "Leserfehler",
                              "Beim Schliessen der Verbindung zum Lesers ist ein Fehler aufgetreten\n"
                              "(Fehlercode: %d)\n", rv);
          continue;
        }

        FXMessageBox::information(&dlg, MBOX_OK,
                                  "Update erfolgreich",
                                  "Das Modul wurde erfolgreich installiert.\n"
                                  "Eventuell muessen Sie nun den Leser kurz abziehen und\n"
                                  "erneut anschliessen.");
        break;
      }
      else {
        fprintf(stderr, "Rejected\n");
        break;
      }
    }
    dlg.hide();
  }
  return 1;
}






