#include  "config.h"
#define DSStartAddr10 40128
#define DSStartAddr12 40129
#define DSStartAddr6 40129

#define TableBaseAddr1 41050
#define TableBaseAddr2 41240
#define TableBaseAddr3 41360

#define xishuDL1 2800/647
#define xishuDY1 570/233
#define xishuGL1 6384/30150

#define xishuDY2 1650/233
#define xishuDL2_1_2 4000/647
#define xishuDL2_3_6 2800/647
#define xishuGL2_1_2 2640/3015
#define xishuGL2_3_6 1848/3015

#define xishuDL3_1_9 2800/647
#define xishuDL3_10_12 280/647
#define xishuGL3_1_9 6384/30150
#define xishuGL3_10_12 798/37687



//uint16 testt1,testt2;
/**************************************************
       新协议中需从JOS读取的数据
******************************************/
uint8 newAFC_Drives_Selected;
uint8 CommandCloseAFCcontactors,CommandCloseCrushercontactors,CommandCloseBSLcontactors;
uint8 newAFCDelayShutDown;

uint16 JosBuff1[190],JosBuff2[120],JosBuff3[225];
/**************************************************
   新协议中LLswitch写给JOS的数据，断通讯后需要立即置为0
******************************************/
//刮板机的电压电流及故障状态
uint16 AFC_MGBP_I,AFC_MGEP_I,AFC_TG_I,AFC_MGBP_U,AFC_MGEP_U,AFC_TG_U;

uint8 AFC_breakStat_MGBP,AFC_breakStat_MGEP,AFC_breakStat_TG;
//转载机的电压电流及故障状态
uint16 BSL_Motor_I,	BSL_Motor_U;
uint8 BSL_breakStat;
//破碎机的电压电流及故障状态
uint16 Crusher_Motor_I,Crusher_Motor_U;
uint8 Crusher_breakStat;
//采煤机的电压电流及故障状态
uint16 Shearer_I,Shearer_U;//采煤机
uint8 Shearer_breakStat;
//接触器闭合状态
uint8 Crusher_Closed,BSL_Closed,MGBP_Closed,MGEP_Closed,TG_Closed;

/**************************************************
     原协议中写给LLswitch的数据,断通讯后需要初始化为0的变量
******************************************/	 



/**************************************************
      原协议中写给JOS的数据	 可以被新协议借用的变量
******************************************/	 
uint8 LLswitchDrivesSelected;
uint8 AFCRunSignal;
/**************************************************
**************************************************
***********复位四组通讯数据为0	******************
**************************************************
**************************************************/	

void UpdateNewJOSdata()
{  //控制输出为0
    KS2_DelayOn(0);// PSA 破碎机
    KS1_DelayOn(0);// ZZA 转载机
	KS3_DelayOn(0);// 华宁启动信号

	KS4_DelayOn(0);// AFC  MGBP
    KS5_DelayOn(0);// AFC  MGEP
	KS6_DelayOn(0);// AFC TG
  //控制状态复位为0
    KS7_DelayOn(0);// 反向先导投入
  newAFC_Drives_Selected=0;
  CommandCloseAFCcontactors=0;CommandCloseCrushercontactors=0;CommandCloseBSLcontactors=0;
  newAFCDelayShutDown=0;
}

void UpdateNewLLdata()
{
 AFC_MGBP_I=0;AFC_MGEP_I=0;AFC_TG_I=0;AFC_MGBP_U=0;AFC_MGEP_U=0;AFC_TG_U=0;
 AFC_breakStat_MGBP=1;AFC_breakStat_MGEP=1;AFC_breakStat_TG=1;
 BSL_Motor_I=0;BSL_Motor_U=0;

 BSL_breakStat=1;
 Crusher_Motor_I=0;Crusher_Motor_U=0;
Crusher_breakStat=1;

Shearer_I=0;Shearer_U=0;
Shearer_breakStat=1;
Crusher_Closed=0;BSL_Closed=0;MGBP_Closed=0;MGEP_Closed=0;TG_Closed=0;

    KS4_DelayOn(0);// AFC  MGBP
    KS5_DelayOn(0);// AFC  MGEP
	KS6_DelayOn(0);// AFC TG
}

void UpdateOldLLdata()
{
 LLswitchDrivesSelected=0;
}
void UpdateOldJOSdata()
{
  ;
}
/**************************************************
     函数名 changeBit
	 参数  *sourceData 参数改变的依据 sourceBit 根据哪一位改变 
	        *destData需要改变的变量   destBit  需要改变变量的哪一位
******************************************/	
void changeBit(uint16 *sourceData,uint8 sourceBit,uint16 *destData,uint8 destBit)
{ uint16 tmp1=0,tmp2=0;
  if((sourceBit>15)||(destBit>15))return;
  tmp1 |= 1<<sourceBit;
  tmp2 |= 1<<destBit;
  if((*sourceData &  tmp1)==0) *destData &=	(~tmp2) ;
  else 	*destData |= tmp2;
}
void RchangeBit(uint16 *sourceData,uint8 sourceBit,uint16 *destData,uint8 destBit)
{ uint16 tmp1=0,tmp2=0;
  if((sourceBit>15)||(destBit>15))return;
  tmp1 |= 1<<sourceBit;
  tmp2 |= 1<<destBit;
  if((*sourceData &  tmp1)==0)*destData |= tmp2  ;
  else 	*destData &=	(~tmp2);
}
/**************************************************
     函数名 XianDaoSet
	 参数  *sourceData 参数改变的依据 sourceBit 根据哪一段改变，分四段 
	        *destData需要改变的变量   
******************************************/	
void XianDaoSet(uint16 *sourceData,uint8 sourceBit,uint16 *destData)
{ uint16 tmp1;
  if((sourceBit>4)||(sourceBit<1))return;
  tmp1 = *sourceData;
  switch(sourceBit)
  {	case  1: 
	*destData=XianDaoSetnew(tmp1&0x000F);
    break;
	case  2: 
	*destData=XianDaoSetnew((tmp1&0x00F0)>>4);
    break;
	case  3: 
	*destData=XianDaoSetnew((tmp1&0x0F00)>>8);
    break;
	case  4: 
	*destData=XianDaoSetnew((tmp1&0xF000)>>12);
    break;
  
  }

}
uint16 XianDaoSetnew(uint16 XDstat)
{
  switch(XDstat)
  {	case 0:
    return 0x0001;
    break;
	case 1:
    return 0x0002;
    break;
	case 2:
    return 0x0004;
    break;
	case 3:
    return 0x0008;
    break;
	case 4:
    return 0x0010;
    break;
	default : return 0x0000;
  }

}

/**************************************************
     如果和3500/1140-10回路开关通讯正常，则把其数据装载到buff中以备发送
******************************************/	

void LoadLL1DatatoSendtoJOS2()
{//DataBuffer1[0]=0x1230;
   uint16 Rtmp;
  //testt1=1234;
  //testt2=testt1*xishuGL1;
 /************     回路 一   ***************/
 changeBit(&DataBuffer1[40128-DSStartAddr10],0,&JosBuff1[41051-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40128-DSStartAddr10],1,&JosBuff1[41051-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40139-DSStartAddr10],0,&JosBuff1[41051-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40139-DSStartAddr10],1,&JosBuff1[41051-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40139-DSStartAddr10],2,&JosBuff1[41051-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40139-DSStartAddr10],3,&JosBuff1[41051-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],1,&JosBuff1[41051-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40139-DSStartAddr10],4,&JosBuff1[41051-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40139-DSStartAddr10],6,&JosBuff1[41051-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40139-DSStartAddr10],5,&JosBuff1[41051-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40139-DSStartAddr10],7,&JosBuff1[41051-TableBaseAddr1],10); //共用的过压
 changeBit(&DataBuffer1[40143-DSStartAddr10],7,&JosBuff1[41051-TableBaseAddr1],11); //共用的欠压
 //JosBuff1[41051-TableBaseAddr1]&=0xfffe;
 XianDaoSet(&DataBuffer1[40220-DSStartAddr10],3,&JosBuff1[41052-TableBaseAddr1]);
 
 JosBuff1[41053-TableBaseAddr1]=DataBuffer1[40129-DSStartAddr10]*xishuDL1;
 JosBuff1[41054-TableBaseAddr1]=DataBuffer1[40130-DSStartAddr10]*xishuDL1;
 JosBuff1[41055-TableBaseAddr1]=DataBuffer1[40131-DSStartAddr10]*xishuDL1;
 JosBuff1[41056-TableBaseAddr1]=DataBuffer1[40144-DSStartAddr10];
 JosBuff1[41057-TableBaseAddr1]=DataBuffer1[40148-DSStartAddr10]*xishuGL1;
 //JosBuff1[41058-TableBaseAddr1]=DataBuffer1[40225-DSStartAddr10]; //1140 无零序电流
 
 /************     回路 二   ***************/
 changeBit(&DataBuffer1[40132-DSStartAddr10],0,&JosBuff1[41069-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40132-DSStartAddr10],1,&JosBuff1[41069-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40143-DSStartAddr10],0,&JosBuff1[41069-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40143-DSStartAddr10],1,&JosBuff1[41069-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40143-DSStartAddr10],2,&JosBuff1[41069-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40143-DSStartAddr10],3,&JosBuff1[41069-TableBaseAddr1],5);
// changeBit(&DataBuffer1[40235-DSStartAddr10],2,&JosBuff1[41069-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40143-DSStartAddr10],4,&JosBuff1[41069-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40143-DSStartAddr10],6,&JosBuff1[41069-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40143-DSStartAddr10],5,&JosBuff1[41069-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40139-DSStartAddr10],7,&JosBuff1[41069-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40143-DSStartAddr10],7,&JosBuff1[41069-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40220-DSStartAddr10],4,&JosBuff1[41070-TableBaseAddr1]);
 
 JosBuff1[41071-TableBaseAddr1]=DataBuffer1[40133-DSStartAddr10]*xishuDL1;
 JosBuff1[41072-TableBaseAddr1]=DataBuffer1[40134-DSStartAddr10]*xishuDL1;
 JosBuff1[41073-TableBaseAddr1]=DataBuffer1[40135-DSStartAddr10]*xishuDL1;
 JosBuff1[41074-TableBaseAddr1]=DataBuffer1[40145-DSStartAddr10];
 JosBuff1[41075-TableBaseAddr1]=DataBuffer1[40149-DSStartAddr10]*xishuGL1;
 //JosBuff1[41076-TableBaseAddr1]=DataBuffer1[40226-DSStartAddr10]; //1140 无零序电流

/************     回路 三   ***************/
 changeBit(&DataBuffer1[40132-DSStartAddr10],2,&JosBuff1[41087-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40132-DSStartAddr10],3,&JosBuff1[41087-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40147-DSStartAddr10],0,&JosBuff1[41087-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40147-DSStartAddr10],1,&JosBuff1[41087-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40147-DSStartAddr10],2,&JosBuff1[41087-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40147-DSStartAddr10],3,&JosBuff1[41087-TableBaseAddr1],5);
// changeBit(&DataBuffer1[40235-DSStartAddr10],3,&JosBuff1[41087-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40147-DSStartAddr10],4,&JosBuff1[41087-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40147-DSStartAddr10],6,&JosBuff1[41087-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40147-DSStartAddr10],5,&JosBuff1[41087-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40139-DSStartAddr10],7,&JosBuff1[41087-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40143-DSStartAddr10],7,&JosBuff1[41087-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40221-DSStartAddr10],1,&JosBuff1[41088-TableBaseAddr1]);
 
 JosBuff1[41089-TableBaseAddr1]=DataBuffer1[40136-DSStartAddr10]*xishuDL1;
 JosBuff1[41090-TableBaseAddr1]=DataBuffer1[40137-DSStartAddr10]*xishuDL1;
 JosBuff1[41091-TableBaseAddr1]=DataBuffer1[40138-DSStartAddr10]*xishuDL1;
 JosBuff1[41092-TableBaseAddr1]=DataBuffer1[40146-DSStartAddr10];
 JosBuff1[41093-TableBaseAddr1]=DataBuffer1[40150-DSStartAddr10]*xishuGL1;
 //JosBuff1[41094-TableBaseAddr1]=DataBuffer1[40227-DSStartAddr10]; //1140 无零序电流

/************     回路 四   ***************/
 changeBit(&DataBuffer1[40151-DSStartAddr10],0,&JosBuff1[41105-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40151-DSStartAddr10],1,&JosBuff1[41105-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40162-DSStartAddr10],0,&JosBuff1[41105-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40162-DSStartAddr10],1,&JosBuff1[41105-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40162-DSStartAddr10],2,&JosBuff1[41105-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40162-DSStartAddr10],3,&JosBuff1[41105-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],4,&JosBuff1[41105-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40162-DSStartAddr10],4,&JosBuff1[41105-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40162-DSStartAddr10],6,&JosBuff1[41105-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40162-DSStartAddr10],5,&JosBuff1[41105-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40162-DSStartAddr10],7,&JosBuff1[41105-TableBaseAddr1],10); //共用的过压
 changeBit(&DataBuffer1[40166-DSStartAddr10],7,&JosBuff1[41105-TableBaseAddr1],11); //共用的欠压

 XianDaoSet(&DataBuffer1[40221-DSStartAddr10],2,&JosBuff1[41106-TableBaseAddr1]);
 
 JosBuff1[41107-TableBaseAddr1]=DataBuffer1[40152-DSStartAddr10]*xishuDL1;
 JosBuff1[41108-TableBaseAddr1]=DataBuffer1[40153-DSStartAddr10]*xishuDL1;
 JosBuff1[41109-TableBaseAddr1]=DataBuffer1[40154-DSStartAddr10]*xishuDL1;
 JosBuff1[41110-TableBaseAddr1]=DataBuffer1[40167-DSStartAddr10];
 JosBuff1[41111-TableBaseAddr1]=DataBuffer1[40171-DSStartAddr10]*xishuGL1;
 //JosBuff1[41112-TableBaseAddr1]=DataBuffer1[40229-DSStartAddr10]; //1140 无零序电流
 
 /************     回路 五   ***************/
 changeBit(&DataBuffer1[40155-DSStartAddr10],0,&JosBuff1[41123-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40155-DSStartAddr10],1,&JosBuff1[41123-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40166-DSStartAddr10],0,&JosBuff1[41123-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40166-DSStartAddr10],1,&JosBuff1[41123-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40166-DSStartAddr10],2,&JosBuff1[41123-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40166-DSStartAddr10],3,&JosBuff1[41123-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],6,&JosBuff1[41123-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40166-DSStartAddr10],4,&JosBuff1[41123-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40166-DSStartAddr10],6,&JosBuff1[41123-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40166-DSStartAddr10],5,&JosBuff1[41123-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40162-DSStartAddr10],7,&JosBuff1[41123-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40166-DSStartAddr10],7,&JosBuff1[41123-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40221-DSStartAddr10],3,&JosBuff1[41124-TableBaseAddr1]);
 
 JosBuff1[41125-TableBaseAddr1]=DataBuffer1[40156-DSStartAddr10]*xishuDL1;
 JosBuff1[41126-TableBaseAddr1]=DataBuffer1[40157-DSStartAddr10]*xishuDL1;
 JosBuff1[41127-TableBaseAddr1]=DataBuffer1[40158-DSStartAddr10]*xishuDL1;
 JosBuff1[41128-TableBaseAddr1]=DataBuffer1[40168-DSStartAddr10];
 JosBuff1[41129-TableBaseAddr1]=DataBuffer1[40172-DSStartAddr10]*xishuGL1;
 //JosBuff1[41130-TableBaseAddr1]=DataBuffer1[40230-DSStartAddr10]; //1140 无零序电流

/************     回路 六   ***************/
 changeBit(&DataBuffer1[40155-DSStartAddr10],2,&JosBuff1[41141-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40155-DSStartAddr10],3,&JosBuff1[41141-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40170-DSStartAddr10],0,&JosBuff1[41141-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40170-DSStartAddr10],1,&JosBuff1[41141-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40170-DSStartAddr10],2,&JosBuff1[41141-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40170-DSStartAddr10],3,&JosBuff1[41141-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],6,&JosBuff1[41141-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40170-DSStartAddr10],4,&JosBuff1[41141-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40170-DSStartAddr10],6,&JosBuff1[41141-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40170-DSStartAddr10],5,&JosBuff1[41141-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40162-DSStartAddr10],7,&JosBuff1[41141-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40166-DSStartAddr10],7,&JosBuff1[41141-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40221-DSStartAddr10],4,&JosBuff1[41142-TableBaseAddr1]);
 
 JosBuff1[41143-TableBaseAddr1]=DataBuffer1[40159-DSStartAddr10]*xishuDL1;
 JosBuff1[41144-TableBaseAddr1]=DataBuffer1[40160-DSStartAddr10]*xishuDL1;
 JosBuff1[41145-TableBaseAddr1]=DataBuffer1[40161-DSStartAddr10]*xishuDL1;
 JosBuff1[41146-TableBaseAddr1]=DataBuffer1[40169-DSStartAddr10];
 JosBuff1[41147-TableBaseAddr1]=DataBuffer1[40173-DSStartAddr10]*xishuGL1;
 //JosBuff1[41148-TableBaseAddr1]=DataBuffer1[40231-DSStartAddr10]; //1140 无零序电流


/************     回路 七   ***************/
 changeBit(&DataBuffer1[40174-DSStartAddr10],0,&JosBuff1[41159-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40174-DSStartAddr10],1,&JosBuff1[41159-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40185-DSStartAddr10],0,&JosBuff1[41159-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40185-DSStartAddr10],1,&JosBuff1[41159-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40185-DSStartAddr10],2,&JosBuff1[41159-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40185-DSStartAddr10],3,&JosBuff1[41159-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],7,&JosBuff1[41159-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40185-DSStartAddr10],4,&JosBuff1[41159-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40185-DSStartAddr10],6,&JosBuff1[41159-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40185-DSStartAddr10],5,&JosBuff1[41159-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40185-DSStartAddr10],7,&JosBuff1[41159-TableBaseAddr1],10); //共用的过压
 changeBit(&DataBuffer1[40189-DSStartAddr10],7,&JosBuff1[41159-TableBaseAddr1],11); //共用的欠压

 XianDaoSet(&DataBuffer1[40222-DSStartAddr10],1,&JosBuff1[41160-TableBaseAddr1]);
 
 JosBuff1[41161-TableBaseAddr1]=DataBuffer1[40175-DSStartAddr10]*xishuDL1;
 JosBuff1[41162-TableBaseAddr1]=DataBuffer1[40176-DSStartAddr10]*xishuDL1;
 JosBuff1[41163-TableBaseAddr1]=DataBuffer1[40177-DSStartAddr10]*xishuDL1;
 JosBuff1[41164-TableBaseAddr1]=DataBuffer1[40190-DSStartAddr10];
 JosBuff1[41165-TableBaseAddr1]=DataBuffer1[40194-DSStartAddr10]*xishuGL1;
 //JosBuff1[41166-TableBaseAddr1]=DataBuffer1[40232-DSStartAddr10]; //1140 无零序电流
 
 /************     回路 八   ***************/
 changeBit(&DataBuffer1[40178-DSStartAddr10],0,&JosBuff1[41176-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40178-DSStartAddr10],1,&JosBuff1[41176-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40189-DSStartAddr10],0,&JosBuff1[41176-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40189-DSStartAddr10],1,&JosBuff1[41176-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40189-DSStartAddr10],2,&JosBuff1[41176-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40189-DSStartAddr10],3,&JosBuff1[41176-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],8,&JosBuff1[41176-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40189-DSStartAddr10],4,&JosBuff1[41176-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40189-DSStartAddr10],6,&JosBuff1[41176-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40189-DSStartAddr10],5,&JosBuff1[41176-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40185-DSStartAddr10],7,&JosBuff1[41176-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40189-DSStartAddr10],7,&JosBuff1[41176-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40222-DSStartAddr10],2,&JosBuff1[41177-TableBaseAddr1]);
 
 JosBuff1[41178-TableBaseAddr1]=DataBuffer1[40179-DSStartAddr10]*xishuDL1;
 JosBuff1[41179-TableBaseAddr1]=DataBuffer1[40180-DSStartAddr10]*xishuDL1;
 JosBuff1[41180-TableBaseAddr1]=DataBuffer1[40181-DSStartAddr10]*xishuDL1;
 JosBuff1[41181-TableBaseAddr1]=DataBuffer1[40191-DSStartAddr10];
 JosBuff1[41182-TableBaseAddr1]=DataBuffer1[40195-DSStartAddr10]*xishuGL1;
 //JosBuff1[41183-TableBaseAddr1]=DataBuffer1[40233-DSStartAddr10]; //1140 无零序电流

/************     回路 九   ***************/
 changeBit(&DataBuffer1[40178-DSStartAddr10],2,&JosBuff1[41194-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40178-DSStartAddr10],3,&JosBuff1[41194-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40193-DSStartAddr10],0,&JosBuff1[41194-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40193-DSStartAddr10],1,&JosBuff1[41194-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40193-DSStartAddr10],2,&JosBuff1[41194-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40193-DSStartAddr10],3,&JosBuff1[41194-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],9,&JosBuff1[41194-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40193-DSStartAddr10],4,&JosBuff1[41194-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40193-DSStartAddr10],6,&JosBuff1[41194-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40193-DSStartAddr10],5,&JosBuff1[41194-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40185-DSStartAddr10],7,&JosBuff1[41194-TableBaseAddr1],10);//共用的过压
 changeBit(&DataBuffer1[40189-DSStartAddr10],7,&JosBuff1[41194-TableBaseAddr1],11);//共用的欠压

 XianDaoSet(&DataBuffer1[40220-DSStartAddr10],2,&JosBuff1[41195-TableBaseAddr1]);
 
 JosBuff1[41196-TableBaseAddr1]=DataBuffer1[40182-DSStartAddr10]*xishuDL1;
 JosBuff1[41197-TableBaseAddr1]=DataBuffer1[40183-DSStartAddr10]*xishuDL1;
 JosBuff1[41198-TableBaseAddr1]=DataBuffer1[40184-DSStartAddr10]*xishuDL1;
 JosBuff1[41199-TableBaseAddr1]=DataBuffer1[40192-DSStartAddr10];
 JosBuff1[41200-TableBaseAddr1]=DataBuffer1[40196-DSStartAddr10]*xishuGL1;
 //JosBuff1[41201-TableBaseAddr1]=DataBuffer1[40234-DSStartAddr10]; //1140 无零序电流
 
/************     回路 十   ***************/
 changeBit(&DataBuffer1[40197-DSStartAddr10],0,&JosBuff1[41212-TableBaseAddr1],0);
 changeBit(&DataBuffer1[40197-DSStartAddr10],1,&JosBuff1[41212-TableBaseAddr1],1); 
 changeBit(&DataBuffer1[40208-DSStartAddr10],0,&JosBuff1[41212-TableBaseAddr1],2);
 changeBit(&DataBuffer1[40208-DSStartAddr10],1,&JosBuff1[41212-TableBaseAddr1],3);
 changeBit(&DataBuffer1[40208-DSStartAddr10],2,&JosBuff1[41212-TableBaseAddr1],4);
 changeBit(&DataBuffer1[40208-DSStartAddr10],3,&JosBuff1[41212-TableBaseAddr1],5);
 //changeBit(&DataBuffer1[40235-DSStartAddr10],10,&JosBuff1[41212-TableBaseAddr1],6);//1140 无漏电保护
 changeBit(&DataBuffer1[40208-DSStartAddr10],4,&JosBuff1[41212-TableBaseAddr1],7);
 changeBit(&DataBuffer1[40208-DSStartAddr10],6,&JosBuff1[41212-TableBaseAddr1],8);
 changeBit(&DataBuffer1[40208-DSStartAddr10],5,&JosBuff1[41212-TableBaseAddr1],9);
 changeBit(&DataBuffer1[40208-DSStartAddr10],7,&JosBuff1[41212-TableBaseAddr1],10); //共用的过压
 changeBit(&DataBuffer1[40212-DSStartAddr10],7,&JosBuff1[41212-TableBaseAddr1],11); //共用的欠压

 XianDaoSet(&DataBuffer1[40223-DSStartAddr10],1,&JosBuff1[41213-TableBaseAddr1]);
 
 JosBuff1[41214-TableBaseAddr1]=DataBuffer1[40198-DSStartAddr10]*xishuDL1;
 JosBuff1[41215-TableBaseAddr1]=DataBuffer1[40199-DSStartAddr10]*xishuDL1;
 JosBuff1[41216-TableBaseAddr1]=DataBuffer1[40200-DSStartAddr10]*xishuDL1;
 JosBuff1[41217-TableBaseAddr1]=DataBuffer1[40213-DSStartAddr10];
 JosBuff1[41218-TableBaseAddr1]=DataBuffer1[40217-DSStartAddr10]*xishuGL1;
 //JosBuff1[41219-TableBaseAddr1]=DataBuffer1[40239-DSStartAddr10]; //1140 无零序电流

 changeBit(&DataBuffer1[40128-DSStartAddr10],2,&JosBuff1[41226-TableBaseAddr1],0); //隔离状态
 changeBit(&DataBuffer1[40128-DSStartAddr10],3,&JosBuff1[41226-TableBaseAddr1],1); //隔离状态
 changeBit(&DataBuffer1[40128-DSStartAddr10],4,&JosBuff1[41226-TableBaseAddr1],2); //隔离状态
 changeBit(&DataBuffer1[40128-DSStartAddr10],5,&JosBuff1[41226-TableBaseAddr1],3); //隔离状态
 changeBit(&DataBuffer1[40174-DSStartAddr10],3,&JosBuff1[41226-TableBaseAddr1],4); //隔离状态
 changeBit(&DataBuffer1[40174-DSStartAddr10],4,&JosBuff1[41226-TableBaseAddr1],5); //隔离状态
 changeBit(&DataBuffer1[40174-DSStartAddr10],5,&JosBuff1[41226-TableBaseAddr1],6); //隔离状态
 changeBit(&DataBuffer1[40174-DSStartAddr10],6,&JosBuff1[41226-TableBaseAddr1],7); //隔离状态
 changeBit(&DataBuffer1[40197-DSStartAddr10],5,&JosBuff1[41226-TableBaseAddr1],8); //隔离状态
 changeBit(&DataBuffer1[40197-DSStartAddr10],6,&JosBuff1[41226-TableBaseAddr1],9); //隔离状态
 JosBuff1[41227-TableBaseAddr1]=DataBuffer1[40140-DSStartAddr10]*xishuDY1; //A母排电压--需计算
 if(DataBuffer1[40142-DSStartAddr10]>0)JosBuff1[41228-TableBaseAddr1]=800000/(DataBuffer1[40142-DSStartAddr10]); //A母排频率--需计算
 JosBuff1[41229-TableBaseAddr1]=DataBuffer1[40163-DSStartAddr10]*xishuDY1; //B母排电压--需计算
 if(DataBuffer1[40165-DSStartAddr10]>0)JosBuff1[41230-TableBaseAddr1]=800000/(DataBuffer1[40165-DSStartAddr10]); //B母排频率--需计算
 
 RchangeBit(&DataBuffer1[40128-DSStartAddr10],6,&JosBuff1[41231-TableBaseAddr1],0); //127IV状态
 RchangeBit(&DataBuffer1[40151-DSStartAddr10],2,&JosBuff1[41231-TableBaseAddr1],1); //127IIV状态
 RchangeBit(&DataBuffer1[40151-DSStartAddr10],3,&JosBuff1[41231-TableBaseAddr1],2); //220V状态
 JosBuff1[41232-TableBaseAddr1]=(DataBuffer1[40223-DSStartAddr10]>>8); //腔体温度
 JosBuff1[41233-TableBaseAddr1]=(DataBuffer1[40223-DSStartAddr10]>>8); //腔体温度
}
/**************************************************

     如果和3300V-6回路开关通讯正常，则把其数据装载到buff中以备发送

******************************************/	
void LoadLL2DatatoSendtoJOS2()
{
 /************     回路 一   ***************/
 changeBit(&DataBuffer2[40129-DSStartAddr6],0,&JosBuff2[41241-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40129-DSStartAddr6],1,&JosBuff2[41241-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40140-DSStartAddr6],0,&JosBuff2[41241-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40140-DSStartAddr6],1,&JosBuff2[41241-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40140-DSStartAddr6],2,&JosBuff2[41241-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40140-DSStartAddr6],3,&JosBuff2[41241-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],0,&JosBuff2[41241-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40140-DSStartAddr6],4,&JosBuff2[41241-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40140-DSStartAddr6],6,&JosBuff2[41241-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40140-DSStartAddr6],5,&JosBuff2[41241-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40140-DSStartAddr6],7,&JosBuff2[41241-TableBaseAddr2],10); //共用的过压
 changeBit(&DataBuffer2[40144-DSStartAddr6],7,&JosBuff2[41241-TableBaseAddr2],11); //共用的欠压

 XianDaoSet(&DataBuffer2[40221-DSStartAddr6],3,&JosBuff2[41242-TableBaseAddr2]);
 
 JosBuff2[41243-TableBaseAddr2]=DataBuffer2[40130-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41244-TableBaseAddr2]=DataBuffer2[40131-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41245-TableBaseAddr2]=DataBuffer2[40132-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41246-TableBaseAddr2]=DataBuffer2[40145-DSStartAddr6];
 JosBuff2[41247-TableBaseAddr2]=DataBuffer2[40149-DSStartAddr6]*xishuGL2_1_2;
 JosBuff2[41248-TableBaseAddr2]=DataBuffer2[40199-DSStartAddr6]; //1140 无零序电流
 
 /************     回路 二   ***************/
 changeBit(&DataBuffer2[40133-DSStartAddr6],0,&JosBuff2[41259-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40133-DSStartAddr6],1,&JosBuff2[41259-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40144-DSStartAddr6],0,&JosBuff2[41259-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40144-DSStartAddr6],1,&JosBuff2[41259-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40144-DSStartAddr6],2,&JosBuff2[41259-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40144-DSStartAddr6],3,&JosBuff2[41259-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],1,&JosBuff2[41259-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40144-DSStartAddr6],4,&JosBuff2[41259-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40144-DSStartAddr6],6,&JosBuff2[41259-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40144-DSStartAddr6],5,&JosBuff2[41259-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40140-DSStartAddr6],7,&JosBuff2[41259-TableBaseAddr2],10);//共用的过压
 changeBit(&DataBuffer2[40144-DSStartAddr6],7,&JosBuff2[41259-TableBaseAddr2],11);//共用的欠压

 XianDaoSet(&DataBuffer2[40221-DSStartAddr6],4,&JosBuff2[41260-TableBaseAddr2]);
 
 JosBuff2[41261-TableBaseAddr2]=DataBuffer2[40134-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41262-TableBaseAddr2]=DataBuffer2[40135-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41263-TableBaseAddr2]=DataBuffer2[40136-DSStartAddr6]*xishuDL2_1_2;
 JosBuff2[41264-TableBaseAddr2]=DataBuffer2[40146-DSStartAddr6];
 JosBuff2[41265-TableBaseAddr2]=DataBuffer2[40150-DSStartAddr6]*xishuGL2_1_2;
 JosBuff2[41266-TableBaseAddr2]=DataBuffer2[40200-DSStartAddr6]; //1140 无零序电流

/************     回路 三   ***************/
 changeBit(&DataBuffer2[40133-DSStartAddr6],2,&JosBuff2[41277-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40133-DSStartAddr6],3,&JosBuff2[41277-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40148-DSStartAddr6],0,&JosBuff2[41277-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40148-DSStartAddr6],1,&JosBuff2[41277-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40148-DSStartAddr6],2,&JosBuff2[41277-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40148-DSStartAddr6],3,&JosBuff2[41277-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],2,&JosBuff2[41277-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40148-DSStartAddr6],4,&JosBuff2[41277-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40148-DSStartAddr6],6,&JosBuff2[41277-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40148-DSStartAddr6],5,&JosBuff2[41277-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40140-DSStartAddr6],7,&JosBuff2[41277-TableBaseAddr2],10);//共用的过压
 changeBit(&DataBuffer2[40144-DSStartAddr6],7,&JosBuff2[41277-TableBaseAddr2],11);//共用的欠压

 XianDaoSet(&DataBuffer2[40222-DSStartAddr6],1,&JosBuff2[41278-TableBaseAddr2]);
 
 JosBuff2[41279-TableBaseAddr2]=DataBuffer2[40137-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41280-TableBaseAddr2]=DataBuffer2[40138-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41281-TableBaseAddr2]=DataBuffer2[40139-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41282-TableBaseAddr2]=DataBuffer2[40147-DSStartAddr6];
 JosBuff2[41283-TableBaseAddr2]=DataBuffer2[40151-DSStartAddr6]*xishuGL2_3_6;
 JosBuff2[41284-TableBaseAddr2]=DataBuffer2[40201-DSStartAddr6]; //1140 无零序电流

/************     回路 四   ***************/
 changeBit(&DataBuffer2[40152-DSStartAddr6],0,&JosBuff2[41295-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40152-DSStartAddr6],1,&JosBuff2[41295-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40163-DSStartAddr6],0,&JosBuff2[41295-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40163-DSStartAddr6],1,&JosBuff2[41295-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40163-DSStartAddr6],2,&JosBuff2[41295-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40163-DSStartAddr6],3,&JosBuff2[41295-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],3,&JosBuff2[41295-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40163-DSStartAddr6],4,&JosBuff2[41295-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40163-DSStartAddr6],6,&JosBuff2[41295-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40163-DSStartAddr6],5,&JosBuff2[41295-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40163-DSStartAddr6],7,&JosBuff2[41295-TableBaseAddr2],10); //共用的过压
 changeBit(&DataBuffer2[40167-DSStartAddr6],7,&JosBuff2[41295-TableBaseAddr2],11); //共用的欠压

 XianDaoSet(&DataBuffer2[40222-DSStartAddr6],2,&JosBuff2[41296-TableBaseAddr2]);
 
 JosBuff2[41297-TableBaseAddr2]=DataBuffer2[40153-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41298-TableBaseAddr2]=DataBuffer2[40154-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41299-TableBaseAddr2]=DataBuffer2[40155-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41300-TableBaseAddr2]=DataBuffer2[40168-DSStartAddr6];
 JosBuff2[41301-TableBaseAddr2]=DataBuffer2[40172-DSStartAddr6]*xishuGL2_3_6;
 JosBuff2[41302-TableBaseAddr2]=DataBuffer2[40202-DSStartAddr6]; //1140 无零序电流
 
 /************     回路 五   ***************/
 changeBit(&DataBuffer2[40156-DSStartAddr6],0,&JosBuff2[41313-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40156-DSStartAddr6],1,&JosBuff2[41313-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40167-DSStartAddr6],0,&JosBuff2[41313-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40167-DSStartAddr6],1,&JosBuff2[41313-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40167-DSStartAddr6],2,&JosBuff2[41313-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40167-DSStartAddr6],3,&JosBuff2[41313-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],4,&JosBuff2[41313-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40167-DSStartAddr6],4,&JosBuff2[41313-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40167-DSStartAddr6],6,&JosBuff2[41313-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40167-DSStartAddr6],5,&JosBuff2[41313-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40163-DSStartAddr6],7,&JosBuff2[41313-TableBaseAddr2],10);//共用的过压
 changeBit(&DataBuffer2[40167-DSStartAddr6],7,&JosBuff2[41313-TableBaseAddr2],11);//共用的欠压

 XianDaoSet(&DataBuffer2[40222-DSStartAddr6],3,&JosBuff2[41314-TableBaseAddr2]);
 
 JosBuff2[41315-TableBaseAddr2]=DataBuffer2[40157-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41316-TableBaseAddr2]=DataBuffer2[40158-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41317-TableBaseAddr2]=DataBuffer2[40159-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41318-TableBaseAddr2]=DataBuffer2[40169-DSStartAddr6];
 JosBuff2[41319-TableBaseAddr2]=DataBuffer2[40173-DSStartAddr6]*xishuGL2_3_6;
 JosBuff2[41320-TableBaseAddr2]=DataBuffer2[40203-DSStartAddr6]; //1140 无零序电流

/************     回路 六   ***************/
 changeBit(&DataBuffer2[40156-DSStartAddr6],2,&JosBuff2[41331-TableBaseAddr2],0);
 changeBit(&DataBuffer2[40156-DSStartAddr6],3,&JosBuff2[41331-TableBaseAddr2],1); 
 changeBit(&DataBuffer2[40171-DSStartAddr6],0,&JosBuff2[41331-TableBaseAddr2],2);
 changeBit(&DataBuffer2[40171-DSStartAddr6],1,&JosBuff2[41331-TableBaseAddr2],3);
 changeBit(&DataBuffer2[40171-DSStartAddr6],2,&JosBuff2[41331-TableBaseAddr2],4);
 changeBit(&DataBuffer2[40171-DSStartAddr6],3,&JosBuff2[41331-TableBaseAddr2],5);
 changeBit(&DataBuffer2[40209-DSStartAddr6],5,&JosBuff2[41331-TableBaseAddr2],6);//1140 无漏电保护
 changeBit(&DataBuffer2[40171-DSStartAddr6],4,&JosBuff2[41331-TableBaseAddr2],7);
 changeBit(&DataBuffer2[40171-DSStartAddr6],6,&JosBuff2[41331-TableBaseAddr2],8);
 changeBit(&DataBuffer2[40171-DSStartAddr6],5,&JosBuff2[41331-TableBaseAddr2],9);
 changeBit(&DataBuffer2[40163-DSStartAddr6],7,&JosBuff2[41331-TableBaseAddr2],10);//共用的过压
 changeBit(&DataBuffer2[40167-DSStartAddr6],7,&JosBuff2[41331-TableBaseAddr2],11);//共用的欠压

 XianDaoSet(&DataBuffer2[40222-DSStartAddr6],4,&JosBuff2[41332-TableBaseAddr2]);
 
 JosBuff2[41333-TableBaseAddr2]=DataBuffer2[40160-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41334-TableBaseAddr2]=DataBuffer2[40161-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41335-TableBaseAddr2]=DataBuffer2[40162-DSStartAddr6]*xishuDL2_3_6;
 JosBuff2[41336-TableBaseAddr2]=DataBuffer2[40170-DSStartAddr6];
 JosBuff2[41337-TableBaseAddr2]=DataBuffer2[40174-DSStartAddr6]*xishuGL2_3_6;
 JosBuff2[41338-TableBaseAddr2]=DataBuffer2[40204-DSStartAddr6]; //1140 无零序电流



 changeBit(&DataBuffer2[40129-DSStartAddr6],2,&JosBuff2[41348-TableBaseAddr2],0); //隔离状态
 changeBit(&DataBuffer2[40129-DSStartAddr6],3,&JosBuff2[41348-TableBaseAddr2],1); //隔离状态
 changeBit(&DataBuffer2[40129-DSStartAddr6],4,&JosBuff2[41348-TableBaseAddr2],2); //隔离状态
 changeBit(&DataBuffer2[40129-DSStartAddr6],5,&JosBuff2[41348-TableBaseAddr2],3); //隔离状态

 JosBuff2[41349-TableBaseAddr2]=DataBuffer2[40141-DSStartAddr6]*xishuDY2; //A母排电压--需计算
 if(DataBuffer2[40143-DSStartAddr6]>0)JosBuff2[41250-TableBaseAddr2]=800000/DataBuffer2[40143-DSStartAddr6]; //A母排频率--需计算
 JosBuff2[41351-TableBaseAddr2]=DataBuffer2[40164-DSStartAddr6]*xishuDY2; //B母排电压--需计算
 if(DataBuffer2[40166-DSStartAddr6]>0)JosBuff2[41352-TableBaseAddr2]=800000/DataBuffer2[40166-DSStartAddr6]; //B母排频率--需计算
 
 RchangeBit(&DataBuffer2[40129-DSStartAddr6],6,&JosBuff2[41353-TableBaseAddr2],0); //127IV状态
 RchangeBit(&DataBuffer2[40152-DSStartAddr6],2,&JosBuff2[41353-TableBaseAddr2],1); //127IIV状态
 RchangeBit(&DataBuffer2[40152-DSStartAddr6],3,&JosBuff2[41353-TableBaseAddr2],2); //220V状态
 JosBuff2[41354-TableBaseAddr2]=(DataBuffer2[40224-DSStartAddr6]>>8); //腔体温度
 JosBuff2[41355-TableBaseAddr2]=(DataBuffer2[40224-DSStartAddr6]>>8); //腔体温度

}
/**************************************************

     如果和1140-12回路开关通讯正常，则把其数据装载到buff中以备发送

******************************************/	
void LoadLL3DatatoSendtoJOS2()
{

 /************     回路 一   ***************/
 changeBit(&DataBuffer3[40129-DSStartAddr12],0,&JosBuff3[41361-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41361-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40140-DSStartAddr12],0,&JosBuff3[41361-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40140-DSStartAddr12],1,&JosBuff3[41361-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40140-DSStartAddr12],2,&JosBuff3[41361-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40140-DSStartAddr12],3,&JosBuff3[41361-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40140-DSStartAddr12],4,&JosBuff3[41361-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40140-DSStartAddr12],6,&JosBuff3[41361-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40140-DSStartAddr12],5,&JosBuff3[41361-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40140-DSStartAddr12],7,&JosBuff3[41361-TableBaseAddr3],10); //共用的过压
 changeBit(&DataBuffer3[40144-DSStartAddr12],7,&JosBuff3[41361-TableBaseAddr3],11); //共用的欠压

 XianDaoSet(&DataBuffer3[40221-DSStartAddr12],3,&JosBuff3[41362-TableBaseAddr3]);
 
 JosBuff3[41363-TableBaseAddr3]=DataBuffer3[40130-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41364-TableBaseAddr3]=DataBuffer3[40131-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41365-TableBaseAddr3]=DataBuffer3[40132-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41366-TableBaseAddr3]=DataBuffer3[40145-DSStartAddr12];
 JosBuff3[41367-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41058-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流
 
 /************     回路 二   ***************/
 changeBit(&DataBuffer3[40133-DSStartAddr12],0,&JosBuff3[41379-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40133-DSStartAddr12],1,&JosBuff3[41379-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40144-DSStartAddr12],0,&JosBuff3[41379-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40144-DSStartAddr12],1,&JosBuff3[41379-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40144-DSStartAddr12],2,&JosBuff3[41379-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40144-DSStartAddr12],3,&JosBuff3[41379-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40144-DSStartAddr12],4,&JosBuff3[41379-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40144-DSStartAddr12],6,&JosBuff3[41379-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40144-DSStartAddr12],5,&JosBuff3[41379-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40140-DSStartAddr12],7,&JosBuff3[41379-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40144-DSStartAddr12],7,&JosBuff3[41379-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40221-DSStartAddr12],4,&JosBuff3[41380-TableBaseAddr3]);
 
 JosBuff3[41381-TableBaseAddr3]=DataBuffer3[40134-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41382-TableBaseAddr3]=DataBuffer3[40135-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41383-TableBaseAddr3]=DataBuffer3[40136-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41384-TableBaseAddr3]=DataBuffer3[40146-DSStartAddr12];
 JosBuff3[41385-TableBaseAddr3]=DataBuffer3[40150-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 三   ***************/
 changeBit(&DataBuffer3[40133-DSStartAddr12],2,&JosBuff3[41397-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40133-DSStartAddr12],3,&JosBuff3[41397-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40148-DSStartAddr12],0,&JosBuff3[41397-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40148-DSStartAddr12],1,&JosBuff3[41397-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40148-DSStartAddr12],2,&JosBuff3[41397-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40148-DSStartAddr12],3,&JosBuff3[41397-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40148-DSStartAddr12],4,&JosBuff3[41397-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40148-DSStartAddr12],6,&JosBuff3[41397-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40148-DSStartAddr12],5,&JosBuff3[41397-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40140-DSStartAddr12],7,&JosBuff3[41397-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40144-DSStartAddr12],7,&JosBuff3[41397-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40222-DSStartAddr12],1,&JosBuff3[41398-TableBaseAddr3]);
 
 JosBuff3[41399-TableBaseAddr3]=DataBuffer3[40137-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41400-TableBaseAddr3]=DataBuffer3[40138-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41401-TableBaseAddr3]=DataBuffer3[40139-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41402-TableBaseAddr3]=DataBuffer3[40147-DSStartAddr12];
 JosBuff3[41403-TableBaseAddr3]=DataBuffer3[40151-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 四   ***************/
 changeBit(&DataBuffer3[40152-DSStartAddr12],0,&JosBuff3[41415-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40152-DSStartAddr12],1,&JosBuff3[41415-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40163-DSStartAddr12],0,&JosBuff3[41415-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40163-DSStartAddr12],1,&JosBuff3[41415-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40163-DSStartAddr12],2,&JosBuff3[41415-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40163-DSStartAddr12],3,&JosBuff3[41415-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40163-DSStartAddr12],4,&JosBuff3[41415-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40163-DSStartAddr12],6,&JosBuff3[41415-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40163-DSStartAddr12],5,&JosBuff3[41415-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40163-DSStartAddr12],7,&JosBuff3[41415-TableBaseAddr3],10); //共用的过压
 changeBit(&DataBuffer3[40167-DSStartAddr12],7,&JosBuff3[41415-TableBaseAddr3],11); //共用的欠压

 XianDaoSet(&DataBuffer3[40222-DSStartAddr12],2,&JosBuff3[41416-TableBaseAddr3]);
 
 JosBuff3[41417-TableBaseAddr3]=DataBuffer3[40153-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41418-TableBaseAddr3]=DataBuffer3[40154-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41419-TableBaseAddr3]=DataBuffer3[40155-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41420-TableBaseAddr3]=DataBuffer3[40168-DSStartAddr12];
 JosBuff3[41421-TableBaseAddr3]=DataBuffer3[40172-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41058-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流
 
 /************     回路 五   ***************/
 changeBit(&DataBuffer3[40156-DSStartAddr12],0,&JosBuff3[41433-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40156-DSStartAddr12],1,&JosBuff3[41433-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40167-DSStartAddr12],0,&JosBuff3[41433-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40167-DSStartAddr12],1,&JosBuff3[41433-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40167-DSStartAddr12],2,&JosBuff3[41433-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40167-DSStartAddr12],3,&JosBuff3[41433-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40167-DSStartAddr12],4,&JosBuff3[41433-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40167-DSStartAddr12],6,&JosBuff3[41433-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40167-DSStartAddr12],5,&JosBuff3[41433-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40163-DSStartAddr12],7,&JosBuff3[41433-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40167-DSStartAddr12],7,&JosBuff3[41433-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40222-DSStartAddr12],3,&JosBuff3[41434-TableBaseAddr3]);
 
 JosBuff3[41435-TableBaseAddr3]=DataBuffer3[40157-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41436-TableBaseAddr3]=DataBuffer3[40158-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41437-TableBaseAddr3]=DataBuffer3[40159-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41438-TableBaseAddr3]=DataBuffer3[40169-DSStartAddr12];
 JosBuff3[41439-TableBaseAddr3]=DataBuffer3[40173-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 六   ***************/
 changeBit(&DataBuffer3[40156-DSStartAddr12],2,&JosBuff3[41451-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40156-DSStartAddr12],3,&JosBuff3[41451-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40171-DSStartAddr12],0,&JosBuff3[41451-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40171-DSStartAddr12],1,&JosBuff3[41451-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40171-DSStartAddr12],2,&JosBuff3[41451-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40171-DSStartAddr12],3,&JosBuff3[41451-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40171-DSStartAddr12],4,&JosBuff3[41451-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40171-DSStartAddr12],6,&JosBuff3[41451-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40171-DSStartAddr12],5,&JosBuff3[41451-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40163-DSStartAddr12],7,&JosBuff3[41451-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40167-DSStartAddr12],7,&JosBuff3[41451-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40222-DSStartAddr12],4,&JosBuff3[41452-TableBaseAddr3]);
 
 JosBuff3[41453-TableBaseAddr3]=DataBuffer3[40160-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41454-TableBaseAddr3]=DataBuffer3[40161-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41455-TableBaseAddr3]=DataBuffer3[40162-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41456-TableBaseAddr3]=DataBuffer3[40170-DSStartAddr12];
 JosBuff3[41457-TableBaseAddr3]=DataBuffer3[40174-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流


/************     回路 七   ***************/
 changeBit(&DataBuffer3[40175-DSStartAddr12],0,&JosBuff3[41469-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40175-DSStartAddr12],1,&JosBuff3[41469-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40186-DSStartAddr12],0,&JosBuff3[41469-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40186-DSStartAddr12],1,&JosBuff3[41469-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40186-DSStartAddr12],2,&JosBuff3[41469-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40186-DSStartAddr12],3,&JosBuff3[41469-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40186-DSStartAddr12],4,&JosBuff3[41469-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40186-DSStartAddr12],6,&JosBuff3[41469-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40186-DSStartAddr12],5,&JosBuff3[41469-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40186-DSStartAddr12],7,&JosBuff3[41469-TableBaseAddr3],10); //共用的过压
 changeBit(&DataBuffer3[40190-DSStartAddr12],7,&JosBuff3[41469-TableBaseAddr3],11); //共用的欠压

 XianDaoSet(&DataBuffer3[40223-DSStartAddr12],1,&JosBuff3[41470-TableBaseAddr3]);
 
 JosBuff3[41471-TableBaseAddr3]=DataBuffer3[40176-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41472-TableBaseAddr3]=DataBuffer3[40177-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41473-TableBaseAddr3]=DataBuffer3[40178-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41474-TableBaseAddr3]=DataBuffer3[40191-DSStartAddr12];
 JosBuff3[41475-TableBaseAddr3]=DataBuffer3[40195-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41058-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流
 
 /************     回路 八   ***************/
 changeBit(&DataBuffer3[40179-DSStartAddr12],0,&JosBuff3[41487-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40179-DSStartAddr12],1,&JosBuff3[41487-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40190-DSStartAddr12],0,&JosBuff3[41487-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40190-DSStartAddr12],1,&JosBuff3[41487-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40190-DSStartAddr12],2,&JosBuff3[41487-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40190-DSStartAddr12],3,&JosBuff3[41487-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40190-DSStartAddr12],4,&JosBuff3[41487-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40190-DSStartAddr12],6,&JosBuff3[41487-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40190-DSStartAddr12],5,&JosBuff3[41487-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40186-DSStartAddr12],7,&JosBuff3[41487-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40190-DSStartAddr12],7,&JosBuff3[41487-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40223-DSStartAddr12],2,&JosBuff3[41488-TableBaseAddr3]);
 
 JosBuff3[41489-TableBaseAddr3]=DataBuffer3[40180-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41490-TableBaseAddr3]=DataBuffer3[40181-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41491-TableBaseAddr3]=DataBuffer3[40182-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41492-TableBaseAddr3]=DataBuffer3[40192-DSStartAddr12];
 JosBuff3[41493-TableBaseAddr3]=DataBuffer3[40196-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 九   ***************/
 changeBit(&DataBuffer3[40179-DSStartAddr12],2,&JosBuff3[41505-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40179-DSStartAddr12],3,&JosBuff3[41505-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40194-DSStartAddr12],0,&JosBuff3[41505-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40194-DSStartAddr12],1,&JosBuff3[41505-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40194-DSStartAddr12],2,&JosBuff3[41505-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40194-DSStartAddr12],3,&JosBuff3[41505-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41051-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40194-DSStartAddr12],4,&JosBuff3[41505-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40194-DSStartAddr12],6,&JosBuff3[41505-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40194-DSStartAddr12],5,&JosBuff3[41505-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40186-DSStartAddr12],7,&JosBuff3[41505-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40190-DSStartAddr12],7,&JosBuff3[41505-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40221-DSStartAddr12],2,&JosBuff3[41506-TableBaseAddr3]);
 
 JosBuff3[41507-TableBaseAddr3]=DataBuffer3[40183-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41508-TableBaseAddr3]=DataBuffer3[40184-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41509-TableBaseAddr3]=DataBuffer3[40185-DSStartAddr12]*xishuDL3_1_9;
 JosBuff3[41510-TableBaseAddr3]=DataBuffer3[40193-DSStartAddr12];
 JosBuff3[41511-TableBaseAddr3]=DataBuffer3[40197-DSStartAddr12]*xishuGL3_1_9;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流
 
/************     回路 十   ***************/
 changeBit(&DataBuffer3[40198-DSStartAddr12],0,&JosBuff3[41523-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40198-DSStartAddr12],1,&JosBuff3[41523-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40209-DSStartAddr12],0,&JosBuff3[41523-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40209-DSStartAddr12],1,&JosBuff3[41523-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40209-DSStartAddr12],2,&JosBuff3[41523-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40209-DSStartAddr12],3,&JosBuff3[41523-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41523-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40209-DSStartAddr12],4,&JosBuff3[41523-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40209-DSStartAddr12],6,&JosBuff3[41523-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40209-DSStartAddr12],5,&JosBuff3[41523-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40209-DSStartAddr12],7,&JosBuff3[41523-TableBaseAddr3],10); //共用的过压
 changeBit(&DataBuffer3[40213-DSStartAddr12],7,&JosBuff3[41523-TableBaseAddr3],11); //共用的欠压

 XianDaoSet(&DataBuffer3[40224-DSStartAddr12],1,&JosBuff3[41524-TableBaseAddr3]);	////?????????????????//
 
 JosBuff3[41525-TableBaseAddr3]=DataBuffer3[40199-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41526-TableBaseAddr3]=DataBuffer3[40200-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41527-TableBaseAddr3]=DataBuffer3[40201-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41528-TableBaseAddr3]=DataBuffer3[40214-DSStartAddr12];
 JosBuff3[41529-TableBaseAddr3]=DataBuffer3[40218-DSStartAddr12]*xishuGL3_10_12;
 //JosBuff3[41058-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 十一   ***************/
 changeBit(&DataBuffer3[40202-DSStartAddr12],0,&JosBuff3[41537-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40202-DSStartAddr12],1,&JosBuff3[41537-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40209-DSStartAddr12],0,&JosBuff3[41537-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40209-DSStartAddr12],1,&JosBuff3[41537-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40209-DSStartAddr12],2,&JosBuff3[41537-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40209-DSStartAddr12],3,&JosBuff3[41537-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41537-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40209-DSStartAddr12],4,&JosBuff3[41537-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40209-DSStartAddr12],6,&JosBuff3[41537-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40209-DSStartAddr12],5,&JosBuff3[41537-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40209-DSStartAddr12],7,&JosBuff3[41537-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40213-DSStartAddr12],7,&JosBuff3[41537-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40224-DSStartAddr12],2,&JosBuff3[41538-TableBaseAddr3]);
 
 JosBuff3[41539-TableBaseAddr3]=DataBuffer3[40203-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41540-TableBaseAddr3]=DataBuffer3[40204-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41541-TableBaseAddr3]=DataBuffer3[40205-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41542-TableBaseAddr3]=DataBuffer3[40215-DSStartAddr12];
 JosBuff3[41543-TableBaseAddr3]=DataBuffer3[40219-DSStartAddr12]*xishuGL3_10_12;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

/************     回路 十二   ***************/
 changeBit(&DataBuffer3[40202-DSStartAddr12],2,&JosBuff3[41555-TableBaseAddr3],0);
 changeBit(&DataBuffer3[40202-DSStartAddr12],3,&JosBuff3[41555-TableBaseAddr3],1); 
 changeBit(&DataBuffer3[40217-DSStartAddr12],0,&JosBuff3[41555-TableBaseAddr3],2);
 changeBit(&DataBuffer3[40217-DSStartAddr12],1,&JosBuff3[41555-TableBaseAddr3],3);
 changeBit(&DataBuffer3[40217-DSStartAddr12],2,&JosBuff3[41555-TableBaseAddr3],4);
 changeBit(&DataBuffer3[40217-DSStartAddr12],3,&JosBuff3[41555-TableBaseAddr3],5);
 //changeBit(&DataBuffer3[40129-DSStartAddr12],1,&JosBuff3[41555-TableBaseAddr3],6);//1140 无漏电保护
 changeBit(&DataBuffer3[40217-DSStartAddr12],4,&JosBuff3[41555-TableBaseAddr3],7);
 changeBit(&DataBuffer3[40217-DSStartAddr12],6,&JosBuff3[41555-TableBaseAddr3],8);
 changeBit(&DataBuffer3[40217-DSStartAddr12],5,&JosBuff3[41555-TableBaseAddr3],9);
 changeBit(&DataBuffer3[40209-DSStartAddr12],7,&JosBuff3[41555-TableBaseAddr3],10);//共用的过压
 changeBit(&DataBuffer3[40213-DSStartAddr12],7,&JosBuff3[41555-TableBaseAddr3],11);//共用的欠压

 XianDaoSet(&DataBuffer3[40221-DSStartAddr12],1,&JosBuff3[41556-TableBaseAddr3]); //数据表暂时无
 
 JosBuff3[41557-TableBaseAddr3]=DataBuffer3[40206-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41558-TableBaseAddr3]=DataBuffer3[40207-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41559-TableBaseAddr3]=DataBuffer3[40208-DSStartAddr12]*xishuDL3_10_12;
 JosBuff3[41560-TableBaseAddr3]=DataBuffer3[40216-DSStartAddr12];
 JosBuff3[41561-TableBaseAddr3]=DataBuffer3[40220-DSStartAddr12]*xishuGL3_10_12;
 //JosBuff3[41076-TableBaseAddr3]=DataBuffer3[40149-DSStartAddr12]; //1140 无零序电流

 changeBit(&DataBuffer3[40129-DSStartAddr12],2,&JosBuff3[41572-TableBaseAddr3],0); //隔离状态
 changeBit(&DataBuffer3[40129-DSStartAddr12],3,&JosBuff3[41572-TableBaseAddr3],1); //隔离状态
 changeBit(&DataBuffer3[40129-DSStartAddr12],4,&JosBuff3[41572-TableBaseAddr3],2); //隔离状态
 changeBit(&DataBuffer3[40129-DSStartAddr12],5,&JosBuff3[41572-TableBaseAddr3],3); //隔离状态

 JosBuff3[41573-TableBaseAddr3]=DataBuffer3[40141-DSStartAddr12]*xishuDY1; //A母排电压--需计算
 if(DataBuffer3[40143-DSStartAddr12]>0)JosBuff3[41574-TableBaseAddr3]=800000/DataBuffer3[40143-DSStartAddr12]; //A母排频率--需计算
 JosBuff3[41575-TableBaseAddr3]=DataBuffer3[40164-DSStartAddr12]*xishuDY1; //B母排电压--需计算
 if(DataBuffer3[40166-DSStartAddr12]>0)JosBuff3[41576-TableBaseAddr3]=800000/DataBuffer3[40166-DSStartAddr12]; //B母排频率--需计算
 
 RchangeBit(&DataBuffer3[40129-DSStartAddr12],6,&JosBuff3[41577-TableBaseAddr3],0); //127IV状态

 RchangeBit(&DataBuffer3[40152-DSStartAddr12],2,&JosBuff3[41577-TableBaseAddr3],1); //127IIV状态

 RchangeBit(&DataBuffer3[40152-DSStartAddr12],3,&JosBuff3[41577-TableBaseAddr3],2); //220V状态

 JosBuff3[41578-TableBaseAddr3]=(DataBuffer3[40224-DSStartAddr12]>>8); //腔体温度
 JosBuff3[41579-TableBaseAddr3]=(DataBuffer3[40224-DSStartAddr12]>>8); //腔体温度

}