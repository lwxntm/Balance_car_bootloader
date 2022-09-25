//
// Created by xiaotian on 2022/9/25.
//

#include "stm_flash.h"
#include "printf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t uwStartSector = 0;
uint32_t uwEndSector = 0;
uint32_t uwAddress = 0;
uint32_t uwSectorCounter = 0;

__IO uint16_t uwData16 = 0;
__IO uint32_t uwMemoryProgramStatus = 0;

/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t Address);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address) {
    uint32_t sector;

    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
        sector = FLASH_Sector_0;
    } else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_Sector_1;
    } else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_Sector_2;
    } else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_Sector_3;
    } else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_Sector_4;
    } else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_Sector_5;
    } else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_Sector_6;
    } else {
        sector = FLASH_Sector_7;
    }
    return sector;
}


FLASH_Status FlashStatus;


/**
 * 用16位-半字的方式读取flash
 * @param flash_addr flash 地址
 * @return 读取到的值
 */
//__INLINE uint16_t read_flash_16bit(uint32_t flash_addr) {
//    return *(__IO uint16_t *) flash_addr;
//}


#if STM32_FLASH_WREN    //如果使能了写

//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite) {
    u16 i;
    for (i = 0; i < NumToWrite; i++) {
        FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
        WriteAddr += 2;//地址增加2.
    }
}

/**
 * 检查一个flash范围内的数据在写入之前是否需要擦除
 * @param start_address 开始地址
 * @param end_address 结束地址
 * @return
 */
int32_t check_flash_range_is_need_erase(uint32_t start_address, uint32_t end_address) {
    //检查flash地址是否2对齐 ，因为要16bit写入
    if ((start_address % 2) != 0 || (end_address % 2) != 0)
        return -1;
    uint8_t need_erase_flag = 0;
    for (uint32_t now_address = start_address; now_address < end_address;) {
        if (read_flash_16bit(now_address) != 0xffff) {
            need_erase_flag = 1;
            break;
        }
        now_address += 2;
    }
    return need_erase_flag;
}

/**
 * 检查一个扇区是否需要擦除，如果全部都是0xff则不需要擦除
 * @param FLASH_Sector
 * @return  -1发生异常    0  不需要擦除   1 需要擦除
 */
int32_t check_flash_sector_is_need_erase(uint32_t Address) {
    uint32_t sector_start_address, sector_end_address;
    //获取该数据所在的扇区
    uint32_t flash_sector = GetSector(Address);
    switch (flash_sector) {
        case FLASH_Sector_0:
            sector_start_address = ADDR_FLASH_SECTOR_0;
            sector_end_address = ADDR_FLASH_SECTOR_1;
            break;
        case FLASH_Sector_1:
            sector_start_address = ADDR_FLASH_SECTOR_1;
            sector_end_address = ADDR_FLASH_SECTOR_2;
            break;
        case FLASH_Sector_2:
            sector_start_address = ADDR_FLASH_SECTOR_2;
            sector_end_address = ADDR_FLASH_SECTOR_3;
            break;
        case FLASH_Sector_3:
            sector_start_address = ADDR_FLASH_SECTOR_3;
            sector_end_address = ADDR_FLASH_SECTOR_4;
            break;
        case FLASH_Sector_4:
            sector_start_address = ADDR_FLASH_SECTOR_4;
            sector_end_address = ADDR_FLASH_SECTOR_5;
            break;
        case FLASH_Sector_5:
            sector_start_address = ADDR_FLASH_SECTOR_5;
            sector_end_address = ADDR_FLASH_SECTOR_6;
            break;
        case FLASH_Sector_6:
            sector_start_address = ADDR_FLASH_SECTOR_6;
            sector_end_address = ADDR_FLASH_SECTOR_7;
            break;
        case FLASH_Sector_7:
            sector_start_address = ADDR_FLASH_SECTOR_7;
            sector_end_address = ADDR_FLASH_SECTOR_8;
            break;
        default:
            return -1;
    }
    uint8_t need_erase_flag = 0;
    for (uint32_t now_address = sector_start_address; now_address < sector_end_address;) {
        if (read_flash_16bit(now_address) != 0xffff) {
            need_erase_flag = 1;
            break;
        }
        now_address += 2;
    }
    if (need_erase_flag)
        return 1;
    return 0;
}


/**
 * 从指定地址开始写入指定长度的数据，如果即将写数据的区域有数据，会先擦除所在的扇区。
 * 所以请勿直接对同一个地址写入两次，会导致其所在的扇区被擦除
 * @param WriteAddr 要写入的flash地址(eg. 0x08060000)
 * @param pBuffer 要写入的uint16_t 的数组
 * @param halfWords_to_write uint16_t数据的个数
 * @return SUCCESS为写入成功， ERROR 为写入过程发生错误
 */
ErrorStatus mflash_write(const u32 WriteAddr, u16 *const pBuffer, const uint32_t halfWords_to_write) {
    /**
     * 本函数的思路：
     * 首先检查参数是否合法
     * 然后解锁flash
     * 判断即将写入的flash范围内的数据是否都是0xffff，如果不是则需要擦除后再写入
     * 然后按16bit把buffer里的数据全部写入到flash里面去
     * 上锁后读取写入的数据，与buffer做对比
     */
    //检查地址是否合法
    if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))return ERROR;//非法地址
    if (halfWords_to_write < 1) //数量非法
        return ERROR;
    FLASH_Unlock();                        //解锁
    int32_t need_erase_flag = check_flash_range_is_need_erase(WriteAddr, WriteAddr + 2 * halfWords_to_write);
    if (need_erase_flag)//需要擦除
    {
        //获取起始地址的扇区
        uint32_t muwStartSector = GetSector(WriteAddr);
        //获取结束地址的扇区，NumToWrite是按16bit计算的，所以乘以二。
        uint32_t muwEndSector = GetSector(WriteAddr + halfWords_to_write * 2);
        /* 开始擦除操作 */
        uint32_t muwSectorCounter = muwStartSector;
        while (muwSectorCounter <= muwEndSector) {
            if (FLASH_EraseSector(muwSectorCounter, VoltageRange_3) != FLASH_COMPLETE) {
                printf_("flash erase sector wrong!\n");
                while (1) {}
            }
            /* jump to the next sector */
            //下面这句看起来不太好理解，实际上我们参考函数 FLASH_EraseSector的第一个参数，
            //就可以发现： FLASH_Sector_0是0x00, FLASH_Sector_1是0x08，
            // FLASH_Sector_2是0x10，也就是说+=8就是擦除下一个扇区的意思
            muwSectorCounter += 8;
        }
    }

    /* Program the user Flash area word by half-word ********************************/
    uwAddress = WriteAddr;
    uint32_t num_programed = 0;  //已经写入的数据的数量
    //创建一个临时的指针变量用来记录数组指向内存的位置
    uint16_t *mpBuffer = pBuffer;
    while (num_programed < halfWords_to_write) {
        if (FLASH_ProgramHalfWord(uwAddress, *mpBuffer) == FLASH_COMPLETE) {
            uwAddress = uwAddress + 2;
            mpBuffer++; //对于uint16_t
            num_programed++;
        } else {
            /* Error occurred while writing data in Flash memory.
               User can add here some code to deal with this error */
            printf_("FLASH_ProgramWord wrong!\n");
            while (1) {
            }
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) */
    FLASH_Lock();


    /* Check if the programmed data is OK ***************************************/
    /*  MemoryProgramStatus = 0: data programmed correctly
        MemoryProgramStatus != 0: number of words not programmed correctly */
    uwAddress = WriteAddr;
    mpBuffer = pBuffer;
    uwMemoryProgramStatus = 0;

    while (uwAddress < WriteAddr + 2 * halfWords_to_write) {
        uwData16 = *(__IO uint16_t *) uwAddress;

        if (uwData16 != *mpBuffer) {
            uwMemoryProgramStatus++;
        }

        uwAddress = uwAddress + 2;
        mpBuffer++;
    }

    /* Check Data correctness */
    if (uwMemoryProgramStatus) {
        /* KO */
        /* Turn on LD2 */
        printf_("FLASH_Program failed!\n");
        return ERROR;
    } else {
        /* OK */
        /* Turn on LD1 */
        printf_("FLASH_Program  finished!\n");
        return SUCCESS;
    }


}

#endif


/**
* @brief  指定地址写入一个16位数据
* @waing  写入的地址请先擦除
* @param  WriteAddress  写入的地址
* @param  data          写入的数据
* @param  None
* @retval 0:成功
* @example
**/
char FlashWriteHalfWord(uint32_t WriteAddress, u16 data) {
    mflash_write(WriteAddress, &data, 1);
//
//    FlashStatus = FLASH_BUSY;//设置为忙
//    if (FLASH_GetStatus() == FLASH_COMPLETE)//可以操作Flash
//    {
//        FLASH_Unlock();
//        FlashStatus = FLASH_ProgramHalfWord(WriteAddress, data);//写入数据
//
//        if (FlashStatus == FLASH_COMPLETE)//操作完成
//        {
//            if (STMFLASH_ReadHalfWord(WriteAddress) == data)//读出的和写入的一致
//                FlashStatus = 0;//读出和写入的一致
//            else
//                FlashStatus = 5;
//        }
//        FLASH_Lock();//上锁
//    }
    return 0;
}


//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead) {
    u16 i;
    for (i = 0; i < NumToRead; i++) {
        pBuffer[i] = read_flash_16bit(ReadAddr);//读取2个字节.
        ReadAddr += 2;//偏移2个字节.
    }
}


/**
 * 写入是否要升级的flag
 * @param update_enable 1表示要升级，0表示不升级
 */
void update_flag_set(uint8_t update_enable) {

    uint16_t update_flag = update_enable ? 1 : 0;
    //把update_flag写入到flash里
    mflash_write(FLASH_UPDATE_FLAGE_ADDR, &update_flag, 1);
}


/**
 * 检查是否需要升级
 * @return  1是需要升级，0是不需要升级
 */
uint8_t update_flag_get() {
    uint16_t update_flag_readed = read_flash_16bit(FLASH_UPDATE_FLAGE_ADDR);
    if (update_flag_readed == (uint16_t) 1) {
        return 1;
    } else {
        return 0;
    }
}