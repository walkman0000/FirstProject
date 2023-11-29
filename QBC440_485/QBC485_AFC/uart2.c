/****************************************************************************
* 文件名：UART2（主） ---JOS 查询控制信息，写入关键数据---03 -10功能
* 功能：向串口发送数据。
* 说明：将跳线器JP5短接；
*      使用外部11.0592MHz晶振，根据CONFIG.H文件配置，Fpclk=11.0592MHz；
*      通讯波特率115200，8位数据位，1位停止位，无奇偶校验。
****************************************************************************/
#include  "config.h"
#define ReadFunction  0
#define WriteFunction  1
#define T2_sendDelay 10 //单位ms
#define T2_PullBusDelay 100
#define   switchToAFCdataBuff_WoedLen 38
//T2定义
uint8 T2_Rec_Buffer[50],T2_Send_Buffer[200],T2_Send_Length, T2_sendCount=0,T2_sendIndex=0,T2_haveRecData=0,T2_recCount=0,T2_PermitRecFlag=0;
//T2定义
uint16 T2_receive_time=0,T2_sendComp_time=0;
uint8 T2_haveRecDone=0,T2_sendOverTimeCount,T2_beforeSendCount=0,T2_realSend=0,T2_ItsRecTime=0;
uint16 T2_sendOverTimeValue=0,T2_beforeSendValue;
uint8 T2_StepSend,T2_sendType;
uint8 switchToAFCdataBuff[switchToAFCdataBuff_WoedLen*2];
uint8 EstablishHealthyFlag,EstablishHealthyValue;
uint8 uart2NormalFlag,uart2HaveNormaledFlag,LL_AFCselectedDriverStarted;
uint8 EstablishHealthyFlagHaveGived,T2_beforePullBusCount;
uint16 T2_JosToYl_CalHeartBeatValue,T2_beforePullBusValue,switchToAFC_heartBeat,JosToYlHeartBeatValue;
uint16 LastJosToYlHeartBeatValue;
#define  UART2_BPS	9600			/* 定义通讯波特率 */

/****************************************************************************
* 名称：IRQ_UART2()
* 功能：UART2的中断服务程序
* 入口参数：无
* 出口参数：无
****************************************************************************/
 void __irq IRQ_UART2(void)                           // 中断服务程序
{  
   	uint32 IIR2=0;
	IIR2=U2IIR;
    if ((IIR2 & 0x0E) == 0x02)                   // 判断是否为发送中断
    {	 
		  if (T2_sendIndex !=  T2_sendCount )
          {
                U2THR =  T2_Send_Buffer[T2_sendIndex];
                T2_sendIndex ++;
          }else	
		  {
		  
		  //U2FCR |= 0x07; 
		  //buffer=U2RBR;
		  T2_PermitRecFlag=1;
		  
		  }	  
    }
	/*else if((U2IIR & 0x0E)==0x0C)
	{
		  
		T2_receive_time=0;
		T2_haveRecData=1;
	  if((U2LSR & 0x01) == 1) // 若U2RBR包含有效数据
	  {
	   	T2_Rec_Buffer[T2_recCount++] = U2RBR; // 保存接收到的数据
	  }             				  

	}  */
	else if(((IIR2 & 0x0E)==0x04)||((IIR2 & 0x0E)==0x0C))
	//else if((U2IIR & 0x0E)==0x04)
	{  
	   T2_receive_time=0;
	   T2_haveRecData=1;
	  if((U2LSR & 0x01) == 1) // 若U2RBR包含有效数据
	  {
	   	T2_Rec_Buffer[T2_recCount++] = U2RBR; // 保存接收到的数据
	  }             				  
         

	}else
	{
	buffer=U2RBR;
   // buffer=U2LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* 名称：UART2_Ini()
* 功能：初始化串口0。设置为8位数据位，1位停止位，无奇偶校验，波特率为115200
* 入口参数：无
* 出口参数：无
****************************************************************************/
void  UART2_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 3;		          // 打开UART0控制器电源-默认为打开
   PCONP   |=   0x01 << 24;		          // 打开UART2控制器电源-默认为关闭
   PINSEL0 |= 0x00500000;       /* RxD2 and TxD2 */
   U2LCR = 0x9B;		            // DLAB = 1，可设置波特率,设置校验	，偶校验
   Fdiv = (Fpclk / 16) / UART2_BPS;  // 设置波特率
   U2DLM = Fdiv / 256;							
   U2DLL = Fdiv % 256;
   U2LCR &= ~(0x01<<7);
   U2IER |= 0x01; //允许接受
   //U2IER |= 0x02; // 允许THRE(transmit holding register empty)中断，即发送中断
   U2FCR = 0x07;		/* Enable and reset TX and RX FIFO. 触发深度为1*/
   VICIntSelect = 0x00000000;           // 设置所有通道为IRQ中断
   VICVectPriority28  = 5;
   VICVectAddr28 = (uint32)IRQ_UART2;       // 设置UART2向量地址
   VICIntEnable |= 1<<28;           // 使能UART2中断
}

void T2_check(void)
{
if(T2_beforeSendCount==1)
  {
   T2_beforeSendValue++;
   if(T2_beforeSendValue>T2_sendDelay*10) //发送之前等待的时间
   {
   	 T2_beforeSendValue=0;
	 T2_beforeSendCount=0;
	 T2_realSend=1;
   }
  }
  if(T2_beforePullBusCount==1)
  {
   T2_beforePullBusValue++;
   if(T2_beforePullBusValue>T2_PullBusDelay*10) //发送之前等待的时间
   {
   	 T2_beforePullBusValue=0;
	 T2_beforePullBusCount=0;
	 Tx2_EN(1);   //允许发
	 T2_beforeSendCount=1;
   }
  }
  //华宁的启动信号在给完之后，所选的接触器启动之后，就需要重新关闭
  if((EstablishHealthyFlagHaveGived==1)&&(LL_AFCselectedDriverStarted==1)){KS3_DelayOn(0);}

  //计算1s内通讯次数递增两次以上
  if(T2_JosToYl_CalHeartBeatValue<20000)T2_JosToYl_CalHeartBeatValue++;
  else{
   if((uart2HaveNormaledFlag==1)&&(EstablishHealthyFlag==0)){
   if(EstablishHealthyValue<5)EstablishHealthyValue++;	//通讯保持10s以上才会给出健康信号
   else{EstablishHealthyFlag=1;EstablishHealthyValue=0;}
   }

  T2_JosToYl_CalHeartBeatValue=0;
  if(abs(JosToYlHeartBeatValue-LastJosToYlHeartBeatValue)>0)
  {
  uart2NormalFlag=1;
  uart2HaveNormaledFlag=1;
  }
  else {
  uart2NormalFlag=0;
  }
  LastJosToYlHeartBeatValue=JosToYlHeartBeatValue;
  }
  //发送完成后等待接收，300ms即为超时
if(T2_sendOverTimeCount==1)
 {
  if(T2_sendOverTimeValue<3000)T2_sendOverTimeValue++; //超时等待300ms//波特率9600超时为2000//波特率4800超时为4000
  else{
  T2_sendOverTimeCount=0;
  T2_sendOverTimeValue=0;
  //SendSameTimes++; //超时了，同一帧需要发送
  T2_StepSend=1;
  }

 }
 if(T2_ItsRecTime==1)
 { 
      if(T2_receive_time<55)
	  {
	   T2_receive_time++;
	  }
	  
	  if((T2_receive_time>50)&&(T2_haveRecData==1))	 //波特率9600超时为4ms
	  {	
	  //已经过了5ms没发送数据了，而且已经有数据进来了，说明一包结束
	  //接收一包数据完成	-处理接收到的数据
	        T2_haveRecData=0;
			if(T2_recCount>1)
			{
			   T2_haveRecDone = 1;
			   T2_haveRecData = 0;
			   	//reverse_light();
			 }
			else{
			T2_recCount=0;	 //接收数目置零
			}		
	  }
  }
  if(T2_PermitRecFlag ==1)
  {
  	if(T2_sendComp_time<0)T2_sendComp_time++;	//发送完成等一段时间再释放总线
	else{
		 Tx2_EN(0);//允许收
	     U2IER = 0x01;

		 while((U2LSR&0x01)==0x01)
		 //if(((U2IIR & 0x0E)==0x0C)||((U2IIR & 0x0E)==0x04))
		 {	
		   buffer=U2RBR;
		 }	
		  
		 T2_sendOverTimeCount = 1;//开始接收的时候就超时计时开始
		 T2_sendComp_time = 0;
		 T2_PermitRecFlag = 0;
	}
  }
}
void UART2_SendData()
{
   uint8 i;
 if(T2_StepSend==1)
{	 T2_StepSend=0;
     //Tx2_EN(1);   //允许发
	 //T2_beforeSendCount=1;
	 T2_beforePullBusCount=1; //延时一段时间再拉总线
}
  if(T2_realSend==1)
 {
    T2_realSend=0;
    T2_sendIndex=1; //发送从数据的第一个发
	T2_recCount=0;	 //接收数目置零
	
	if(T2_sendType==ReadFunction)
	{T2_sendCount=8;	 //01 03 02 93 00 06 34 5D
	T2_Send_Buffer[0]=1;
	T2_Send_Buffer[1]=3;
	T2_Send_Buffer[2]=0x06;	  //41662 1661=0x067D
	T2_Send_Buffer[3]=0x7D;
	T2_Send_Buffer[4]=0x00;
	T2_Send_Buffer[5]=0x08;
	T2_Send_Buffer[6]=CRC16(T2_Send_Buffer,6,1);
	T2_Send_Buffer[7]=CRC16(T2_Send_Buffer,6,0);
	T2_sendType=WriteFunction;
	}else{ 
	T2_sendType=ReadFunction;
	T2_sendCount=9+switchToAFCdataBuff_WoedLen*2;	 //01 03 02 93 00 06 34 5D
	T2_Send_Buffer[0]=1;
	T2_Send_Buffer[1]=0x10;//写的起始地址41001
	T2_Send_Buffer[2]=0x03;
	T2_Send_Buffer[3]=0xE8;
	T2_Send_Buffer[4]=0x00;
	T2_Send_Buffer[5]=switchToAFCdataBuff_WoedLen;
	T2_Send_Buffer[6]=switchToAFCdataBuff_WoedLen*2;
	switchToAFCdataBuff[72]= (switchToAFC_heartBeat>>8) ;
	switchToAFCdataBuff[73]= switchToAFC_heartBeat ;
	Load_switchToAFCdataBuff();
	for(i=0;i<(switchToAFCdataBuff_WoedLen*2);i++)
	{
	  T2_Send_Buffer[7+i]= switchToAFCdataBuff[i];
	}
	T2_Send_Buffer[7+(switchToAFCdataBuff_WoedLen*2)]=CRC16(T2_Send_Buffer,7+(switchToAFCdataBuff_WoedLen*2),1);
	T2_Send_Buffer[8+(switchToAFCdataBuff_WoedLen*2)]=CRC16(T2_Send_Buffer,7+(switchToAFCdataBuff_WoedLen*2),0);
	}
	
	U2IER = 0x02;
    U2THR = T2_Send_Buffer[0];
 }

 
}
void T2_recCheck()
{ 
//uint8 tmpBit1,tmpBit0;
if((uart2NormalFlag==0)&&(uart2HaveNormaledFlag==1))
  {uart2HaveNormaledFlag=0;
   EstablishHealthyFlag=0;
   EstablishHealthyFlagHaveGived=0;
   KS4_DelayOn(0);
   KS5_DelayOn(0);
   KS6_DelayOn(0);
   KS7_DelayOn(0);
   UpdateNewJOSdata(); 
  }
 if(T2_haveRecDone == 1)
{  uint16 i,startAddress;
	T2_haveRecDone=0;
	

	if((T2_Rec_Buffer[T2_recCount-2]==CRC16(T2_Rec_Buffer,T2_recCount-2,1))&&(T2_Rec_Buffer[T2_recCount-1]==CRC16(T2_Rec_Buffer,T2_recCount-2,0)))
    { 
	 
	 if(T2_Rec_Buffer[0]==(0x01))
	 {
	 if((T2_Rec_Buffer[1]==0x03)&&(T2_Rec_Buffer[2]==0x10))
	 { //JOS发给YL通讯板的控制信息	 01 03 10 00 5F 00 00 00 01 00 00 00 07 00 07 00 01 00 01 AC 9D
	  //if(uart0NormalFlag2==1){} //只有当和3300通讯正常才允许控制
	   JosToYlHeartBeatValue=(T2_Rec_Buffer[3]<<8)+T2_Rec_Buffer[4]; //JOS1的心跳
	   KS2_DelayOn(T2_Rec_Buffer[16]&0x01);// PSA 破碎机
	   KS1_DelayOn(T2_Rec_Buffer[18]&0x01);	// ZZA 转载机
	   newAFC_Drives_Selected=T2_Rec_Buffer[12];
	   //LLswitchDrivesSelected = 7;	//测试联力开关的刮板机选择
	   if((newAFC_Drives_Selected!=0)&&(newAFC_Drives_Selected!=LLswitchDrivesSelected)&&(LLswitchDrivesSelected==0)){	
	   //是作为备用模式启动各回路
	   KS7_DelayOn(1);
	   if(T2_Rec_Buffer[8]>0){//断延时
	   KS4_DelayOn(0);
       KS5_DelayOn(0);
       KS6_DelayOn(0);
	   } else{
	   //此处可能依托于 joy的启动信号的保持 
	   KS4_DelayOn(T2_Rec_Buffer[14]&0x01);
       KS5_DelayOn(T2_Rec_Buffer[14]&0x02);
       KS6_DelayOn(T2_Rec_Buffer[14]&0x04);
	   }
	  
	   }else if((newAFC_Drives_Selected==LLswitchDrivesSelected)&&(newAFC_Drives_Selected>0)){
	   KS7_DelayOn(0);
	   	  	KS4_DelayOn(LLswitchDrivesSelected&0x01);
      		KS5_DelayOn(LLswitchDrivesSelected&0x02);
      		KS6_DelayOn(LLswitchDrivesSelected&0x04);
	   }else if((newAFC_Drives_Selected==LLswitchDrivesSelected)&&(newAFC_Drives_Selected==0)){
	   KS7_DelayOn(0);
	   }
	   else if(newAFC_Drives_Selected!=LLswitchDrivesSelected)
	   {//KS7_DelayOn(0);
	   	KS4_DelayOn(0);
        KS5_DelayOn(0);
        KS6_DelayOn(0);
	   }	
	   if(EstablishHealthyFlag==1){//通讯已经保持一段时间，可以给启动信号了
	   //当所选电机和需要闭合的电机一致时 
	   if((T2_Rec_Buffer[12]==T2_Rec_Buffer[14])&&(T2_Rec_Buffer[14]>0)&&(EstablishHealthyFlagHaveGived==0)){
	   KS3_DelayOn(1);// 华宁控制启动信号模拟
	   EstablishHealthyFlagHaveGived=1;
	   }
	   
	   }
	   newAFCDelayShutDown=T2_Rec_Buffer[8/*3+5*2-1*/];//40664(40660-40665)
	  if(newAFCDelayShutDown==1){KS3_DelayOn(0);EstablishHealthyFlagHaveGived=0;EstablishHealthyFlag=0;}// 华宁控制启动信号模拟
	 }
	 if(T2_Rec_Buffer[1]==0x10)
	 {
	  //JOS通知YL控制信息已经收到
	 }
	 T2_sendOverTimeValue = 0;//超时计时值从0开始---因为已经收到了正确的数据了。
	 T2_StepSend=1;//接收成功完成，继续马上发送
	}else
	{
	 T2_haveRecData=0;
	 T2_recCount=0;
	 T2_receive_time=0;//继续接收
	}
	}else
	{
	 T2_haveRecData=0;
	 T2_recCount=0;
	 T2_receive_time=0;//继续接收
	}
	
}
}
void Load_switchToAFCdataBuff()
{  
  //AFC_MGBP_U=1234;
  //AFC_MGEP_I=56;
  switchToAFCdataBuff[0]=AFC_MGBP_I>>8;
  switchToAFCdataBuff[1]=AFC_MGBP_I;   //41001
  switchToAFCdataBuff[4]=AFC_MGBP_U>>8;
  switchToAFCdataBuff[5]=AFC_MGBP_U;  //41003
  switchToAFCdataBuff[6]=AFC_MGEP_I>>8;
  switchToAFCdataBuff[7]=AFC_MGEP_I;   //41004
  switchToAFCdataBuff[10]=AFC_MGEP_U>>8;
  switchToAFCdataBuff[11]=AFC_MGEP_U;  //41006
  switchToAFCdataBuff[12]=AFC_TG_I>>8;
  switchToAFCdataBuff[13]=AFC_TG_I;   //41007
  switchToAFCdataBuff[16]=AFC_TG_U>>8;
  switchToAFCdataBuff[17]=AFC_TG_U;  //41009
  switchToAFCdataBuff[18]=BSL_Motor_I>>8;
  switchToAFCdataBuff[19]=BSL_Motor_I;   //41010
  switchToAFCdataBuff[22]=BSL_Motor_U>>8;
  switchToAFCdataBuff[23]=BSL_Motor_U;  //41012
  switchToAFCdataBuff[24]=Crusher_Motor_I>>8;
  switchToAFCdataBuff[25]=Crusher_Motor_I;   //41013
  switchToAFCdataBuff[28]=Crusher_Motor_U>>8;
  switchToAFCdataBuff[29]=Crusher_Motor_U;  //41015
  switchToAFCdataBuff[30]=Shearer_I>>8;
  switchToAFCdataBuff[31]=Shearer_I;   //41016
  switchToAFCdataBuff[34]=Shearer_U>>8;
  switchToAFCdataBuff[35]=Shearer_U;  //41018
  //switchToAFCdataBuff[48]=AFC_breakStat_MGBP>>8;
  switchToAFCdataBuff[49]=1;//AFC_breakStat_MGBP;   //41025
  //switchToAFCdataBuff[50]=AFC_breakStat_MGEP>>8;
  switchToAFCdataBuff[51]=1;//AFC_breakStat_MGEP;  //41026
  //switchToAFCdataBuff[52]=AFC_breakStat_TG>>8;
  switchToAFCdataBuff[53]=1;//AFC_breakStat_TG;   //41027
  //switchToAFCdataBuff[54]=BSL_breakStat>>8;
  switchToAFCdataBuff[55]=1;//BSL_breakStat;  //41028 ???????????????????????????????????????????????
  //switchToAFCdataBuff[56]=Crusher_breakStat>>8;
  switchToAFCdataBuff[57]=1;//Crusher_breakStat;   //41029
  //switchToAFCdataBuff[58]=Shearer_breakStat>>8;
  switchToAFCdataBuff[59]=1;//Shearer_breakStat;  //41030
  //switchToAFCdataBuff[60]=Crusher_Closed>>8;
  switchToAFCdataBuff[61]=Crusher_Closed;   //41031
  //switchToAFCdataBuff[64]=BSL_Closed>>8;
  switchToAFCdataBuff[65]=BSL_Closed;  //41033
  //switchToAFCdataBuff[66]=MGBP_Closed>>8;
  switchToAFCdataBuff[67]=MGEP_Closed;//1;   //41034
  //switchToAFCdataBuff[68]=MGEP_Closed>>8;
  switchToAFCdataBuff[69]=MGBP_Closed;  //41035
  //switchToAFCdataBuff[70]=TG_Closed>>8;
  switchToAFCdataBuff[71]=TG_Closed;// 1;  //41036
  //switchToAFCdataBuff[72]=AFC_MGEP_U>>8;
  //switchToAFCdataBuff[73]=AFC_MGEP_U;  //41037 心跳
  //switchToAFCdataBuff[74]=0;
  switchToAFCdataBuff[75]=LLswitchDrivesSelected;  //41038 心跳

}
