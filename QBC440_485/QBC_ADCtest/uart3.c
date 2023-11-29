/****************************************************************************
* �ļ�����UART3���ӣ� ---LLswitchgear 3500����ͨѶ---��Ӧ03 10����
* ���ܣ��򴮿ڷ������ݡ�
* ˵������������JP5�̽ӣ�
*      ʹ���ⲿ11.0592MHz���񣬸���CONFIG.H�ļ����ã�Fpclk=11.0592MHz��
*      ͨѶ������115200��8λ����λ��1λֹͣλ������żУ�顣
****************************************************************************/
#include  "config.h"
#define T3_sendDelay 20 //��λms
#define T3_PullBusDelay 10
//uint8 LocalAddress = 0x01;
//T3����
uint8 T3_Rec_Buffer[100],T3_Send_Buffer[20],T3_Send_Length, T3_sendCount=0,T3_sendIndex=0,T3_haveRecData=0,T3_recCount=0,T3_PermitRecFlag=0;
//T3����
uint16 T3_receive_time=0,T3_sendComp_time=0;
uint8 T3_haveRecDone=0,T3_sendOverTimeCount,T3_beforeSendCount=0,T3_realSend=0,T3_ItsRecTime=0;
uint16 T3_sendOverTimeValue=0,T3_beforeSendValue;


uint8 AFCToswitch_heartBeat;

uint16 T3_YlToLL3500_CalHeartBeatValue,T3_beforePullBusValue;
uint8  YlToLL3500HeartBeatValue,LastYlToLL3500HeartBeatValue,uart3NormalFlag,uart3HaveNormaledFlag,T3_beforePullBusCount;

#define  UART3_BPS	9600			/* ����ͨѶ������ */

/****************************************************************************
* ���ƣ�IRQ_UART3()
* ���ܣ�UART3���жϷ������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
 void __irq IRQ_UART3(void)                           // �жϷ������
{  
   	uint32 IIR3=0;
	IIR3=U3IIR;
    if ((IIR3 & 0x0E) == 0x02)                   // �ж��Ƿ�Ϊ�����ж�
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
	  if((U3LSR & 0x01) == 1) // ��U2RBR������Ч����
	  {
	   	T3_Rec_Buffer[T3_recCount++] = U3RBR; // ������յ�������
	  }             				  

	} */
	else if(((IIR3 & 0x0E)==0x04)||((IIR3 & 0x0E)==0x0C))
	//else if((U3IIR & 0x0E)==0x04)
	{  
	   T3_receive_time=0;
	   T3_haveRecData=1;
	  if((U3LSR & 0x01) == 1) // ��U0RBR������Ч����
	  {
	   	T3_Rec_Buffer[T3_recCount++] = U3RBR; // ������յ�������
	  }             				  
         

	}else
	{
	buffer=U3RBR;
    //buffer=U3LSR;
	}

	VICVectAddr = 0x00;
}


/****************************************************************************
* ���ƣ�UART3_Ini()
* ���ܣ���ʼ������0������Ϊ8λ����λ��1λֹͣλ������żУ�飬������Ϊ115200
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void  UART3_Ini(void)
{  uint16 Fdiv;
   //PCONP   |=   0x01 << 3;		          // ��UART0��������Դ-Ĭ��Ϊ��
   PCONP   |=   0x01 << 25;		          // ��UART3��������Դ-Ĭ��Ϊ�ر�
   PINSEL1 |= (0x0F<<18);       /* RxD0 and TxD0 */
   U3LCR = 0x9B;		            // DLAB = 1�������ò�����
   Fdiv = (Fpclk / 16) / UART3_BPS;  // ���ò�����
   U3DLM = Fdiv / 256;							
   U3DLL = Fdiv % 256;
   U3LCR &= ~(0x01<<7);
   U3IER |= 0x01; //�������
   //U0IER |= 0x02; // ����THRE(transmit holding register empty)�жϣ��������ж�
   U3FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
   VICIntSelect = 0x00000000;           // ��������ͨ��ΪIRQ�ж�
   VICVectPriority29  = 6;
   VICVectAddr29 = (uint32)IRQ_UART3;       // ����UART3������ַ
   VICIntEnable |= 1<<29;           // ʹ��UART3�ж�
}

void T3_check(void)
{
//����1s��ͨѶ����������������
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
   if(T3_beforePullBusValue>T3_PullBusDelay*10) //����֮ǰ�ȴ���ʱ��
   {
   	 T3_beforePullBusValue=0;
	 T3_beforePullBusCount=0;
	  Tx3_EN(1);   //����
	 T3_beforeSendCount=1;
   }
  }
 if(T3_ItsRecTime==1)
 { 
      if(T3_receive_time<50)
	  {
	   T3_receive_time++;
	  }
	  
	  if((T3_receive_time>47)&&(T3_haveRecData==1))	 //������9600��ʱΪ4ms
	  {	
	  //�Ѿ�����5msû���������ˣ������Ѿ������ݽ����ˣ�˵��һ������
	  //����һ���������	-������յ�������
	        T3_haveRecData=0;
			if(T3_recCount>1)
			{
			   T3_haveRecDone = 1;
			   T3_haveRecData = 0;
			 }
			else{
			T3_recCount=0;	 //������Ŀ����
			}		
	  }
  }
  if(T3_PermitRecFlag ==1)
  {
  	if(T3_sendComp_time<20)T3_sendComp_time++;	//������ɵ�һ��ʱ�����ͷ�����
	else{
		 Tx3_EN(0);//������
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
   UpdateOldLLdata();//��ʼ����3500ͨѶ�л�õ����ݣ����� ���������Ƶ������ѡ��λ
 
 }
 if(T3_haveRecDone == 1)
 {  uint16 i,startAddress;
	T3_haveRecDone=0;

	if((T3_Rec_Buffer[T3_recCount-2]==CRC16(T3_Rec_Buffer,T3_recCount-2,1))&&(T3_Rec_Buffer[T3_recCount-1]==CRC16(T3_Rec_Buffer,T3_recCount-2,0)))
    {
	 YlToLL3500HeartBeatValue++;
	 if((T3_Rec_Buffer[0]==0x01)&&(T3_Rec_Buffer[1]==0x03)/*&&(T3_Rec_Buffer[5]==0x06)*/)
	 {
	 //while(1);//���Կ��Ź�
	 //Tx3_EN(1);   //����
	 //T3_beforeSendCount=1;
	 T3_beforePullBusCount=1; //��ʱһ��ʱ����������
	 T3_sendCount=17;
	 LoadT3_sendBuff(17);
	 }else if((T3_Rec_Buffer[0]==0x01)&&(T3_Rec_Buffer[1]==0x10)&&(T3_Rec_Buffer[5]==0x27))
	 {
	  LLswitchDrivesSelected = T3_Rec_Buffer[7+19*2-1]; //��ַ40019 bit0 MGBP bit1 MGEP bit2 TG
	  //���Jos������ѡ���LL���صı�Ƶ����ѡ��һ�£��������Ĺ���״̬���������ѹ�����Ȳ���������ͨѶ��ȡ
	  //�����ר������ͨ����ȡ
	  if(newAFC_Drives_Selected==LLswitchDrivesSelected){GetUart3KeyInfo(T3_Rec_Buffer);

	  }
	  
	  //Tx3_EN(1);   //����

	 T3_beforePullBusCount=1; //��ʱһ��ʱ���������� 
	 //T3_beforeSendCount=1;
	 T3_sendCount=8;
	 LoadT3_sendBuff(8);
	 }
	else{
	 T3_haveRecData=0;
	 T3_recCount=0;
	 T3_receive_time=0;//��������
	}

	}else
	{
	 T3_haveRecData=0;
	 T3_recCount=0;
	 T3_receive_time=0;//��������
	}
	
}
if(T3_realSend==1)
{
    T3_realSend=0;
    T3_sendIndex=1; //���ʹ����ݵĵ�һ����
	T3_recCount=0;	 //������Ŀ����
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
	 T3_Send_Buffer[12]=newAFCDelayShutDown; //40664L//AFC���������ֹͣ����
	 T3_Send_Buffer[13]=0; //40665H
	 T3_Send_Buffer[14]=0; //40665L//BSL�ΰ������ֹͣ����
	 T3_Send_Buffer[15]=CRC16(T3_Send_Buffer,15,1);
	 T3_Send_Buffer[16]=CRC16(T3_Send_Buffer,15,0);
	 
  }else if(sendNum==8){
  	T3_Send_Buffer[0]=1;
	 T3_Send_Buffer[1]=0x10;
	 T3_Send_Buffer[2]=0;
	 T3_Send_Buffer[3]=0;//40660H
	 T3_Send_Buffer[4]=0;//40660L
	 T3_Send_Buffer[5]=0x27; //40661H   ԭ��Ӧ�� 0x27��uart1�������� ��switchToAFC_heartBeat
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
