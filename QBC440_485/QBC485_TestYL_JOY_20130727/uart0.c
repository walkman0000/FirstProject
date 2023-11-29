/****************************************************************************
* 文件名：UART0（主） ---LLswitchgear 01 02 03 地址的03数据查询
* 功能：向串口发送数据。
* 说明：将跳线器JP5短接；
*      使用外部11.0592MHz晶振，根据CONFIG.H文件配置，Fpclk=11.0592MHz；
*      通讯波特率115200，8位数据位，1位停止位，无奇偶校验。
****************************************************************************/
#include  "config.h"

#define T0_sendDelay  10 //单位毫秒 T0发送前拉总线的时间
#define T0_PullBusDelay 5
#define T0_sendOverDelayTime  50 //单位毫秒 T0发送完成之后等待接收的时间
#define T0_SameFrameSendCount  1
//uint8 JudgeOneFrameTime;
uint8 ToBeQueryAddress=0x01,EnsureQueryAddress=0x01;
uint8 send[9]={0xfe,0x1,0x0,0x5,0xd1,0xb6,0xb7,0xc9,0x41};
uint8 buffer,SendNextFlag/*进行下一次发送的标志*/,SendSameTimes/*同一地址同一帧已经发送的次数*/;
//T0定义
uint8 T0_Rec_Buffer[200],T0_Send_Length, T0_sendCount=0,T0_sendIndex=0,T0_haveRecData=0,T0_recCount=0,T0_PermitRecFlag=0,T0_ItsRecTime=0;	 
uint8 T0_Send_Buffer[10],T0_Need_reply=0,T0_sendOverTimeCount,T0_beforeSendCount=0,T0_realSend=0;
uint16 T0_sendOverTimeValue;
uint16 DataBuffer1[120],DataBuffer2[100],DataBuffer3[100];
uint8 T0_SendFrameNum=1;//同一个地址的第几帧标志
//T0定义
uint16 T0_receive_time=0,T0_sendComp_time=0,T0_beforeSendValue=0;
uint8 T0_haveRecDone=0;

uint8 T0_YlToLLHeartBeatValue1,T0_YlToLLHeartBeatValue2,T0_YlToLLHeartBeatValue3;
uint8 LastT0_YlToLLHeartBeatValue1,LastT0_YlToLLHeartBeatValue2,LastT0_YlToLLHeartBeatValue3;
uint8 uart0NormalFlag1,uart0NormalFlag2,uart0NormalFlag3,MonitorAddr3Value;
uint8 uart0HaveNormaledFlag1,uart0HaveNormaledFlag2,uart0HaveNormaledFlag3,T0_beforePullBusCount;
uint16 T0_YlToLL_CalHeartBeatValue,T0_beforePullBusValue;
uint16 T1_HoldInSendModeValue;
//uint16 T0_sendOverDelayTime;
uint8 check3500Time;
/****************************************************************************
* 名称：DelayNS()
* 功能：长软件延时
* 入口参数：dly		延时参数，值越大，延时越久
* 出口参数：无
****************************************************************************/
void  DelayNS(uint32  dly)
{  uint32  i;

   for(; dly>0; dly--) 
      for(i=0; i<50000; i++);
}


#define  UART0_BPS	9600			/* 定义通讯波特率 */
/****************************************************************************
* 名称：IRQ_UART0()
* 功能：UART0的中断服务程序
* 入口参数：无
* 出口参数：无
****************************************************************************/
 void __irq IRQ_UART0(void)                           // 中断服务程序
{  
   	uint32 IIR0=0;
	IIR0=U0IIR;
    if ((IIR0 & 0x0E) == 0x02)                   // 判断是否为发送中断
    {	 
		  if (T0_sendIndex !=  T0_sendCount )
          {
                U0THR =  T0_Send_Buffer[T0_sendIndex];
                T0_sendIndex ++;
				//reverse_light();
          }else	
		  {
		   T0_PermitRecFlag=1;
		   //U0FCR |= 0x07; 
		  // buffer=U0RBR;
		   //reverse_light();
		  }	  
    }
	/*else if((U0IIR & 0x0E)==0x0C)
	{  T0_receive_time=0;
	   T0_haveRecData=1;
	  if((U0LSR & 0x01) == 1) // 若U0RBR包含有效数据
	  {
	   	T0_Rec_Buffer[T0_recCount++] = U0RBR; // 保存接收到的数据
	  }             				  
         

	}*/ 
	else if(((IIR0 & 0x0E)==0x04)||((IIR0 & 0x0E)==0x0C))
	//else if((U0IIR & 0x0E)==0x04)
	{  //reverse_light();   
	   T0_receive_time=0;
	   T0_sendOverTimeValue=0;
	   T0_haveRecData=1;
	  if((U0LSR & 0x01) == 1) // 若U0RBR包含有效数据
	  {
	   	T0_Rec_Buffer[T0_recCount++] = U0RBR; // 保存接收到的数据
	  }             				  
         

	}else
	{
	//U0FCR |= 0x07;
	buffer=U0RBR;
    //buffer=U0LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* 名称：UART0_Ini()
* 功能：初始化串口0。设置为8位数据位，1位停止位，无奇偶校验，波特率为115200
* 入口参数：无
* 出口参数：无
****************************************************************************/
void  UART0_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 3;		          // 打开UART0控制器电源-默认为打开
   //PCONP   |=   0x01 << 24;		          // 打开UART0控制器电源-默认为关闭
   PINSEL0 |= 0x00000050;       /* RxD0 and TxD0 */
   U0LCR = 0x9B;		            // DLAB = 1，可设置波特率
   Fdiv = (Fpclk / 16) / UART0_BPS;  // 设置波特率	   4800 =0x00D8	  9600 =0x006C
   U0DLM = Fdiv / 256;	 //4800 =0x0000	  9600 =0x0000						
   U0DLL = Fdiv % 256;	 //4800 =0x00D8	  9600 =0x006C
   U0LCR &= ~(0x01<<7);
   U0IER |= 0x01; //允许接受
   //U0IER |= 0x02; // 允许THRE(transmit holding register empty)中断，即发送中断
   U0FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // 设置所有通道为IRQ中断
   VICVectPriority6  = 3;
   VICVectAddr6 = (uint32)IRQ_UART0;       // 设置UART0向量地址
   VICIntEnable |= 1<<6;           // 使能UART0中断
}
void  UART0_IniByBuad(uint16 myBuad)
{  uint16 Fdiv;
   VICIntEnable &= (~(1<<6));           // 禁止UART0中断
   if(myBuad==9600)Fdiv=0x006C;
   else Fdiv=0x00D8;
   //PCONP   |=   0x01 << 3;		          // 打开UART0控制器电源-默认为打开
   //PCONP   |=   0x01 << 24;		          // 打开UART0控制器电源-默认为关闭
   PINSEL0 |= 0x00000050;       /* RxD0 and TxD0 */
   U0LCR = 0x9B;		            // DLAB = 1，可设置波特率
   //Fdiv = (Fpclk / 16) / UART0_BPS;  // 设置波特率	   4800 =0x00D8	  9600 =0x006C
   U0DLM = Fdiv / 256;	 //4800 =0x0000	  9600 =0x0000						
   U0DLL = Fdiv % 256;	 //4800 =0x00D8	  9600 =0x006C
   U0LCR &= ~(0x01<<7);
   U0IER |= 0x01; //允许接受
   //U0IER |= 0x02; // 允许THRE(transmit holding register empty)中断，即发送中断
   U0FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   //VICIntSelect = 0x00000000;           // 设置所有通道为IRQ中断
   //VICVectPriority6  = 3;
   //VICVectAddr6 = (uint32)IRQ_UART0;       // 设置UART0向量地址
   VICIntEnable |= 1<<6;           // 使能UART0中断
}
/****************************************************************************
* 名称： CRC16
* 功能：向串口UART0发送字符串"Hello World!"
****************************************************************************/
unsigned char CRC16( unsigned char * puchMsg,unsigned char usDataLen,unsigned char byte_flag)        // 得到CRC的高字节 (byte_flag == 1)
{															         // 低字节（byte_flag == 1）
  unsigned char uchCRCHi;
  unsigned char uchCRCLo;
  unsigned char uIndex;
uchCRCHi=0xFF;
uchCRCLo=0xFF;
  while ( usDataLen-- )
  {
    uIndex = uchCRCHi ^ * puchMsg++;
	uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
	uchCRCLo = auchCRCLo[uIndex];
  }

  if(byte_flag)
  {
    return(uchCRCHi);
  }
  else
  {
    return(uchCRCLo);
  }
}

void Tx0_EN(uint8 flag)
{
 if(flag==0)
 {IOCLR0 |= 1<<9	; 
 T0_ItsRecTime=1;
 T0_haveRecData=0 ;
 T0_recCount=0; 
 }//允许收
 else
 { IOPIN0=(IOSET0&(~(1<<9)))|(0x01 << 9);//IOSET0 =  IOSET0   | (0x01 << 9); //允许发
  T0_ItsRecTime=0;	  
 }
}

void Tx1_EN(uint8 flag)
{
 if(flag==0)
 {IOCLR1 |= 1<<12	;
   T1_ItsRecTime=1;
 T1_haveRecData=0 ;
 T1_recCount=0;
 } //允许收
 else {
 T1_ItsRecTime=0;
 IOPIN1=(IOSET1&(~(1<<12)))|(0x01 << 12);
 //IOSET1 =  IOSET1   | (0x01 << 12); //允许发
 }
}

void Tx2_EN(uint8 flag)
{
 if(flag==0)
 {
 T2_ItsRecTime=1;
 T2_haveRecData=0 ;
 T2_recCount=0;
 IOCLR0 |= 0x0001	;  //允许收
 }
 else 
 {
 IOPIN0=(IOSET0&(~(1)))|(1);
 //IOSET0 =  IOSET0   | (0x0001);  //允许发
 T2_ItsRecTime=0;
 }
 
}
void Tx3_EN(uint8 flag)
{
 if(flag==0)
 {
 T3_ItsRecTime=1;
 T3_haveRecData=0 ;
 T3_recCount=0;
 FIO2SET&=(~(1<<22));
 FIO2CLR |= 1<<22	;  //允许收
 }
 else 
 {
 
 FIO2PIN=(FIO2SET&(~(1<<22)))|(0x01 << 22);//FIO2SET =  FIO2SET   | (0x01 << 22); //允许发
 T3_ItsRecTime=0;
 }
 
}
void T1_CheckSendHold()
{
if(T1_HoldInSendModeValue<500)T1_HoldInSendModeValue++;
else{//
 T1_HoldInSendModeValue = 0;
if((T1_ItsRecTime == 1)&&((IOPIN1 &(1<<12))!=0))
{
	UART1_Ini();
	Tx1_EN(0);
}
}

}
void Load_sendBuffer(uint8 addr,uint16 startAddr,uint16 len)
{    
    T0_Send_Buffer[0]= addr;
	T0_Send_Buffer[1]= 0x03;
	T0_Send_Buffer[2]= startAddr>>8;
	T0_Send_Buffer[3]= startAddr;
	T0_Send_Buffer[4]= len>>8;
	T0_Send_Buffer[5]= len;
	T0_Send_Buffer[6]=CRC16(T0_Send_Buffer,6,1);
    T0_Send_Buffer[7]=CRC16(T0_Send_Buffer,6,0);

}



void T0_check(void)
{ //监控与三个联力开关的通讯状态
  if(T0_YlToLL_CalHeartBeatValue<50000)T0_YlToLL_CalHeartBeatValue++;  //5s才能巡检一次三台开关 
  else{
  //地址1的通讯监控
  T0_YlToLL_CalHeartBeatValue=0;
  if(abs(T0_YlToLLHeartBeatValue1-LastT0_YlToLLHeartBeatValue1)>0){uart0NormalFlag1=1;uart0HaveNormaledFlag1=1;}
  else uart0NormalFlag1=0;
  LastT0_YlToLLHeartBeatValue1=T0_YlToLLHeartBeatValue1;
  //地址2的通讯监控
  if(abs(T0_YlToLLHeartBeatValue2-LastT0_YlToLLHeartBeatValue2)>0){uart0NormalFlag2=1;uart0HaveNormaledFlag2=1;}
  else uart0NormalFlag2=0;
  LastT0_YlToLLHeartBeatValue2=T0_YlToLLHeartBeatValue2;
  if(MonitorAddr3Value<3)MonitorAddr3Value++;	//每隔4S检测一下地址3的通讯情况
   else{MonitorAddr3Value=0;
   //地址3的通讯监控
  if(abs(T0_YlToLLHeartBeatValue3-LastT0_YlToLLHeartBeatValue3)>0){uart0NormalFlag3=1;uart0HaveNormaledFlag3=1;}
  else uart0NormalFlag3=0;
  LastT0_YlToLLHeartBeatValue3=T0_YlToLLHeartBeatValue3;
   }
  
  }

 //发送前的延时
 if(T0_beforeSendCount==1)
  {
   T0_beforeSendValue++;
   if(T0_beforeSendValue>T0_sendDelay*10) //发送之前的拉总线延时
   {
   	 T0_beforeSendValue=0;
	 T0_beforeSendCount=0;
	 T0_realSend=1;
   }
  }
 if(T0_beforePullBusCount==1)
  {
   T0_beforePullBusValue++;
   if(T0_beforePullBusValue>T0_PullBusDelay*10) //发送之前等待的时间
   {
   	 T0_beforePullBusValue=0;
	 T0_beforePullBusCount=0;
	  Tx0_EN(1);   //允许发
	 T0_beforeSendCount=1;
   }
  }
  //接收数据的延时断帧
 if(T0_ItsRecTime==1)
 { 
      if(T0_receive_time<168)T0_receive_time++;
	  if((T0_receive_time>165)&&(T0_haveRecData==1))	 //波特率9600超时为4ms =49//波特率4800超时为8ms=165	JudgeOneFrameTime
	  {	
	  //已经过了5ms没发送数据了，而且已经有数据进来了，说明一包结束
	  //接收一包数据完成	-处理接收到的数据
	        if(T0_recCount>3)
			{
			   T0_haveRecDone = 1;
			   T0_haveRecData=0;
			 }
			else{
			T0_recCount=0;	 //接收数目置零
			}		
	  }
  }
  if(T0_PermitRecFlag ==1) //波特率9600超时为20//波特率4800超时为20
  {
  	if(T0_sendComp_time<20)T0_sendComp_time++;	//发送完成等一段时间再释放总线
	else{
	     //reverse_light();
		 Tx0_EN(0);//允许收
	     U0IER = 0x01;
		 while((U0LSR&0x01)==0x01)
		 //if(((U0IIR & 0x0E)==0x0C)||((U0IIR & 0x0E)==0x04))
		 {	
		   buffer=U0RBR;
		 }	
		 
		 T0_sendOverTimeCount=1;
		 T0_sendOverTimeValue=0;
		 T0_sendComp_time =0;
		 T0_PermitRecFlag =0;
	}
  }
}

void T0_recCheck()
{ uint16 i,j,T0_startAddress,T0_endAddress;
  uint16 t1;
 if(T0_haveRecDone == 1)
{
	T0_haveRecDone=0;
	T0_sendOverTimeCount=0;
	T0_sendOverTimeValue=0;
	if((T0_Rec_Buffer[T0_recCount-2]==CRC16(T0_Rec_Buffer,T0_recCount-2,1))&&(T0_Rec_Buffer[T0_recCount-1]==CRC16(T0_Rec_Buffer,T0_recCount-2,0)))
    {
	  T0_sendCount = T0_recCount;
	  for(i=0;i< T0_sendCount;i++)
	  {
	   T0_Send_Buffer[i]= i;
	  } 
	 
	  T0_beforePullBusCount = 1;
	 
	}else
	{  
	 T0_haveRecData=0;
	 T0_recCount=0;
	 T0_receive_time=0;//继续接收
	}

}

}  


void UART0_SendData()
{

	 if(T0_realSend==1) //发送前拉总线一段时间
	 {T0_realSend=0;
    T0_sendIndex=1; //发送从数据的第一个发
	T0_recCount=0;	 //接收数目置零

	U0IER = 0x02;
    U0THR = T0_Send_Buffer[0];
   	  }
}
void QueryTheAddress()
{ 
  Load_sendBuffer(ToBeQueryAddress,128,2);
}

