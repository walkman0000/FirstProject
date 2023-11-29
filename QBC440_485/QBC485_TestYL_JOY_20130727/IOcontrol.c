#include "config.h"

#define AFC_baseRegAddr	 40128
#define Shearer_baseRegAddr	 40129
#define JCQ1 (1<<14)
#define JCQ3 (1<<15)
#define GZ2 (1<<24)
#define GZ1 (1<<25)
#define JCQ2 (1<<28)
#define JCQ4 (1<<29)
#define GZ3 (1<<30)
#define GZ4 (1<<31)
#define Allpin 	 (JCQ1|JCQ3|GZ2|GZ1|JCQ2|JCQ4|GZ3|GZ4)
uint8 NeedChangeCMD,NeedChangeCMD1;
uint16 NeedChangeCMDValue;
void IOinit()
{  //p1.18-p1.25
   PINSEL3  = PINSEL3 & ~(0xFFFF << 4) ;
   IODIR1 =  IODIR1   | (0xFF << 18);
   PINSEL8  = PINSEL8 & ~(0x0F<<28) ;//p4.14,p4.15
   FIO4DIR =  FIO4DIR   & ~(0x03 << 14);
   //PINMODE8 = PINMODE8 |(0x0F<<28) ;
   PINMODE8 = 0xFFFFFFFF;
   PINSEL9  = PINSEL9 & ~(0x0F<<16) ;//p4.24,p4.25
   FIO4DIR =  FIO4DIR   & ~(0x03 << 24);
   PINSEL9  = PINSEL9 & ~(0xFF<<28) ;//p4.28-p4.31
   FIO4DIR =  FIO4DIR   & ~(0x0F << 28);//
   PINMODE9 = 0xFFFFFFFF;
   //PINMODE9 = 0xAA0A0000;
   
}
void checkWriteCMD()
{uint8 i;
 for(i=0;i<8;i++)
 {
  if(QBCWriteBuff[1+i] != 1)QBCWriteBuff[1+i]=0;
 }
 //if(fQBCWriteBuff[1]!=QBCWriteBuff[1])
 KS1_DelayOn(QBCWriteBuff[1]);
 KS2_DelayOn(QBCWriteBuff[2]);
 KS3_DelayOn(QBCWriteBuff[3]);
 KS4_DelayOn(QBCWriteBuff[4]);
 KS5_DelayOn(QBCWriteBuff[5]);
 KS6_DelayOn(QBCWriteBuff[6]);
 KS7_DelayOn(QBCWriteBuff[7]);
 KS8_DelayOn(QBCWriteBuff[8]);
}
void KS1_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<19	;
  else IOPIN1=(IOSET1&(~(1<<19)))|(0x01 << 19);//IOSET1 |= 1<<19	; 
}
void KS2_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<18	;
  else IOPIN1=(IOSET1&(~(1<<18)))|(0x01 << 18);//IOSET1 |= 1<<18	; 
}
void KS3_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<21	;
  else IOPIN1=(IOSET1&(~(1<<21)))|(0x01 << 21);//IOSET1 |= 1<<21	; 
}
void KS4_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<20	;
  else IOPIN1=(IOSET1&(~(1<<20)))|(0x01 << 20);//IOSET1 |= 1<<20	; 
}
void KS5_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<23	;
  else IOPIN1=(IOSET1&(~(1<<23)))|(0x01 << 23);//IOSET1 |= 1<<23	; 
}
void KS6_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<22	;
  else IOPIN1=(IOSET1&(~(1<<22)))|(0x01 << 22);//IOSET1 |= 1<<22	; 
}
void KS7_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<25	;
  else IOPIN1=(IOSET1&(~(1<<25)))|(0x01 << 25);//IOSET1 |= 1<<25	; 
}
void KS8_DelayOn(int flag)
{ if(flag==0)IOCLR1 |= 1<<24	;
  else IOPIN1=(IOSET1&(~(1<<24)))|(0x01 << 24);//IOSET1 |= 1<<24	; 
}
void CheckIOpin()
{ uint32 tmp;
tmp=  FIO4PIN;
//(JCQ1|JCQ3|GZ2|GZ1|JCQ2|JCQ4|GZ3|GZ4)
 if((tmp&JCQ1)==0)
 {
  
  QBCReadBuff[1] =1;
  NeedChangeCMD1 = 7;
 }
 else 
 {
  if(QBCReadBuff[1] == 1)
  {
   NeedChangeCMD = 1;
   NeedChangeCMDValue = 0;
  }
  QBCReadBuff[1] =0; 
  NeedChangeCMD1 = 0;
 }
  if((tmp&JCQ2)==0)QBCReadBuff[2] =1;
 else QBCReadBuff[2] =0;
  if((tmp&JCQ3)==0)QBCReadBuff[3] =1;
 else QBCReadBuff[3] =0;
  if((tmp&JCQ4)==0)QBCReadBuff[4] =1;
 else QBCReadBuff[4] =0;
  if((tmp&GZ1)==0)QBCReadBuff[5] =1;
 else QBCReadBuff[5] =0;
   if((tmp&GZ2)==0)QBCReadBuff[6] =1;
 else QBCReadBuff[6] =0;
   if((tmp&GZ3)==0)QBCReadBuff[7] =1;
 else QBCReadBuff[7] =0;
   if((tmp&GZ4)==0)QBCReadBuff[8] =1;
 else QBCReadBuff[8] =0;

 KS1_DelayOn(QBCReadBuff[1]);
 KS2_DelayOn(QBCReadBuff[2]);
 KS3_DelayOn(QBCReadBuff[3]);
}

void refreshWriteBuff()
{ uint8 i;
  if(uart1NormalFlag == 0)
  {
  	for(i=0;i<9;i++)
	{
	 QBCWriteBuff[i] = 0;
	}
  }
}

