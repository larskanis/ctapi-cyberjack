/***************************************************************************
    begin       : Thu Mar 26 2009
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


#include "fxcy_readerkeys.hpp"
#include "fxcy_app.hpp"




FXDEFMAP(FXCY_ReaderKeys) FXCY_ReaderKeysMap[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, FXCY_ReaderKeys::onCmdAccept),
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, FXCY_ReaderKeys::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderKeys::ID_BUTTON_MODUL, FXCY_ReaderKeys::onCmdButtonModule),
};


FXIMPLEMENT(FXCY_ReaderKeys, FXDialogBox, FXCY_ReaderKeysMap, ARRAYNUMBER(FXCY_ReaderKeysMap))




FXCY_ReaderKeys::FXCY_ReaderKeys(Cyberjack::Reader *r,
                                 FXWindow *owner,
                                 const FXString& name,
                                 FXuint opts,
                                 FXint x, FXint y, FXint w, FXint h)
:FXDialogBox(owner, name, opts, x, y, w, h)
,m_reader(r)
,m_editKeyFile(NULL)
{
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;
  FXMatrix *m;

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  m=new FXMatrix(vf, 3, MATRIX_BY_COLUMNS | LAYOUT_FILL_X | LAYOUT_FILL_Y);

  new FXLabel(m, "Schluesseldatei", NULL, LABEL_NORMAL);
  m_editKeyFile=new FXTextField(m, 40, this, ID_EDIT_MODULE, TEXTFIELD_NORMAL);
  new FXButton(m, "...", NULL, this, ID_BUTTON_MODUL, BUTTON_NORMAL);

  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
  new FXButton(hf, "Abbrechen", NULL, this, ID_CANCEL,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
  new FXButton(hf, "Ausfuehren", NULL, this, ID_ACCEPT,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
}



FXCY_ReaderKeys::FXCY_ReaderKeys()
:FXDialogBox()
,m_reader(NULL)
,m_editKeyFile(NULL)
{
}



FXCY_ReaderKeys::~FXCY_ReaderKeys() {
}



void FXCY_ReaderKeys::create() {
  FXDialogBox::create();
}



long FXCY_ReaderKeys::onCmdAccept(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,TRUE);
  return 1;
}



long FXCY_ReaderKeys::onCmdReject(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,FALSE);
  return 1;
}



long FXCY_ReaderKeys::onCmdButtonModule(FXObject*, FXSelector, void *ptr) {
  FXString path=m_editKeyFile->getText();
  if (path.empty())
    path=m_lastPath;
  FXString s=FXFileDialog::getOpenFilename(this, "Schluesseldatei auswaehlen",
                                           path,
                                           "Schluesseldateien (*.bky)\nAlle Dateien (*)");
  if (!s.empty()) {
    int lpos=s.rfind('/');
    if (lpos!=-1)
      m_lastPath=s.left(lpos+1);
    m_editKeyFile->setText(s);
  }
  return 1;
}




FXString FXCY_ReaderKeys::getKeyFile() const {
  return m_editKeyFile->getText();
}






