//
// Created by xiaotian on 2022/9/25.
//
#include "main.h"
#include "uart1.h"


/**
 * 本文件暂时提供了基于UART1 (PA9-PA10)的串口通信功能，后面会替换为ESP8266的通信
 */



char  Usart1ReadBuff[Usart1ReadLen]={0};  //接收数据临时缓存
u32 Usart1ReadCnt = 0;//串口1接收到的数据个数
u32 Usart1ReadCntCopy = 0;//串口1接收到的数据个数
u32 Usart1IdleCnt = 0;//空闲检测用
u8  Usart1ReadFlage=0;//串口1接收到一条完整数据

u8  UpdateStartFlage = 0;

//更新程序使用
u8  UpdateUsart1Buff[UpdateUsart1ReadLen]={0};  //接收数据缓存
u8  UpdateOverflow = 0;//是不是溢出

/**
 * printf_ 需要实现的底层函数
 * @param character
 */
void _putchar(char character) {
    USART_ClearITPendingBit(USART1, USART_IT_TC);//解决printf丢失第一个字节的问题
    USART_SendData(USART1, (uint8_t) character);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
}

/**
 * 串口1初始化
 * @param bound 波特率
 */
void uart1_init(u32 bound) {
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);    //使能USART1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);    //使能GPIOA时钟

    //USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //复用
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9 10
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);


    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    //收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_Cmd(USART1, ENABLE);                    //使能串口1


    NVIC_InitTypeDef NVIC_InitStructure;
    /*串口--1*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化VIC寄存器
}


void USART1_IRQHandler(void)                    //串口1中断服务程序
{
    u8 Res;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        Res = USART_ReceiveData(USART1);    //读取接收到的数据

        //如果Usart1ReadBuff 还有空间，则把接收到的数据丢进去
        if(Usart1ReadCnt < Usart1ReadLen-1)
        {
            Usart1ReadBuff[Usart1ReadCnt] = Res;
        }
        else
        {
            Usart1ReadCnt=0;
        }
        Usart1ReadCnt ++;	//数据个数
        Usart1IdleCnt = 0;

//        if(UpdataStartFlage)//如果置位升级标志
//        {
//            if(PutData(&pRb,&Res,1) == -1)//写入环形队列
//            {
//                UpdateOverflow = 1;//环形队列溢出
//            }
//        }
    }
}
