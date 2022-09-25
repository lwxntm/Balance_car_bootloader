//
// Created by xiaotian on 2022/9/25.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_UART1_H
#define F401_BALANCE_CAR_BOOTLOADER_UART1_H


/*缓冲区*/
#ifndef USART_C_//如果没有定义
#define USART_C_ extern
#else
#define USART_C_
#endif


#define UpdateUsart1ReadLen  			5  	//定义最大接收字节数(缓冲程序)

#define Usart1ReadLen 150

USART_C_ u8  UpdateStartFlage;


USART_C_ char  Usart1ReadBuff[Usart1ReadLen];  //接收数据缓存
USART_C_ u32 Usart1ReadCnt;//串口1接收到的数据个数
USART_C_ u32 Usart1ReadCntCopy;//串口1接收到的数据个数
USART_C_ u32 Usart1IdleCnt;//空闲检测用
USART_C_ u8  Usart1ReadFlage;//串口1接收到一条完整数据

USART_C_ u8  UpdateUsart1Buff[UpdateUsart1ReadLen];  //接收数据缓存
USART_C_ u8  UpdateOverflow;//是不是溢出




void uart1_init(u32 bound);



#endif //F401_BALANCE_CAR_BOOTLOADER_UART1_H
