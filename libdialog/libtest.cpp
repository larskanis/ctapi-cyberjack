

#include "dialog.h"

#include <stdio.h>
#include <unistd.h>



int main(int argc, char **argv) {
  RSCT_DIALOG *dlg;
  int rv;
  int stage;

  dlg=rsct_dialog_new("Sichere PIN-Eingabe",
		      3,
		      0x20, 0x18,
		      15,
		      "Bitte geben Sie Ihre bisherige PIN ein",
		      "Bitte geben Sie die neue PIN ein",
		      "Bitte wiederholen Sie die neue PIN");

  rv=rsct_dialog_open(dlg);
  if (rv<0) {
    fprintf(stderr, "rsct_dialog_open: %d\n", rv);
    return 2;
  }


  for(stage=0; stage<3; stage++) {
    int i;

    rv=rsct_dialog_set_stage(dlg, stage);
    if (rv<0) {
      fprintf(stderr, "rsct_dialog_set_stage(%d): %d\n", stage, rv);
      return 2;
    }

    for (i=1; i<6; i++) {
      sleep(3);
      rv=rsct_dialog_set_char_num(dlg, i, 1);
      if (rv<0) {
	fprintf(stderr, "rsct_dialog_set_char_num(%d): %d\n", i, rv);
	return 2;
      }
    }
  }

  rsct_dialog_close(dlg);

  return 0;
}


