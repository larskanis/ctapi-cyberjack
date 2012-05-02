#ifndef _CJPPA_LINUX_H
#define _CJPPA_LINUX_H

#include "ausb/ausb_l.h"

#define CJ_LINUX_DEBUGLOG	"/tmp/cj.log"

#define USB_TIMEOUT		60000
#define USB_READ_TIMEOUT	USB_TIMEOUT
#define USB_WRITE_TIMEOUT	5000

int cjppLinux_SetInterruptEventNotificationProc(cjccidHANDLE cjccid, 
						ausb_dev_handle *ah);

#endif /* _CJPPA_LINUX_H */
