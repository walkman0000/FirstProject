#ifndef  IOcontrol_H
#define  IOcontrol_H

extern void  IOinit(void);
extern void KS1_DelayOn(int flag);
extern void KS2_DelayOn(int flag);
extern void KS3_DelayOn(int flag);
extern void KS4_DelayOn(int flag);
extern void KS5_DelayOn(int flag);
extern void KS6_DelayOn(int flag);
extern void KS7_DelayOn(int flag);
extern void KS8_DelayOn(int flag);
extern void CheckIOpin();
extern void checkWriteCMD();
extern void refreshWriteBuff();
#endif