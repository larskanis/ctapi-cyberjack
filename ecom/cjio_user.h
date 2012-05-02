#ifndef _CJIO_USER_H
#define _CJIO_USER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int cjIoOpen_libusb(CJ_INFO *ci, int reset_before);
int cjIoClose_libusb(CJ_INFO *ci);
int cjIoSendBlock_libusb(CJ_INFO *ci, BYTE *data, int datalen);
int cjIoReceiveBlock_libusb(CJ_INFO *ci, BYTE *data, int *datalen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
