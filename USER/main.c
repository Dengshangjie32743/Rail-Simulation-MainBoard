#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "malloc.h"
#include "usmart.h"
#include "pcf8574.h"
#include "mpu.h"
#include "lwip_comm.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "lwipopts.h"
#include "includes.h"
#include "lwip/sockets.h"
#include "udp_socket.h"
#include "Use_Task.h"
/* begin modified by deng 2023.10.24 */
#include "fdcan.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"
#include "Log.h"
/* end modified by deng 2023.10.24 */
#include "TypeInc.h"
#include "simPro.h"
#include "PWM.h"
#include "BTM.h"
#include "Cloop.h"
#include "Radar.h"
#include "Speed.h"
#include "IO.h"
/* begin modified by deng 2023年11月20日 */
#include "use_Task.h"
/* end modified by deng 2023年11月20日 */


int32_t Udp_Sock = -1;
//启动任务
OS_STK START_TASK_STK[START_STK_SIZE];

/*
 * 主函数
 */
int main(void)
{
    MPU_Memory_Protection();        //保护相关存储区域
    Cache_Enable();                 //打开L1-Cache
    HAL_Init();                     //初始化HAL库
    Stm32_Clock_Init(160, 5, 2, 4); //设置时钟,400Mhz
    delay_init(400);                //延时初始化
/* begin modified by deng 2023.10.24 */
    //    uart_init(115200);        //串口初始化
    //    usmart_dev.init(200);     //初始化USMART
    //    LED_Init();               //初始化LED
    //    KEY_Init();               //初始化按键
    SDRAM_Init();                   //初始化SDRAM
    //    LCD_Init();               //初始化LCD
    //    PCF8574_Init();           //初始化PCF8574（开发板的网络复位）
/* end modified by deng 2023.10.24 */
    my_mem_init(SRAMIN);     //初始化内部内存池
    my_mem_init(SRAMEX);     //初始化外部内存池
    my_mem_init(SRAMDTCM);   //初始化DTCM内存池
/* begin modified by deng 2023.11.6 */
    MX_GPIO_Init();          //GPIO初始化
    MX_ADC1_Init();          //AD采集初始化
    MX_SPI2_Init();          //SPI初始化
    MX_FDCAN1_Init();        //FDCAN初始化
    MX_FDCAN2_Init();        //CAN初始化
    MX_UART4_Init();         //RS485初始化
    MX_USART6_UART_Init();   //RS422初始化(暂时用于串口打印)
/* end modified by deng 2023.11.6 */
    OSInit();                //OS初始化
/* begin modified by deng 2023.11.6 */
    printf("\r\nSMB Init OK!");
/* end modified by deng 2023.11.6  */
    OSTaskCreate(Start_task, (void*) 0, (OS_STK*) &START_TASK_STK[START_STK_SIZE - 1], START_TASK_PRIO);
/* begin modified by deng 2023年11月20日 */
    printf("\r\nSMB Task Start OK!");
/* end modified by deng 2023年11月20日 */
    OSStart(); //开启UCOS
}

//start任务
void Start_task(void *pdata)
{
    int32_t lRet = SCP_OK;
    OS_CPU_SR cpu_sr;
    uint8_t ret=0;
    (void)pdata;

    OSStatInit();              //初始化统计任务

    //仿真协议初始化
    SP_Init();

    //日志初始化(含消息队列创建)
    lRet = LOG_Init();
    if (lRet != SCP_OK)
    {
        printf("SP_InitSmb error LOG_Init(%d)", lRet);
        return; 
    }

    // PWM初始化
    lRet = PWM_Init();
    if (lRet != SCP_OK)
    {
/* begin modified by deng 2023年11月20日 */
        LOG(LOG_ERROR, LOG_MOD_PWM, "SP_InitSmb error PWM_Init(%d)", lRet);
/* end modified by deng 2023年11月20日 */
        return;
    }

    // IO初始化
    lRet = IO_Init();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_IO, "SP_InitSmb error IO_Init(%d)", lRet);
        return;
    }

    // 电流环初始化
    lRet = CL_Init();
    if (lRet != SCP_OK)
    {
/* begin modified by deng 2023年11月20日 */
        LOG(LOG_ERROR, LOG_MOD_CL, "SP_InitSmb error CL_Init(%d)", lRet);
/* end modified by deng 2023年11月20日 */
        return;
    }

    // BTM初始化
    lRet = BTM_Init();
    if (lRet != SCP_OK)
    {
/* begin modified by deng 2023年11月20日 */
        LOG(LOG_ERROR, LOG_MOD_BTM, "SP_InitSmb error BTM_Init(%d)", lRet);
/* end modified by deng 2023年11月20日 */
        return;
    }

    // 速度计初始化
    lRet = SPD_Init();
    if (lRet != SCP_OK)
    {
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SP_InitSmb error SPD_Init lRet(%d)", lRet);
        return;
    }

    // 雷达初始化
    lRet = RADAR_Init();
    if (lRet != SCP_OK)
    {
/* begin modified by deng 2023年11月20日 */
        LOG(LOG_ERROR, LOG_MOD_RADAR, "SP_InitSmb error RADAR_Init lRet(%d)", lRet);
/* end modified by deng 2023年11月20日 */
        return;
    }

    OS_ENTER_CRITICAL();      //关中断
    // 网络任务初始化
    ret = fn_Eth_Data_Task_Init();
    if (ret != OS_ERR_NONE)
    {
/* begin modified by deng 2023年11月20日 */
        LOG(LOG_ERROR, LOG_MOD_LED, "fn_Eth_Data_Task error ret(%d)", ret);
/* end modified by deng 2023年11月20日 */
        return;
    }
    OS_EXIT_CRITICAL();          //开中断
    OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
}
