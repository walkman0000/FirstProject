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
#ifndef _UART2_H_
#define _UART2_H_
/*********************************************************************************************************
 *               ͷ�ļ�����                                             
*********************************************************************************************************/
#include "config.h"
extern uint8 T2_Rec_Buffer[],T2_Send_Length, T2_sendCount,T2_sendIndex,T2_haveRecData,T2_recCount,T2_PermitRecFlag;
extern uint8 T2_haveRecDone,T2_ItsRecTime;
extern uint8 T2_StepSend,switchToAFCdataBuff[],uart2NormalFlag;
extern uint16 T2_receive_time,switchToAFC_heartBeat;
extern void T2_check(void);
extern void T2_recCheck();
extern void  UART2_Ini(void);
extern void Load_switchToAFCdataBuff();


#endif