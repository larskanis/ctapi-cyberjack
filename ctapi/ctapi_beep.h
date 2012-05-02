#ifndef _CJCTAPI_BEEP_H
#define _CJCTAPI_BEEP_H


struct beep_struct;


struct beep_struct *beep_init(void);
void beep_whatever(struct beep_struct *bs);
void beep_fini(struct beep_struct *bs);


#endif
