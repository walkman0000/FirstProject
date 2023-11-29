#include "config.h"

uint16 light_on=0;
uint8 testF=0;
uint16 secondTwo=0;
uint16 Msencond300  ;
uint32 lastT0CR0,tmpCalHZ;
/****************************************************************************
* ���ƣ�IRQ_Time0()
* ���ܣ��ж�Time0������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void reverse_light()
{
 testF=testF^1;
//  if(testF==0)IOCLR1 |= 1<<5	;
//  else 	IOPIN1=(IOSET1&(~(1<<5)))|(0x01 << 5);//IOSET1 |= 1<<5 ;
    if(testF==0)FIO3CLR |= 1<<24	;
  else 	FIO3SET |= (0x01 << 24);//IOSET1 |= 1<<5 ;
  /*
  if(testF==0){
  KS1_DelayOn(0)	;
  KS2_DelayOn(0)	;KS3_DelayOn(0)	;
  KS4_DelayOn(0)	;KS5_DelayOn(0)	;
  KS6_DelayOn(0)	;KS7_DelayOn(0)	;
  KS8_DelayOn(0)	;
  }
  
  else {
   	KS1_DelayOn(1) ; KS2_DelayOn(1) ;
	KS3_DelayOn(1) ;KS4_DelayOn(1) ;
	KS5_DelayOn(1) ;KS6_DelayOn(1) ;
	KS7_DelayOn(1) ;KS8_DelayOn(1) ;
  }	*/
  
}
void   __irq IRQ_Time0(void)
{ 
  //0.1us�ж�
  #if 0	   //���ԵƵ���˸
  if(light_on<9999)light_on++; //1s
  else{
  light_on=0;
  reverse_light();
  }
  #endif
  T0_check();
  T1_check();
  T2_check(); 
  T3_check();
  T0IR=0x01 ;	//���Ӧ��ͨ��0��IRλд��1�Ḵλ�жϣ�д��0��Ч
  VICVectAddr = 0x00;
}

void __irq IRQ_Timer1(void)
{
   #if 1	   //���ԵƵ���˸
  if(light_on<9999)light_on++;
  else{
  light_on=0;
  reverse_light();
  }
  #endif
  if(secondTwo<9999)secondTwo++;
  else{
  secondTwo=0;
  //if(T2_StepSend==0)T2_StepSend=1;
  }
  if(tmpCalHZ<5000)tmpCalHZ++;
  if(Msencond300<3000)Msencond300++;
  else{
  Msencond300=0;
  switchToAFC_heartBeat++;  
  }
   T1IR=0x01 ;
   VICVectAddr = 0x00;  
}


/****************************************************************************
* ���ƣ�Time0Init()
* ���ܣ���ʼ����ʱ��0����ʱʱ��Ϊ1S�� 
* ��ڲ�������
* ���ڲ�������			 12000000
****************************************************************************/
void  Time0Init(void)
{  
	T0PR = 0;			    	// ���ö�ʱ��0��ƵΪ1��Ƶ����16588800Hz	//���ö�ʱ��0��ƵΪ10��Ƶ����1658880Hz T0PR  = 9;
	T0MCR = 0x03;		   		// ƥ��ͨ��0ƥ���жϲ���λT0TC
	T0MR0 = 1659;	    		// �Ƚ�ֵ(0.1mS��ʱֵ) ��Ƶ 18M����
//	T0MR1 = 12000;	    		// �Ƚ�ֵ(1S��ʱֵ)
	T0IR = 0xffffffff;	
	T0EMR =	0x30;
	T0TCR = 0x03;		   		// ��������λT0TC
	T0TCR = 0x01; 
	//IRQ
	//485���ڵ�
	#if 1
	PINSEL2  = PINSEL2 & ~(0x03 << 10) ;
	IODIR1 =  IODIR1   | (0x01 << 5);
	#endif
	T0CCR   = 6 << 3;						                            /* ����CAP3.1�½��ز���       */

 


	VICIntSelect = 0x00;
	//VICVectCntl4 = 0x24;
	VICVectPriority4 = 1;
	VICVectAddr4 = (uint32)IRQ_Time0;
	VICIntEnable = 0x00000010 ;

}

void Timer1_Init(void)
{ 							   
   T1PR  = 0;			       // ���ö�ʱ��1��ƵΪ1��Ƶ����16588800Hz	//���ö�ʱ��1��ƵΪ10��Ƶ����1658880Hz T1PR  = 9;
   T1MCR = 0x03;		 	   // ƥ��ͨ��1ƥ���жϲ���λT0TC
   T1MR0 = 1800;			   // ����MR0�Ƚ�ֵ(us����ʱֵ) ��ʱ 1 ms �������ж���λ����ʱ����������ʱ����λ����ʱ����
   T1TCR |= 0x00;  
   
   /* ���ö�ʱ��0�ж�IRQ */

   VICIntSelect = 0x00;						// �����ж�ͨ������ΪIRQ�ж�
   VICVectPriority5 = 2;
   VICVectCntl5 = 0x25;						// ��ʱ��1�ж�ͨ���������ȼ�(����������9)
   VICVectAddr5 = (uint32)IRQ_Timer1; 		// �����жϷ�������ַ���� 
   //VICIntEnable |= 0x00000020;				// ʹ�ܶ�ʱ��1�ж�	
   VICIntEnable |= 1<<5;
   T1TCR = 0x03;		   	                // ��������λT1TC
   T1TCR = 0x01;   			
}



