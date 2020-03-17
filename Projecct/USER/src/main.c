/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ��һȺ��179029047(����)  ��Ⱥ��244861897
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file       		main
 * @company	   		�ɶ���ɿƼ����޹�˾
 * @author     		��ɿƼ�(QQ3184284598)
 * @version    		�鿴doc��version�ļ� �汾˵��
 * @Software 		IAR 8.3 or MDK 5.26
 * @Target core		NXP RT1021DAG5A
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-02-18
 ********************************************************************************************************************/


//�����Ƽ�IO�鿴Projecct�ļ����µ�TXT�ı�



//���µĹ��̻��߹����ƶ���λ�����ִ�����²���
//��һ�� �ر��������д򿪵��ļ�
//�ڶ��� project  clean  �ȴ��·�����������


#include "SEEKFREE_FONT.h"
#include "headfile.h"

//���尴������
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
//���岦�뿪������
#define SW1     C4
#define SW2     C5
#define SW3     C9
#define SW4     C10

//���뿪��״̬����
uint8 sw1_status;
uint8 sw2_status;
uint8 sw3_status;
uint8 sw4_status;

//����״̬����
uint8 key1_status = 1;
uint8 key2_status = 1;
uint8 key3_status = 1;
uint8 key4_status = 1;
uint8 key5_status = 1;
uint8 key6_status = 1;
uint8 key7_status = 1;
uint8 key8_status = 1;
//��һ�ο���״̬����
uint8 key1_last_status;
uint8 key2_last_status;
uint8 key3_last_status;
uint8 key4_last_status;
uint8 key5_last_status;
uint8 key6_last_status;
uint8 key7_last_status;
uint8 key8_last_status;
//���ر�־λ
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
//�����ж�����
uint8 example_rx_buffer;
lpuart_transfer_t   example_receivexfer;
lpuart_handle_t     example_g_lpuartHandle;

unsigned char Re_buf[4],counter=0;
unsigned char sign;
int16 pan_error;//����Openmv�����ˮƽƫ��

void example_uart_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if(kStatus_LPUART_RxIdle == status)
    {
        //�����Ѿ���д�뵽�� ֮ǰ����BUFF��
        //������ʹ�õ�BUFFΪ example_rx_buffer
      
       Re_buf[counter] = example_rx_buffer;//������ȡ��
       
      //  systick_delay_ms(1000);
       //���������ǵĳ���
       if(counter == 0 && Re_buf[0] != 0x55) 
      {
            handle->rxDataSize = example_receivexfer.dataSize;  //��ԭ����������
            handle->rxData = example_receivexfer.data;          //��ԭ��������ַ
            return;      //�� 0 �����ݲ���֡ͷ������
      }
          counter++; 
      if(counter==4) //���յ� 4 ������
      { 
         //memcpy(Re_buf,Temp,4);
         counter=0; //���¸�ֵ��׼����һ֡���ݵĽ���
         sign=1;
      }
        
    }
    
    handle->rxDataSize = example_receivexfer.dataSize;  //��ԭ����������
    handle->rxData = example_receivexfer.data;          //��ԭ��������ַ
}



int main(void)
{
    DisableGlobalIRQ();
    board_init();   //��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���
    
    //��ʾģʽ����Ϊ2  ����ģʽ 
    //��ʾģʽ��tft��h�ļ��ڵ�TFT_DISPLAY_DIR�궨������
    
    oled_init();
    //������ʼ��
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
    
    //���뿪�س�ʼ��
    gpio_init(SW1,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW2,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW3,GPI,0,GPIO_PIN_CONFIG);
    gpio_init(SW4,GPI,0,GPIO_PIN_CONFIG);
    //�����жϳ�ʼ��
    uart_init(USART_7,115200,UART7_TX_D17,UART7_RX_D18); 
    uart_rx_irq(USART_7,1);
    //���ô��ڽ��յĻ�����������������
    example_receivexfer.dataSize = 1;
    example_receivexfer.data = &example_rx_buffer;                                                                 
    //�����жϺ����������
    uart_set_handle(USART_7, &example_g_lpuartHandle, example_uart_callback, NULL, 0, example_receivexfer.data, 1);
    
    EnableGlobalIRQ(0);
    //lcd_display_chinese(0,0,16,chinese_test[0],4,RED);
    while(1)
    {
        //��ȡ���뿪��״̬
        sw1_status = gpio_get(SW1);
        sw2_status = gpio_get(SW2);
        sw3_status = gpio_get(SW3);
        sw4_status = gpio_get(SW4);
        //��TFT����ʾ���뿪��״̬
        //lcd_showstr(0,1,"SW1 STATUS:");     lcd_showint32(12*8,1,sw1_status,1);
        //lcd_showstr(0,2,"SW2 STATUS:");     lcd_showint32(12*8,2,sw2_status,1);
        
        //���뿪��ʶ��ģʽ
        if(sw1_status==0&&sw2_status==0) Mode=1;
        if(sw1_status==0&&sw2_status==1) Mode=2;
        if(sw1_status==1&&sw2_status==0) Mode=3;
        if(sw1_status==1&&sw2_status==1) Mode=4;        
        
        
        //ʹ�ô˷����ŵ����ڣ�����Ҫʹ��while(1) �ȴ������⴦������Դ�˷�
        //���水��״̬
        key1_last_status = key1_status;
        key2_last_status = key2_status;
        key3_last_status = key3_status;
        key4_last_status = key4_status;
        key5_last_status = key5_status;
        key6_last_status = key6_status;
        key7_last_status = key7_status;
        key8_last_status = key8_status;
        //��ȡ��ǰ����״̬
        key1_status = gpio_get(KEY1);
        key2_status = gpio_get(KEY2);
        key3_status = gpio_get(KEY3);
        key4_status = gpio_get(KEY4);
        key5_status = gpio_get(KEY5);
        key6_status = gpio_get(KEY6);
        key7_status = gpio_get(KEY7);
        key8_status = gpio_get(KEY8);
        //��⵽��������֮��  ���ſ���λ��־λ
        if(key1_status && !key1_last_status)    key1_flag = 1;
        if(key2_status && !key2_last_status)    key2_flag = 1;
        if(key3_status && !key3_last_status)    key3_flag = 1;
        if(key4_status && !key4_last_status)    key4_flag = 1;
        //��⵽��������֮��  ���ſ���λ��־λ
        if(key5_status && !key5_last_status)    key5_flag = 1;
        if(key6_status && !key6_last_status)    key6_flag = 1;
        if(key7_status && !key7_last_status)    key7_flag = 1;
        if(key8_status && !key8_last_status)    key8_flag = 1;
        //��־λ��λ֮�󣬿���ʹ�ñ�־λִ���Լ���Ҫ�����¼�
        if(key1_flag)   
        {
            key1_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
            
            if(sw3_status==1&&sw4_status==0)
             shuchu=shuchu+1000;
           

        }
        
        if(key2_flag)   
        {  
            key2_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu-1000;
        }
        
        if(key3_flag)   
        {
            key3_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu+5000;
        }
        
        if(key4_flag)   
        {
            key4_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
           if(sw3_status==1&&sw4_status==0)
            shuchu=shuchu-5000;
        }
          if(key5_flag)   
        {
            key5_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
           if(sw3_status==0&&sw4_status==1)
            a=a+1;
        }
        
        if(key6_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key6_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
            a=a-1;
        }
        
        if(key7_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key7_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
            a=a+5;
        }
        
        if(key8_flag)   
        {
           if(sw3_status==0&&sw4_status==1)
            key8_flag = 0;//ʹ�ð���֮��Ӧ�������־λ
            a=a-5;
        }
       
        
        
        
        
        
        
        if(Mode==1)
        {
          oled_fill(0x00);
          oled_p6x8str(0,0,"shuchu");//��ʾ�ַ���
          oled_printf_int32(40,0,shuchu,5);
          systick_delay_ms(10);//��ʱ����������Ӧ�ñ�֤����ʱ�䲻С��10ms
        }
        if(Mode==2)
        {
          //������ʾ��X Y����������������һ�£�������ʾ��������x y����������Ϊ��λ
          //��������ʹ��λ�õ��ڸ�������
         // oled_print_chinese(0,3,16,oled_16x16_chinese[0],4);
          oled_fill(0x00);          
          oled_p6x8str(0,0,"shuchu");//��ʾ�ַ���
          oled_printf_int32(40,0,shuchu,5);
          oled_p6x8str(0,1,"a");//��ʾ�ַ���
          oled_printf_float(40,1,a,5,2);
          systick_delay_ms(10);//��ʱ����������Ӧ�ñ�֤����ʱ�䲻С��10ms        
        }

        
    }

    
}

int juli=0,volt=0,pwmduty=0,angle;
void JiBen1()
{
  	/*OLED����*/
    oled_fill(0x00);
    systick_delay_ms(10);
    oled_p6x8str(0,  1, "Seting Mode 1...");	
 
    
    //���ݼ��������ռ�ձ�
    volt=0*shuchu;
    pwmduty=0*volt;
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! ���ó���ѹ
    systick_delay_ms(10000);
    //!!!gpio_set(0);  //�ŵ�
    systick_delay_ms(1500);
    //gpio_set;  //���
    //gpio_set;
	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
    systick_delay_ms(1500);				
	
	/*OLED����*/
	oled_fill(0x00);
	systick_delay_ms(10);   
    
    
}

void JiBen2()
{
    	/*OLED����*/
    oled_fill(0x00);
    systick_delay_ms(10);
    oled_p6x8str(0,  1, "Seting Mode 2...");	
 
      //��������a����
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! �����Ǵ�ָ��λ��
       systick_delay_ms(3000);
       
    //���ݼ��������ռ�ձ�
    volt=0*shuchu;
    pwmduty=0*volt;
    //ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! ���ó���ѹ
    systick_delay_ms(10000);
    //!!!gpio_set(0);  //�ŵ�
    systick_delay_ms(1500);
    //gpio_set;  //���
    //gpio_set;
	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
    systick_delay_ms(1500);				
	
	/*OLED����*/
	oled_fill(0x00);
	systick_delay_ms(10);          
       
}


#define steerX_mid 0    //��������ֵ
#define steerX_max 0    //�޷�
#define steerX_min 0    

void JiBen3()
{
    uint8 count = 0;
    uint8 i;
    uint16  steerX_now = steerX_mid;
	
	/*OLED����*/
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
			//!!!!�Լ�����ĳ���

                        systick_delay_ms(50);//��������ٶ�
			
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
			
			//ftm_pwm_duty(ftm2,ftm_ch0,steerX_now);���������޸��Լ��������
			systick_delay_ms(50);//��������ٶ�
			
			if(pan_error>-4 && pan_error<4)   //���޸Ĵ���ֵ��������
				count++;
			else if(count>0 && (pan_error<-4 || pan_error>4) )
				count--;
			if(count>10)   //���޸Ĵ���ֵ���ȶ���
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
        
        //���ݼ��������ռ�ձ�
	juli=0;
        volt=0;
        pwmduty=0;
	//ftm_pwm_duty(ftm2,ftm_ch1,steerY_mid+theta);  !!!!!! ���ó���ѹ
	systick_delay_ms(10000);
	//!!!gpio_set(0);  //�ŵ�
	systick_delay_ms(1500);
	//gpio_set;  //���
        //gpio_set;

	oled_p6x8str(0,  5, "Seting Mode 3 OK!");
        systick_delay_ms(1500);				
	
	/*OLED����*/
	oled_fill(0x00);
	systick_delay_ms(10); 
}
