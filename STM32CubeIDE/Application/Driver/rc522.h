/*
 * rcc522.h
 *
 *  Created on: Jul 5, 2026
 *      Author: lqdung
 */


#ifndef RC522_H
#define RC522_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ============================================================
 * MFRC522 Definitions
 * ============================================================ */
#define RC522_MAX_UID_SIZE        10
#define RC522_BLOCK_SIZE          16

/* ============================================================
 * Status
 * ============================================================ */
typedef enum
{
    RC522_OK = 0,
    RC522_ERROR,
    RC522_TIMEOUT,
    RC522_NO_TAG,
    RC522_COLLISION,
    RC522_CRC_ERROR,
    RC522_AUTH_ERROR,
    RC522_INVALID_PARAM
} RC522_StatusTypeDef;

/* ============================================================
 * Card UID
 * ============================================================ */
typedef struct
{
    uint8_t uid[RC522_MAX_UID_SIZE];
    uint8_t size;
    uint8_t sak;
} RC522_UID;

/* ============================================================
 * Device Handle
 * ============================================================ */
typedef struct
{
    SPI_HandleTypeDef *hspi;

    GPIO_TypeDef *csPort;
    uint16_t csPin;

} RC522_HandleTypeDef;

/* ============================================================
 * Public API
 * ============================================================ */

/**
 * @brief Initialize RC522 driver
 */
RC522_StatusTypeDef RC522_Init(RC522_HandleTypeDef *dev,
                               SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *csPort,
                               uint16_t csPin);

/**
 * @brief Check if a new card is present
 */
RC522_StatusTypeDef RC522_IsCardPresent(RC522_HandleTypeDef *dev);

/**
 * @brief Read UID of current card
 */
RC522_StatusTypeDef RC522_ReadUID(RC522_HandleTypeDef *dev,
                                  RC522_UID *uid);

/**
 * @brief Authenticate one block
 */
RC522_StatusTypeDef RC522_Authenticate(RC522_HandleTypeDef *dev,
                                       uint8_t blockAddr,
                                       uint8_t keyType,
                                       const uint8_t key[6],
                                       const RC522_UID *uid);

/**
 * @brief Read one 16-byte block
 */
RC522_StatusTypeDef RC522_ReadBlock(RC522_HandleTypeDef *dev,
                                    uint8_t blockAddr,
                                    uint8_t data[RC522_BLOCK_SIZE]);

/**
 * @brief Write one 16-byte block
 */
RC522_StatusTypeDef RC522_WriteBlock(RC522_HandleTypeDef *dev,
                                     uint8_t blockAddr,
                                     const uint8_t data[RC522_BLOCK_SIZE]);

/**
 * @brief Stop encrypted communication
 */
RC522_StatusTypeDef RC522_StopCrypto(RC522_HandleTypeDef *dev);

/**
 * @brief Put current PICC into HALT state
 */
RC522_StatusTypeDef RC522_Halt(RC522_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* RC522_H */
