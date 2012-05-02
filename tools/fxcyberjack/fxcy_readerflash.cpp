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


#include "fxcy_readerflash.hpp"
#include "fxcy_app.hpp"




FXDEFMAP(FXCY_ReaderFlash) FXCY_ReaderFlashMap[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, FXCY_ReaderFlash::onCmdAccept),
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, FXCY_ReaderFlash::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderFlash::ID_BUTTON_MODUL, FXCY_ReaderFlash::onCmdButtonModule),
  FXMAPFUNC(SEL_COMMAND, FXCY_ReaderFlash::ID_BUTTON_SIGNATURE, FXCY_ReaderFlash::onCmdButtonSignature),
};


FXIMPLEMENT(FXCY_ReaderFlash, FXDialogBox, FXCY_ReaderFlashMap, ARRAYNUMBER(FXCY_ReaderFlashMap))




FXCY_ReaderFlash::FXCY_ReaderFlash(Cyberjack::Reader *r,
                                   FXWindow *owner,
                                   const FXString& name,
                                   FXuint opts,
                                   FXint x, FXint y, FXint w, FXint h)
:FXDialogBox(owner, name, opts, x, y, w, h)
,m_reader(r)
,m_editModuleFile(NULL)
,m_editSignatureFile(NULL)
{
  FXVerticalFrame *vf;
  FXHorizontalFrame *hf;
  FXMatrix *m;

  vf=new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  m=new FXMatrix(vf, 3, MATRIX_BY_COLUMNS | LAYOUT_FILL_X | LAYOUT_FILL_Y);

  new FXLabel(m, "Moduldatei", NULL, LABEL_NORMAL);
  m_editModuleFile=new FXTextField(m, 40, this, ID_EDIT_MODULE, TEXTFIELD_NORMAL);
  new FXButton(m, "...", NULL, this, ID_BUTTON_MODUL, BUTTON_NORMAL);

  new FXLabel(m, "Signaturdatei", NULL, LABEL_NORMAL);
  m_editSignatureFile=new FXTextField(m, 40, this, ID_EDIT_SIGNATURE, TEXTFIELD_NORMAL);
  new FXButton(m, "...", NULL, this, ID_BUTTON_SIGNATURE, BUTTON_NORMAL);

  new FXHorizontalSeparator(vf, LAYOUT_FILL_X | SEPARATOR_GROOVE);
  hf=new FXHorizontalFrame(vf, LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
  new FXButton(hf, "Abbrechen", NULL, this, ID_CANCEL,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
  new FXButton(hf, "Ausfuehren", NULL, this, ID_ACCEPT,
               BUTTON_NORMAL | BUTTON_DEFAULT | BUTTON_INITIAL | LAYOUT_RIGHT);
}



FXCY_ReaderFlash::FXCY_ReaderFlash()
:FXDialogBox()
,m_reader(NULL)
,m_editModuleFile(NULL)
,m_editSignatureFile(NULL)
{
}



FXCY_ReaderFlash::~FXCY_ReaderFlash() {
}



void FXCY_ReaderFlash::create() {
  FXDialogBox::create();
}



long FXCY_ReaderFlash::onCmdAccept(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,TRUE);
  return 1;
}



long FXCY_ReaderFlash::onCmdReject(FXObject*, FXSelector, void *ptr) {
  getApp()->stopModal(this,FALSE);
  return 1;
}



long FXCY_ReaderFlash::onCmdButtonModule(FXObject*, FXSelector, void *ptr) {
  FXString path=m_editModuleFile->getText();
  if (path.empty())
    path=m_lastPath;
  FXString s=FXFileDialog::getOpenFilename(this, "Moduldatei auswaehlen",
                                           path,
                                           "Moduldateien (*.bin)\nAlle Dateien (*)");
  if (!s.empty()) {
    int lpos=s.rfind('/');
    if (lpos!=-1)
      m_lastPath=s.left(lpos+1);

    if (s.right(4).lower()==".bin")
      m_editSignatureFile->setText(s+".ecoma.sgn");
    m_editModuleFile->setText(s);
  }
  return 1;
}



long FXCY_ReaderFlash::onCmdButtonSignature(FXObject*, FXSelector, void *ptr) {
  FXString path=m_editSignatureFile->getText();
  if (path.empty())
    path=m_lastPath;
  FXString s=FXFileDialog::getOpenFilename(this, "Signaturdatei auswaehlen",
                                           path,
                                           "Signaturdateien (*.sgn)\nAlle Dateien (*)");
  if (!s.empty()) {
    int lpos=s.rfind('/');
    if (lpos!=-1)
      m_lastPath=s.left(lpos+1);
    m_editSignatureFile->setText(s);
  }
  return 1;
}



FXString FXCY_ReaderFlash::getModuleFile() const {
  return m_editModuleFile->getText();
}



FXString FXCY_ReaderFlash::getSignatureFile() const {
  return m_editSignatureFile->getText();
}




