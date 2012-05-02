

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <fx.h>

#include "fxcy_app.hpp"
#include "fxcy_readerview.hpp"
#include "libcyberjack/driver_ctapi.hpp"
#include "fxcy_mainwindow.hpp"

#include "Platform.h"
#include "Reader.h"
#include "eca_defines.h"


int test1(int argc, char **argv) {
  FXApp a("libtest", "Martin Preuss");
  FXDialogBox *dbox;
  FXVerticalFrame *vf;
  FXCY_ReaderView *rview;
  Cyberjack::Driver *dr;
  int rv;

  a.init(argc,argv);
  a.create();

  dbox=new FXDialogBox(&a, "Test", DECOR_ALL);
  vf=new FXVerticalFrame(dbox, LAYOUT_FILL_X | LAYOUT_FILL_Y,
                         0, 0, 0, 0, 1, 1, 1, 1);

  rview=new FXCY_ReaderView(vf, LAYOUT_FILL_X | LAYOUT_FILL_Y);

  dbox->create();

  dr=new Cyberjack::DriverCtapi();

  rv=dr->open();
  if (rv<0) {
    fprintf(stderr, "Error in open: %d\n", rv);
    return 2;
  }
  rv=dr->enumReaders();
  if (rv<0) {
    fprintf(stderr, "Error in enumReaders: %d\n", rv);
    return 2;
  }
  rview->applyReaderList(dr->getUpdateCounter(), dr->getReaders());
  rv=dr->close();
  if (rv<0) {
    fprintf(stderr, "Error in close: %d\n", rv);
    return 2;
  }


  dbox->resize(500, 300);
  dbox->show();
  a.runModalFor(dbox);

  return 0;
}



int main(int argc, char **argv) {
  FXCY_App a("libtest", "Martin Preuss");
  FXCY_MainWindow *mw;
  int rv;

  a.init(argc,argv);
  a.create();

  Debug.setLevelMask(0xffffffff);

  mw=new FXCY_MainWindow(&a, "FXcyberJack");
  mw->create();

  rv=mw->openDriverPcsc();
  if (rv<0) {
    fprintf(stderr, "No PC/SC, trying CTAPI\n");
    rv=mw->openDriverCtapi();
    if (rv<0) {
      fprintf(stderr, "Error in open: %d\n", rv);
      return 2;
    }
  }

  rv=mw->updateReaderList(true);
  if (rv<0) {
    fprintf(stderr, "Error in enumReaders: %d\n", rv);
    return 2;
  }

  mw->resize(500, 300);
  mw->show();
  a.runModalFor(mw);

#if 0
  fprintf(stderr, "Closing driver\n");
  rv=mw->closeDriver();
  if (rv<0) {
    fprintf(stderr, "Error in close: %d\n", rv);
    return 2;
  }
#endif

  fprintf(stderr, "Bye.\n");
  return 0;
}

