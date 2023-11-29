#include "config.h"

uint16 light_on=0;
uint8 testF=0;
uint16 secondTwo=0;
uint16 Msencond300  ;
uint32 lastT0CR0,tmpCalHZ;
/****************************************************************************
* 名称：IRQ_Time0()
* 功能：中断Time0服务函数
* 入口参数：无
* 出口参数：无
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
  //0.1us中断
  #if 0	   //测试灯的闪烁
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
  T0IR=0x01 ;	//向对应的通道0的IR位写入1会复位中断，写入0无效
  VICVectAddr = 0x00;
}

void __irq IRQ_Timer1(void)
{
   #if 1	   //测试灯的闪烁
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
* 名称：Time0Init()
* 功能：初始化定时器0，定时时间为1S。 
* 入口参数：无
* 出口参数：无			 12000000
****************************************************************************/
void  Time0Init(void)
{  
	T0PR = 0;			    	// 设置定时器0分频为1分频，得16588800Hz	//设置定时器0分频为10分频，得1658880Hz T0PR  = 9;
	T0MCR = 0x03;		   		// 匹配通道0匹配中断并复位T0TC
	T0MR0 = 1659;	    		// 比较值(0.1mS定时值) 主频 18M晶振
//	T0MR1 = 12000;	    		// 比较值(1S定时值)
	T0IR = 0xffffffff;	
	T0EMR =	0x30;
	T0TCR = 0x03;		   		// 启动并复位T0TC
	T0TCR = 0x01; 
	//IRQ
	//485串口灯
	#if 1
	PINSEL2  = PINSEL2 & ~(0x03 << 10) ;
	IODIR1 =  IODIR1   | (0x01 << 5);
	#endif
	T0CCR   = 6 << 3;						                            /* 设置CAP3.1下降沿捕获，       */

 


	VICIntSelect = 0x00;
	//VICVectCntl4 = 0x24;
	VICVectPriority4 = 1;
	VICVectAddr4 = (uint32)IRQ_Time0;
	VICIntEnable = 0x00000010 ;

}

void Timer1_Init(void)
{ 							   
   T1PR  = 0;			       // 设置定时器1分频为1分频，得16588800Hz	//设置定时器1分频为10分频，得1658880Hz T1PR  = 9;
   T1MCR = 0x03;		 	   // 匹配通道1匹配中断并复位T0TC
   T1MR0 = 1800;			   // 设置MR0比较值(us级定时值) 定时 1 ms ，用于判断上位机超时、传感器超时、上位机延时发送
   T1TCR |= 0x00;  
   
   /* 设置定时器0中断IRQ */

   VICIntSelect = 0x00;						// 所有中断通道设置为IRQ中断
   VICVectPriority5 = 2;
   VICVectCntl5 = 0x25;						// 定时器1中断通道分配优先级(向量控制器9)
   VICVectAddr5 = (uint32)IRQ_Timer1; 		// 设置中断服务程序地址向量 
   //VICIntEnable |= 0x00000020;				// 使能定时器1中断	
   VICIntEnable |= 1<<5;
   T1TCR = 0x03;		   	                // 启动并复位T1TC
   T1TCR = 0x01;   			
}



