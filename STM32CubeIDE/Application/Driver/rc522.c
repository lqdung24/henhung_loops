/*
 * rc522.c
 *
 *  Created on: Jul 5, 2026
 *      Author: lqdung
 */

/**
 ******************************************************************************
 * @file    rc522.c
 * @author  Refactored from Tilen Majerle library
 * @brief   MFRC522 HAL Driver
 ******************************************************************************
 */

#include "rc522.h"
#include "rc522_reg.h"

#include <string.h>

/*=========================================================================*/
/*                      Private Function Prototypes                         */
/*=========================================================================*/

static HAL_StatusTypeDef RC522_WriteRegister(RC522_HandleTypeDef *dev,
                                             uint8_t reg,
                                             uint8_t value);

static HAL_StatusTypeDef RC522_ReadRegister(RC522_HandleTypeDef *dev,
                                            uint8_t reg,
                                            uint8_t *value);

static HAL_StatusTypeDef RC522_SetBitMask(RC522_HandleTypeDef *dev,
                                          uint8_t reg,
                                          uint8_t mask);

static HAL_StatusTypeDef RC522_ClearBitMask(RC522_HandleTypeDef *dev,
                                            uint8_t reg,
                                            uint8_t mask);

static void RC522_Reset(RC522_HandleTypeDef *dev);

static void RC522_AntennaOn(RC522_HandleTypeDef *dev);

static void RC522_AntennaOff(RC522_HandleTypeDef *dev);

/* Các hàm sẽ được implement ở Part 2 */
static RC522_StatusTypeDef RC522_ToCard(RC522_HandleTypeDef *dev,
                                        uint8_t command,
                                        uint8_t *sendData,
                                        uint8_t sendLen,
                                        uint8_t *backData,
                                        uint16_t *backBits);

static void RC522_CalculateCRC(RC522_HandleTypeDef *dev,
                               uint8_t *data,
                               uint8_t length,
                               uint8_t *result);

/*=========================================================================*/
/*                          Public Functions                               */
/*=========================================================================*/

RC522_StatusTypeDef RC522_Init(RC522_HandleTypeDef *dev,
                               SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *csPort,
                               uint16_t csPin)
{
    if (dev == NULL ||
        hspi == NULL ||
        csPort == NULL)
    {
        return RC522_INVALID_PARAM;
    }

    dev->hspi   = hspi;
    dev->csPort = csPort;
    dev->csPin  = csPin;

    HAL_GPIO_WritePin(dev->csPort,
                      dev->csPin,
                      GPIO_PIN_SET);

    RC522_Reset(dev);

    RC522_WriteRegister(dev, RC522_REG_T_MODE, 0x8D);
    RC522_WriteRegister(dev, RC522_REG_T_PRESCALER, 0x3E);

    RC522_WriteRegister(dev, RC522_REG_T_RELOAD_L, 30);
    RC522_WriteRegister(dev, RC522_REG_T_RELOAD_H, 0);

    /* 48 dB gain */
    RC522_WriteRegister(dev, RC522_REG_RF_CFG, 0x70);

    RC522_WriteRegister(dev, RC522_REG_TX_ASK, 0x40);

    RC522_WriteRegister(dev, RC522_REG_MODE, 0x3D);

    RC522_AntennaOn(dev);

    return RC522_OK;
}

/*=========================================================================*/
/*                        Register Access                                  */
/*=========================================================================*/

static HAL_StatusTypeDef RC522_WriteRegister(RC522_HandleTypeDef *dev,
                                             uint8_t reg,
                                             uint8_t value)
{
    uint8_t tx[2];

    tx[0] = (reg << 1) & 0x7E;
    tx[1] = value;

    HAL_GPIO_WritePin(dev->csPort,
                      dev->csPin,
                      GPIO_PIN_RESET);

    HAL_StatusTypeDef ret =
        HAL_SPI_Transmit(dev->hspi,
                         tx,
                         2,
                         HAL_MAX_DELAY);

    HAL_GPIO_WritePin(dev->csPort,
                      dev->csPin,
                      GPIO_PIN_SET);

    return ret;
}

static HAL_StatusTypeDef RC522_ReadRegister(RC522_HandleTypeDef *dev,
                                            uint8_t reg,
                                            uint8_t *value)
{
    uint8_t addr;

    addr = ((reg << 1) & 0x7E) | 0x80;

    HAL_GPIO_WritePin(dev->csPort,
                      dev->csPin,
                      GPIO_PIN_RESET);

    HAL_StatusTypeDef ret;

    ret = HAL_SPI_Transmit(dev->hspi,
                           &addr,
                           1,
                           HAL_MAX_DELAY);

    if (ret == HAL_OK)
    {
        ret = HAL_SPI_Receive(dev->hspi,
                              value,
                              1,
                              HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(dev->csPort,
                      dev->csPin,
                      GPIO_PIN_SET);

    return ret;
}

/*=========================================================================*/
/*                         Bit Operations                                  */
/*=========================================================================*/

static HAL_StatusTypeDef RC522_SetBitMask(RC522_HandleTypeDef *dev,
                                          uint8_t reg,
                                          uint8_t mask)
{
    uint8_t value;

    if (RC522_ReadRegister(dev,
                           reg,
                           &value) != HAL_OK)
    {
        return HAL_ERROR;
    }

    value |= mask;

    return RC522_WriteRegister(dev,
                               reg,
                               value);
}

static HAL_StatusTypeDef RC522_ClearBitMask(RC522_HandleTypeDef *dev,
                                            uint8_t reg,
                                            uint8_t mask)
{
    uint8_t value;

    if (RC522_ReadRegister(dev,
                           reg,
                           &value) != HAL_OK)
    {
        return HAL_ERROR;
    }

    value &= ~mask;

    return RC522_WriteRegister(dev,
                               reg,
                               value);
}

/*=========================================================================*/
/*                       Internal Functions                                */
/*=========================================================================*/

static void RC522_Reset(RC522_HandleTypeDef *dev)
{
    RC522_WriteRegister(dev,
                        RC522_REG_COMMAND,
                        RC522_CMD_SOFT_RESET);

    HAL_Delay(50);
}

static void RC522_AntennaOn(RC522_HandleTypeDef *dev)
{
    uint8_t value;

    RC522_ReadRegister(dev,
                       RC522_REG_TX_CONTROL,
                       &value);

    if ((value & 0x03) == 0)
    {
        RC522_SetBitMask(dev,
                         RC522_REG_TX_CONTROL,
                         0x03);
    }
}

static void RC522_AntennaOff(RC522_HandleTypeDef *dev)
{
    RC522_ClearBitMask(dev,
                       RC522_REG_TX_CONTROL,
                       0x03);
}

/*=========================================================================*/
/*                         CRC Calculation                                 */
/*=========================================================================*/

static void RC522_CalculateCRC(RC522_HandleTypeDef *dev,
                               uint8_t *data,
                               uint8_t length,
                               uint8_t *result)
{
    uint8_t i;
    uint8_t irq;

    RC522_ClearBitMask(dev,
                       RC522_REG_DIV_IRQ,
                       0x04);

    RC522_SetBitMask(dev,
                     RC522_REG_FIFO_LEVEL,
                     0x80);

    RC522_WriteRegister(dev,
                        RC522_REG_COMMAND,
                        RC522_CMD_IDLE);

    for (i = 0; i < length; i++)
    {
        RC522_WriteRegister(dev,
                            RC522_REG_FIFO_DATA,
                            data[i]);
    }

    RC522_WriteRegister(dev,
                        RC522_REG_COMMAND,
                        RC522_CMD_CALC_CRC);

    i = 0xFF;

    do
    {
        RC522_ReadRegister(dev,
                           RC522_REG_DIV_IRQ,
                           &irq);
        i--;
    }
    while ((i != 0) && !(irq & 0x04));

    RC522_ReadRegister(dev,
                       RC522_REG_CRC_RESULT_L,
                       &result[0]);

    RC522_ReadRegister(dev,
                       RC522_REG_CRC_RESULT_H,
                       &result[1]);
}

/*=========================================================================*/
/*                       Communication Core                                */
/*=========================================================================*/

static RC522_StatusTypeDef RC522_ToCard(RC522_HandleTypeDef *dev,
                                        uint8_t command,
                                        uint8_t *sendData,
                                        uint8_t sendLen,
                                        uint8_t *backData,
                                        uint16_t *backBits)
{
    RC522_StatusTypeDef status = RC522_ERROR;

    uint8_t irqEnable = 0;
    uint8_t waitIRQ = 0;

    uint8_t irq;
    uint8_t error;
    uint8_t fifoLevel;
    uint8_t lastBits;

    uint16_t i;

    switch (command)
    {
        case RC522_CMD_AUTHENT:
            irqEnable = 0x12;
            waitIRQ   = 0x10;
            break;

        case RC522_CMD_TRANSCEIVE:
            irqEnable = 0x77;
            waitIRQ   = 0x30;
            break;

        default:
            break;
    }

    RC522_WriteRegister(dev,
                        RC522_REG_COM_I_EN,
                        irqEnable | 0x80);

    RC522_ClearBitMask(dev,
                       RC522_REG_COM_IRQ,
                       0x80);

    RC522_SetBitMask(dev,
                     RC522_REG_FIFO_LEVEL,
                     0x80);

    RC522_WriteRegister(dev,
                        RC522_REG_COMMAND,
                        RC522_CMD_IDLE);

    for (i = 0; i < sendLen; i++)
    {
        RC522_WriteRegister(dev,
                            RC522_REG_FIFO_DATA,
                            sendData[i]);
    }

    RC522_WriteRegister(dev,
                        RC522_REG_COMMAND,
                        command);

    if (command == RC522_CMD_TRANSCEIVE)
    {
        RC522_SetBitMask(dev,
                         RC522_REG_BIT_FRAMING,
                         RC522_BIT_FRAMING_START_SEND);
    }

    i = 2000;

    do
    {
        RC522_ReadRegister(dev,
                           RC522_REG_COM_IRQ,
                           &irq);

        i--;

    } while ((i != 0) &&
             !(irq & 0x01) &&
             !(irq & waitIRQ));

    RC522_ClearBitMask(dev,
                       RC522_REG_BIT_FRAMING,
                       RC522_BIT_FRAMING_START_SEND);

    if (i == 0)
    {
        return RC522_TIMEOUT;
    }

    RC522_ReadRegister(dev,
                       RC522_REG_ERROR,
                       &error);

    if (error & 0x1B)
    {
        if (error & RC522_ERROR_COLLISION)
        {
            return RC522_COLLISION;
        }

        if (error & RC522_ERROR_CRC)
        {
            return RC522_CRC_ERROR;
        }

        return RC522_ERROR;
    }

    status = RC522_OK;

    if (irq & irqEnable & 0x01)
    {
        status = RC522_NO_TAG;
    }

    if (command == RC522_CMD_TRANSCEIVE)
    {
        RC522_ReadRegister(dev,
                           RC522_REG_FIFO_LEVEL,
                           &fifoLevel);

        RC522_ReadRegister(dev,
                           RC522_REG_CONTROL,
                           &lastBits);

        lastBits &= 0x07;

        if (lastBits)
        {
            *backBits = (fifoLevel - 1) * 8 + lastBits;
        }
        else
        {
            *backBits = fifoLevel * 8;
        }

        if (fifoLevel == 0)
        {
            fifoLevel = 1;
        }

        if (fifoLevel > RC522_FIFO_SIZE)
        {
            fifoLevel = RC522_FIFO_SIZE;
        }

        for (i = 0; i < fifoLevel; i++)
        {
            RC522_ReadRegister(dev,
                               RC522_REG_FIFO_DATA,
                               &backData[i]);
        }
    }

    return status;
}

/*=========================================================================*/
/*                          Request Card                                   */
/*=========================================================================*/

static RC522_StatusTypeDef RC522_Request(RC522_HandleTypeDef *dev,
                                         uint8_t reqMode,
                                         uint8_t *tagType)
{
    uint16_t backBits;

    RC522_WriteRegister(dev,
                        RC522_REG_BIT_FRAMING,
                        0x07);

    tagType[0] = reqMode;

    RC522_StatusTypeDef status;

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          tagType,
                          1,
                          tagType,
                          &backBits);

    if ((status != RC522_OK) || (backBits != 0x10))
    {
        return RC522_ERROR;
    }

    return RC522_OK;
}

/*=========================================================================*/
/*                        Anti Collision                                   */
/*=========================================================================*/

static RC522_StatusTypeDef RC522_Anticoll(RC522_HandleTypeDef *dev,
                                          uint8_t *uid)
{
    uint16_t backBits;

    uint8_t i;
    uint8_t check = 0;

    RC522_WriteRegister(dev,
                        RC522_REG_BIT_FRAMING,
                        0x00);

    uid[0] = RC522_PICC_ANTICOLL;
    uid[1] = 0x20;

    RC522_StatusTypeDef status;

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          uid,
                          2,
                          uid,
                          &backBits);

    if (status != RC522_OK)
        return status;

    for (i = 0; i < 4; i++)
    {
        check ^= uid[i];
    }

    if (check != uid[4])
    {
        return RC522_CRC_ERROR;
    }

    return RC522_OK;
}

/*=========================================================================*/
/*                           Select Card                                   */
/*=========================================================================*/

static RC522_StatusTypeDef RC522_SelectTag(RC522_HandleTypeDef *dev,
                                           RC522_UID *uid)
{
    uint8_t buffer[9];

    uint16_t recvBits;

    buffer[0] = RC522_PICC_SELECTTAG;
    buffer[1] = 0x70;

    memcpy(&buffer[2],
           uid->uid,
           5);

    RC522_CalculateCRC(dev,
                       buffer,
                       7,
                       &buffer[7]);

    RC522_StatusTypeDef status;

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          buffer,
                          9,
                          buffer,
                          &recvBits);

    if (status != RC522_OK)
        return status;

    if (recvBits != 0x18)
        return RC522_ERROR;

    uid->sak = buffer[0];

    return RC522_OK;
}

/*=========================================================================*/
/*                       Public API                                        */
/*=========================================================================*/

RC522_StatusTypeDef RC522_IsCardPresent(RC522_HandleTypeDef *dev)
{
    uint8_t type[2];

    return RC522_Request(dev,
                         RC522_PICC_REQIDL,
                         type);
}

RC522_StatusTypeDef RC522_ReadUID(RC522_HandleTypeDef *dev,
                                  RC522_UID *uid)
{
    RC522_StatusTypeDef status;

    memset(uid,
           0,
           sizeof(RC522_UID));

    status = RC522_Request(dev,
                           RC522_PICC_REQIDL,
                           uid->uid);

    if (status != RC522_OK)
        return status;

    status = RC522_Anticoll(dev,
                            uid->uid);

    if (status != RC522_OK)
        return status;

    uid->size = 4;

    status = RC522_SelectTag(dev,
                             uid);

    if (status != RC522_OK)
        return status;

    return RC522_OK;
}

/*=========================================================================*/
/*                         Authentication                                  */
/*=========================================================================*/

RC522_StatusTypeDef RC522_Authenticate(RC522_HandleTypeDef *dev,
                                       uint8_t blockAddr,
                                       uint8_t keyType,
                                       const uint8_t key[6],
                                       const RC522_UID *uid)
{
    uint8_t buffer[12];
    uint16_t backBits;
    uint8_t status2;

    buffer[0] = keyType;
    buffer[1] = blockAddr;

    memcpy(&buffer[2], key, 6);
    memcpy(&buffer[8], uid->uid, 4);

    RC522_StatusTypeDef status;

    status = RC522_ToCard(dev,
                          RC522_CMD_AUTHENT,
                          buffer,
                          sizeof(buffer),
                          buffer,
                          &backBits);

    if (status != RC522_OK)
        return status;

    RC522_ReadRegister(dev,
                       RC522_REG_STATUS2,
                       &status2);

    if (!(status2 & RC522_STATUS2_MF_CRYPTO1_ON))
        return RC522_AUTH_ERROR;

    return RC522_OK;
}

/*=========================================================================*/
/*                           Read Block                                    */
/*=========================================================================*/

RC522_StatusTypeDef RC522_ReadBlock(RC522_HandleTypeDef *dev,
                                    uint8_t blockAddr,
                                    uint8_t data[16])
{
    uint16_t backBits;

    data[0] = RC522_PICC_MF_READ;
    data[1] = blockAddr;

    RC522_CalculateCRC(dev,
                       data,
                       2,
                       &data[2]);

    RC522_StatusTypeDef status;

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          data,
                          4,
                          data,
                          &backBits);

    if (status != RC522_OK)
        return status;

    if (backBits != 0x90)
        return RC522_ERROR;

    return RC522_OK;
}

/*=========================================================================*/
/*                           Write Block                                   */
/*=========================================================================*/

RC522_StatusTypeDef RC522_WriteBlock(RC522_HandleTypeDef *dev,
                                     uint8_t blockAddr,
                                     const uint8_t data[16])
{
    uint8_t buffer[18];

    uint16_t backBits;

    RC522_StatusTypeDef status;

    buffer[0] = RC522_PICC_MF_WRITE;
    buffer[1] = blockAddr;

    RC522_CalculateCRC(dev,
                       buffer,
                       2,
                       &buffer[2]);

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          buffer,
                          4,
                          buffer,
                          &backBits);

    if (status != RC522_OK)
        return status;

    if (backBits != 4 || ((buffer[0] & 0x0F) != 0x0A))
        return RC522_ERROR;

    memcpy(buffer,
           data,
           16);

    RC522_CalculateCRC(dev,
                       buffer,
                       16,
                       &buffer[16]);

    status = RC522_ToCard(dev,
                          RC522_CMD_TRANSCEIVE,
                          buffer,
                          18,
                          buffer,
                          &backBits);

    if (status != RC522_OK)
        return status;

    if (backBits != 4 || ((buffer[0] & 0x0F) != 0x0A))
        return RC522_ERROR;

    return RC522_OK;
}

/*=========================================================================*/
/*                       Stop Crypto                                       */
/*=========================================================================*/

RC522_StatusTypeDef RC522_StopCrypto(RC522_HandleTypeDef *dev)
{
    return RC522_ClearBitMask(dev,
                              RC522_REG_STATUS2,
                              RC522_STATUS2_MF_CRYPTO1_ON) == HAL_OK ?
                              RC522_OK :
                              RC522_ERROR;
}

/*=========================================================================*/
/*                             Halt                                        */
/*=========================================================================*/

RC522_StatusTypeDef RC522_Halt(RC522_HandleTypeDef *dev)
{
    uint8_t buffer[4];

    uint16_t backBits;

    buffer[0] = RC522_PICC_HLTA;
    buffer[1] = 0x00;

    RC522_CalculateCRC(dev,
                       buffer,
                       2,
                       &buffer[2]);

    RC522_ToCard(dev,
                 RC522_CMD_TRANSCEIVE,
                 buffer,
                 4,
                 buffer,
                 &backBits);

    return RC522_OK;
}


