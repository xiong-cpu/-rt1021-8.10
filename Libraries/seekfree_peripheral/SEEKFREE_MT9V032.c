/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		总钻风
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.26
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-02-18
 * @note		
					接线定义：
					------------------------------------ 
						模块管脚            单片机管脚
						SDA(51的RX)         B24
						SCL(51的TX)         B25
						场中断(VSY)         B26
						行中断(HREF)		B27
						像素中断(PCLK)      B28
						数据口(D0-D7)		B23-B16 B23对应摄像头接口D0
					------------------------------------ 
	
					默认分辨率是            188*120
					默认FPS                 50帧
 ********************************************************************************************************************/



#include "fsl_common.h"
#include "fsl_gpio.h"
#include "zf_systick.h"
#include "zf_pit.h"
#include "zf_camera.h"
#include "zf_gpio.h"
#include "zf_iomuxc.h"
#include "SEEKFREE_MT9V032.h"


//数据务必定义在noncache区域，因为摄像头数据搬移是由DMA完成的，对于内核来说，它并不知道这个数组内容在什么时候被更新过，
//如果不定义在noncache区域，则会造成摄像头数据采集完成后，内核去访问这个数组的时候，读取的数据是直接冲cache里面读取的而不是去获取更新后的内容
//从而造成不同步的问题。
AT_NONCACHEABLE_SECTION_ALIGN(uint8 image[MT9V032_H][MT9V032_W],4);

uint8   receive[3];
uint8   receive_num = 0;
vuint8  uart_receive_flag = 1;

//需要配置到摄像头的数据
int16 MT9V032_CFG[CONFIG_FINISH][2]=
{
    {AUTO_EXP,          0},   //自动曝光设置      范围1-63 0为关闭 如果自动曝光开启  EXP_TIME命令设置的数据将会变为最大曝光时间，也就是自动曝光时间的上限
                              //一般情况是不需要开启这个功能，因为比赛场地光线一般都比较均匀，如果遇到光线非常不均匀的情况可以尝试设置该值，增加图像稳定性
    {EXP_TIME,          450}, //曝光时间          摄像头收到后会自动计算出最大曝光时间，如果设置过大则设置为计算出来的最大曝光值
    {FPS,               50},  //图像帧率          摄像头收到后会自动计算出最大FPS，如果过大则设置为计算出来的最大FPS
    {SET_COL,           MT9V032_W}, //图像列数量        范围1-752     K60采集不允许超过188
    {SET_ROW,           MT9V032_H}, //图像行数量        范围1-480
    {LR_OFFSET,         0},   //图像左右偏移量    正值 右偏移   负值 左偏移  列为188 376 752时无法设置偏移    摄像头收偏移数据后会自动计算最大偏移，如果超出则设置计算出来的最大偏移
    {UD_OFFSET,         0},   //图像上下偏移量    正值 上偏移   负值 下偏移  行为120 240 480时无法设置偏移    摄像头收偏移数据后会自动计算最大偏移，如果超出则设置计算出来的最大偏移
    {GAIN,              32},  //图像增益          范围16-64     增益可以在曝光时间固定的情况下改变图像亮暗程度

    
    {INIT,              0}    //摄像头开始初始化
};

//从摄像头内部获取到的配置数据
int16 GET_CFG[CONFIG_FINISH-1][2]=
{
    {AUTO_EXP,          0},   //自动曝光设置      
    {EXP_TIME,          0},   //曝光时间          
    {FPS,               0},   //图像帧率          
    {SET_COL,           0},   //图像列数量        
    {SET_ROW,           0},   //图像行数量        
    {LR_OFFSET,         0},   //图像左右偏移量    
    {UD_OFFSET,         0},   //图像上下偏移量    
    {GAIN,              0},   //图像增益          
};


uint8 rx_buffer;
lpuart_transfer_t   receivexfer;
lpuart_handle_t     g_lpuartHandle;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      MT9V032摄像头串口中断函数
//  @param      NULL
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       该函数在ISR文件 串口3中断程序被调用
//-------------------------------------------------------------------------------------------------------------------
void mt9v032_uart_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if(kStatus_LPUART_RxIdle == status)
    {
        receive[receive_num] = rx_buffer;
        receive_num++;
    
        if(1==receive_num && 0XA5!=receive[0])  receive_num = 0;
        if(3 == receive_num)
        {
            receive_num = 0;
            uart_receive_flag = 1;
        }
    }
    handle->rxDataSize = receivexfer.dataSize;  //还原缓冲区长度
    handle->rxData = receivexfer.data;          //还原缓冲区地址
}



//-------------------------------------------------------------------------------------------------------------------
//  @brief      获取摄像头内部配置信息
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:				调用该函数前请先初始化uart3
//-------------------------------------------------------------------------------------------------------------------
void get_config(void)
{
	uint16 temp, i;
    uint8  send_buffer[4];
    
    for(i=0; i<CONFIG_FINISH-1; i++)
    {
        send_buffer[0] = 0xA5;
        send_buffer[1] = GET_STATUS;
        temp = GET_CFG[i][0];
        send_buffer[2] = temp>>8;
        send_buffer[3] = (uint8)temp;
        
        uart_putbuff(MT9V032_COF_UART,send_buffer,4);
        
        //等待接受回传数据
        while(!uart_receive_flag);
        uart_receive_flag = 0;
        
        GET_CFG[i][1] = receive[1]<<8 | receive[2];
        
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      获取摄像头固件版本
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:				调用该函数前请先初始化uart3
//-------------------------------------------------------------------------------------------------------------------
uint16 get_version(void)
{
    uint16 temp;
    uint8  send_buffer[4];
    send_buffer[0] = 0xA5;
    send_buffer[1] = GET_STATUS;
    temp = GET_VERSION;
    send_buffer[2] = temp>>8;
    send_buffer[3] = (uint8)temp;
    
    uart_putbuff(MT9V032_COF_UART,send_buffer,4);
        
    //等待接受回传数据
    while(!uart_receive_flag);
    uart_receive_flag = 0;
    
    return ((uint16)(receive[1]<<8) | receive[2]);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      单独设置摄像头曝光时间
//  @param      light   设置曝光时间越大图像越亮，摄像头收到后会根据分辨率及FPS计算最大曝光时间如果设置的数据过大，那么摄像头将会设置这个最大值
//  @return     uint16  当前曝光值，用于确认是否正确写入
//  @since      v1.0
//  Sample usage:				
//-------------------------------------------------------------------------------------------------------------------
uint16 set_exposure_time(uint16 light)
{
	uint16 temp;
    uint8  send_buffer[4];

    send_buffer[0] = 0xA5;
    send_buffer[1] = SET_EXP_TIME;
    temp = light;
    send_buffer[2] = temp>>8;
    send_buffer[3] = (uint8)temp;
    
    uart_putbuff(MT9V032_COF_UART,send_buffer,4);
    
    //等待接受回传数据
    while(!uart_receive_flag);
    uart_receive_flag = 0;
    
    temp = receive[1]<<8 | receive[2];
    return temp;

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      对摄像头内部寄存器进行写操作
//  @param      addr    摄像头内部寄存器地址
//  @param      data    需要写入的数据
//  @return     uint16  寄存器当前数据，用于确认是否写入成功
//  @since      v1.0
//  Sample usage:				
//-------------------------------------------------------------------------------------------------------------------
uint16 set_mt9v032_reg(uint8 addr, uint16 data)
{
	uint16 temp;
    uint8  send_buffer[4];
    
    send_buffer[0] = 0xA5;
    send_buffer[1] = SET_ADDR;
    temp = addr;
    send_buffer[2] = temp>>8;
    send_buffer[3] = (uint8)temp;
    
    uart_putbuff(MT9V032_COF_UART,send_buffer,4);
    systick_delay_ms(10);
    
    send_buffer[0] = 0xA5;
    send_buffer[1] = SET_DATA;
    temp = data;
    send_buffer[2] = temp>>8;
    send_buffer[3] = (uint8)temp;
    
    uart_putbuff(MT9V032_COF_UART,send_buffer,4);
    
    //等待接受回传数据
    while(!uart_receive_flag);
    uart_receive_flag = 0;
    
    temp = receive[1]<<8 | receive[2];
    return temp;

}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      MT9V032摄像头初始化
//  @param      NULL
//  @return     void					
//  @since      v1.0
//  Sample usage:		
//-------------------------------------------------------------------------------------------------------------------
void camera_init(void)
{
    //设置参数    具体请参看使用手册
    uint16 temp, i;
    uint8  send_buffer[4];

    
    uart_init (MT9V032_COF_UART, 9600,MT9V032_COF_UART_TX,MT9V032_COF_UART_RX);	//初始换串口 配置摄像头    
    uart_rx_irq(MT9V032_COF_UART,1);
    //配置串口接收的缓冲区及缓冲区长度
    receivexfer.dataSize = 1;
    receivexfer.data = &rx_buffer;
    
    uart_set_handle(MT9V032_COF_UART, &g_lpuartHandle, mt9v032_uart_callback, NULL, 0, receivexfer.data, 1);
    EnableGlobalIRQ(0);
    
    //等待摄像头上电初始化成功
    systick_delay_ms(1000);
    uart_receive_flag = 0;
    
    //开始配置摄像头并重新初始化
    for(i=0; i<CONFIG_FINISH; i++)
    {
        send_buffer[0] = 0xA5;
        send_buffer[1] = MT9V032_CFG[i][0];
        temp = MT9V032_CFG[i][1];
        send_buffer[2] = temp>>8;
        send_buffer[3] = (uint8)temp;
        
        uart_putbuff(MT9V032_COF_UART,send_buffer,4);
        systick_delay_ms(2);
    }
    //等待摄像头初始化成功
    while(!uart_receive_flag);
    uart_receive_flag = 0;
    while((0xff != receive[1]) || (0xff != receive[2]));
    //以上部分对摄像头配置的数据全部都会保存在摄像头上51单片机的eeprom中
    //利用set_exposure_time函数单独配置的曝光数据不存储在eeprom中
    
    //获取配置便于查看配置是否正确
    get_config();
    
    //摄像头采集初始化
    DisableGlobalIRQ();

    dma_mux_init();
    flexio_camera(MT9V032_DATA_PIN, MT9V032_PCLK_PIN, MT9V032_HREF_PIN);
    flexio_dma_init(&image[0][0], MT9V032_W*MT9V032_H, mt9v032_dma);
    flexio_enable_rxdma();
    NVIC_SetPriority(DMA0_DMA16_IRQn,1);            //设置DMA中断优先级 范围0-15 越小优先级越高
    
    //设置场中断
    gpio_interrupt_init(MT9V032_VSYNC_PIN, FALLING, GPIO_INT_CONFIG);
    NVIC_SetPriority(GPIO1_Combined_16_31_IRQn,0);  //设置场中断优先级 范围0-15 越小优先级越高
	//EnableGlobalIRQ(0);
}


uint8   mt9v032_finish_flag = 0;    //一场图像采集完成标志位
//-------------------------------------------------------------------------------------------------------------------
//  @brief      MT9V032摄像头场中断
//  @param      NULL
//  @return     void			
//  @since      v1.0
//  Sample usage:				在isr.c里面先创建对应的中断函数，然后调用该函数(之后别忘记清除中断标志位)
//-------------------------------------------------------------------------------------------------------------------
void mt9v032_vsync(void)
{
    CLEAR_GPIO_FLAG(MT9V032_VSYNC_PIN);
    dma_restart(image[0]);

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      MT9V032摄像头DMA完成中断
//  @param      NULL
//  @return     void			
//  @since      v1.0
//  Sample usage:				
//-------------------------------------------------------------------------------------------------------------------
void mt9v032_dma(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	mt9v032_finish_flag = 1;//一副图像从采集开始到采集结束耗时3.8MS左右(50FPS、188*120分辨率)
    
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      总钻风摄像头图像发送至上位机查看图像
//  @param      uartn			使用的串口号
//  @return     void			
//  @since      v1.0
//  Sample usage:				
//-------------------------------------------------------------------------------------------------------------------
void seekfree_sendimg_032(UARTN_enum uartn)
{
	uart_putchar(uartn,0x00);uart_putchar(uartn,0xff);uart_putchar(uartn,0x01);uart_putchar(uartn,0x01);//发送命令
    uart_putbuff(uartn, (uint8_t *)image, MT9V032_H*MT9V032_W);  //发送图像
}
