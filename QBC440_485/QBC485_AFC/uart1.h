/****************************************Copyright (c)**************************************************
**                                    ������Զ�������޹�˾
**                                     ��ҵͨ��������ҵ��
**                                http://www.embedcontrol.com
**-------------------------------------�ļ���Ϣ--------------------------------------------------------
**	��   ��  	��:	LPC2400CAN.h
** 	��  ǰ  ��  ��:	v1.0
** 	��		    ��:	2007��5��06��
**	��		    ��:	
**-------------------------------------��ʷ����--------------------------------------------------------
**	��   ��  	��:	LPC2400CAN.h
** 	��  		��:	v1.0
** 	��		    ��:	2008��4��26��
**	��    ��    ��:	������
**	��		    ��:	LPC2400������ʵ��̳�ʵ������
**-------------------------------------��ʷ�޸�--------------------------------------------------------
**	��   ��  	��:	
** 	��  		��:	
** 	��		    ��:	
**	��   ��     ��:	
**	��		    ��:	
*********************************************************************************************************/
#ifndef _UART1_H_
#define _UART1_H_
/*********************************************************************************************************
 *               ͷ�ļ�����                                             
*********************************************************************************************************/
#include "config.h"

extern uint8 T1_StepSendFlag,uart1NormalFlag;
extern uint8 T1_Rec_Buffer[],T1_Send_Length, T1_sendCount,T1_sendIndex,T1_haveRecData,T1_recCount,T1_PermitRecFlag,T1_sendOverTimeCount,T1_ItsRecTime;
extern uint8 T1_haveRecDone,T1_beforeSendCount,T1_realSend;
extern uint16 QBCReadBuff[9],QBCWriteBuff[9];
extern uint16 T1_receive_time,T1_beforeSendValue,T1_sendOverTimeValue;
extern void T1_check(void);
extern void T1_recCheck();
extern void  UART1_Ini(void);
extern void UART1_SendData();


#endif