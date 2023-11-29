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
** 	日		    期:	2011年5月1日
**	创    建    人:	刘彦龙
**	描		    述:	榆林有色协议需要的关键上传数据
**-------------------------------------历史修改--------------------------------------------------------
**	文   件  	名:	
** 	版  		本:	
** 	日		    期:	
**	修   改     人:	
**	描		    述:	
*********************************************************************************************************/
#ifndef _KEYDATATOSEND_H_
#define _KEYDATATOSEND_H_
/*********************************************************************************************************
 *               头文件包含                                             
*********************************************************************************************************/
#include "config.h"
extern uint16 JosBuff1[],JosBuff2[],JosBuff3[];
//新协议中需要写给JOS的数据
extern uint8 newAFC_Drives_Selected;
extern uint8 CommandCloseAFCcontactors,CommandCloseCrushercontactors,CommandCloseBSLcontactors;
extern uint8 newAFCDelayShutDown;
/**************************************************
新协议中LLswitch写给JOS的数据，断通讯后需要立即置为0
******************************************/

//刮板机的电压电流及故障状态
extern uint16 AFC_MGBP_I,AFC_MGEP_I,AFC_TG_I,AFC_MGBP_U,AFC_MGEP_U,AFC_TG_U;

extern uint8 AFC_breakStat_MGBP,AFC_breakStat_MGEP,AFC_breakStat_TG;
//转载机的电压电流及故障状态
extern uint16 BSL_Motor_I,	BSL_Motor_U;
extern uint8 BSL_breakStat;
//破碎机的电压电流及故障状态
extern uint16 Crusher_Motor_I,Crusher_Motor_U;
extern uint8 Crusher_breakStat;
//采煤机的电压电流及故障状态
extern uint16 Shearer_I,Shearer_U;//采煤机
extern uint8 Shearer_breakStat;
//接触器闭合状态
extern uint8 Crusher_Closed,BSL_Closed,MGBP_Closed,MGEP_Closed,TG_Closed;

/**************************************************
     原协议中写给LLswitch的数据,断通讯后需要初始化为0的变量
******************************************/	 



/**************************************************
      原协议中写给JOS的数据	 可以被新协议借用的变量
******************************************/	 
extern uint8 LLswitchDrivesSelected;

/**************************************************
**************************************************
***********复位四组通讯数据为0	******************
**************************************************
**************************************************/	

extern void UpdateNewJOSdata();
extern void UpdateNewLLdata();
extern void UpdateOldLLdata();
extern void UpdateOldJOSdata();

extern void changeBit(uint16 *sourceData,uint8 sourceBit,uint16 *destData,uint8 destBit);
extern void XianDaoSet(uint16 *sourceData,uint8 sourceBit,uint16 *destData);
extern uint16 XianDaoSetnew(uint16 XDstat);
extern void LoadLL1DatatoSendtoJOS2();

#endif