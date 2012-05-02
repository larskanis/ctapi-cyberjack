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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "fxcy_readerflash2.hpp"
#include "fxcy_app.hpp"




FXDEFMAP(FXCY_ReaderFlash2) FXCY_ReaderFlash2Map[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, FXCY_ReaderFlash2::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderFlash2::ID_BUTTON_FLASH, FXCY_ReaderFlash2::onCmdButtonFlash),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderFlash2::ID_LISTBOX, FXCY_ReaderFlash2::onCmdListBox),
};


FXIMPLEMENT(FXCY_ReaderFlash2, FXDialogBox, FXCY_ReaderFlash2Map, ARRAYNUMBER(FXCY_ReaderFlash2Map))




FXCY_ReaderFlash2::FXCY_ReaderFlash2(Cyberjack::Reader *r,
				     FXWindow *owner,
				     const FXString& name,
				     FXuint opts,
				     FXint x, FXint y, FXint w, FXint h)
:FXDialogBox(owner, name, opts, x, y, w, h)
,m_reader(r)
,m_moduleStore(NULL)
,m_listBox(NULL)
,m_moduleList(NULL)
{
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X);
  new FXLabel(hf, "Modul-Auswahl", NULL, LABEL_NORMAL);
  m_listBox=new FXListBox(hf, this, ID_LISTBOX,
			  LAYOUT_FILL_COLUMN |
			  FRAME_SUNKEN | FRAME_THICK |
			  LISTBOX_NORMAL);

  m_moduleList=new FXFoldingList(vf, this, ID_MODULE_LIST,
				 FOLDINGLIST_NORMAL |
                                 FOLDINGLIST_BROWSESELECT |
				 LAYOUT_FILL_X |
				 LAYOUT_FILL_Y);
  m_moduleList->appendHeader("Name", NULL, 100);
  m_moduleList->appendHeader("Beschreibung", NULL, 320);

  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
  new FXButton(hf, "Abbrechen", NULL, this, ID_CANCEL,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
  m_flashButton=new FXButton(hf, "Ausfuehren", NULL, this, ID_BUTTON_FLASH,
			     BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
}



FXint FXCY_ReaderFlash2::getDefaultWidth() {
  return 480;
}



FXint FXCY_ReaderFlash2::getDefaultHeight() {
  return FXDialogBox::getDefaultHeight()+100;
}



FXCY_ReaderFlash2::FXCY_ReaderFlash2()
:FXDialogBox()
,m_reader(NULL)
,m_moduleStore(NULL)
,m_listBox(NULL)
,m_moduleList(NULL)
{
}



FXCY_ReaderFlash2::~FXCY_ReaderFlash2() {
}



long FXCY_ReaderFlash2::onCmdReject(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,FALSE);
  return 1;
}



Cyberjack::Module *FXCY_ReaderFlash2::findModuleByName(const char *s) {
  std::list<Cyberjack::Module> &ml=m_moduleStore->getModules();
  std::list<Cyberjack::Module>::iterator it;

  /* add entries to combo box */
  for (it=ml.begin(); it!=ml.end(); it++) {
    if (strcasecmp(it->getModuleName().c_str(), s)==0)
      return &(*it);
  }

  return NULL;
}



long FXCY_ReaderFlash2::onCmdListBox(FXObject*, FXSelector, void *ptr) {
  FXint i=(FXint)(FXival)ptr;

  selectedTemplate(i);
  return 1;
}



void FXCY_ReaderFlash2::selectedTemplate(int idx) {
  Cyberjack::MTemplate *tmpl;
  int nv;

  m_moduleList->clearItems();
  tmpl=(Cyberjack::MTemplate*) m_listBox->getItemData(idx);
  if (m_moduleStore && tmpl) {
    const std::list<std::string> &modNames=tmpl->getModuleNames();

    if (!modNames.empty()) {
      std::list<std::string>::const_iterator it;

      for (it=modNames.begin(); it!=modNames.end(); it++) {
	Cyberjack::Module *m=findModuleByName(it->c_str());
	if (m) {
	  FXString str;

	  str=FXString(m->getModuleName().c_str());
	  str+="\t";
          str+=FXString(m->getDescr().c_str());
	  FXFoldingItem *item=new FXFoldingItem(str);
	  m_moduleList->appendItem(NULL, item, FALSE);
	}
      }
    }
  }

  nv=m_listBox->getNumItems();
  if (nv>10)
    nv=10;
  m_listBox->setNumVisible(nv);

  if (m_moduleList->getNumItems())
    m_flashButton->enable();
  else
    m_flashButton->disable();
}



long FXCY_ReaderFlash2::onCmdButtonFlash(FXObject*, FXSelector, void *ptr) {
  Cyberjack::MTemplate *tmpl;

  tmpl=(Cyberjack::MTemplate*) m_listBox->getItemData(m_listBox->getCurrentItem());
  if (m_moduleStore && tmpl) {
    const std::list<std::string> &modNames=tmpl->getModuleNames();

    if (!modNames.empty()) {
      std::list<std::string>::const_iterator it;
      int rv;

      /* read keyfile (not really necessary, may we should omit that) */
      if (!m_moduleStore->readKeyFile()) {
	FXMessageBox::error(this, MBOX_OK,
			    "Fehler",
			    "Die Schluesseldatei \"%s\" konnte nicht geladen werden.\n",
			    m_moduleStore->getKeyFileName().c_str());
	return 1;
      }

      /* read binaries */
      for (it=modNames.begin(); it!=modNames.end(); it++) {
	Cyberjack::Module *m=findModuleByName(it->c_str());
	if (m) {
	  if (!m->readModuleBinaries()) {
	    FXMessageBox::error(this, MBOX_OK,
				"Fehler",
				"Die Firmware-Dateien fuer das Modul \"%s\" konnten nicht geladen werden.\n",
				it->c_str());
	    return 1;
	  }
	}
      }

      if (MBOX_CLICKED_YES==FXMessageBox::question(this, MBOX_YES_NO,
						   "Firmware aktualisieren",
						   "Diese Aktion entfernt alle Module vom Leser \n"
						   "und fuegt die ausgewaehlten Module neu hinzu.\n"
						   "\n"
						   "Wenn Sie \"Ja\" anklicken wird die Firmware auf dem\n"
						   "Leser aktualisiert.\n"
						   "Sie muessen das in diesem Fall auf der Tastatur des Lesers\n"
						   "mit \"Ok\" bestaetigen.\n"
						   "Sie werden fuer jedes zu aktualisierende Modul auf dem\n"
						   "Display des Leser um Bestaetigung gebeten.\n"
						   "\n"
						   "Moechten Sie die Aktualisierung durchfuehren?")) {
	/* ok, now we have all needed binaries */
	rv=m_reader->connect(Cyberjack::Object_Reader);
	if (rv!=Cyberjack::ErrorCode_Ok) {
	  FXMessageBox::error(this, MBOX_OK,
			      "Leserfehler",
			      "Beim Ansprechen des Lesers ist ein Fehler aufgetreten\n"
			      "(Fehlercode: %d)\n", rv);
	  return 1;
	}
  
	/* delete all modules */
	rv=m_reader->deleteAllModules(false);
	if (rv!=Cyberjack::ErrorCode_Ok) {
	  FXMessageBox::error(this, MBOX_OK,
			      "Fehler",
			      "Beim Loeschen aller Module des Lesers ist ein Fehler aufgetreten.\n"
			      "(Fehlercode: %d)\n", rv);
	  m_reader->disconnect();
	  return 1;
	}
  
	/* flash the given modules */
	for (it=modNames.begin(); it!=modNames.end(); it++) {
	  Cyberjack::Module *m=findModuleByName(it->c_str());
	  if (m) {
	    std::string data;
  
	    /* send module */
	    data=m->getBinFileData();
	    fprintf(stderr, "Found binfile with %d bytes length\n", int(data.length()));
	    rv=m_reader->sendModuleToFlash((const uint8_t*) (data.data()), data.length());
	    if (rv!=Cyberjack::ErrorCode_Ok) {
	      FXMessageBox::error(this, MBOX_OK,
				  "Fehler",
				  "Beim Senden des Modules [%s] an den Treiber ist ein Fehler aufgetreten.\n"
				  "(Fehlercode: %d)\n",
				  it->c_str(),
				  rv);
	      m_reader->disconnect();
	      return 1;
	    }
  
	    /* send signature */
	    data=m->getSigFileData();
	    fprintf(stderr, "Found sigfile with %d bytes length\n", int(data.length()));
	    rv=m_reader->sendSignatureToFlash((const uint8_t*) (data.data()), data.length());
	    if (rv!=Cyberjack::ErrorCode_Ok) {
	      FXMessageBox::error(this, MBOX_OK,
				  "Fehler",
				  "Beim Senden der Signatur des Modules [%s] an den Treiber ist ein Fehler "
				  "aufgetreten.\n"
				  "(Fehlercode: %d)\n",
				  it->c_str(),
				  rv);
	      m_reader->disconnect();
	      return 1;
	    }
  
	    /* now flash */
	    rv=m_reader->flash();
	    if (rv!=Cyberjack::ErrorCode_Ok) {
	      FXMessageBox::error(this, MBOX_OK,
				  "Fehler",
				  "Beim Hochladen des Modules ist ein Fehler aufgetreten.\n"
				  "(Fehlercode: %d)\n", rv);
	      m_reader->disconnect();
	      return 1;
	    }
	  }
	}
  
	/* disconnect */
	rv=m_reader->disconnect();
	if (rv!=Cyberjack::ErrorCode_Ok) {
	  FXMessageBox::error(this, MBOX_OK,
			      "Leserfehler",
			      "Beim Schliessen der Verbindung zum Lesers ist ein Fehler aufgetreten\n"
			      "(Fehlercode: %d)\n", rv);
	  return 1;
	}
  
	FXMessageBox::information(this, MBOX_OK,
				  "Update erfolgreich",
				  "Die Module wurden erfolgreich installiert.\n"
				  "Eventuell muessen Sie nun den Leser kurz abziehen und\n"
				  "erneut anschliessen.");
      }
      getApp()->stopModal(this,TRUE);
    }
  }

  return 1;
}



bool FXCY_ReaderFlash2::selectModuleStore() {
  /* get reader type */
  const cj_ReaderInfo &ri=m_reader->getReaderInfo();
  FXString hwstring=FXString((const char*)ri.HardwareString).trim().lower();
  std::list<Cyberjack::ModuleStore>::iterator it;

  m_moduleStoreList.clear();
  m_moduleStore=NULL;

  if (!Cyberjack::ModuleStore::readModuleStores(m_moduleStoreList)) {
    FXMessageBox::error(this, MBOX_OK,
			"Fehler",
                        "%s",
			"Es konnten keine Firmware-Dateien auf Ihrem System gefunden werden.\n"
			"Bitte laden Sie die entsprechenden Dateien von\n"
			"    http://www.reiner-sct.de/\n"
			"herunter und starten Sie diese Anwendung erneut.");
    return false;
  }

  for (it=m_moduleStoreList.begin(); it!=m_moduleStoreList.end(); it++) {
    FXString fs;

    fs=FXString(it->getHwString().c_str()).lower();
    if (!fs.empty()) {
      if (strstr(fs.text(), hwstring.text())!=NULL) {
	m_moduleStore=&(*it);
	return true;
      }
    }
  }

  return false;
}



void FXCY_ReaderFlash2::toGui() {
  if (selectModuleStore()) {
    std::list<Cyberjack::MTemplate> &tl=m_moduleStore->getTemplates();
    std::list<Cyberjack::MTemplate>::iterator it;

    /* add entries to combo box */
    for (it=tl.begin(); it!=tl.end(); it++) {
      FXString str=FXString(it->getDescr().c_str()).trim();
      Cyberjack::MTemplate &tmpl=*it;

      if (!str.empty())
	m_listBox->appendItem(str, NULL, &tmpl);
    }
  }
  if (m_listBox->getNumItems()) {
    m_listBox->setCurrentItem(0, FALSE);
    selectedTemplate(0);
  }
}


