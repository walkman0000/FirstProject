#include  "config.h" 

#define MacroON 3
unsigned char testff;
unsigned char tBuff[]={0x01,0x03,0x00,0x00,0x00,0x06} ;
uint8 CRC_lowdata,CRC_highdata;
uint16 KS_ONFlagValue,KS_ONFlag;
void TxEN_Init(void)
{ //T0	 P0.9
//PINSEL0  = PINSEL0 & ~(0x03 << 18) ;
//IODIR0 =  IODIR0   | (0x01 << 9);
//IOCLR0 |= 1<<9	;
//Tx0_EN(0);
//T1 P1.12
PINSEL2  = PINSEL2 & ~(0x03 << 24) ;
IODIR1 =  IODIR1   | (0x01 << 12);
IOCLR1 |= 1<<12	;
Tx1_EN(0);
////T2  P0.0
//PINSEL0  = PINSEL0 & ~(0x03) ;
//IODIR0 =  IODIR0   | (0x01);
//IOCLR0 |= 1	;
//Tx2_EN(0);
////T3  P2.22
//PINSEL5  = PINSEL5 & ~(0x03<<12) ;
//FIO2DIR =  FIO2DIR   | (0x01 << 22);
//FIO2CLR |= 1<<22	;
//Tx3_EN(0);
}
/*testFunc(uint16 *pJosBuff)
{ uint8 tBuff2[2];
 tBuff2[0]=(*(pJosBuff+(3>>1)+0))>>8 ;
 tBuff2[1]=(*(pJosBuff+(4>>1)+0));
}*/
void IniWDT()
{
 WDCLKSEL=0; //选择内部4MRC振荡源，经4分频后得1M
WDTC=0xff000;//定时大约为1s //;0x400000
WDMOD=0x03;
}
void FeedWDT()
{
 WDFEED=0x55;
 WDFEED=0xAA;
}
void delay(uint32 kk)
{ uint32 i,j;
  for(i=0;i<1000;i++)
  {
  	 for(j=0;j<kk;j++)
	 {
	  ;
	 }
  }
}
int main()
{  uint16 i;
/*uint16 tBuff1[3];


tBuff1[0]=0x1122;
tBuff1[1]=0x3344;
tBuff1[2]=0x5566;
testFunc(tBuff1);*/	
delay(1000);
TxEN_Init();
Time0Init();
Timer1_Init();
//UART0_Ini();
UART1_Ini();
//UART2_Ini();
//UART3_Ini();
IOinit();

//IniWDT();	//初始化看门狗
KS_ONFlag = 1;
while(1)
{ 	//FeedWDT(); //喂狗
		UART1_SendData();
	  T1_recCheck();// 上传三台数据信息到jos
    if(mainDelay_1mS_Count>9)
	 { 
		mainDelay_1mS_Count = 0;
		if(mainDelay_1S_Count<1000)mainDelay_1S_Count++;
		T1_CheckSendHold();

		T1_check();
	 }
	 if(mainDelay_1S_Count>=1000)
	 {
	   mainDelay_1S_Count = 0;
		 //T1_StepSendFlag=1;
	   if(KS_ONFlag ==1){if(KS_ONFlagValue<100)  KS_ONFlagValue++;	}
	 }
	 if(KS_ONFlagValue>(MacroON*11))
	 {
	  KS_ONFlagValue = 0;
	  KS_ONFlag = 1;
	 }else if(KS_ONFlagValue>(MacroON*9))
	 {	 //检查返回
	   //KS_ONFlagValue = 0;
	   //KS_ONFlag = 0;
	   if(QBCReadBuff[1] ==1)
	   {
	   	KS5_DelayOn(1);
	   }
	   
	 }
	 else if(KS_ONFlagValue>(MacroON*6))
	 {
	   KS1_DelayOn(0);
	   KS2_DelayOn(0);
	   KS3_DelayOn(0);
	   KS4_DelayOn(0);
	 }
	 else if(KS_ONFlagValue>(MacroON*3))KS4_DelayOn(1);
	 else if(KS_ONFlagValue>(MacroON*2))KS3_DelayOn(1);
	 else if(KS_ONFlagValue>(MacroON*1)) KS2_DelayOn(1);
	 else if(KS_ONFlagValue>0)KS1_DelayOn(1);
    CheckIOpin();
	
} 

}
