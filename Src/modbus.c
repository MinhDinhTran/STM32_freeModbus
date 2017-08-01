#include "modbus.h"

/*定义线圈状态寄存器的地址起始值和存储数组*/
uint8_t   ucRegCoilsStart = REG_COILS_START;
uint8_t   ucRegCoilsBuf[REG_COILS_SIZE / 8];

/*定义线圈输入状态寄存器的地址起始值和存储数组*/
uint8_t   ucRegDiscreteStart = REG_DISCRETE_START;
uint8_t   ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8];

/*定义保持寄存器的地址起始值和存储数组*/
uint16_t   usRegHoldingStart = REG_HOLDING_START;
uint16_t   usRegHoldingBuf[REG_HOLDING_NREGS] = {1,2,3,4,5,6,7,8,9,0};

/*定义输入寄存器的地址起始值和存储数组*/
uint16_t   usRegInputStart = REG_INPUT_START;
uint16_t   usRegInputBuf[REG_INPUT_NREGS];


/*
 * 以下函数声明在mb.h文件中，使用时必须include modbus.h头文件才能使freeModbus正常工作
 *
 * */

/**
* @brief 输入寄存器处理函数，输入寄存器可读，但不可写。
* @param pucRegBuffer 返回数据指针
* usAddress 寄存器起始地址
* usNRegs 寄存器长度
* @retval eStatus 寄存器状态
*/

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int16_t             iRegIndex;

//  用作例子
    usRegInputBuf[0] = 0x11;
//  例子结束

    //查询是否在寄存器范围内
    //为了避免警告，修改为有符号整数
    if( ((int16_t) usAddress >= REG_INPUT_START ) \
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        //获得操作偏移量，本次操作起始地址-输入寄存器的初始地址
        iRegIndex = ( int16_t )( usAddress - usRegInputStart );
        //逐个赋值
        while( usNRegs > 0 )
        {
            //赋值高字节
            *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] >> 8 );
            //赋值低字节
            *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] & 0xFF );
            //偏移量增加
            iRegIndex++;
            //被操作寄存器数量递减
            usNRegs--;
        }
    }
    else
    {
        //返回错误状态，寄存器数量不对
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
* @brief 保持寄存器处理函数，保持寄存器可读，可读可写
* @param pucRegBuffer 读操作时--返回数据指针，写操作时--输入数据指针
* usAddress 寄存器起始地址
* usNRegs 寄存器长度
* eMode 操作方式，读或者写
* @retval eStatus 寄存器状态
*/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
eMBRegisterMode eMode )
{
    //错误状态
    eMBErrorCode eStatus = MB_ENOERR;
    //偏移量
    int16_t iRegIndex;

    //判断寄存器是不是在范围内
    if( ( (int16_t)usAddress >= REG_HOLDING_START ) \
    && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        //计算偏移量
        iRegIndex = ( int16_t )( usAddress - REG_HOLDING_START );

        switch ( eMode )
        {
            //读处理函数
            case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            //写处理函数
            case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        //返回错误状态
        eStatus = MB_ENOREG;
    }

    return eStatus;
}


/**
* @brief 线圈寄存器处理函数，线圈寄存器可读，可读可写
* @param pucRegBuffer 读操作---返回数据指针，写操作--返回数据指针
* usAddress 寄存器起始地址
* usNRegs 寄存器长度
* eMode 操作方式，读或者写
* @retval eStatus 寄存器状态
*/
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
eMBRegisterMode eMode )
{
    //错误状态
    eMBErrorCode eStatus = MB_ENOERR;
    //寄存器个数
    int16_t iNCoils = ( int16_t )usNCoils;
    //寄存器偏移量
    int16_t usBitOffset;

    //检查寄存器是否在指定范围内
    if( ( (int16_t)usAddress >= REG_COILS_START ) &&
    ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
    {
        //计算寄存器偏移量
        usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
        switch ( eMode )
        {
            //读操作
            case MB_REG_READ:
                while( iNCoils > 0 )
                {
                    *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
                    ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
                    iNCoils -= 8;
                    usBitOffset += 8;
                }
            break;

            //写操作
            case MB_REG_WRITE:
            while( iNCoils > 0 )
                {
                    xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
                    ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
                    *pucRegBuffer++ );
                    iNCoils -= 8;
                }
            break;
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
* @brief 开关输入寄存器处理函数，开关输入寄存器，可读
* @param pucRegBuffer 读操作---返回数据指针，写操作--返回数据指针
* usAddress 寄存器起始地址
* usNRegs 寄存器长度
* eMode 操作方式，读或者写
* @retval eStatus 寄存器状态
*/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    //错误状态
    eMBErrorCode eStatus = MB_ENOERR;
    //操作寄存器个数
    int16_t iNDiscrete = ( int16_t )usNDiscrete;
    //偏移量
    uint16_t usBitOffset;

    //判断寄存器时候再制定范围内
    if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
    ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
    {
        //获得偏移量
        usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );

        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
            ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset += 8;
        }

    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
