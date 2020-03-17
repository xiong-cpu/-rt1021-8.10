/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2019,��ɿƼ�
 * All rights reserved.
 * ��������QQȺ��һȺ��179029047(����)  ��Ⱥ��244861897
 *
 * �����������ݰ�Ȩ������ɿƼ����У�δ��������������ҵ��;��
 * ��ӭ��λʹ�ò������������޸�����ʱ���뱣����ɿƼ��İ�Ȩ������
 *
 * @file       		flash
 * @company	   		�ɶ���ɿƼ����޹�˾
 * @author     		��ɿƼ�(QQ3184284598)
 * @version    		�鿴doc��version�ļ� �汾˵��
 * @Software 		IAR 8.3 or MDK 5.26
 * @Target core		NXP RT1021DAG5A
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-02-18
 ********************************************************************************************************************/


#ifndef _zf_flash_h
#define _zf_flash_h


#include "common.h"

#define EXAMPLE_FLEXSPI             FLEXSPI
#define EXAMPLE_FLEXSPI_AMBA_BASE   FlexSPI_AMBA_BASE
#define EXAMPLE_FLEXSPI_CLOCK       kCLOCK_FlexSpi

#define FLASH_BASE_ADDR             0x60000000
#define FLASH_SIZE                  8192    //64Mbit 
#define SECTOR_SIZE                 4096    //4KB
#define FLASH_SECTOR_NUM            2048

#define FLASH_PAGE_SIZE             256
#define FLASH_SECTOR_PAGE_NUM       (SECTOR_SIZE/FLASH_PAGE_SIZE)






//-------------------------------------------------------------------------------------------------------------------
//  @brief      ʹ�ú궨���flash�������ݶ�ȡ
//  @param      sector_num 		��Ҫд����������
//  @param      offset	 		��ַƫ�� ƫ��1���� Ϊ�ĸ��ֽ�
//  @param      type		 	��ȡ����������
//  @return     				���ظ�����ַ������
//  @since      v1.0
//  Sample usage:               flash_read(20,0,uint32);//��ȡ20������ƫ��0��������Ϊuint32
//  @note       ��Ҫ�ر�ע����� RTϵ����cache�������ÿ�ζ�ȡflash֮ǰ����SCB_InvalidateDCache();ִ�и�������cache
//              ������˵û��һ���������һ�£�
//-------------------------------------------------------------------------------------------------------------------
#define     flash_read(sector_num,offset,type)      (*(type *)((uint32)((FLASH_BASE_ADDR + (sector_num)*SECTOR_SIZE) + (offset*4))))


#define     float_conversion_uint32(float_data)     (*(uint32 *)&float_data)

#define     uint32_conversion_float(uint32_data)    (*(float *)&uint32_data)






void   flash_init(void);
uint8  flash_check(uint32 sector_num, uint32 page_num);
uint8  flash_erase_sector(uint32 sector_num);
uint8  flash_write_page(uint32 sector_num, uint32 page_num, const void *buf, uint8 len);
void   flash_read_page(uint32 sector_num, uint32 page_num, void *buf, uint8 len);


#endif

