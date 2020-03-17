/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		main
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.26
 * @Target core		NXP RT1021DAG5A
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-02-18
 ********************************************************************************************************************/


//整套推荐IO查看Projecct文件夹下的TXT文本



//打开新的工程或者工程移动了位置务必执行以下操作
//第一步 关闭上面所有打开的文件
//第二步 project  clean  等待下方进度条走完


#include "SEEKFREE_FONT.h"
#include "headfile.h"

//定义按键引脚
#define KEY1    D19
#define KEY2    D22
#define KEY3    C2
#define KEY4    C0
#define KEY1    D19
#define KEY2    D22
#define KEY3    C2
#define KEY4    C0
#define KEY5    C3
#define KEY6    D23
#define KEY7    C1
#define KEY8    D24
//定义拨码开关引脚
#define SW1     C4
#define SW2     C5
#define SW3     C9
#define SW4     C10

//拨码开关状态变量
uint8 sw1_status;
uint8 sw2_status;
uint8 sw3_status;
uint8 sw4_status;

//开关状态变量
uint8 key1_status = 1;
uint8 key2_status = 1;
uint8 key3_status = 1;
uint8 key4_status = 1;
uint8 key5_status = 1;
uint8 key6_status = 1;
uint8 key7_status = 1;
uint8 key8_status = 1;
//上一次开关状态变量
uint8 key1_last_status;
uint8 key2_last_status;
uint8 key3_last_status;
uint8 key4_last_status;
uint8 key5_last_status;
uint8 key6_last_status;
uint8 key7_last_status;
uint8 key8_last_status;
//开关标志位
uint8 key1_flag;
uint8 key2_flag;
uint8 key3_flag;
uint8 key4_flag;
uint8 key5_flag;
uint8 key6_flag;
uint8 key7_flag;
uint8 key8_flag;
int shuchu;
float a;
char OK=0;
char Mode=0;
//串口中断配置
uint8 example_rx_buffer;
lpuart_transfer_t   example_receivexfer;
lpuart_handle_t     example_g_lpuartHandle;

unsigned char Re_buf[4],counter=0;
unsigned char sign;
int16 pan_error;//接受Openmv舵机的水平偏差

void example_uart_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if(kStatus_LPUART_RxIdle == status)
    {
        //数据已经被写入到了 之前社会的BUFF中
        //本例程使用的BUFF为 example_rx_buffer
      
       Re_buf[counter] = example_rx_buffer;//将数据取出
       
      //  systick_delay_ms(1000);
       //接受陀螺仪的程序
       if(counter == 0 && Re_buf[0] != 0x55) 
      {
            handle->rxDataSize = example_receivexfer.dataSize;  //还原缓冲区长度
            handle->rxData = example_receivexfer.data;          //还原缓冲区地址
            return;      //第 0 号数据不是帧头，跳过
      }
          counter++; 
      if(counter==4) //接收到 4 个数据
      { 
         //memcpy(Re_buf,Temp,4);
         counter=0; //重新赋值，准备下一帧数据的接收
         sign=1;
      }
        
    }
    
    handle->rxDataSize = example_receivexfer.dataSize;  //还原缓冲区长度
    handle->rxData = example_receivexfer.data;          //还原缓冲区地址
}



int main(void)
{
    DisableGlobalIRQ();
    board_init();   //务必保留，本函数用于初始化MPU 时钟 调试串口
    
    //显示模式设置为2  横屏模式 
    //显示模式在tft的h文件内的TFT_DISPLAY_DIR宏定义设置
    
    oled_init();
    //按键初始化
    gpio_init(KEY1,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY2,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY3,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY4,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY5,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY6,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY7,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(KEY8,GPI,0,GPIO_PIN_CONFIG);
    shuchu=25000;
    a=0;
    
    //拨码开关初始化
    gpio_init(SW1,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW2,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW3,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW4,GPI,0,GPIO_PIN_CONFIG);
    //串口中断初始化
    uart_init(USART_7,115200,UART7_TX_D17,UART7_RX_D18); 
    uart_rx_irq(USART_7,1);
    //配置串口接收的缓冲区及缓冲区长度
    example_receivexfer.dataSize = 1;
    example_receivexfer.data = &example_rx_buffer;                                                                 
    //设置中断函数及其参数
    uart_set_handle(USART_7, &example_g_lpuartHandle, example_uart_callback, NULL, 0, example_receivexfer.data, 1);
    
    EnableGlobalIRQ(0);
    //lcd_display_chinese(0,0,16,chinese_test[0],4,RED);
    while(1)
    {
        //获取拨码开关状态
        sw1_status = gpio_get(SW1);
        sw2_status = gpio_get(SW2);
        sw3_status = gpio_get(SW3);
        sw4_status = gpio_get(SW4);
        //在TFT上显示拨码开关状态
        //lcd_showstr(0,1,"SW1 STATUS:");     lcd_showint32(12*8,1,sw1_status,1);
        //lcd_showstr(0,2,"SW2 STATUS:");     lcd_showint32(12*8,2,sw2_status,1);
        
        //拨码开关识别模式
        if(sw1_status==0&&sw2_status==0) Mode=1;
        if(sw1_status==0&&sw2_status==1) Mode=2;
        if(sw1_status==1&&sw2_status==0) Mode=3;
        if(sw1_status==1&&sw2_status==1) Mode=4;        
        
        
        //使用此方法优点在于，不需要使用while(1) 等待，避免处理器资源浪费
        //保存按键状态
        key1_last_status = key1_status;
        key2_last_status = key2_status;
        key3_last_status = key3_status;
        key4_last_status = key4_status;
        key5_last_status = key5_status;
        key6_last_status = key6_status;
        key7_last_status = key7_status;
        key8_last_status = key8_status;
        //读取当前按键状态
        key1_status = gpio_get(KEY1);
        key2_status = gpio_get(KEY2);
        key3_status = gpio_get(KEY3);
        key4_status = gpio_get(KEY4);
        key5_status = gpio_get(KEY5);
        key6_status = gpio_get(KEY6);
        key7_status = gpio_get(KEY7);
        key8_status = gpio_get(KEY8);
        //检测到按键按下之后  并放开置位标志位
        if(key1_status && !key1_last_status)    key1_flag = 1;
        if(key2_status && !key2_last_status)    key2_flag = 1;
        if(key3_status && !key3_last_status)    key3_flag = 1;
        if(key4_status && !key4_last_status)    key4_flag = 1;
        //检测到按键按下之后  并放开置位标志位
        if(key5_status && !key5_last_status)    key5_flag = 1;
        if(key6_status && !key6_last_status)    key6_flag = 1;
        if(key7_status && !key7_last_status)    key7_flag = 1;
        if(key8_status && !key8_last_status)    key8_flag = 1;
        //标志位置位之后，可以使用标志位执行自己想要做的事件
        if(key1_flag)   
        {
            key1_flag = 0;//使用按键之后，应该清除标志位
            
            if(sw3_status==1&&sw4_status==0)
             shuchu=shuchu+1000;
           

        }
        
        if(key2_flag)   
        {  
            key2_flag = 0;//使用按键之后，应该清除标志位
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu-1000;
        }
        
        if(key3_flag)   
        {
            key3_flag = 0;//使用按键之后，应该清除标志位
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu+5000;
        }
        
        if(key4_flag)   
        {
            key4_flag = 0;//使用按键之后，应该清除标志位
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu-5000;
        }
          if(key5_flag)   
        {
            key5_flag = 0;//使用按键之后，应该清除标志位
           if(sw3_status==0&&sw4_status==1)
            a=a+1;
        }
        
        if(key6_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key6_flag = 0;//使用按键之后，应该清除标志位
            a=a-1;
        }
        
        if(key7_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key7_flag = 0;//使用按键之后，应该清除标志位
            a=a+5;
        }
        
        if(key8_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key8_flag = 0;//使用按键之后，应该清除标志位
            a=a-5;
        }
       
        
        
        
        
        
        
        if(Mode==1)
        {
          oled_fill(0x00);
          oled_p6x8str(0,0,"shuchu");//显示字符串
          oled_printf_int32(40,0,shuchu,5);
          systick_delay_ms(10);//延时，按键程序应该保证调用时间不小于10ms
        }
        if(Mode==2)
        {
          //汉字显示的X Y坐标与其他函数不一致，汉字显示函数坐标x y都是以像素为单位
          //这样可以使得位置调节更加随意
         // oled_print_chinese(0,3,16,oled_16x16_chinese[0],4);
          oled_fill(0x00);          
          oled_p6x8str(0,0,"shuchu");//显示字符串
          oled_printf_int32(40,0,shuchu,5);
          oled_p6x8str(0,1,"a");//显示字符串
          oled_printf_float(40,1,a,5,2);
          systick_delay_ms(10);//延时，按键程序应该保证调用时间不小于10ms        
        }

        
    }

    
}

int juli=0,volt=0,pwmduty=0,angle;
void JiBen1()
{
  	/*OLED清屏*/
    oled_fill(0x00);
    systick_delay_ms(10);
    oled_p6x8str(0,  1, "Seting Mode 1...");	
 
    
    //根据检测距离解算占空比
    volt=0*shuchu;
    pwmduty=0*volt;
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! 设置充电电压
    systick_delay_ms(10000);
    //!!!gpio_set(0);  //放电
    systick_delay_ms(1500);
    //gpio_set;  //充电
    //gpio_set;
	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
    systick_delay_ms(1500);				
	
	/*OLED清屏*/
	oled_fill(0x00);
	systick_delay_ms(10);   
    
    
}

void JiBen2()
{
    	/*OLED清屏*/
    oled_fill(0x00);
    systick_delay_ms(10);
    oled_p6x8str(0,  1, "Seting Mode 2...");	
 
      //根据输入a解算
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! 舵机打角打到指定位置
       systick_delay_ms(3000);
       
    //根据检测距离解算占空比
    volt=0*shuchu;
    pwmduty=0*volt;
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! 设置充电电压
    systick_delay_ms(10000);
    //!!!gpio_set(0);  //放电
    systick_delay_ms(1500);
    //gpio_set;  //充电
    //gpio_set;
	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
    systick_delay_ms(1500);				
	
	/*OLED清屏*/
	oled_fill(0x00);
	systick_delay_ms(10);          
       
}


#define steerX_mid 0    //舵机软件中值
#define steerX_max 0    //限幅
#define steerX_min 0    

void JiBen3()
{
    uint8 count = 0;
    uint8 i;
    uint16  steerX_now = steerX_mid;
	
	/*OLED清屏*/
    oled_fill(0x00);
    systick_delay_ms(10);
    oled_p6x8str(0,  1, "Seting Mode 3...");	
	while(1)
	{
		while(pan_error<0)
		{   
			
			if(pan_error>-5)
			steerX_now +=1;	
			else if(pan_error>-10)
			steerX_now +=2;	
			else
			steerX_now +=3;
			
			if(steerX_now>steerX_max)
				steerX_now = steerX_max;
			if(steerX_now<steerX_min)
				steerX_now = steerX_min;
			
			//ftm_pwm_duty(ftm2,ftm_ch0,steerX_now);
			//!!!!自己舵机的程序

                        systick_delay_ms(50);//减慢舵机速度
			
			if(pan_error>-4 && pan_error<4)
				count++;
			else if(count>0 && (pan_error<-4 || pan_error>4) )
				count--;
			if(count>10)
				break;
		}
		
		while(pan_error>=0)
		{   
			if(pan_error<5)
			steerX_now -=1;	
			else if(pan_error<10)
			steerX_now -=2;	
			else		
			steerX_now -=3;
			
			if(steerX_now>steerX_max)
				steerX_now = steerX_max;
			if(steerX_now<steerX_min)
				steerX_now = steerX_min;
			
			//ftm_pwm_duty(ftm2,ftm_ch0,steerX_now);！！！！修改自己舵机程序
			systick_delay_ms(50);//减慢舵机速度
			
			if(pan_error>-4 && pan_error<4)   //可修改此阈值调灵敏度
				count++;
			else if(count>0 && (pan_error<-4 || pan_error>4) )
				count--;
			if(count>10)   //可修改此阈值调稳定性
				break;
		}
		
		if(pan_error>-4 && pan_error<4)
		{	count++;
			
		}
		else if(count>0 && (pan_error<-4 || pan_error>4) )
			count--;
		if(count>10)
			break;
	}
	
	
	oled_p6x8str(0,  5, "Stoping ... ");	
	
	systick_delay_ms(150);
        
        //根据检测距离解算占空比
	juli=0;
        volt=0;
        pwmduty=0;
	//ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! 设置充电电压
	systick_delay_ms(10000);
	//!!!gpio_set(0);  //放电
	systick_delay_ms(1500);
	//gpio_set;  //充电
        //gpio_set;

	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
        systick_delay_ms(1500);				
	
	/*OLED清屏*/
	oled_fill(0x00);
	systick_delay_ms(10); 
}
