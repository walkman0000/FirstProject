/****************************************************************************
* �ļ�����UART1������ ---JOS �ϴ���̨���ص�����---10����
* ���ܣ��򴮿ڷ������ݡ�
* ˵������������JP5�̽ӣ�
*      ʹ���ⲿ11.0592MHz���񣬸���CONFIG.H�ļ����ã�Fpclk=11.0592MHz��
*      ͨѶ������115200��8λ����λ��1λֹͣλ������żУ�顣
****************************************************************************/
#include  "config.h"
#define T1_sendDelay 10 //��λms
#define T1_PullBusDelay 20
#define T1_sendOverDelayTime 30 //��λms
uint8 T1_StepSendFlag/*������һλ�ı�־*/,T1_StepSendNum=0/*д���ݵĽ׶δ���*/,T1_WriteSequence/*д���͵�˳��*/;
//T1����
uint8 T1_Rec_Buffer[80],T1_Send_Buffer[130],T1_Send_Length, T1_sendCount=0,T1_sendIndex=0,T1_haveRecData=0,T1_recCount=0,T1_PermitRecFlag=0;
//T1����
uint16 T1_receive_time=0,T1_sendComp_time=0;
uint8 T1_haveRecDone=0,T1_sendOverTimeCount,T1_beforeSendCount=0,T1_realSend=0,T1_ItsRecTime=0;
uint16 T1_sendOverTimeValue=0,T1_beforeSendValue,T1_YlToJOS2_CalHeartBeatValue,T1_beforePullBusValue;
uint8 uart1NormalFlag,uart1HaveNormaledFlag,T1_beforePullBusCount;
uint16 LastT1_HeartBeatValue,T1_HeartBeatValue;
#define  UART1_BPS	9600			/* ����ͨѶ������ */
#define QBCAddress 1
uint16 QBCReadBuff[9],QBCWriteBuff[9],T1_333ms;
/****************************************************************************
* ���ƣ�DelayNS()
* ���ܣ��������ʱ
* ��ڲ�����dly		��ʱ������ֵԽ����ʱԽ��
* ���ڲ�������
****************************************************************************/

/****************************************************************************
* ���ƣ�IRQ_UART1()
* ���ܣ�UART1���жϷ������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
 void __irq IRQ_UART1(void)                           // �жϷ������
{  
   	uint32 IIR1=0;
	IIR1=U1IIR;
    if ((IIR1 & 0x0E) == 0x02)                   // �ж��Ƿ�Ϊ�����ж�
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
	  if((U1LSR & 0x01) == 1) // ��U1RBR������Ч����
	  {
	   	T1_Rec_Buffer[T1_recCount++] = U1RBR; // ������յ�������
	  }             				  
         

	} */
	else if(((IIR1 & 0x0E)==0x04)||((IIR1 & 0x0E)==0x0C))
	{  T1_receive_time=0;
	   T1_sendOverTimeValue=0;
	   T1_haveRecData=1;
	  if((U1LSR & 0x01) == 1) // ��U1RBR������Ч����
	  {
	   	T1_Rec_Buffer[T1_recCount++] = U1RBR; // ������յ�������
	  }             				  
         

	}else
	{
	buffer=U1RBR;
    //buffer=U1LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* ���ƣ�UART1_Ini()
* ���ܣ���ʼ������0������Ϊ8λ����λ��1λֹͣλ������żУ�飬������Ϊ115200
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void  UART1_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 4;		          // ��UART1��������Դ-Ĭ��Ϊ��
   
   PINSEL0 |= 0x40000000;       /* TxD1 */
   PINSEL1 |= 0x00000001;       /* RxD1 */
   U1LCR = 0x9B;		            // DLAB = 1�������ò�����
   Fdiv = (Fpclk / 16) / UART1_BPS;  // ���ò�����
   U1DLM = Fdiv / 256;							
   U1DLL = Fdiv % 256;
   U1LCR &= ~(0x01<<7);
   U1IER |= 0x01; //�������
   //U0IER |= 0x02; // ����THRE(transmit holding register empty)�жϣ��������ж�
   U1FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // ��������ͨ��ΪIRQ�ж�
   VICVectPriority7  = 4;
   VICVectAddr7 = (uint32)IRQ_UART1;       // ����UART1������ַ
   VICIntEnable |= 1<<7;           // ʹ��UART1�ж�
}

void T1_check(void)
{
if(T1_YlToJOS2_CalHeartBeatValue<2000)T1_YlToJOS2_CalHeartBeatValue++;
  else{
  //��ַ1��ͨѶ���
  T1_YlToJOS2_CalHeartBeatValue=0;
  if(abs(T1_HeartBeatValue-LastT1_HeartBeatValue)>0){uart1NormalFlag=1;}
  else uart1NormalFlag=0;

  LastT1_HeartBeatValue=T1_HeartBeatValue;

 } 
 //��ⳬʱ
 if(T1_sendOverTimeCount==1)
 {
  if(T1_sendOverTimeValue<T1_sendOverDelayTime)T1_sendOverTimeValue++; //�ȴ��ĳ�ʱʱ��
  else{
  T1_sendOverTimeCount=0;
  T1_sendOverTimeValue=0;

  T1_StepSendFlag=1; //��ʱ�ˣ�����������һ֡
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
   if(T1_beforePullBusValue>T1_PullBusDelay) //����֮ǰ�ȴ���ʱ��
   {
   	 T1_beforePullBusValue=0;
	 T1_beforePullBusCount=0;
	  Tx1_EN(1);   //����
	 T1_beforeSendCount=1;
   }
  }
 if(T1_ItsRecTime==1)
 { 
      if(T1_receive_time<6)
	  {
	   T1_receive_time++;
	  }
	  
	  if((T1_receive_time>5)&&(T1_haveRecData==1))	 //������9600��ʱΪ4ms
	  {	
	  //�Ѿ�����5msû���������ˣ������Ѿ������ݽ����ˣ�˵��һ������
	  //����һ���������	-������յ�������
	        T1_haveRecData=0;
			if(T1_recCount>1)
			{
			   T1_haveRecDone = 1;
			   T1_haveRecData = 0;
			 }
			else{
			T1_recCount=0;	 //������Ŀ����
			}		
	  }
  }
  if(T1_PermitRecFlag ==1)
  {
  	if(T1_sendComp_time<0)T1_sendComp_time++;	//������ɵ�һ��ʱ�����ͷ�����
	else{
		 Tx1_EN(0);//������
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
	T1_sendOverTimeCount=0;//��ʱ��ʱֹͣ
	T1_sendOverTimeValue=0;
	if((T1_Rec_Buffer[T1_recCount-2]==CRC16(T1_Rec_Buffer,T1_recCount-2,1))&&(T1_Rec_Buffer[T1_recCount-1]==CRC16(T1_Rec_Buffer,T1_recCount-2,0)))
    {if((T1_Rec_Buffer[0]==QBCAddress))
	 { 
	 //if((T1_Rec_Buffer[1]==0x03))T1_YlToJOS2HeartBeatValue=(T1_Rec_Buffer[3]<<8)+T1_Rec_Buffer[4];//����������buff��7����ȡ
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
		T1_StepSendFlag=1; //������ȷ��������һ֡
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
		T1_StepSendFlag=1; //������ȷ��������һ֡
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
//		  T1_StepSendFlag=1; //������ȷ��������һ֡
//		 }
//	   }
	 }
	 else if(T1_Rec_Buffer[1]==0x10)
	 {
	        T1_sendCount= 8;
			T1_Send_Buffer[0]=QBCAddress;
			T1_Send_Buffer[1]=0x10;
			T1_Send_Buffer[2]=T1_Rec_Buffer[2];	//д����ʼ��ַ41049 =0x0418
			T1_Send_Buffer[3]=T1_Rec_Buffer[3];
			T1_Send_Buffer[4]=T1_Rec_Buffer[4];
			T1_Send_Buffer[5]=T1_Rec_Buffer[5];
			T1_Send_Buffer[6]=CRC16(T1_Send_Buffer,6,1);
			T1_Send_Buffer[7]=CRC16(T1_Send_Buffer,6,0);
			T1_StepSendFlag=1; //������ȷ��������һ֡
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
	 T1_receive_time=0;//��������
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
	T1_Send_Buffer[2]=0x06;	//������ʼ��ַ41662 =0x067D	01 03 06 7D 00 01 14 9A
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
	T1_Send_Buffer[2]=0x04;	//д����ʼ��ַ41049 =0x0418
	T1_Send_Buffer[3]=0x18;
	T1_Send_Buffer[4]=0;
	T1_Send_Buffer[5]=1;
	T1_Send_Buffer[6]=2;
	T1_Send_Buffer[7]=switchToAFC_heartBeat>>8;
	T1_Send_Buffer[8]= switchToAFC_heartBeat;//����
	T1_Send_Buffer[9]=CRC16(T1_Send_Buffer,9,1);
	T1_Send_Buffer[10]=CRC16(T1_Send_Buffer,9,0);
	
}
void LoadT1SendDataBuff(uint16 *pJosBuff,uint16 startAddr,uint16 beginAddr,uint8 wordLen)
{    uint8 i,j,josbeginAddr;
    josbeginAddr=startAddr-(beginAddr-40001); 
    T1_sendCount= 9+wordLen*2;
    T1_Send_Buffer[0]=1;
	T1_Send_Buffer[1]=0x10;//д����ʼ��ַ41001
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

//���Ͳ��裬�ȷ�01 �ٷ� 02 ��� 03
// 01 41050-41234 ��185���֣���4�η� 50 50 50 35      Ӧ�ò���1,2,3,4
// 02 41240-41355 ��116���֣���3�η� 40 40 36	      Ӧ�ò���5,6,7
// 03 41360-41579 ��220���֣���5�η� 50 50 50 50 20   Ӧ�ò���8,9,10,11,12
void UART1_SendData()
{ //T1_WriteSequence    1 ���Ͷ����� 2 ����д���� 3 ��������
   uint8 i;

 if(T1_StepSendFlag==1)
  {	 T1_StepSendFlag=0;
     
	 T1_beforePullBusCount=1;
     //Tx1_EN(1);   //����
	 //T1_beforeSendCount=1;
  }

if(T1_realSend==1)
{
    T1_realSend=0;
    T1_sendIndex=1; //���ʹ����ݵĵ�һ����
	T1_recCount=0;	 //������Ŀ����

	U1IER = 0x02;
    U1THR = T1_Send_Buffer[0];
}
}
