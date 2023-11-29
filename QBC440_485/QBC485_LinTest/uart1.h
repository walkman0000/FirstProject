/****************************************Copyright (c)**************************************************
**                                    广州致远电子有限公司
**                                     工业通信网络事业部
**                                http://www.embedcontrol.com
**-------------------------------------文件信息--------------------------------------------------------
**	文   件  	名:	LPC2400CAN.h
** 	当  前  版  本:	v1.0
** 	日		    期:	2007年5月06日
**	描		    述:	
**-------------------------------------历史创建--------------------------------------------------------
**	文   件  	名:	LPC2400CAN.h
** 	版  		本:	v1.0
** 	日		    期:	2008年4月26日
**	创    建    人:	刘彦龙
**	描		    述:	LPC2400开发板实验教程实例代码
**-------------------------------------历史修改--------------------------------------------------------
**	文   件  	名:	
** 	版  		本:	
** 	日		    期:	
**	修   改     人:	
**	描		    述:	
*********************************************************************************************************/
#ifndef _UART1_H_
#define _UART1_H_
/*********************************************************************************************************
 *               头文件包含                                             
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