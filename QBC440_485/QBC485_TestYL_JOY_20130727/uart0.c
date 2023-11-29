/****************************************************************************
* �ļ�����UART0������ ---LLswitchgear 01 02 03 ��ַ��03���ݲ�ѯ
* ���ܣ��򴮿ڷ������ݡ�
* ˵������������JP5�̽ӣ�
*      ʹ���ⲿ11.0592MHz���񣬸���CONFIG.H�ļ����ã�Fpclk=11.0592MHz��
*      ͨѶ������115200��8λ����λ��1λֹͣλ������żУ�顣
****************************************************************************/
#include  "config.h"

#define T0_sendDelay  10 //��λ���� T0����ǰ�����ߵ�ʱ��
#define T0_PullBusDelay 5
#define T0_sendOverDelayTime  50 //��λ���� T0�������֮��ȴ����յ�ʱ��
#define T0_SameFrameSendCount  1
//uint8 JudgeOneFrameTime;
uint8 ToBeQueryAddress=0x01,EnsureQueryAddress=0x01;
uint8 send[9]={0xfe,0x1,0x0,0x5,0xd1,0xb6,0xb7,0xc9,0x41};
uint8 buffer,SendNextFlag/*������һ�η��͵ı�־*/,SendSameTimes/*ͬһ��ַͬһ֡�Ѿ����͵Ĵ���*/;
//T0����
uint8 T0_Rec_Buffer[200],T0_Send_Length, T0_sendCount=0,T0_sendIndex=0,T0_haveRecData=0,T0_recCount=0,T0_PermitRecFlag=0,T0_ItsRecTime=0;	 
uint8 T0_Send_Buffer[10],T0_Need_reply=0,T0_sendOverTimeCount,T0_beforeSendCount=0,T0_realSend=0;
uint16 T0_sendOverTimeValue;
uint16 DataBuffer1[120],DataBuffer2[100],DataBuffer3[100];
uint8 T0_SendFrameNum=1;//ͬһ����ַ�ĵڼ�֡��־
//T0����
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
* ���ƣ�DelayNS()
* ���ܣ��������ʱ
* ��ڲ�����dly		��ʱ������ֵԽ����ʱԽ��
* ���ڲ�������
****************************************************************************/
void  DelayNS(uint32  dly)
{  uint32  i;

   for(; dly>0; dly--) 
      for(i=0; i<50000; i++);
}


#define  UART0_BPS	9600			/* ����ͨѶ������ */
/****************************************************************************
* ���ƣ�IRQ_UART0()
* ���ܣ�UART0���жϷ������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
 void __irq IRQ_UART0(void)                           // �жϷ������
{  
   	uint32 IIR0=0;
	IIR0=U0IIR;
    if ((IIR0 & 0x0E) == 0x02)                   // �ж��Ƿ�Ϊ�����ж�
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
	  if((U0LSR & 0x01) == 1) // ��U0RBR������Ч����
	  {
	   	T0_Rec_Buffer[T0_recCount++] = U0RBR; // ������յ�������
	  }             				  
         

	}*/ 
	else if(((IIR0 & 0x0E)==0x04)||((IIR0 & 0x0E)==0x0C))
	//else if((U0IIR & 0x0E)==0x04)
	{  //reverse_light();   
	   T0_receive_time=0;
	   T0_sendOverTimeValue=0;
	   T0_haveRecData=1;
	  if((U0LSR & 0x01) == 1) // ��U0RBR������Ч����
	  {
	   	T0_Rec_Buffer[T0_recCount++] = U0RBR; // ������յ�������
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
* ���ƣ�UART0_Ini()
* ���ܣ���ʼ������0������Ϊ8λ����λ��1λֹͣλ������żУ�飬������Ϊ115200
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void  UART0_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 3;		          // ��UART0��������Դ-Ĭ��Ϊ��
   //PCONP   |=   0x01 << 24;		          // ��UART0��������Դ-Ĭ��Ϊ�ر�
   PINSEL0 |= 0x00000050;       /* RxD0 and TxD0 */
   U0LCR = 0x9B;		            // DLAB = 1�������ò�����
   Fdiv = (Fpclk / 16) / UART0_BPS;  // ���ò�����	   4800 =0x00D8	  9600 =0x006C
   U0DLM = Fdiv / 256;	 //4800 =0x0000	  9600 =0x0000						
   U0DLL = Fdiv % 256;	 //4800 =0x00D8	  9600 =0x006C
   U0LCR &= ~(0x01<<7);
   U0IER |= 0x01; //�������
   //U0IER |= 0x02; // ����THRE(transmit holding register empty)�жϣ��������ж�
   U0FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // ��������ͨ��ΪIRQ�ж�
   VICVectPriority6  = 3;
   VICVectAddr6 = (uint32)IRQ_UART0;       // ����UART0������ַ
   VICIntEnable |= 1<<6;           // ʹ��UART0�ж�
}
void  UART0_IniByBuad(uint16 myBuad)
{  uint16 Fdiv;
   VICIntEnable &= (~(1<<6));           // ��ֹUART0�ж�
   if(myBuad==9600)Fdiv=0x006C;
   else Fdiv=0x00D8;
   //PCONP   |=   0x01 << 3;		          // ��UART0��������Դ-Ĭ��Ϊ��
   //PCONP   |=   0x01 << 24;		          // ��UART0��������Դ-Ĭ��Ϊ�ر�
   PINSEL0 |= 0x00000050;       /* RxD0 and TxD0 */
   U0LCR = 0x9B;		            // DLAB = 1�������ò�����
   //Fdiv = (Fpclk / 16) / UART0_BPS;  // ���ò�����	   4800 =0x00D8	  9600 =0x006C
   U0DLM = Fdiv / 256;	 //4800 =0x0000	  9600 =0x0000						
   U0DLL = Fdiv % 256;	 //4800 =0x00D8	  9600 =0x006C
   U0LCR &= ~(0x01<<7);
   U0IER |= 0x01; //�������
   //U0IER |= 0x02; // ����THRE(transmit holding register empty)�жϣ��������ж�
   U0FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   //VICIntSelect = 0x00000000;           // ��������ͨ��ΪIRQ�ж�
   //VICVectPriority6  = 3;
   //VICVectAddr6 = (uint32)IRQ_UART0;       // ����UART0������ַ
   VICIntEnable |= 1<<6;           // ʹ��UART0�ж�
}
/****************************************************************************
* ���ƣ� CRC16
* ���ܣ��򴮿�UART0�����ַ���"Hello World!"
****************************************************************************/
unsigned char CRC16( unsigned char * puchMsg,unsigned char usDataLen,unsigned char byte_flag)        // �õ�CRC�ĸ��ֽ� (byte_flag == 1)
{															         // ���ֽڣ�byte_flag == 1��
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
 }//������
 else
 { IOPIN0=(IOSET0&(~(1<<9)))|(0x01 << 9);//IOSET0 =  IOSET0   | (0x01 << 9); //����
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
 } //������
 else {
 T1_ItsRecTime=0;
 IOPIN1=(IOSET1&(~(1<<12)))|(0x01 << 12);
 //IOSET1 =  IOSET1   | (0x01 << 12); //����
 }
}

void Tx2_EN(uint8 flag)
{
 if(flag==0)
 {
 T2_ItsRecTime=1;
 T2_haveRecData=0 ;
 T2_recCount=0;
 IOCLR0 |= 0x0001	;  //������
 }
 else 
 {
 IOPIN0=(IOSET0&(~(1)))|(1);
 //IOSET0 =  IOSET0   | (0x0001);  //����
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
 FIO2CLR |= 1<<22	;  //������
 }
 else 
 {
 
 FIO2PIN=(FIO2SET&(~(1<<22)))|(0x01 << 22);//FIO2SET =  FIO2SET   | (0x01 << 22); //����
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
{ //����������������ص�ͨѶ״̬
  if(T0_YlToLL_CalHeartBeatValue<50000)T0_YlToLL_CalHeartBeatValue++;  //5s����Ѳ��һ����̨���� 
  else{
  //��ַ1��ͨѶ���
  T0_YlToLL_CalHeartBeatValue=0;
  if(abs(T0_YlToLLHeartBeatValue1-LastT0_YlToLLHeartBeatValue1)>0){uart0NormalFlag1=1;uart0HaveNormaledFlag1=1;}
  else uart0NormalFlag1=0;
  LastT0_YlToLLHeartBeatValue1=T0_YlToLLHeartBeatValue1;
  //��ַ2��ͨѶ���
  if(abs(T0_YlToLLHeartBeatValue2-LastT0_YlToLLHeartBeatValue2)>0){uart0NormalFlag2=1;uart0HaveNormaledFlag2=1;}
  else uart0NormalFlag2=0;
  LastT0_YlToLLHeartBeatValue2=T0_YlToLLHeartBeatValue2;
  if(MonitorAddr3Value<3)MonitorAddr3Value++;	//ÿ��4S���һ�µ�ַ3��ͨѶ���
   else{MonitorAddr3Value=0;
   //��ַ3��ͨѶ���
  if(abs(T0_YlToLLHeartBeatValue3-LastT0_YlToLLHeartBeatValue3)>0){uart0NormalFlag3=1;uart0HaveNormaledFlag3=1;}
  else uart0NormalFlag3=0;
  LastT0_YlToLLHeartBeatValue3=T0_YlToLLHeartBeatValue3;
   }
  
  }

 //����ǰ����ʱ
 if(T0_beforeSendCount==1)
  {
   T0_beforeSendValue++;
   if(T0_beforeSendValue>T0_sendDelay*10) //����֮ǰ����������ʱ
   {
   	 T0_beforeSendValue=0;
	 T0_beforeSendCount=0;
	 T0_realSend=1;
   }
  }
 if(T0_beforePullBusCount==1)
  {
   T0_beforePullBusValue++;
   if(T0_beforePullBusValue>T0_PullBusDelay*10) //����֮ǰ�ȴ���ʱ��
   {
   	 T0_beforePullBusValue=0;
	 T0_beforePullBusCount=0;
	  Tx0_EN(1);   //����
	 T0_beforeSendCount=1;
   }
  }
  //�������ݵ���ʱ��֡
 if(T0_ItsRecTime==1)
 { 
      if(T0_receive_time<168)T0_receive_time++;
	  if((T0_receive_time>165)&&(T0_haveRecData==1))	 //������9600��ʱΪ4ms =49//������4800��ʱΪ8ms=165	JudgeOneFrameTime
	  {	
	  //�Ѿ�����5msû���������ˣ������Ѿ������ݽ����ˣ�˵��һ������
	  //����һ���������	-������յ�������
	        if(T0_recCount>3)
			{
			   T0_haveRecDone = 1;
			   T0_haveRecData=0;
			 }
			else{
			T0_recCount=0;	 //������Ŀ����
			}		
	  }
  }
  if(T0_PermitRecFlag ==1) //������9600��ʱΪ20//������4800��ʱΪ20
  {
  	if(T0_sendComp_time<20)T0_sendComp_time++;	//������ɵ�һ��ʱ�����ͷ�����
	else{
	     //reverse_light();
		 Tx0_EN(0);//������
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
	 T0_receive_time=0;//��������
	}

}

}  


void UART0_SendData()
{

	 if(T0_realSend==1) //����ǰ������һ��ʱ��
	 {T0_realSend=0;
    T0_sendIndex=1; //���ʹ����ݵĵ�һ����
	T0_recCount=0;	 //������Ŀ����

	U0IER = 0x02;
    U0THR = T0_Send_Buffer[0];
   	  }
}
void QueryTheAddress()
{ 
  Load_sendBuffer(ToBeQueryAddress,128,2);
}

