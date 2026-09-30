/**
 * @brief 速度模块（PC-->>SIM）
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup SMB
 * @version V1.0.0
 *
 * @date 2023-08-31   Dengshangjie：首次创建
 *
 */

#include "Smb.h"
#include "Speed.h"
#include "spi.h"
/* begin modified by deng 2023.11.6 */
#include "Slib.h"
#include "Log.h"
/* end modified by deng 2023.11.6  */
#include "Use_Task.h"

//业务的速度单位是cm/s，车仿的速度单位是m/s

static uint32_t g_ulConversion; //**< 速度和频率转换：“速度 = (pi * 频率 * 直径) / 轮孔数” */
static int32_t g_lFrequencyA;  //**< 速度计A频率 /
static int32_t g_lFrequencyB;  //**< 速度计B频率 /
/* begin modified by deng 2023年11月30日 */
//static int32_t  g_lCurrentPulseA; //**< 速度计A累计脉冲数 /
//static int32_t  g_lCurrentPulseB; //**< 速度计B累计脉冲数 /
int32_t  g_lCurrentPulseA; //**< 速度计A累计脉冲数 /
int32_t  g_lCurrentPulseB; //**< 速度计B累计脉冲数 /
/* end modified by deng 2023年11月30日 */
OS_EVENT *g_pstSpeedMutex = NULL;

// 初始化速度
int32_t SPD_Init(void)
{
    uint8_t err = 0;
    /* begin add by deng 2024年12月1日 */
    //fpga复位信号
    uint8_t aucSpeedInit[46] = {0xff, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
    /* end add by deng 2024年12月1日 */

    //初始化速度
    if (g_pstSpeedMutex == NULL)
    {
        //初始化速度和频率转换值
        g_ulConversion = 0U;

        //初始化频率
        g_lFrequencyA = 0U;
        g_lFrequencyB = 0U;

        //初始化累计脉冲
        g_lCurrentPulseA = 0U;
        g_lCurrentPulseB = 0U;

        g_pstSpeedMutex = OSMutexCreate(SPEED_MUTEX_PRIO, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexCreate error err(%d)", err);
            return SCP_ERROR;
        }
    }
    else
    {
        OSMutexPend(g_pstSpeedMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }
        //初始化速度和频率转换值
        g_ulConversion = 0U;

          //初始化频率
        g_lFrequencyA = 0U;
        g_lFrequencyB = 0U;

        //初始化累计脉冲
        g_lCurrentPulseA = 0U;
        g_lCurrentPulseB = 0U;

        err = OSMutexPost(g_pstSpeedMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }

/* begin add by deng 2024年12月1日 */
    //与spi耦合（todo:去耦合）
    HAL_SendSpeed(aucSpeedInit, sizeof(aucSpeedInit));//fpga复位信号
	HAL_SendSpeed(aucSpeedInit, sizeof(aucSpeedInit));//fpga复位信号
//    SPD_SetVelocity(200u, 200u);//发段脉冲
//    HAL_Delay(5);//延时
//    SPD_SetVelocity(0u, 0u);//速度置为0
/* end add by deng 2024年12月1日 */
    return SCP_OK;
}

// 设置速度和频率换算值
int32_t SPD_SetVelocityConversion(uint32_t ulConversion)
{
    uint8_t err = 0;

    // 检查输入值是否有效
    if (ulConversion == 0U)
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocityConversion Invalid Conversion value."); // 无效输入，返回错误并打印消息
        return SCP_ERROR;
    }
    else
    {
        OSMutexPend(g_pstSpeedMutex, 0, &err);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_Init OSMutexPend error err(%d)", err);
            return SCP_ERROR;
        }
        // 将速度和频率换算值赋值给局部变量
        g_ulConversion = ulConversion;

        err = OSMutexPost(g_pstSpeedMutex);
        if (err != OS_ERR_NONE)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_Init OSMutexPost error err(%d)", err);
            return SCP_ERROR;
        }
    }
    return SCP_OK;  // 返回成功，表示已经成功获取速度和频率换算值
}

// 设置速度
int32_t SPD_SetVelocity(int32_t lVelocityA, int32_t lVelocityB)
{
/* begin modified by deng 2023.11.6 */
//    SMB_SpeedChannelCtrl_S stSpeedMsgA[3];
//    SMB_SpeedChannelCtrl_S stSpeedMsgB[3];
    int32_t lFrequencyA;
    int32_t lFrequencyB;
    uint32_t ulConversion;
    S_SpeedMsg_S stSpeedMsg = {0};
    uint8_t err = 0;
    uint8_t ucDutyCycle = 50U;    //设置占空比为默认值50%
    static uint8_t ucDirState = 1u; //1为正向，0为负向
/* begin add by deng 2024年12月1日 */
//    static uint8_t ucPosDir = 0U;
//    static uint8_t ucNegDir = 0U;
/* end add by deng 2024年12月1日 */

TRY_BEGIN
    OSMutexPend(g_pstSpeedMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_Init OSMutexPend error err(%d)", err);
        THROW(SCP_ERROR);
    }

    ulConversion = g_ulConversion;

    err = OSMutexPost(g_pstSpeedMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_Init OSMutexPost error err(%d)", err);
        THROW(SCP_ERROR);
    }

    /* begin modified by deng 2024年8月15日 用于调试*/
//    LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocity lVelocityA(%d) & lVelocityB(%d)", lVelocityA, lVelocityB);
    /* begin modified by deng 2024年8月15日 */

    // 检查转换值是否为无效
    if (ulConversion == 0U)
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocity Invalid Conversion value");// 无效转换值，返回错误
        THROW(SCP_ERROR);
    }
/* begin modified by deng 2024年8月15日 */
    // 速度A转换为频率
    lFrequencyA = (S_ABS(lVelocityA) * 1000) / ulConversion;  //转换值为放大1000倍，所以速度放大1000倍（目前车仿：轮径842cm，轮圈200孔）
//    g_lFrequencyA = (lVelocityA * 2000000)/(842 *3142);//轮径842cm，轮圈200孔(约等于Freq = V / 1.3)
//    g_lFrequencyA = (lVelocityA * 8410000)/(805 *3142);//轮径805cm，轮圈约841孔(约等于Freq = V / 0.3)(新版适配141项目)
   
 // 速度B转换为频率
    lFrequencyB = (S_ABS(lVelocityB) * 1000) / ulConversion;  //转换值为放大1000倍，所以速度放大1000倍（目前车仿：轮径842cm，轮圈200孔）
//    g_lFrequencyB = (lVelocityB * 2000000)/(842 *3142);//轮径842cm，轮圈200孔(约等于Freq = V / 1.3)
//    g_lFrequencyB = (lVelocityB * 8410000)/(805 *3142);//轮径805cm，轮圈约841孔(约等于Freq = V / 0.3)(新版适配141项目)
/* end modified by deng 2024年8月15日 */

    OSMutexPend(g_pstSpeedMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPend error err(%d)", err);
        THROW(SCP_ERROR);
    }
/* begin modified by deng 2024年12月1日 */
//    g_lFrequencyA = lFrequencyA;
//    g_lFrequencyB = lFrequencyB;
    if ((lVelocityA >= 0) && (lVelocityB >= 0))
    {
        g_lFrequencyA = lFrequencyA;
        g_lFrequencyB = lFrequencyB;
    }
    else if ((lVelocityA < 0) && (lVelocityB < 0))
    {
        g_lFrequencyA = -lFrequencyA;
        g_lFrequencyB = -lFrequencyB;

    }
/* end modified by deng 2024年12月1日 */
    err = OSMutexPost(g_pstSpeedMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPost error err(%d)", err);
        THROW(SCP_ERROR);
    }

    //设置速度计参数
    if ((lVelocityA > 0) && (lVelocityB > 0))
    {
        //车辆前进
        stSpeedMsg.ucSPDHeadA = 0xFF;     //报文头A
        stSpeedMsg.ucSPDHeadB = 0xFE;     //报文头B
/* begin modified by deng 2025年3月1日 */
//        stSpeedMsg.ucChannelValid = 0x3F; //默认所有通道使能
        stSpeedMsg.ucChannelValid = 0x1B; //使能1、2通道
        //方向转向处理
        if(ucDirState == 1u)
        {
            //继续正向行驶不做处理
            stSpeedMsg.ausMotorAChannelFreq[0] = ntohl(SPEED_FREQ_TOTAL / lFrequencyA); //电机A通道1频率
            stSpeedMsg.ausMotorAChannelFreq[1] = ntohl(SPEED_FREQ_TOTAL / lFrequencyA); //电机A通道2频率
            stSpeedMsg.ausMotorAChannelFreq[2] = ntohl(SPEED_FREQ_TOTAL / lFrequencyA); //电机A通道3频率
            stSpeedMsg.ausMotorBChannelFreq[0] = ntohl(SPEED_FREQ_TOTAL / lFrequencyB); //电机B通道1频率
            stSpeedMsg.ausMotorBChannelFreq[1] = ntohl(SPEED_FREQ_TOTAL / lFrequencyB); //电机B通道2频率
            stSpeedMsg.ausMotorBChannelFreq[2] = ntohl(SPEED_FREQ_TOTAL / lFrequencyB); //电机B通道3频率
        }
        else if(ucDirState == 0u)
        {
            //发送一次反向置位
            stSpeedMsg.ausMotorAChannelFreq[0] = ntohl((SPEED_FREQ_TOTAL / lFrequencyA) + 0x80000000u); //电机A通道1频率
            stSpeedMsg.ausMotorAChannelFreq[1] = ntohl((SPEED_FREQ_TOTAL / lFrequencyA) + 0x80000000u); //电机A通道2频率
            stSpeedMsg.ausMotorAChannelFreq[2] = ntohl((SPEED_FREQ_TOTAL / lFrequencyA) + 0x80000000u); //电机A通道3频率
            stSpeedMsg.ausMotorBChannelFreq[0] = ntohl((SPEED_FREQ_TOTAL / lFrequencyB) + 0x80000000u); //电机B通道1频率
            stSpeedMsg.ausMotorBChannelFreq[1] = ntohl((SPEED_FREQ_TOTAL / lFrequencyB) + 0x80000000u); //电机B通道2频率
            stSpeedMsg.ausMotorBChannelFreq[2] = ntohl((SPEED_FREQ_TOTAL / lFrequencyB) + 0x80000000u); //电机B通道3频率
        }
        ucDirState = 1u;//正向行驶

//        stSpeedMsg.ausMotorAPhase[0] = ntohs((500 * 1  ) / 360);//电机A通道1相位,默认相位为0°
//        stSpeedMsg.ausMotorAPhase[1] = ntohs((500 * 120) / 360);//电机A通道2相位,默认相位为120°
//        stSpeedMsg.ausMotorAPhase[2] = ntohs((500 * 240) / 360);//电机A通道3相位,默认相位为240°
//        stSpeedMsg.ausMotorBPhase[0] = ntohs((500 * 240) / 360);//电机B通道1相位,默认相位为240°
//        stSpeedMsg.ausMotorBPhase[1] = ntohs((500 * 120) / 360);//电机B通道2相位,默认相位为120°
//        stSpeedMsg.ausMotorBPhase[2] = ntohs((500 * 1  ) / 360);//电机B通道3相位,默认相位为0°
        stSpeedMsg.ausMotorAPhase[0] = ntohs((10 * 1) / 360);//电机A通道1相位,默认相位为0°
        stSpeedMsg.ausMotorAPhase[1] = ntohs((10 * 90) / 360);//电机A通道2相位,默认相位为90°
        stSpeedMsg.ausMotorBPhase[0] = ntohs((10 * 90) / 360);//电机B通道1相位,默认相位为90°
        stSpeedMsg.ausMotorBPhase[1] = ntohs((10 * 1) / 360);//电机B通道2相位,默认相位为0°
/* end modified by deng 2025年3月1日 */
/* end modified by deng 2024年12月1日 */
        stSpeedMsg.aucMotorADutyCycle[0] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[1] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[2] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[0] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[1] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[2] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.ucDataXOR = S_CheckXOR((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S) - 1U); //异或校验

        // 通过FPGA设置速度计A和B
        if (HAL_SendSpeed((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S)) != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocity Failed to Set Speed Positive data");
            THROW(SCP_ERROR);
        }
    }
    else if((lVelocityA < 0) && (lVelocityB < 0))
    {
        //车辆后退
        stSpeedMsg.ucSPDHeadA = 0xFF;     //报文头A
        stSpeedMsg.ucSPDHeadB = 0xFE;     //报文头B
/* begin modified by deng 2025年3月1日 */
//        stSpeedMsg.ucChannelValid = 0x3F; //默认所有通道使能
        stSpeedMsg.ucChannelValid = 0x1B; //使能1、2通道
/* end modified by deng 2025年3月1日 */
/* begin modified by deng 2024年12月1日 */
        //方向转向处理
        if (ucDirState == 0u)
        {
            //继续正向行驶不做处理
            stSpeedMsg.ausMotorAChannelFreq[0] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)); //电机A通道1频率
            stSpeedMsg.ausMotorAChannelFreq[1] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)); //电机A通道2频率
            stSpeedMsg.ausMotorAChannelFreq[2] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)); //电机A通道3频率
            stSpeedMsg.ausMotorBChannelFreq[0] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)); //电机B通道1频率
            stSpeedMsg.ausMotorBChannelFreq[1] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)); //电机B通道2频率
            stSpeedMsg.ausMotorBChannelFreq[2] = ntohl(SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)); //电机B通道3频率
        }
        else if(ucDirState == 1u)
        {
            //发送一次反向置位
            stSpeedMsg.ausMotorAChannelFreq[0] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)) + 0x80000000u); //电机A通道1频率
            stSpeedMsg.ausMotorAChannelFreq[1] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)) + 0x80000000u); //电机A通道2频率
            stSpeedMsg.ausMotorAChannelFreq[2] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyA)) + 0x80000000u); //电机A通道3频率
            stSpeedMsg.ausMotorBChannelFreq[0] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)) + 0x80000000u); //电机B通道1频率
            stSpeedMsg.ausMotorBChannelFreq[1] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)) + 0x80000000u); //电机B通道2频率
            stSpeedMsg.ausMotorBChannelFreq[2] = ntohl((SPEED_FREQ_TOTAL / S_ABS(lFrequencyB)) + 0x80000000u); //电机B通道3频率
        }
        ucDirState = 0u;//负向行驶

/* begin modified by deng 2025年3月1日 */
//        stSpeedMsg.ausMotorAPhase[0] = ntohs((500 * 1  ) / 360);//电机A通道1相位,默认相位为0°
//        stSpeedMsg.ausMotorAPhase[1] = ntohs((500 * 120) / 360);//电机A通道2相位,默认相位为120°
//        stSpeedMsg.ausMotorAPhase[2] = ntohs((500 * 240) / 360);//电机A通道3相位,默认相位为240°
//        stSpeedMsg.ausMotorBPhase[0] = ntohs((500 * 240) / 360);//电机B通道1相位,默认相位为240°
//        stSpeedMsg.ausMotorBPhase[1] = ntohs((500 * 120) / 360);//电机B通道2相位,默认相位为120°
//        stSpeedMsg.ausMotorBPhase[2] = ntohs((500 * 1  ) / 360);//电机B通道3相位,默认相位为0°
        stSpeedMsg.ausMotorAPhase[0] = ntohs((10 * 90) / 360);//电机A通道1相位,默认相位为90°
        stSpeedMsg.ausMotorAPhase[1] = ntohs((10 * 1) / 360);//电机A通道2相位,默认相位为1°
        stSpeedMsg.ausMotorBPhase[0] = ntohs((10 * 1) / 360);//电机B通道1相位,默认相位为1°
        stSpeedMsg.ausMotorBPhase[1] = ntohs((10 * 90) / 360);//电机B通道2相位,默认相位为90°
/* end modified by deng 2025年3月1日 */
/* end modified by deng 2024年12月1日 */
        stSpeedMsg.aucMotorADutyCycle[0] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[1] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[2] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[0] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[1] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[2] = ucDutyCycle;//默认不设置占空比
        stSpeedMsg.ucDataXOR = S_CheckXOR((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S) - 1U); //异或校验

        // 通过FPGA设置速度计A和B。
        if (HAL_SendSpeed((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S)) != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocity Failed to Set Speed negative data");
            THROW(SCP_ERROR);
        }
    }
    else if((lVelocityA == 0) && (lVelocityB == 0))
    {
        //车辆静止a
        stSpeedMsg.ucSPDHeadA = 0xFF;     //报文头A
        stSpeedMsg.ucSPDHeadB = 0xFE;     //报文头B
/* begin modified by deng 2024年12月1日 */
//        stSpeedMsg.ucChannelValid = 0x00; //默认所有通道不使能
//        stSpeedMsg.ausMotorAChannelFreq[0] = 0u; //电机A通道1频率
//        stSpeedMsg.ausMotorAChannelFreq[1] = 0u; //电机A通道2频率
//        stSpeedMsg.ausMotorAChannelFreq[2] = 0u; //电机A通道3频率
//        stSpeedMsg.ausMotorBChannelFreq[0] = 0u; //电机B通道1频率
//        stSpeedMsg.ausMotorBChannelFreq[1] = 0u; //电机B通道2频率
//        stSpeedMsg.ausMotorBChannelFreq[2] = 0u; //电机B通道3频率
/* begin modified by deng 2025年3月1日 */
//        stSpeedMsg.ucChannelValid = 0x3F; //默认所有通道使能
        stSpeedMsg.ucChannelValid = 0x1B; //使能1、2通道
/* end modified by deng 2025年3月1日 */
        stSpeedMsg.ausMotorAChannelFreq[0] = ntohl(0xEFFFFFFFu); //电机A通道1频率
        stSpeedMsg.ausMotorAChannelFreq[1] = ntohl(0xEFFFFFFFu); //电机A通道2频率
        stSpeedMsg.ausMotorAChannelFreq[2] = ntohl(0xEFFFFFFFu); //电机A通道3频率
        stSpeedMsg.ausMotorBChannelFreq[0] = ntohl(0xEFFFFFFFu); //电机B通道1频率
        stSpeedMsg.ausMotorBChannelFreq[1] = ntohl(0xEFFFFFFFu); //电机B通道2频率
        stSpeedMsg.ausMotorBChannelFreq[2] = ntohl(0xEFFFFFFFu); //电机B通道3频率
/* end modified by deng 2024年12月1日 */
        stSpeedMsg.ausMotorAPhase[0] = 1u;//电机A通道1相位,默认相位为0°
        stSpeedMsg.ausMotorAPhase[1] = 1u;//电机A通道2相位,默认相位为0°
        stSpeedMsg.ausMotorAPhase[2] = 1u;//电机A通道3相位,默认相位为0°
        stSpeedMsg.ausMotorBPhase[0] = 1u;//电机B通道1相位,默认相位为0°
        stSpeedMsg.ausMotorBPhase[1] = 1u;//电机B通道2相位,默认相位为0°
        stSpeedMsg.ausMotorBPhase[2] = 1u;//电机B通道3相位,默认相位为0°
        stSpeedMsg.aucMotorADutyCycle[0] = 255u;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[1] = 255u;//默认不设置占空比
        stSpeedMsg.aucMotorADutyCycle[2] = 255u;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[0] = 255u;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[1] = 255u;//默认不设置占空比
        stSpeedMsg.aucMotorBDutyCycle[2] = 255u;//默认不设置占空比
        stSpeedMsg.ucDataXOR = S_CheckXOR((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S) - 1U); //异或校验

        // 通过FPGA设置速度计A和B
        if (HAL_SendSpeed((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S)) != SCP_OK)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetVelocity Failed to Set Speed zero data");
            THROW(SCP_ERROR);
        }
    }
    // 无效速度值
    else
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, " Invalid usFrequency value sFrequencyA(%d) or sFrequencyB(%d)",lFrequencyA,lFrequencyB);// 无效转换值，返回错误
        THROW(SCP_ERROR);
    }

TRY_END
    return ERROR_CODE;
/* end modified by deng 2023.11.6  */
}

// 获取速度
void SPD_GetVelocity(int32_t *plVelocityA, int32_t *plVelocityB)
{  
    int32_t lVelocityA;
    int32_t lVelocityB;
    uint8_t err = 0;

    OSMutexPend(g_pstSpeedMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPend error err(%d)", err);
    }
    // 将频率值转为速度值
    lVelocityA = g_lFrequencyA * g_ulConversion;
    lVelocityB = g_lFrequencyB * g_ulConversion;

    err = OSMutexPost(g_pstSpeedMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPost error err(%d)", err);
    }

    // 将速度值赋值给指针
    *plVelocityA = lVelocityA;  
    *plVelocityB = lVelocityB;  
  
    // 打印接收到的速度值  
/* begin modified by deng 2023年11月25日 */
    LOG(LOG_INFO, LOG_MOD_SPEED, "Geted velocity value A: %d\n", lVelocityA);
    LOG(LOG_INFO, LOG_MOD_SPEED, "Geted velocity value B: %d\n", lVelocityB);
/* end modified by deng 2023年11月25日 */
}  


// 设置速度计频率（目前不支持反向）
int32_t SPD_SetSpeedFreq(SMB_RecvSpeedMsg_S *pstSpeedMsg)
{
    uint8_t ucNum;
    S_SpeedMsg_S stSpeedMsg;
    uint8_t ucChannelValid = 0x00; //所有通道无效
    uint32_t usFrequenceA[3];
    int16_t sPhaseA[3];
    uint8_t ucDutyCycleA[3] = {0x00,0x00,0x00};
/* begin add by deng 2024年3月31日 */
    uint32_t usFrequenceB[3];
    int16_t sPhaseB[3];
    uint8_t ucDutyCycleB[3] = {0x00,0x00,0x00};
/* end add by deng 2024年3月31日 */
    int32_t lErrorCode = SCP_OK;
    int32_t lRet = SCP_OK;

    //判断速度计A和B参数
    for (ucNum = 0; ucNum < CHANNEL_COUNT; ucNum++)
    {
        if (pstSpeedMsg->stSpeedA[ucNum].ucEnable == 1u)
        {
            ucChannelValid = ucChannelValid | (1u << ucNum);   //通道A使能
        }
        else
        {
            ucDutyCycleA[ucNum] = pstSpeedMsg->stSpeedA[ucNum].ucDutyCycle;  //设置占空比
        }
        /* begin add by deng 2024年3月31日 */
        if (pstSpeedMsg->stSpeedB[ucNum].ucEnable == 1u)
        {
            ucChannelValid = ucChannelValid | (1u << (ucNum + 3u));   //通道B使能
        }
        else
        {
            ucDutyCycleB[ucNum] = pstSpeedMsg->stSpeedB[ucNum].ucDutyCycle;  //设置占空比
        }
        /* end add by deng 2024年3月31日 */

        pstSpeedMsg->stSpeedA[ucNum].usFreq = S_ABS(pstSpeedMsg->stSpeedA[ucNum].usFreq); //取A频率的绝对值
/* begin add by deng 2024年3月31日 */
        pstSpeedMsg->stSpeedB[ucNum].usFreq = S_ABS(pstSpeedMsg->stSpeedB[ucNum].usFreq); //取B频率的绝对值
/* end add by deng 2024年3月31日 */
        if (pstSpeedMsg->stSpeedA[ucNum].usFreq > SPEED_FREQ_MAX | pstSpeedMsg->stSpeedB[ucNum].usFreq > SPEED_FREQ_MAX)
        {
            LOG(LOG_ERROR, LOG_MOD_SPEED, " SPD_SetSpeedFreqA Channel(%d) error Frequence data(%d)", ucNum, pstSpeedMsg->stSpeedA[ucNum].usFreq);
            LOG(LOG_ERROR, LOG_MOD_SPEED, " SPD_SetSpeedFreqB Channel(%d) error Frequence data(%d)", ucNum, pstSpeedMsg->stSpeedB[ucNum].usFreq);
            lErrorCode = SCP_ERROR;  // 返回错误状态
        }

        usFrequenceA[ucNum] = pstSpeedMsg->stSpeedA[ucNum].usFreq; //A通道频率
        sPhaseA[ucNum] = pstSpeedMsg->stSpeedA[ucNum].sPhase;//A相位
/* begin add by deng 2024年3月31日 */
        usFrequenceB[ucNum] = pstSpeedMsg->stSpeedB[ucNum].usFreq; //B通道频率
        sPhaseB[ucNum] = pstSpeedMsg->stSpeedB[ucNum].sPhase;//B相位
/* end add by deng 2024年3月31日 */
    }

    //设置速度计
    stSpeedMsg.ucSPDHeadA = 0xFF;  //报文头A
    stSpeedMsg.ucSPDHeadB = 0xFE;  //报文头B
/* begin modified by deng 2024年3月31日 */
//    stSpeedMsg.ucChannelValid = ucChannelValid<<3u | ucChannelValid;  //通道使能
    stSpeedMsg.ucChannelValid =  ucChannelValid;  //通道使能
/* end modified by deng 2024年3月31日 */
/* begin modified by deng 2024年12月1日 */
//    stSpeedMsg.ausMotorAChannelFreq[0] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[0]); //电机A通道1频率
//    stSpeedMsg.ausMotorAChannelFreq[1] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[1]); //电机A通道2频率
//    stSpeedMsg.ausMotorAChannelFreq[2] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[2]); //电机A通道3频率
//    stSpeedMsg.ausMotorBChannelFreq[0] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[0]); //电机B通道1频率
//    stSpeedMsg.ausMotorBChannelFreq[1] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[1]); //电机B通道2频率
//    stSpeedMsg.ausMotorBChannelFreq[2] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[2]); //电机B通道3频率
    //速度计A
    if (usFrequenceA[0] == 0u)
    {
        stSpeedMsg.ausMotorAChannelFreq[0] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorAChannelFreq[0] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[0]); //电机A通道1频率
    }
    if (usFrequenceA[1] == 0u)
    {
        stSpeedMsg.ausMotorAChannelFreq[1] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorAChannelFreq[1] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[1]); //电机A通道2频率

    }
    if (usFrequenceA[2] == 0u)
    {
        stSpeedMsg.ausMotorAChannelFreq[2] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorAChannelFreq[2] = ntohs(SPEED_FREQ_TOTAL / usFrequenceA[2]); //电机A通道3频率
    }
    //速度计B
    if (usFrequenceB[0] == 0u)
    {
        stSpeedMsg.ausMotorBChannelFreq[0] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorBChannelFreq[0] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[0]); //电机B通道1频率
    }
    if (usFrequenceB[1] == 0u)
    {
        stSpeedMsg.ausMotorBChannelFreq[1] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorBChannelFreq[1] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[1]); //电机B通道2频率

    }
    if (usFrequenceB[2] == 0u)
    {
        stSpeedMsg.ausMotorBChannelFreq[2] = 0xEFFFFFFFu;
    }
    else
    {
        stSpeedMsg.ausMotorBChannelFreq[2] = ntohs(SPEED_FREQ_TOTAL / usFrequenceB[2]); //电机B通道3频率
    }
//    stSpeedMsg.ausMotorAPhase[0] = ntohs((sPhaseA[0] * SPEED_FREQ_TOTAL) / (360 * usFrequenceA[0]));//电机A通道1相位
//    stSpeedMsg.ausMotorAPhase[1] = ntohs((sPhaseA[1] * SPEED_FREQ_TOTAL) / (360 * usFrequenceA[1]));//电机A通道2相位
//    stSpeedMsg.ausMotorAPhase[2] = ntohs((sPhaseA[2] * SPEED_FREQ_TOTAL) / (360 * usFrequenceA[2]));//电机A通道3相位
//    stSpeedMsg.ausMotorBPhase[0] = ntohs((sPhaseA[0] * SPEED_FREQ_TOTAL) / (360 * usFrequenceB[0]));//电机B通道1相位
//    stSpeedMsg.ausMotorBPhase[1] = ntohs((sPhaseA[1] * SPEED_FREQ_TOTAL) / (360 * usFrequenceB[1]));//电机B通道2相位
//    stSpeedMsg.ausMotorBPhase[2] = ntohs((sPhaseA[2] * SPEED_FREQ_TOTAL) / (360 * usFrequenceB[2]));//电机B通道3相位
    stSpeedMsg.ausMotorAPhase[0] = ntohs((sPhaseA[0] * 10) / 360);//电机A通道1相位
    stSpeedMsg.ausMotorAPhase[1] = ntohs((sPhaseA[1] * 10) / 360);//电机A通道2相位
    stSpeedMsg.ausMotorAPhase[2] = ntohs((sPhaseA[2] * 10) / 360);//电机A通道3相位
    stSpeedMsg.ausMotorBPhase[0] = ntohs((sPhaseB[0] * 10) / 360);//电机B通道1相位
    stSpeedMsg.ausMotorBPhase[1] = ntohs((sPhaseB[1] * 10) / 360);//电机B通道2相位
    stSpeedMsg.ausMotorBPhase[2] = ntohs((sPhaseB[2] * 10) / 360);//电机B通道3相位
/* end modified by deng 2024年12月1日 */
    stSpeedMsg.aucMotorADutyCycle[0] = ucDutyCycleA[0];
    stSpeedMsg.aucMotorADutyCycle[1] = ucDutyCycleA[1];
    stSpeedMsg.aucMotorADutyCycle[2] = ucDutyCycleA[2];
    stSpeedMsg.aucMotorBDutyCycle[0] = ucDutyCycleB[0];
    stSpeedMsg.aucMotorBDutyCycle[1] = ucDutyCycleB[1];
    stSpeedMsg.aucMotorBDutyCycle[2] = ucDutyCycleB[2];
    stSpeedMsg.ucDataXOR = S_CheckXOR((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S) - 1u); //异或校验

    // 通过FPGA设置速度计A和B
    lRet = HAL_SendSpeed((uint8_t *)&stSpeedMsg, sizeof(S_SpeedMsg_S));
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_SPEED, "SPD_SetSpeedFreq Failed to Set Speed data");
        lErrorCode = SCP_ERROR;  // 返回错误状态
    }

    // 返回成功状态  
    return lErrorCode;
}


// 获取累计脉冲值
void SPD_GetCurrentPulse(int32_t *plCurrentPulseA, int32_t *plCurrentPulseB)
{     
    uint8_t err = 0;

    OSMutexPend(g_pstSpeedMutex, 0, &err);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPend error err(%d)", err);
    }

    // 将从硬件驱动中读取的PWM频率和占空比赋值给传入的指针  
    *plCurrentPulseA = g_lCurrentPulseA;  
    *plCurrentPulseB = g_lCurrentPulseB;   


    err = OSMutexPost(g_pstSpeedMutex);
    if (err != OS_ERR_NONE)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SPD_Init OSMutexPost error err(%d)", err);
    }

    // 打印接收到的累计脉冲值
/* begin modified by deng 2023年11月25日 */
//    LOG(LOG_INFO, LOG_MOD_PWM, "Geted Current Pulse A: %d\n", g_lCurrentPulseA);
//    LOG(LOG_INFO, LOG_MOD_PWM, "Geted Current Pulse B: %d\n", g_lCurrentPulseB);
/* end modified by deng 2023年11月25日 */
}  

