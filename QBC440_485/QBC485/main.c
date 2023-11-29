#include  "config.h" 


unsigned char testff;
unsigned char tBuff[]={0x01,0x03,0x00,0x00,0x00,0x06} ;
uint8 CRC_lowdata,CRC_highdata;

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
int main()
{  uint16 i;
/*uint16 tBuff1[3];


tBuff1[0]=0x1122;
tBuff1[1]=0x3344;
tBuff1[2]=0x5566;
testFunc(tBuff1);*/	

TxEN_Init();
Time0Init();
Timer1_Init();
//UART0_Ini();
UART1_Ini();
//UART2_Ini();
//UART3_Ini();
IOinit();

IniWDT();	//初始化看门狗

while(1)
{ 	FeedWDT(); //喂狗
    refreshWriteBuff();
	checkWriteCMD();
    if(mainDelay_1mS_Count>9)
	 { 
		mainDelay_1mS_Count = 0;
		T1_CheckSendHold();

		T1_check();

	 }
    CheckIOpin();
	//UART0_SendData();
	UART1_SendData();
	//UART2_SendData();
	//T0_recCheck();//询问三台开关数据
	T1_recCheck();// 上传三台数据信息到jos
	//T2_recCheck();// jos控制信息
	//T3_recCheck();//联力开关主站通讯
} 

}
