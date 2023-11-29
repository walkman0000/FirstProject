#include  "config.h" 
#define	UART_BPS	9600	

unsigned char testff;
unsigned char tBuff[]={0x01,0x03,0x00,0x00,0x00,0x06} ;
uint8 CRC_lowdata,CRC_highdata,ADC_Send_Enable,canlighton;
uint32 addr0_data=0,ADC_Data,light_on2;
void IOInit()
{
  PINSEL7 &=(~(0x03<<16));
  FIO3DIR |= (1<<24);		//P3.24为BEEP输出，P3.23为急停输入
  //FIO3PIN |=(FIO3SET&(~(1<<24)))|(0x01 << 24);
  FIO3SET |= (0x01 << 24);
  //FIO3PIN |= 0xffffffff;  
}
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
  /*********************************************************************************************************
** 函数名称 ：IRQ_AD
** 函数功能 ：AD中断服务程序 
** 入口参数 ：无
** 出口参数 ：无
**********************************************************************************************************/
void __irq IRQ_ADC(void)
{
			
	addr0_data 		= (uint32)AD0DR0;				                    /* 读取ADC数据                   */
	ADC_Send_Enable = 1; 					                            /* 使能串口发送ADC0数据的控制位	 */	
	  ADC_Data = (addr0_data >> 6) & 0x3ff;
			ADC_Data = ADC_Data * 2500;				                    /* 参考电压由精密恒压源提供的2.5V*/
			ADC_Data = ADC_Data / 1024;
	VICVectAddr 	= 0x00;					                            /* 通知VIC中断处理结束           */
	//AD0CR |= (1<<24);
}
/**********************************************************************************************************
** 函数名称 ：UART0_SendByte
** 函数功能 ：向串口发送字节数据，并等待数据发送完毕。
** 入口参数 ：data		要发送的数据
** 出口参数 ：无
**********************************************************************************************************/
void UART0_SendByte(uint8 data)
{
	U0THR = data;
	
	while ((U0LSR & 0x40) == 0);			                            /* 等待数据发送完毕              */
}
void PC_DispChar(uint8 x, uint8 y, uint8 chr, uint8 color)
{
	UART0_SendByte(0xff);			                                    /* 起始字符                     */
	UART0_SendByte(x);
	UART0_SendByte(y);
	UART0_SendByte(chr);
	UART0_SendByte(color);
}
/**********************************************************************************************************
** 函数名称 ：UART0_IniDft
** 函数功能 ：按默认值初始化串口0的引脚和通讯参数。设置为8位数据位，1位停止位，无奇偶校验，波特率115200。
** 入口参数 ：
** 出口参数 ：无
***********************************************************************************************************/
void UART0_IniDft(void)
{
	uint32 	Fdiv = 0;

	/* 
	 *  初始化相关的IO 
	 */
	PINSEL0 = (0x01 << 4) | (0x01 << 6);	                            /* 设置P0.2和P0.3连接到UART0     */
	
	/* 
	 *  初始化串口通讯参数 
	 */
	U0LCR 	= 0x9B;							                            /* 设置为8位数据位，1位停止位，  */
	                                                                    /* 无奇偶校验，DLAB=1            */
	                                                                    /* 允许设置波特率                */
	Fdiv 	= (Fpclk / 16) / 9600;		                            /* 设置波特率                    */
	U0DLM 	= (unsigned long)Fdiv / 256;
	U0DLL 	= (unsigned long)Fdiv % 256;
	U0LCR &= ~(0x01<<7);
	//U0LCR 	= 0x03;							                            /* 令DLAB位为0                   */
	U0FCR  &= ~0x07;						                            /* 禁用FIFO                      */
    U0IER  &= ~0x07;						                            /* 禁止UART0产生中断             */
}
void ISendStr(uint8 x, uint8 y, uint8 color, char *str)
{
	while (1){
		if (*str == '\0')	break;	                                    /* 结束字符                     */
		PC_DispChar(x++, y, *str++, color);
		if (x >= 80){
			x = 0;
			y++;
		}
	}
}
void ADCIni()
{
   PCONP  |= 1 << 12;						                            /* 打开ADC的功率控制位           */
	
	PINSEL1 = ((unsigned)0x01) << 14;		                            /* 设置P0.23为AIN0[0]功能        */
	
	/* 
	 *  进行ADC模块设置
	 */
	AD0INTEN = (1 << 0);                                                /* 使能AD通道0转换结束后产生中断 */
	AD0CR = (1 << 0)					 |	                            /* SEL=0x01,选择通道0            */
			((Fpclk / 1000000 - 1) << 8) |	                            /* CLKDIV=Fpclk/1000000-1,       */
			                                                            /* 转换时钟为1MHz                */
			(0 << 16)					 |	                            /* BURST=0,软件控制转换操作      */
			(0 << 17)					 |	                            /* CLKS=0, 使用11clock转换       */
			(1 << 21)					 |                              /* PDN=1,正常工作模式            */
			(1 << 24)				;	// |	                            /* START=2,当EDGE选择的边沿出    */
			                                                            /* 现在P2.10管脚时启动AD转换     */
			//(1 << 27);					                                /* 下降沿                        */


	
	
	/* 
	 *  进行VIC设置 
	 */
	 VICIntSelect = 0x00;
	//VICVectCntl4 = 0x24;
	VICVectPriority18 = 1;
	VICVectAddr18 = (uint32)IRQ_ADC;
	VICIntEnable |= (1<<18);
	//vicIrqFuncSet(18, 0x00, (uint32)IRQ_ADC);                           /* 设置ADC中断最高优先级	     */
	//IRQEnable();                                                        /* 打开IRQ中断                   */
}
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
void delay(uint32 pval)
{
  uint32 i ;
  for(i=0;i<pval;i++)
  {
  	 ;
  }
}
void reverse_light2()
{
 testF=testF^1;
  if(testF==0) FIO3CLR |= 1<<25	;
  else 	FIO3PIN=(FIO3SET&(~(1<<25)))|(0x01 << 25);
  lastT0CR0 = 100000/lastT0CR0;
	UART0_SendByte(lastT0CR0>>8);
	UART0_SendByte(lastT0CR0);
  
 }
/*********************************************************************************************************
** 函数名称:  Timer3_CapInt
** 函数功能:  定时器3捕获中断服务程序
** 入口参数:  无
** 出口参数:  无
*********************************************************************************************************/
void __irq Timer0_CapInt(void)
{
//	T0IR   = 1 << 4;						                            /* 清除CAP3.1中断标志           */
//
////   	FIO3CLR = BEEP;							                            /* 蜂鸣器响1声                  */
////    delay(500);
////    FIO3SET = BEEP;
//	lastT0CR0 = T0CR0;
	if((T0IR&0x10)!=0)
	{
	T0IR   = 1 << 4;
	lastT0CR0 = tmpCalHZ;
//	lastT0CR0 = 100000/lastT0CR0;
//	UART0_SendByte(lastT0CR0>>8);
//	UART0_SendByte(lastT0CR0);
	tmpCalHZ = 0;
	}
	if((T0IR&0x01)!=0)
	{
	if(light_on2<9999)light_on2++; //1s
    else{
	canlighton = 1;
     }
//	if(tmpCalHZ<5000)tmpCalHZ++;
	 T0IR=0x01 ;
	}
    VICVectAddr = 0x00;						                            /* 向量中断处理结束             */
}
void iniCapTime0()
{
 	PINSEL7 = 2 << 14;			    		                            /* P3.23连接捕获CAP3.1          */
	//FIO3DIR  = BEEP;						                            /* 设置BEEP为输出               */
    //FIO3SET  = BEEP;
    T0PR = 0;
	T0PC = 0;
	T0MCR = 0x03;		   		// 匹配通道0匹配中断并复位T0TC
	T0MR0 = 1800;	    		// 比较值(0.1mS定时值) 主频 18M晶振
//	T0MR1 = 12000;	    		// 比较值(1S定时值)
	T0IR = 0xffffffff;	
	T0EMR =	0x30;
	T0TCR = 0x03;		   		// 启动并复位T0TC
	T0TCR = 0x01;  
    //PCONP  |= 1 << 23;						                            /* 打开定时器3的功率控制，      */
                                                                        /* 定时器3默认是关闭的;         */
                                                                        /* 注意不能改变其他的功率控制位 */	
	T0CCR   = 6 << 0;						                            /* 设置CAP3.1下降沿捕获，       */
	                                                                    /* 并产生中断,注意捕获功能用作" */
	                                                                    /* 外部中断"使用                */
	                                                                    /* 定时器可以不打开             */
	
	//IRQEnable();							                            /* 使能IRQ中断                  */
 
    /* 设置Timer3中断 */
	VICIntSelect  = 0x00000000;				                            /* 设置所有的通道为IRQ中断      */
	VICVectPriority4  = 0x00;					                            /* 最高优先级                   */
	VICVectAddr4 = (uint32)Timer0_CapInt;	                            /* 设置Timer3向量地址           */
	VICIntEnable  = 1 << 4;				                            /* 使能Timer3中断               */
}

int main()
{  
	//uint32 ADC_Data;
	char str[20];
	 uint8 tmp;
//	PINSEL7  = PINSEL7 & ~(0x03 << 18) ;
//	FIO3DIR =  FIO3DIR   | (0x01 << 25);

	
	UART0_IniDft();
	IOInit();							                            /* 串口0初始化为8位数据位，      */
	Timer1_Init();                                                                    /* 1位停止位，无奇偶校验         */
	//iniCapTime0();
	//ADCIni();
	while(1)
	{ 	;
//	if(canlighton == 1)
//	{
//	  canlighton = 0;
//	  light_on2=0;
//     reverse_light2();
//	}
//		if(ADC_Send_Enable == 1)
//		{
//		 
//			delay(10000000);
//			//sprintf(str, "%4d mV VIN0", ADC_Data);	                    /* 格式转换                      */
//			//ISendStr(0, 0, 0x30, str);             	                    /* 串口0发送数据给上位机显示     */
////			tmp = 	(ADC_Data>>8);
////			UART0_SendByte(tmp);
////			tmp = 	(ADC_Data);
////			UART0_SendByte(tmp);
////			addr0_data 		= 0;
////            UART0_SendByte(lastT0CR0>>8);
////			UART0_SendByte(lastT0CR0);
//			AD0CR |= (1<<24);
//			ADC_Send_Enable = 0;
//		}
	} 

}
