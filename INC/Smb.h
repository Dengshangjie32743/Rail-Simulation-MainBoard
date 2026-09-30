/**
 * @brief 仿真单板接口文件
 * @copyright CRRC QingDao SiFang Rolling Stock Research Insititue Co.,Ltd.
 * @author Hardware group
 * @file
 * @ingroup B2b
 * @version V1.0.0
 *
 * @date 2023-10-01   朱成寅：首次创建
 *
 */

#ifndef SMB_H_
#define SMB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "TypeInc.h"

#define SMB_INTERFACE_VERSION      (1u) /**< 协议版本号 */

/* 报文类型：
 * 0x00开头表示平台测试使用，0x01开头表示业务仿真使用，0x8开头表示故障注入使用。
 * 这只是大体上的划分，用于不用人员看文档时的着重点，平台测试时也允许使用业务接口，实际使用可以很灵活。
 * BTM由于时效和透传的特殊性，暂不列出，后期需要和业务反正讨论后定下来（包含SMB单板中BTM是否只是透传，需要再讨论）
 */
#define  SMB_RECV_MSG_TYPE_INIT         0x0000    /**< 初始化报文，方向:PC->SIM */

#define  SMB_RECV_MSG_TYPE_ASK          0x0001    /**< 数据查询报文，方向:PC->SIM */
#define  SMB_RECV_MSG_TYPE_IO           0x0002    /**< 开关量IO输出报文，方向:PC->SIM */ 
#define  SMB_RECV_MSG_TYPE_SEPPD        0x0003    /**< 速度计报文，方向:PC->SIM */
#define  SMB_RECV_MSG_TYPE_BTM          0x0004    /**< BTM报文，方向:PC->SIM */
#define  SMB_RECV_MSG_TYPE_VELOCITY     0x0005    /**< 速度报文，方向:PC->SIM */

#define  SMB_SEND_MSG_TYPE_APP          0x0020    /**< 返回SMB中的所有数据，周期性上报的一条报文，方向:SIM->PC */
#define  SMB_SEND_MSG_TYPE_CLOOP_PWM    0x0021    /**< 电流环数据和PWM数据，方向:SIM->PC */
#define  SMB_SEND_MSG_TYPE_IO           0x0022    /**< 开关量IO输入报文，方向:SIM->PC */
#define  SMB_SEND_MSG_TYPE_BTM          0x0023    /**< BTM报文，方向:SIM->PC */

#define  SMB_INJECT_MSG_TYPE_RADAR1     0x0101    /**< 雷达故障注入报文1，方向:PC->SIM */
#define  SMB_INJECT_MSG_TYPE_RADAR2     0x0102    /**< 雷达故障注入报文2，方向:PC->SIM */
#define  SMB_INJECT_MSG_TYPE_BTM1       0x0103    /**< BTM故障注入报文1，方向:PC->SIM */


typedef struct {
    uint8_t ucVersion;          /**< 协议版本号 */
    uint8_t ucRes;              /**< 保留位 */
    uint16_t usMsgType;         /**< 报文类型：SMB_xxx */
} SMB_MsgHead_S;

/**
 * @brief 硬仿初始化报文
 * SMB_RECV_MSG_TYPE_INIT
 * 收到报文后，硬仿初始化自身数据，速度计频率变为0，累计脉冲清零，雷达报文中的速度值清零，各模块的发送buf、故障注入信息等 
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint32_t ulConversion;      /**< 速度和频率换算值，用于SMB_RECV_MSG_TYPE_VELOCITY报文中下发的速度值转成频率值 */
    uint32_t ulPulseDistance;   /**< 脉冲距，即一个速度计脉冲表示的距离 ，单位um待讨论*/
    uint32_t ulCycle;           /**< SMB_SEND_MSG_TYPE_APP报文的上报周期，默认为0ms（可配置范围：0表示不上报，或配置成20~10000ms） */
} SMB_InitMsg_S;


#define SMB_IO_BIT_NUM 24u /**< 单个单板最大码位 */
#if (SMB_IO_BIT_NUM != 24)
#error SMB_IO_NUM must be a multiple of 8
#endif
typedef struct {
    uint8_t ucBoardId;     /**< 单板序列号 */
    uint8_t aucData[3u];   /**< 3个字节，每个bit表示1个码位，共24个码位，第1字节最高位为第1个码位，第3字节最低位为第24个码位 */
} SMB_IoBoardInfo_S;
/* begin modified by deng 2023年11月30日 */
#define SMB_IO_BOARD_NUM 12u /**< 单个单板最大码位 */
#define SMB_IO_BOARD_RESV_NUM 4u /**< 保留单板码位(配合fdcan最大发送64个字节) */
#define SMB_IO_BOARD_PACK_NUM (SMB_IO_BOARD_NUM+SMB_IO_BOARD_RESV_NUM) /**< 发送的总码位 */
/* end modified by deng 2023年11月30日 */
/**
 * @brief 开关量IO输入输出报文（变长报文，由astIoBoardInfo字段决定报文长度）
 * SMB_RECV_MSG_TYPE_IO、SMB_SEND_MSG_TYPE_IO
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    SMB_IoBoardInfo_S astIoBoardInfo[SMB_IO_BOARD_NUM];
} SMB_IoMsg_S;

/**
 * @brief 速度报文
 * SMB_RECV_MSG_TYPE_VELOCITY
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    int32_t lVelocityA;         /**< 速度计A速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s待讨论*/
    int32_t lVelocityB;         /**< 速度计B速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s讨论*/
    int32_t lVelocityRadar;     /**< 雷达速度计A速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s讨论*/
} SMB_RecvVelocityMsg_S;


typedef struct {
    uint8_t ucEnable;       /**< 通道使能，1：启用。0：不启用 */
    uint8_t ucDutyCycle;    /**< 占空比 */
    uint16_t usFreq;          /**< 通道频率，注意是有符号的 */
    uint16_t usCount;       /**< 脉冲数：按照以上频率发送的多少个脉冲，发送完后停止发送，如果填0xffff，表示一直发送 */
    int16_t sPhase;         /**< 相位，注意是有符号的，相位是相对于第一个通道的正常情况的，即：3个通道的相位配置正常情况应该是0、120、240 */
} SMB_SpeedChannelCtrl_S;
/**
 * @brief 速度计报文
 * SMB_RECV_MSG_TYPE_SEPPD
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    SMB_SpeedChannelCtrl_S stSpeedA[3]; /**< 速度计A的3个通道的数据 */
    SMB_SpeedChannelCtrl_S stSpeedB[3]; /**< 速度计B的3个通道的数据 */
} SMB_RecvSpeedMsg_S;


#define SMB_BTM_DATA_LEN 104u
/**
 * @brief BTM报文
 * SMB_RECV_MSG_TYPE_BTM
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint8_t aucBtmData[SMB_BTM_DATA_LEN];   /**< CAN最多发送32帧 */
} SMB_BtmMsg_S;

/* begin add by deng 2024年1月30日 */
/**
 * @brief BTM故障1报文
 * SMB_INJECT_MSG_TYPE_BTM1
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint32_t uInjectBTMMsg1;
} SMB_RecvInjectBTMMsg1_S;
/* end add by deng 2024年1月30日 */
/**
 * @brief 数据查询报文
 * SMB_RECV_MSG_TYPE_ASK
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint16_t      usAskMsgType; /**< 查询的数据类型，SMB收到后会会回复一条对应的SMB_SEND_MSG_TYPE_xxx报文 */
    uint8_t       aucRes[2];
} SMB_RecvAskMsg_S;


/**
 * @brief 电流环数据和PWM数据
 * SMB_SEND_MSG_TYPE_CLOOP_PWM
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint32_t ulCurrentCLoop;           /**< 电流环数据，单位mA */
    uint32_t ulPwmFreq;         /**< pwm频率，单位Hz */
    uint32_t ulPwmDutyCycle;    /**< 占空比 */
} SMB_SendCloopPwmMsg_S;

/**
 * @brief 电流环数据和PWM数据（变长报文，由astIoBoardInfo字段决定报文长度）
 * SMB_SEND_MSG_TYPE_APP
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    int32_t lVelocityA;         /**< 速度计A速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s待讨论*/
    int32_t lVelocityB;         /**< 速度计B速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s讨论*/
    int32_t lVelocityRadar;     /**< 雷达速度计A速度，正负表示方向，此速度直接影响平台速度计频率和雷达的输出 ，单位cm/s讨论*/
    int32_t lCurrentPulse;      /**< 当前累计脉冲值，只上报速度计A的第一路脉冲的累计值，因为在业务使用时，大都只需要计算车辆位置，一路脉冲就够了 */
    uint32_t ulCLoop;           /**< 电流环数据 */
    uint32_t ulPwmFreq;         /**< pwm频率，单位Hz */
    uint32_t ulPwmDutyCycle;    /**< 占空比，单位%，精度1% */
    SMB_IoBoardInfo_S astIoBoardInfo[SMB_IO_BOARD_NUM];
} SMB_SendAppMsg_S;

/**
 * @brief 雷达故障注入报文1
 * SMB_INJECT_MSG_TYPE_RADAR1
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    uint8_t ucSendCycle;        /**< 发送周期 */
    uint8_t ucSendCount;        /**< aucData的发送次数，发送完毕后继续发送正常的报文 */
    uint8_t ucLen;              /**< aucData字段有效数据长度，无效的aucData的数据不会被读取 */
    uint8_t aucData[29u];       /**< 雷达故障注入数据，一直发这条雷达报文 */
} SMB_RecvInjectRadarMsg1_S;

typedef struct {
    uint8_t ucEnable;           /**< 1：启用。0：不启用 */
    uint8_t ucData;             /**< 雷达数据 */ 
} SMB_RadarInjectField_S;
/**
 * @brief 雷达故障注入报文2
 * SMB_INJECT_MSG_TYPE_RADAR2
 */
typedef struct {
    SMB_MsgHead_S stMsgHead;
    SMB_RadarInjectField_S astField[10u];       /**< 针对雷达报文中去掉头尾后的10个字节，做定制修改，可以固定修改每个字段的值，备注数组里面的10不能改 */
} SMB_RecvInjectRadarMsg2_S;





#ifdef __cplusplus
}
#endif

#endif /* SMB_H_ */
