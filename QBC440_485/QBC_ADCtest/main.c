#include  "config.h" 
#define	UART_BPS	9600	

unsigned char testff;
unsigned char tBuff[]={0x01,0x03,0x00,0x00,0x00,0x06} ;
uint8 CRC_lowdata,CRC_highdata,ADC_Send_Enable,canlighton;
uint32 addr0_data=0,ADC_Data,light_on2;
void IOInit()
{
  PINSEL7 &=(~(0x03<<16));
  FIO3DIR |= (1<<24);		//P3.24ΪBEEP�����P3.23Ϊ��ͣ����
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
** �������� ��IRQ_AD
** �������� ��AD�жϷ������ 
** ��ڲ��� ����
** ���ڲ��� ����
**********************************************************************************************************/
void __irq IRQ_ADC(void)
{
			
	addr0_data 		= (uint32)AD0DR0;				                    /* ��ȡADC����                   */
	ADC_Send_Enable = 1; 					                            /* ʹ�ܴ��ڷ���ADC0���ݵĿ���λ	 */	
	  ADC_Data = (addr0_data >> 6) & 0x3ff;
			ADC_Data = ADC_Data * 2500;				                    /* �ο���ѹ�ɾ��ܺ�ѹԴ�ṩ��2.5V*/
			ADC_Data = ADC_Data / 1024;
	VICVectAddr 	= 0x00;					                            /* ֪ͨVIC�жϴ������           */
	//AD0CR |= (1<<24);
}
/**********************************************************************************************************
** �������� ��UART0_SendByte
** �������� ���򴮿ڷ����ֽ����ݣ����ȴ����ݷ�����ϡ�
** ��ڲ��� ��data		Ҫ���͵�����
** ���ڲ��� ����
**********************************************************************************************************/
void UART0_SendByte(uint8 data)
{
	U0THR = data;
	
	while ((U0LSR & 0x40) == 0);			                            /* �ȴ����ݷ������              */
}
void PC_DispChar(uint8 x, uint8 y, uint8 chr, uint8 color)
{
	UART0_SendByte(0xff);			                                    /* ��ʼ�ַ�                     */
	UART0_SendByte(x);
	UART0_SendByte(y);
	UART0_SendByte(chr);
	UART0_SendByte(color);
}
/**********************************************************************************************************
** �������� ��UART0_IniDft
** �������� ����Ĭ��ֵ��ʼ������0�����ź�ͨѶ����������Ϊ8λ����λ��1λֹͣλ������żУ�飬������115200��
** ��ڲ��� ��
** ���ڲ��� ����
***********************************************************************************************************/
void UART0_IniDft(void)
{
	uint32 	Fdiv = 0;

	/* 
	 *  ��ʼ����ص�IO 
	 */
	PINSEL0 = (0x01 << 4) | (0x01 << 6);	                            /* ����P0.2��P0.3���ӵ�UART0     */
	
	/* 
	 *  ��ʼ������ͨѶ���� 
	 */
	U0LCR 	= 0x9B;							                            /* ����Ϊ8λ����λ��1λֹͣλ��  */
	                                                                    /* ����żУ�飬DLAB=1            */
	                                                                    /* �������ò�����                */
	Fdiv 	= (Fpclk / 16) / 9600;		                            /* ���ò�����                    */
	U0DLM 	= (unsigned long)Fdiv / 256;
	U0DLL 	= (unsigned long)Fdiv % 256;
	U0LCR &= ~(0x01<<7);
	//U0LCR 	= 0x03;							                            /* ��DLABλΪ0                   */
	U0FCR  &= ~0x07;						                            /* ����FIFO                      */
    U0IER  &= ~0x07;						                            /* ��ֹUART0�����ж�             */
}
void ISendStr(uint8 x, uint8 y, uint8 color, char *str)
{
	while (1){
		if (*str == '\0')	break;	                                    /* �����ַ�                     */
		PC_DispChar(x++, y, *str++, color);
		if (x >= 80){
			x = 0;
			y++;
		}
	}
}
void ADCIni()
{
   PCONP  |= 1 << 12;						                            /* ��ADC�Ĺ��ʿ���λ           */
	
	PINSEL1 = ((unsigned)0x01) << 14;		                            /* ����P0.23ΪAIN0[0]����        */
	
	/* 
	 *  ����ADCģ������
	 */
	AD0INTEN = (1 << 0);                                                /* ʹ��ADͨ��0ת������������ж� */
	AD0CR = (1 << 0)					 |	                            /* SEL=0x01,ѡ��ͨ��0            */
			((Fpclk / 1000000 - 1) << 8) |	                            /* CLKDIV=Fpclk/1000000-1,       */
			                                                            /* ת��ʱ��Ϊ1MHz                */
			(0 << 16)					 |	                            /* BURST=0,�������ת������      */
			(0 << 17)					 |	                            /* CLKS=0, ʹ��11clockת��       */
			(1 << 21)					 |                              /* PDN=1,��������ģʽ            */
			(1 << 24)				;	// |	                            /* START=2,��EDGEѡ��ı��س�    */
			                                                            /* ����P2.10�ܽ�ʱ����ADת��     */
			//(1 << 27);					                                /* �½���                        */


	
	
	/* 
	 *  ����VIC���� 
	 */
	 VICIntSelect = 0x00;
	//VICVectCntl4 = 0x24;
	VICVectPriority18 = 1;
	VICVectAddr18 = (uint32)IRQ_ADC;
	VICIntEnable |= (1<<18);
	//vicIrqFuncSet(18, 0x00, (uint32)IRQ_ADC);                           /* ����ADC�ж�������ȼ�	     */
	//IRQEnable();                                                        /* ��IRQ�ж�                   */
}
void IniWDT()
{
 WDCLKSEL=0; //ѡ���ڲ�4MRC��Դ����4��Ƶ���1M
WDTC=0xff000;//��ʱ��ԼΪ1s //;0x400000
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
** ��������:  Timer3_CapInt
** ��������:  ��ʱ��3�����жϷ������
** ��ڲ���:  ��
** ���ڲ���:  ��
*********************************************************************************************************/
void __irq Timer0_CapInt(void)
{
//	T0IR   = 1 << 4;						                            /* ���CAP3.1�жϱ�־           */
//
////   	FIO3CLR = BEEP;							                            /* ��������1��                  */
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
    VICVectAddr = 0x00;						                            /* �����жϴ������             */
}
void iniCapTime0()
{
 	PINSEL7 = 2 << 14;			    		                            /* P3.23���Ӳ���CAP3.1          */
	//FIO3DIR  = BEEP;						                            /* ����BEEPΪ���               */
    //FIO3SET  = BEEP;
    T0PR = 0;
	T0PC = 0;
	T0MCR = 0x03;		   		// ƥ��ͨ��0ƥ���жϲ���λT0TC
	T0MR0 = 1800;	    		// �Ƚ�ֵ(0.1mS��ʱֵ) ��Ƶ 18M����
//	T0MR1 = 12000;	    		// �Ƚ�ֵ(1S��ʱֵ)
	T0IR = 0xffffffff;	
	T0EMR =	0x30;
	T0TCR = 0x03;		   		// ��������λT0TC
	T0TCR = 0x01;  
    //PCONP  |= 1 << 23;						                            /* �򿪶�ʱ��3�Ĺ��ʿ��ƣ�      */
                                                                        /* ��ʱ��3Ĭ���ǹرյ�;         */
                                                                        /* ע�ⲻ�ܸı������Ĺ��ʿ���λ */	
	T0CCR   = 6 << 0;						                            /* ����CAP3.1�½��ز���       */
	                                                                    /* �������ж�,ע�Ⲷ��������" */
	                                                                    /* �ⲿ�ж�"ʹ��                */
	                                                                    /* ��ʱ�����Բ���             */
	
	//IRQEnable();							                            /* ʹ��IRQ�ж�                  */
 
    /* ����Timer3�ж� */
	VICIntSelect  = 0x00000000;				                            /* �������е�ͨ��ΪIRQ�ж�      */
	VICVectPriority4  = 0x00;					                            /* ������ȼ�                   */
	VICVectAddr4 = (uint32)Timer0_CapInt;	                            /* ����Timer3������ַ           */
	VICIntEnable  = 1 << 4;				                            /* ʹ��Timer3�ж�               */
}

int main()
{  
	//uint32 ADC_Data;
	char str[20];
	 uint8 tmp;
//	PINSEL7  = PINSEL7 & ~(0x03 << 18) ;
//	FIO3DIR =  FIO3DIR   | (0x01 << 25);

	
	UART0_IniDft();
	IOInit();							                            /* ����0��ʼ��Ϊ8λ����λ��      */
	Timer1_Init();                                                                    /* 1λֹͣλ������żУ��         */
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
//			//sprintf(str, "%4d mV VIN0", ADC_Data);	                    /* ��ʽת��                      */
//			//ISendStr(0, 0, 0x30, str);             	                    /* ����0�������ݸ���λ����ʾ     */
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
