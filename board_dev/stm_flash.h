//
// Created by xiaotian on 2022/9/25.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_STM_FLASH_H
#define F401_BALANCE_CAR_BOOTLOADER_STM_FLASH_H

#include "stm32f4xx.h"

/*
 * Sector 0 0x0800 0000 - 0x0800 3FFF 16 Kbytes   bootloader
Sector 1 0x0800 4000 - 0x0800 7FFF 16 Kbytes      --
Sector 2 0x0800 8000 - 0x0800 BFFF 16 Kbytes      --
Sector 3 0x0800 C000 - 0x0800 FFFF 16 Kbytes      user-datas
Sector 4 0x0801 0000 - 0x0801 FFFF 64 Kbytes      user-applications
Sector 5 0x0802 0000 - 0x0803 FFFF 128 Kbytes     --
Sector 6 0x0804 0000 - 0x0805 FFFF 128 Kbytes     --
Sector 7 0x0806 0000 - 0x0807 FFFF 128 Kbytes     --
 读取操作：一次最多可以读取16个字节(128bits)
 Byte, half-word, word and double word write
 每次擦除至少要擦除一个扇区
 * */

//不同扇区的起始地址
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
//下面的扇区在512K flash 的stm32设备上不存在，暂时注释掉
#if 0

#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */
#endif


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_4   /* Start address of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_8  /* End address of user Flash area */



//用户根据自己的需要设置
#define STM32_FLASH_SIZE 512 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1        //使能FLASH写入(0，不使能;1，使能)
#define FLASH_IAP_SIZE (16*3)         //bootloader所用容量大小(KB)
#define FLASH_DATA_SIZE 16					//存储用户数据所用容量大小(KB)

//用户程序大小 = (总FLASH容量 - 存储用户数据所用容量大小(KB) - bootloader所用容量大小(KB))
#define FLASH_USER_SIZE  (STM32_FLASH_SIZE - FLASH_DATA_SIZE - FLASH_IAP_SIZE)  //用户程序大小(KB)

#define STM32_FLASH_BASE 0x8000000 	//STM32 FLASH的起始地址


//用户程序运行地址:FLASH的起始地址 + bootloader所用程序大小 +DATA 所用大小
#define FLASH_APP1_ADDR  (STM32_FLASH_BASE+1024*FLASH_IAP_SIZE +1024* FLASH_DATA_SIZE)

//存储用户数据地址:
#define FLASH_DATA_ADDR  (STM32_FLASH_BASE+1024*FLASH_IAP_SIZE)

//把更新标志放到 FLASH_DATA_ADDR
#define FLASH_UPDATE_FLAGE_ADDR (FLASH_DATA_ADDR) //存储更新标志

void mflash_write(const u32 WriteAddr, u16 *const pBuffer, const u16 halfWords_to_write);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据
char FlashWriteHalfWord(uint32_t WriteAddress,u16 data);

#define read_flash_16bit(flash_addr) (*(__IO uint16_t *)(flash_addr))


#endif //F401_BALANCE_CAR_BOOTLOADER_STM_FLASH_H
