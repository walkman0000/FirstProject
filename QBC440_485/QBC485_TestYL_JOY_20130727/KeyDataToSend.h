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
** 	��		    ��:	2011��5��1��
**	��    ��    ��:	������
**	��		    ��:	������ɫЭ����Ҫ�Ĺؼ��ϴ�����
**-------------------------------------��ʷ�޸�--------------------------------------------------------
**	��   ��  	��:	
** 	��  		��:	
** 	��		    ��:	
**	��   ��     ��:	
**	��		    ��:	
*********************************************************************************************************/
#ifndef _KEYDATATOSEND_H_
#define _KEYDATATOSEND_H_
/*********************************************************************************************************
 *               ͷ�ļ�����                                             
*********************************************************************************************************/
#include "config.h"
extern uint16 JosBuff1[],JosBuff2[],JosBuff3[];
//��Э������Ҫд��JOS������
extern uint8 newAFC_Drives_Selected;
extern uint8 CommandCloseAFCcontactors,CommandCloseCrushercontactors,CommandCloseBSLcontactors;
extern uint8 newAFCDelayShutDown;
/**************************************************
��Э����LLswitchд��JOS�����ݣ���ͨѶ����Ҫ������Ϊ0
******************************************/

//�ΰ���ĵ�ѹ����������״̬
extern uint16 AFC_MGBP_I,AFC_MGEP_I,AFC_TG_I,AFC_MGBP_U,AFC_MGEP_U,AFC_TG_U;

extern uint8 AFC_breakStat_MGBP,AFC_breakStat_MGEP,AFC_breakStat_TG;
//ת�ػ��ĵ�ѹ����������״̬
extern uint16 BSL_Motor_I,	BSL_Motor_U;
extern uint8 BSL_breakStat;
//������ĵ�ѹ����������״̬
extern uint16 Crusher_Motor_I,Crusher_Motor_U;
extern uint8 Crusher_breakStat;
//��ú���ĵ�ѹ����������״̬
extern uint16 Shearer_I,Shearer_U;//��ú��
extern uint8 Shearer_breakStat;
//�Ӵ����պ�״̬
extern uint8 Crusher_Closed,BSL_Closed,MGBP_Closed,MGEP_Closed,TG_Closed;

/**************************************************
     ԭЭ����д��LLswitch������,��ͨѶ����Ҫ��ʼ��Ϊ0�ı���
******************************************/	 



/**************************************************
      ԭЭ����д��JOS������	 ���Ա���Э����õı���
******************************************/	 
extern uint8 LLswitchDrivesSelected;

/**************************************************
**************************************************
***********��λ����ͨѶ����Ϊ0	******************
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