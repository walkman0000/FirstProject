/****************************************************************************
* 文件名：UART1（主） ---JOS 上传三台开关的数据---10功能
* 功能：向串口发送数据。
* 说明：将跳线器JP5短接；
*      使用外部11.0592MHz晶振，根据CONFIG.H文件配置，Fpclk=11.0592MHz；
*      通讯波特率115200，8位数据位，1位停止位，无奇偶校验。
****************************************************************************/
#include  "config.h"
#define T1_sendDelay 10 //单位ms
#define T1_PullBusDelay 20
#define T1_sendOverDelayTime 30 //单位ms
uint8 T1_StepSendFlag/*发送下一位的标志*/,T1_StepSendNum=0/*写数据的阶段代码*/,T1_WriteSequence/*写类型的顺序*/;
//T1定义
uint8 T1_Rec_Buffer[80],T1_Send_Buffer[130],T1_Send_Length, T1_sendCount=0,T1_sendIndex=0,T1_haveRecData=0,T1_recCount=0,T1_PermitRecFlag=0;
//T1定义
uint16 T1_receive_time=0,T1_sendComp_time=0;
uint8 T1_haveRecDone=0,T1_sendOverTimeCount,T1_beforeSendCount=0,T1_realSend=0,T1_ItsRecTime=0;
uint16 T1_sendOverTimeValue=0,T1_beforeSendValue,T1_YlToJOS2_CalHeartBeatValue,T1_beforePullBusValue;
uint8 uart1NormalFlag,uart1HaveNormaledFlag,T1_beforePullBusCount;
uint16 LastT1_HeartBeatValue,T1_HeartBeatValue;
#define  UART1_BPS	9600			/* 定义通讯波特率 */
#define QBCAddress 1
uint16 QBCReadBuff[9],QBCWriteBuff[9],T1_333ms;
/****************************************************************************
* 名称：DelayNS()
* 功能：长软件延时
* 入口参数：dly		延时参数，值越大，延时越久
* 出口参数：无
****************************************************************************/

/****************************************************************************
* 名称：IRQ_UART1()
* 功能：UART1的中断服务程序
* 入口参数：无
* 出口参数：无
****************************************************************************/
 void __irq IRQ_UART1(void)                           // 中断服务程序
{  
   	uint32 IIR1=0;
	IIR1=U1IIR;
    if ((IIR1 & 0x0E) == 0x02)                   // 判断是否为发送中断
    {	 
		  if (T1_sendIndex !=  T1_sendCount )
          {
                U1THR =  T1_Send_Buffer[T1_sendIndex];
                T1_sendIndex ++;
          }else	
		  {
		  //U1FCR |= 0x07; 
		  //buffer=U1RBR;
		  T1_PermitRecFlag=1;
		  }	  
    }
	/*else if((IIR1 & 0x0E)==0x0C)
	{  T1_receive_time=0;
	   T1_haveRecData=1;
	  if((U1LSR & 0x01) == 1) // 若U1RBR包含有效数据
	  {
	   	T1_Rec_Buffer[T1_recCount++] = U1RBR; // 保存接收到的数据
	  }             				  
         

	} */
	else if(((IIR1 & 0x0E)==0x04)||((IIR1 & 0x0E)==0x0C))
	{  T1_receive_time=0;
	   T1_sendOverTimeValue=0;
	   T1_haveRecData=1;
	  if((U1LSR & 0x01) == 1) // 若U1RBR包含有效数据
	  {
	   	T1_Rec_Buffer[T1_recCount++] = U1RBR; // 保存接收到的数据
	  }             				  
         

	}else
	{
	buffer=U1RBR;
    //buffer=U1LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* 名称：UART1_Ini()
* 功能：初始化串口0。设置为8位数据位，1位停止位，无奇偶校验，波特率为115200
* 入口参数：无
* 出口参数：无
****************************************************************************/
void  UART1_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 4;		          // 打开UART1控制器电源-默认为打开
   
   PINSEL0 |= 0x40000000;       /* TxD1 */
   PINSEL1 |= 0x00000001;       /* RxD1 */
   U1LCR = 0x9B;		            // DLAB = 1，可设置波特率
   Fdiv = (Fpclk / 16) / UART1_BPS;  // 设置波特率
   U1DLM = Fdiv / 256;							
   U1DLL = Fdiv % 256;
   U1LCR &= ~(0x01<<7);
   U1IER |= 0x01; //允许接受
   //U0IER |= 0x02; // 允许THRE(transmit holding register empty)中断，即发送中断
   U1FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // 设置所有通道为IRQ中断
   VICVectPriority7  = 4;
   VICVectAddr7 = (uint32)IRQ_UART1;       // 设置UART1向量地址
   VICIntEnable |= 1<<7;           // 使能UART1中断
}

void T1_check(void)
{
if(T1_YlToJOS2_CalHeartBeatValue<2000)T1_YlToJOS2_CalHeartBeatValue++;
  else{
  //地址1的通讯监控
  T1_YlToJOS2_CalHeartBeatValue=0;
  if(abs(T1_HeartBeatValue-LastT1_HeartBeatValue)>0){uart1NormalFlag=1;}
  else uart1NormalFlag=0;

  LastT1_HeartBeatValue=T1_HeartBeatValue;

 } 
 //检测超时
 if(T1_sendOverTimeCount==1)
 {
  if(T1_sendOverTimeValue<T1_sendOverDelayTime)T1_sendOverTimeValue++; //等待的超时时间
  else{
  T1_sendOverTimeCount=0;
  T1_sendOverTimeValue=0;

  T1_StepSendFlag=1; //超时了，继续发送下一帧
  }

 }
 if(T1_333ms<333)T1_333ms++;
 else{
 T1_333ms = 0;
 QBCReadBuff[0]++;
 }
if(T1_beforeSendCount==1)
  {
   T1_beforeSendValue++;
   if(T1_beforeSendValue>T1_sendDelay)
   {
   	 T1_beforeSendValue=0;
	 T1_beforeSendCount=0;
	 T1_realSend=1;
   }
  }
 if(T1_beforePullBusCount==1)
  {
   T1_beforePullBusValue++;
   if(T1_beforePullBusValue>T1_PullBusDelay) //发送之前等待的时间
   {
   	 T1_beforePullBusValue=0;
	 T1_beforePullBusCount=0;
	  Tx1_EN(1);   //允许发
	 T1_beforeSendCount=1;
   }
  }
 if(T1_ItsRecTime==1)
 { 
      if(T1_receive_time<6)
	  {
	   T1_receive_time++;
	  }
	  
	  if((T1_receive_time>5)&&(T1_haveRecData==1))	 //波特率9600超时为4ms
	  {	
	  //已经过了5ms没发送数据了，而且已经有数据进来了，说明一包结束
	  //接收一包数据完成	-处理接收到的数据
	        T1_haveRecData=0;
			if(T1_recCount>1)
			{
			   T1_haveRecDone = 1;
			   T1_haveRecData = 0;
			 }
			else{
			T1_recCount=0;	 //接收数目置零
			}		
	  }
  }
  if(T1_PermitRecFlag ==1)
  {
  	if(T1_sendComp_time<0)T1_sendComp_time++;	//发送完成等一段时间再释放总线
	else{
		 Tx1_EN(0);//允许收
	     U1IER = 0x01;
		 U1FCR |= 0x02;
         while((U1LSR&0x01)==0x01)
		 //if(((U1IIR & 0x0E)==0x0C)||((U1IIR & 0x0E)==0x04))
		 {	
		   buffer=U1RBR;
		 }	
		 
		 //T1_sendOverTimeCount=1;
		 T1_sendComp_time =0;
		 T1_PermitRecFlag =0;
	}
  }
}

void T1_recCheck()
{
 if(T1_haveRecDone == 1)
{  uint16 i,startAddress,readCount,writeCount,tmpH,tmpL;
	T1_haveRecDone=0;
	T1_sendOverTimeCount=0;//超时计时停止
	T1_sendOverTimeValue=0;
	if((T1_Rec_Buffer[T1_recCount-2]==CRC16(T1_Rec_Buffer,T1_recCount-2,1))&&(T1_Rec_Buffer[T1_recCount-1]==CRC16(T1_Rec_Buffer,T1_recCount-2,0)))
    {if((T1_Rec_Buffer[0]==QBCAddress))
	 { 
	 //if((T1_Rec_Buffer[1]==0x03))T1_YlToJOS2HeartBeatValue=(T1_Rec_Buffer[3]<<8)+T1_Rec_Buffer[4];//正常心跳从buff【7】获取
	 if(T1_Rec_Buffer[1]==0x03)
	 {
	   if((T1_Rec_Buffer[2]==0x02)&&(T1_Rec_Buffer[3]==0x93))
	   {
	   	T1_sendCount = 7;
		T1_Send_Buffer[0]=0x01;
		T1_Send_Buffer[1]=0x03;
		T1_Send_Buffer[2]=2;
		T1_Send_Buffer[3]=0;
		T1_Send_Buffer[4]=(QBCReadBuff[1]<<2);
		T1_Send_Buffer[5]=CRC16(T1_Send_Buffer,5,1);
		T1_Send_Buffer[6]=CRC16(T1_Send_Buffer,5,0);
		T1_StepSendFlag=1; //接收正确，发送下一帧
	   }else if((T1_Rec_Buffer[2]==0x03)&&(T1_Rec_Buffer[3]==0x83))
	   {
	   	T1_sendCount = 9;
		T1_Send_Buffer[0]=0x01;
		T1_Send_Buffer[1]=0x03;
		T1_Send_Buffer[2]=4;
		T1_Send_Buffer[3]=0;
		T1_Send_Buffer[4]=((QBCReadBuff[2]<<0)|(QBCReadBuff[3]<<4));
		T1_Send_Buffer[5]=(switchToAFC_heartBeat>>8);
		T1_Send_Buffer[6]=switchToAFC_heartBeat;
		T1_Send_Buffer[7]=CRC16(T1_Send_Buffer,7,1);
		T1_Send_Buffer[8]=CRC16(T1_Send_Buffer,7,0);
		T1_StepSendFlag=1; //接收正确，发送下一帧
	   }
//	   startAddress = (T1_Rec_Buffer[2]<<8)+T1_Rec_Buffer[3];
//	   readCount = (T1_Rec_Buffer[4]<<8)+T1_Rec_Buffer[5];
//	   if((startAddress >= 0x80)&&(readCount<=9))
//	   {
//	   	 if((startAddress + readCount -1)<=0x88)
//		 {
//			T1_sendCount= readCount*2+5;
//			T1_Send_Buffer[0]=QBCAddress;
//			T1_Send_Buffer[1]=0x03;
//			T1_Send_Buffer[2]=readCount*2;	//
//			for(i=0;i< T1_Send_Buffer[2];i++)
//			{
//			 if(i%2==0)
//			  {
//			  T1_Send_Buffer[i+3]= (QBCReadBuff[(i>>1)+startAddress-0x80])>>8;
//			  }else
//			  {
//			  T1_Send_Buffer[i+3]= QBCReadBuff[(i>>1)+startAddress-0x80];
//			  }
//			}
//			T1_Send_Buffer[T1_Send_Buffer[2]+3]=CRC16(T1_Send_Buffer,readCount*2+3,1);
//			T1_Send_Buffer[T1_Send_Buffer[2]+4]=CRC16(T1_Send_Buffer,readCount*2+3,0);
//		  T1_StepSendFlag=1; //接收正确，发送下一帧
//		 }
//	   }
	 }
	 else if(T1_Rec_Buffer[1]==0x10)
	 {
	        T1_sendCount= 8;
			T1_Send_Buffer[0]=QBCAddress;
			T1_Send_Buffer[1]=0x10;
			T1_Send_Buffer[2]=T1_Rec_Buffer[2];	//写的起始地址41049 =0x0418
			T1_Send_Buffer[3]=T1_Rec_Buffer[3];
			T1_Send_Buffer[4]=T1_Rec_Buffer[4];
			T1_Send_Buffer[5]=T1_Rec_Buffer[5];
			T1_Send_Buffer[6]=CRC16(T1_Send_Buffer,6,1);
			T1_Send_Buffer[7]=CRC16(T1_Send_Buffer,6,0);
			T1_StepSendFlag=1; //接收正确，发送下一帧
	   		if(T1_Rec_Buffer[6]==0x4A)
			{
			 KS1_DelayOn(T1_Rec_Buffer[52]);
			}
	   T1_HeartBeatValue = QBCWriteBuff[0] ;
	 }
	 

	}
	}

	if(T1_StepSendFlag == 0)
	{
	 T1_haveRecData=0;
	 T1_recCount=0;
	 T1_receive_time=0;//继续接收
	}
	
	
}

}
/************************************************************************



*************************************************************************/
void LoadT1ReadHeartBuff()
{
    T1_sendCount= 8;
    T1_Send_Buffer[0]=1;
	T1_Send_Buffer[1]=0x03;
	T1_Send_Buffer[2]=0x06;	//读的起始地址41662 =0x067D	01 03 06 7D 00 01 14 9A
	T1_Send_Buffer[3]=0x7D;
	T1_Send_Buffer[4]=0;
	T1_Send_Buffer[5]=1;
	T1_Send_Buffer[6]=CRC16(T1_Send_Buffer,6,1);
	T1_Send_Buffer[7]=CRC16(T1_Send_Buffer,6,0);
	
}
void LoadT1WriteHeartBuff()
{
    T1_sendCount= 11;
    T1_Send_Buffer[0]=1;
	T1_Send_Buffer[1]=0x10;
	T1_Send_Buffer[2]=0x04;	//写的起始地址41049 =0x0418
	T1_Send_Buffer[3]=0x18;
	T1_Send_Buffer[4]=0;
	T1_Send_Buffer[5]=1;
	T1_Send_Buffer[6]=2;
	T1_Send_Buffer[7]=switchToAFC_heartBeat>>8;
	T1_Send_Buffer[8]= switchToAFC_heartBeat;//心跳
	T1_Send_Buffer[9]=CRC16(T1_Send_Buffer,9,1);
	T1_Send_Buffer[10]=CRC16(T1_Send_Buffer,9,0);
	
}
void LoadT1SendDataBuff(uint16 *pJosBuff,uint16 startAddr,uint16 beginAddr,uint8 wordLen)
{    uint8 i,j,josbeginAddr;
    josbeginAddr=startAddr-(beginAddr-40001); 
    T1_sendCount= 9+wordLen*2;
    T1_Send_Buffer[0]=1;
	T1_Send_Buffer[1]=0x10;//写的起始地址41001
	T1_Send_Buffer[2]=startAddr>>8;
	T1_Send_Buffer[3]=startAddr;
	T1_Send_Buffer[4]=0x00;
	T1_Send_Buffer[5]=wordLen;
	T1_Send_Buffer[6]=wordLen*2;
	for(i=0;i<(wordLen*2);i++)
	{
	  if(i%2==0)
	  {
	  T1_Send_Buffer[i+7]= (*(pJosBuff+(i>>1)+josbeginAddr))>>8;
	  }else
	  {
	  T1_Send_Buffer[i+7]= (*(pJosBuff+(i>>1)+josbeginAddr));
	  }
	}
	T1_Send_Buffer[7+(wordLen*2)]=CRC16(T1_Send_Buffer,7+(wordLen*2),1);
	T1_Send_Buffer[8+(wordLen*2)]=CRC16(T1_Send_Buffer,7+(wordLen*2),0);
}

//发送步骤，先发01 再发 02 最后发 03
// 01 41050-41234 共185个字，分4次发 50 50 50 35      应用步骤1,2,3,4
// 02 41240-41355 共116个字，分3次发 40 40 36	      应用步骤5,6,7
// 03 41360-41579 共220个字，分5次发 50 50 50 50 20   应用步骤8,9,10,11,12
void UART1_SendData()
{ //T1_WriteSequence    1 发送读心跳 2 发送写心跳 3 发送数据
   uint8 i;

 if(T1_StepSendFlag==1)
  {	 T1_StepSendFlag=0;
     
	 T1_beforePullBusCount=1;
     //Tx1_EN(1);   //允许发
	 //T1_beforeSendCount=1;
  }

if(T1_realSend==1)
{
    T1_realSend=0;
    T1_sendIndex=1; //发送从数据的第一个发
	T1_recCount=0;	 //接收数目置零

	U1IER = 0x02;
    U1THR = T1_Send_Buffer[0];
}
}
