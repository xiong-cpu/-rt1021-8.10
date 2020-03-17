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

#include "fsl_common.h"
#include "fsl_flexspi.h"
#include "zf_flash.h"



#define NOR_CMD_LUT_SEQ_IDX_READID1             10
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS1         11
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1        12
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1        13
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD    14
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1        15

#define FLASH_BUSY_STATUS_POL       1           //����æ��־λ����  
#define FLASH_BUSY_STATUS_OFFSET    0           //����æ��־λƫ��λ��



const uint32_t customLUT[64] = {
        /* Read ID */
        [4 * NOR_CMD_LUT_SEQ_IDX_READID1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xAB, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x18),
        [4 * NOR_CMD_LUT_SEQ_IDX_READID1 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        /* Read extend parameters */
        [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),
        /* Write Enable */
        [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        /* Erase Sector  */
        [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x20, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
//        /* Page Program - single mode */
//        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1] =
//            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x02, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
//        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1 + 1] =
//            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
            
        /* Page Program - quad mode */
        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x32, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
		    /* Normal read mode -SDR */
        [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x03, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
};

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ڲ����ã��û��������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr))
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write enable */
    flashXfer.deviceAddress = baseAddr;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Command;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ڲ����ã��û��������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_wait_bus_busy(FLEXSPI_Type *base))
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue;
    status_t status;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Read;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READSTATUS1;
    flashXfer.data = &readValue;
    flashXfer.dataSize = 4;

    do
    {
        status = FLEXSPI_TransferBlocking(base, &flashXfer);
        if (status != kStatus_Success)
        {
            return status;
        }
        if (FLASH_BUSY_STATUS_POL)
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);
	
		
    return status;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ڲ����ã��û��������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address))
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    status = flexspi_nor_write_enable(base, address);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress = address;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Command;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1;
    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    return status;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ڲ����ã��û��������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src, uint16 num))
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    status = flexspi_nor_write_enable(base, dstAddr);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress = dstAddr;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Write;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD;
    flashXfer.data = (uint32_t *)src;
    flashXfer.dataSize = num;
    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    return status;
}

////-------------------------------------------------------------------------------------------------------------------
////  @brief      �ڲ����ã��û��������
////  @param      void 		    
////  @return     				
////  @since      v1.0
////  Sample usage:               
////-------------------------------------------------------------------------------------------------------------------
//AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_flash_read_sector(FLEXSPI_Type *base, uint32_t address,const uint32_t *src,size_t leng))
//{
//	//uint32_t temp;
//    flexspi_transfer_t flashXfer;
//    flashXfer.deviceAddress = address;
//    flashXfer.port = kFLEXSPI_PortA1;
//    flashXfer.cmdType = kFLEXSPI_Read;
//    flashXfer.SeqNumber = 1;
//    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1;
//    flashXfer.data = (uint32_t *)src;
//    flashXfer.dataSize = leng;
//
//    status_t status = FLEXSPI_TransferBlocking(base, &flashXfer);
//
//   // *vendorId = temp;
//
//    return status;
//}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �ڲ����ã��û��������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
AT_NONCACHEABLE_SECTION_INIT(status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId))
{
    uint32_t temp;
    status_t status;
        
    flexspi_transfer_t flashXfer;
    flashXfer.deviceAddress = 0;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Read;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READID1;
    flashXfer.data = &temp;
    flashXfer.dataSize = 1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    *vendorId = temp;

    return status;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      FLASH��ʼ������
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
uint8 vendorID;
void flash_init(void)
{
    status_t status;
    //����LUT��
    
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI,NOR_CMD_LUT_SEQ_IDX_READID1*4,&customLUT[NOR_CMD_LUT_SEQ_IDX_READID1*4], 64);
    status = flexspi_nor_get_vendor_id(EXAMPLE_FLEXSPI, &vendorID);
    
    if (status != kStatus_Success)  assert(0);//��ȡIDʧ�� ��������
    if (0x16 != vendorID)           assert(0);//IDУ��ʧ�� ��������
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      У��FLASH�Ƿ�������
//  @param      sector_num 		��Ҫд����������
//  @param      page_num	 	��ǰ����ҳ�ı��
//  @return     				����1�����ݣ�����0û�����ݣ������Ҫ�������ݵ�����д���µ�������Ӧ�ö������������в�������
//  @since      v1.0
//  Sample usage:               flash_check(500,0);//У��500����������=��0ҳ�Ƿ�������
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_check(uint32 sector_num, uint32 page_num)
{
    uint8 i;
    
    SCB_CleanInvalidateDCache(); //��ȡflashǰ���������ջ���
    for(i=0; i<FLASH_PAGE_SIZE/4; i++)
    {
        if(0xFFFFFFFF != flash_read(sector_num,page_num*FLASH_PAGE_SIZE/4+i,uint32))
        {
            return 1;//flash��Ҫ����
        }
    }
    
    return 0;//flash����Ҫ����
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      FLASH��������
//  @param      sector_num 		��Ҫ�������������
//  @return     				������������ʧ�ܣ�����0�����ɹ�
//  @since      v1.0
//  Sample usage:               uint32 dat = flash_erase_sector(1000);
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_erase_sector(uint32 sector_num)
{
    status_t status;
    uint8 isr;
    
    assert(FLASH_SECTOR_NUM>sector_num);//У������Ϸ���
    isr = DisableGlobalIRQ();
    status = flexspi_nor_flash_erase_sector(EXAMPLE_FLEXSPI, sector_num * SECTOR_SIZE);
    EnableGlobalIRQ(isr);
    return status;
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      FLASHҳ���
//  @param      sector_num 		��Ҫд����������
//  @param      page_num		д�뽫����д��������ڼ�ҳ
//  @param      *buf	 		�����׵�ַ
//  @param      len		 		д����ֽ��� ˵������len=1���ʾд��len*4���ֽ�
//  @return     				��������д��ʧ�ܣ�����0д��ɹ�
//  @since      v1.0
//  Sample usage:               flash_write_page(500,0,(const void *)buf,64);//��buf������������� ǰ64���֣�������ֱ�ʾ�ĸ��ֽڣ� д��flash500��������0ҳ
//  һ��flash���� ��Ϊ��n��������Ȼ��һ���ж��ҳ��ÿһ��ҳ�й̶���С
//  ���İ��Դ���flash ÿ������Ϊ4KB��ÿ��ҳ��256�ֽڣ����һ��������16ҳ
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_write_page(uint32 sector_num, uint32 page_num, const void *buf, uint8 len)
{
    uint8 isr;
    status_t status;

    
    assert(FLASH_SECTOR_NUM>sector_num);                //У������Ϸ���
    assert(FLASH_SECTOR_PAGE_NUM>page_num);             //У������Ϸ���
    assert(FLASH_PAGE_SIZE>=(len*4));                   //У������Ϸ���
    
    isr = DisableGlobalIRQ();
    status = flexspi_nor_flash_page_program(EXAMPLE_FLEXSPI, sector_num * SECTOR_SIZE + FLASH_PAGE_SIZE*page_num, buf, len*4);
    EnableGlobalIRQ(isr);
    
    return status;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      FLASHҳ��ȡ
//  @param      sector_num 		��Ҫд����������
//  @param      page_num		д�뽫����д��������ڼ�ҳ
//  @param      *buf	 		�����׵�ַ
//  @param      len		 		��ȡ���ֽ��� ˵������len=1���ʾд��len*4���ֽ�
//  @return     void
//  @since      v1.0
//  Sample usage:               flash_read_page(500,0,(void *)buf,64);//��flash500��������0ҳ��������� ǰ64���֣�������ֱ�ʾ�ĸ��ֽڣ� д��buf����
//  һ��flash���� ��Ϊ��n��������Ȼ��һ���ж��ҳ��ÿһ��ҳ�й̶���С
//  ���İ��Դ���flash ÿ������Ϊ4KB��ÿ��ҳ��256�ֽڣ����һ��������16ҳ
//-------------------------------------------------------------------------------------------------------------------
void flash_read_page(uint32 sector_num, uint32 page_num, void *buf, uint8 len)
{
    uint8 i;
    
    assert(FLASH_SECTOR_NUM>sector_num);                //У������Ϸ���
    assert(FLASH_SECTOR_PAGE_NUM>page_num);             //У������Ϸ���
    assert(FLASH_PAGE_SIZE>=(len*4));                   //У������Ϸ���
    SCB_CleanInvalidateDCache();                        //��ȡflashǰ���������ջ���
    
    for(i=0; i<len; i++)
    {
        *((uint32 *)buf + i) = flash_read(sector_num,page_num*FLASH_PAGE_SIZE+i,uint32);
        
    }
}




