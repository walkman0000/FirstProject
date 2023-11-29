/****************************************************************************
* 文件名：UART3（从） ---LLswitchgear 3500的主通讯---回应03 10功能
* 功能：向串口发送数据。
* 说明：将跳线器JP5短接；
*      使用外部11.0592MHz晶振，根据CONFIG.H文件配置，Fpclk=11.0592MHz；
*      通讯波特率115200，8位数据位，1位停止位，无奇偶校验。
****************************************************************************/
#include  "config.h"
#define T3_sendDelay 20 //单位ms
#define T3_PullBusDelay 10
//uint8 LocalAddress = 0x01;
//T3定义
uint8 T3_Rec_Buffer[100],T3_Send_Buffer[20],T3_Send_Length, T3_sendCount=0,T3_sendIndex=0,T3_haveRecData=0,T3_recCount=0,T3_PermitRecFlag=0;
//T3定义
uint16 T3_receive_time=0,T3_sendComp_time=0;
uint8 T3_haveRecDone=0,T3_sendOverTimeCount,T3_beforeSendCount=0,T3_realSend=0,T3_ItsRecTime=0;
uint16 T3_sendOverTimeValue=0,T3_beforeSendValue;


uint8 AFCToswitch_heartBeat;

uint16 T3_YlToLL3500_CalHeartBeatValue,T3_beforePullBusValue;
uint8  YlToLL3500HeartBeatValue,LastYlToLL3500HeartBeatValue,uart3NormalFlag,uart3HaveNormaledFlag,T3_beforePullBusCount;

#define  UART3_BPS	9600			/* 定义通讯波特率 */

/****************************************************************************
* 名称：IRQ_UART3()
* 功能：UART3的中断服务程序
* 入口参数：无
* 出口参数：无
****************************************************************************/
 void __irq IRQ_UART3(void)                           // 中断服务程序
{  
   	uint32 IIR3=0;
	IIR3=U3IIR;
    if ((IIR3 & 0x0E) == 0x02)                   // 判断是否为发送中断
    {	 
		  if (T3_sendIndex !=  T3_sendCount )
          {
                U3THR =  T3_Send_Buffer[T3_sendIndex];
                T3_sendIndex ++;
          }else	
		  {
		  //U3FCR |= 0x07; 
		  //buffer=U3RBR;
		  T3_PermitRecFlag=1;
		  }	  
    }
	/*else if((U3IIR & 0x0E)==0x0C)
	{
		  
		T3_receive_time=0;
		T3_haveRecData=1;
	  if((U3LSR & 0x01) == 1) // 若U2RBR包含有效数据
	  {
	   	T3_Rec_Buffer[T3_recCount++] = U3RBR; // 保存接收到的数据
	  }             				  

	} */
	else if(((IIR3 & 0x0E)==0x04)||((IIR3 & 0x0E)==0x0C))
	//else if((U3IIR & 0x0E)==0x04)
	{  
	   T3_receive_time=0;
	   T3_haveRecData=1;
	  if((U3LSR & 0x01) == 1) // 若U0RBR包含有效数据
	  {
	   	T3_Rec_Buffer[T3_recCount++] = U3RBR; // 保存接收到的数据
	  }             				  
         

	}else
	{
	buffer=U3RBR;
    //buffer=U3LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* 名称：UART3_Ini()
* 功能：初始化串口0。设置为8位数据位，1位停止位，无奇偶校验，波特率为115200
* 入口参数：无
* 出口参数：无
****************************************************************************/
void  UART3_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 3;		          // 打开UART0控制器电源-默认为打开
   PCONP   |=   0x01 << 25;		          // 打开UART3控制器电源-默认为关闭
   PINSEL1 |= (0x0F<<18);       /* RxD0 and TxD0 */
   U3LCR = 0x9B;		            // DLAB = 1，可设置波特率
   Fdiv = (Fpclk / 16) / UART3_BPS;  // 设置波特率
   U3DLM = Fdiv / 256;							
   U3DLL = Fdiv % 256;
   U3LCR &= ~(0x01<<7);
   U3IER |= 0x01; //允许接受
   //U0IER |= 0x02; // 允许THRE(transmit holding register empty)中断，即发送中断
   U3FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // 设置所有通道为IRQ中断
   VICVectPriority29  = 6;
   VICVectAddr29 = (uint32)IRQ_UART3;       // 设置UART3向量地址
   VICIntEnable |= 1<<29;           // 使能UART3中断
}

void T3_check(void)
{
//计算1s内通讯次数递增两次以上
  if(T3_YlToLL3500_CalHeartBeatValue<10000)T3_YlToLL3500_CalHeartBeatValue++;
  else{
  T3_YlToLL3500_CalHeartBeatValue=0;
  if(abs(YlToLL3500HeartBeatValue-LastYlToLL3500HeartBeatValue)>0){uart3NormalFlag=1;uart3HaveNormaledFlag=1;}
  else uart3NormalFlag=0;
  LastYlToLL3500HeartBeatValue=YlToLL3500HeartBeatValue;
  }

if(T3_beforeSendCount==1)
  {
   T3_beforeSendValue++;
   if(T3_beforeSendValue>T3_sendDelay*10)
   {
   	 T3_beforeSendValue=0;
	 T3_beforeSendCount=0;
	 T3_realSend=1;
   }
  }
  if(T3_beforePullBusCount==1)
  {
   T3_beforePullBusValue++;
   if(T3_beforePullBusValue>T3_PullBusDelay*10) //发送之前等待的时间
   {
   	 T3_beforePullBusValue=0;
	 T3_beforePullBusCount=0;
	  Tx3_EN(1);   //允许发
	 T3_beforeSendCount=1;
   }
  }
 if(T3_ItsRecTime==1)
 { 
      if(T3_receive_time<50)
	  {
	   T3_receive_time++;
	  }
	  
	  if((T3_receive_time>47)&&(T3_haveRecData==1))	 //波特率9600超时为4ms
	  {	
	  //已经过了5ms没发送数据了，而且已经有数据进来了，说明一包结束
	  //接收一包数据完成	-处理接收到的数据
	        T3_haveRecData=0;
			if(T3_recCount>1)
			{
			   T3_haveRecDone = 1;
			   T3_haveRecData = 0;
			 }
			else{
			T3_recCount=0;	 //接收数目置零
			}		
	  }
  }
  if(T3_PermitRecFlag ==1)
  {
  	if(T3_sendComp_time<20)T3_sendComp_time++;	//发送完成等一段时间再释放总线
	else{
		 Tx3_EN(0);//允许收
	     U3IER = 0x01;

		 while((U3LSR&0x01)==0x01)
		 //if(((U3IIR & 0x0E)==0x0C)||((U3IIR & 0x0E)==0x04))
		 {	
		   buffer=U3RBR;
		 }	
		  
		 T3_sendOverTimeCount=1;
		 T3_sendComp_time =0;
		 T3_PermitRecFlag =0;
	}
  }
}

void T3_recCheck()
{
 if((uart3NormalFlag==0)&&(uart3HaveNormaledFlag==1))
 { uart3HaveNormaledFlag=0;
   UpdateOldLLdata();//初始化从3500通讯中获得的数据，例如 ，运输机变频的驱动选择位
 
 }
 if(T3_haveRecDone == 1)
 {  uint16 i,startAddress;
	T3_haveRecDone=0;

	if((T3_Rec_Buffer[T3_recCount-2]==CRC16(T3_Rec_Buffer,T3_recCount-2,1))&&(T3_Rec_Buffer[T3_recCount-1]==CRC16(T3_Rec_Buffer,T3_recCount-2,0)))
    {
	 YlToLL3500HeartBeatValue++;
	 if((T3_Rec_Buffer[0]==0x01)&&(T3_Rec_Buffer[1]==0x03)/*&&(T3_Rec_Buffer[5]==0x06)*/)
	 {
	 //while(1);//测试看门狗
	 //Tx3_EN(1);   //允许发
	 //T3_beforeSendCount=1;
	 T3_beforePullBusCount=1; //延时一段时间再拉总线
	 T3_sendCount=17;
	 LoadT3_sendBuff(17);
	 }else if((T3_Rec_Buffer[0]==0x01)&&(T3_Rec_Buffer[1]==0x10)&&(T3_Rec_Buffer[5]==0x27))
	 {
	  LLswitchDrivesSelected = T3_Rec_Buffer[7+19*2-1]; //地址40019 bit0 MGBP bit1 MGEP bit2 TG
	  //如果Jos的驱动选择和LL开关的变频驱动选择一致，则驱动的故障状态和运输机电压电流等参数，从主通讯获取
	  //否则从专门数据通道获取
	  if(newAFC_Drives_Selected==LLswitchDrivesSelected){GetUart3KeyInfo(T3_Rec_Buffer);

	  }
	  
	  //Tx3_EN(1);   //允许发

	 T3_beforePullBusCount=1; //延时一段时间再拉总线 
	 //T3_beforeSendCount=1;
	 T3_sendCount=8;
	 LoadT3_sendBuff(8);
	 }
	else{
	 T3_haveRecData=0;
	 T3_recCount=0;
	 T3_receive_time=0;//继续接收
	}

	}else
	{
	 T3_haveRecData=0;
	 T3_recCount=0;
	 T3_receive_time=0;//继续接收
	}
	
}
if(T3_realSend==1)
{
    T3_realSend=0;
    T3_sendIndex=1; //发送从数据的第一个发
	T3_recCount=0;	 //接收数目置零
	/*T3_sendCount=6;
	T3_Send_Buffer[0]=1;
	T3_Send_Buffer[1]=3;
	T3_Send_Buffer[2]=0xEE;
	T3_Send_Buffer[3]=0xEE;
	T3_Send_Buffer[4]=CRC16(T3_Send_Buffer,4,1);
	T3_Send_Buffer[5]=CRC16(T3_Send_Buffer,4,0);
	*/
	U3IER = 0x02;
    U3THR = T3_Send_Buffer[0];
}
}
void LoadT3_sendBuff(uint8 sendNum)	// uint8 *pBuff,
{
  if(sendNum==17)
  {
  	 T3_Send_Buffer[0]=1;
	 T3_Send_Buffer[1]=3;
	 T3_Send_Buffer[2]=0x0C;
	 T3_Send_Buffer[3]=0;//40660H
	 T3_Send_Buffer[4]=0;//40660L
	 T3_Send_Buffer[5]=0; //40661H	
	 T3_Send_Buffer[6]=(uart2NormalFlag|(uart1NormalFlag<<1));//uart2NormalFlag;// 1; //40661H	bit0 AFC healthy bit1 BSL healthy 
	 T3_Send_Buffer[7]=0; //40662H
	 T3_Send_Buffer[8]=AFCToswitch_heartBeat++; //40662L
	 T3_Send_Buffer[9]=0; //40663H
	 T3_Send_Buffer[10]=0; //40663L	bit0 BSL pre_start bit1 AFC pre_start
	 T3_Send_Buffer[11]=0; //40664H
	 T3_Send_Buffer[12]=newAFCDelayShutDown; //40664L//AFC运输机正常停止命令
	 T3_Send_Buffer[13]=0; //40665H
	 T3_Send_Buffer[14]=0; //40665L//BSL刮板机正常停止命令
	 T3_Send_Buffer[15]=CRC16(T3_Send_Buffer,15,1);
	 T3_Send_Buffer[16]=CRC16(T3_Send_Buffer,15,0);
	 
  }else if(sendNum==8){
  	T3_Send_Buffer[0]=1;
	 T3_Send_Buffer[1]=0x10;
	 T3_Send_Buffer[2]=0;
	 T3_Send_Buffer[3]=0;//40660H
	 T3_Send_Buffer[4]=0;//40660L
	 T3_Send_Buffer[5]=0x27; //40661H   原本应是 0x27与uart1测试心跳 用switchToAFC_heartBeat
	 T3_Send_Buffer[6]=CRC16(T3_Send_Buffer,6,1);
	 T3_Send_Buffer[7]=CRC16(T3_Send_Buffer,6,0);
  }
}

void GetUart3KeyInfo(uint8 *pBuff)
{
  AFC_MGBP_I=(*(pBuff+6+1)<<8)+*(pBuff+6+2);//40001
  AFC_MGBP_U=(*(pBuff+6+5)<<8)+*(pBuff+6+6);//40003
  AFC_MGEP_I=(*(pBuff+6+7)<<8)+*(pBuff+6+8);//40004
  AFC_MGEP_U=(*(pBuff+6+11)<<8)+*(pBuff+6+12);//40006
  AFC_TG_I=(*(pBuff+6+13)<<8)+*(pBuff+6+14); //40007  
  AFC_TG_U=(*(pBuff+6+17)<<8)+*(pBuff+6+18);//40009

  //AFCRunSignal= *(pBuff+6+46);	//40023
  AFC_breakStat_MGBP=*(pBuff+6+50);	//40025
  AFC_breakStat_MGEP=*(pBuff+6+52);	//40026
  AFC_breakStat_TG=*(pBuff+6+54);  //40027

}
