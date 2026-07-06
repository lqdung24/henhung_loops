#ifndef RC522_REG_H
#define RC522_REG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*=============================================================================
 * SPI
 *===========================================================================*/
#define RC522_SPI_READ_MASK                 0x80
#define RC522_SPI_WRITE_MASK                0x7E

/*=============================================================================
 * MFRC522 Commands
 *===========================================================================*/
#define RC522_CMD_IDLE                      0x00
#define RC522_CMD_MEM                       0x01
#define RC522_CMD_RANDOM_ID                 0x02
#define RC522_CMD_CALC_CRC                  0x03
#define RC522_CMD_TRANSMIT                  0x04
#define RC522_CMD_NO_CMD_CHANGE             0x07
#define RC522_CMD_RECEIVE                   0x08
#define RC522_CMD_TRANSCEIVE                0x0C
#define RC522_CMD_AUTHENT                   0x0E
#define RC522_CMD_SOFT_RESET                0x0F

/*=============================================================================
 * PICC Commands (ISO14443A)
 *===========================================================================*/
#define RC522_PICC_REQA                     0x26
#define RC522_PICC_WUPA                     0x52

#define RC522_PICC_CT                       0x88

#define RC522_PICC_SEL_CL1                  0x93
#define RC522_PICC_SEL_CL2                  0x95
#define RC522_PICC_SEL_CL3                  0x97

#define RC522_PICC_HLTA                     0x50

#define RC522_PICC_MF_AUTH_KEY_A            0x60
#define RC522_PICC_MF_AUTH_KEY_B            0x61

#define RC522_PICC_MF_READ                  0x30
#define RC522_PICC_MF_WRITE                 0xA0
#define RC522_PICC_MF_DECREMENT             0xC0
#define RC522_PICC_MF_INCREMENT             0xC1
#define RC522_PICC_MF_RESTORE               0xC2
#define RC522_PICC_MF_TRANSFER              0xB0

/*----------------------------------------------------------------------------
 * Compatibility aliases (TM_MFRC522 library)
 *---------------------------------------------------------------------------*/
#define RC522_PICC_REQIDL                   RC522_PICC_REQA
#define RC522_PICC_REQALL                   RC522_PICC_WUPA

#define RC522_PICC_ANTICOLL                 RC522_PICC_SEL_CL1
#define RC522_PICC_SELECTTAG                RC522_PICC_SEL_CL1

#define RC522_PICC_AUTHENT1A                RC522_PICC_MF_AUTH_KEY_A
#define RC522_PICC_AUTHENT1B                RC522_PICC_MF_AUTH_KEY_B

#define RC522_PICC_READ                     RC522_PICC_MF_READ
#define RC522_PICC_WRITE                    RC522_PICC_MF_WRITE
#define RC522_PICC_DECREMENT                RC522_PICC_MF_DECREMENT
#define RC522_PICC_INCREMENT                RC522_PICC_MF_INCREMENT
#define RC522_PICC_RESTORE                  RC522_PICC_MF_RESTORE
#define RC522_PICC_TRANSFER                 RC522_PICC_MF_TRANSFER

#define RC522_PICC_HALT                     RC522_PICC_HLTA

/*=============================================================================
 * Register Map
 *===========================================================================*/

/* Page 0 */
#define RC522_REG_COMMAND                   0x01
#define RC522_REG_COM_I_EN                  0x02
#define RC522_REG_DIV_I_EN                  0x03
#define RC522_REG_COM_IRQ                   0x04
#define RC522_REG_DIV_IRQ                   0x05
#define RC522_REG_ERROR                     0x06
#define RC522_REG_STATUS1                   0x07
#define RC522_REG_STATUS2                   0x08
#define RC522_REG_FIFO_DATA                 0x09
#define RC522_REG_FIFO_LEVEL                0x0A
#define RC522_REG_WATER_LEVEL               0x0B
#define RC522_REG_CONTROL                   0x0C
#define RC522_REG_BIT_FRAMING               0x0D
#define RC522_REG_COLL                      0x0E

/* Page 1 */
#define RC522_REG_MODE                      0x11
#define RC522_REG_TX_MODE                   0x12
#define RC522_REG_RX_MODE                   0x13
#define RC522_REG_TX_CONTROL                0x14
#define RC522_REG_TX_ASK                    0x15
#define RC522_REG_TX_SEL                    0x16
#define RC522_REG_RX_SEL                    0x17
#define RC522_REG_RX_THRESHOLD              0x18
#define RC522_REG_DEMOD                     0x19
#define RC522_REG_MIFARE                    0x1C
#define RC522_REG_SERIAL_SPEED              0x1F

/* Page 2 */
#define RC522_REG_CRC_RESULT_H              0x21
#define RC522_REG_CRC_RESULT_L              0x22
#define RC522_REG_MOD_WIDTH                 0x24
#define RC522_REG_RF_CFG                    0x26
#define RC522_REG_GS_N                      0x27
#define RC522_REG_CW_GS_P                   0x28
#define RC522_REG_MOD_GS_P                  0x29
#define RC522_REG_T_MODE                    0x2A
#define RC522_REG_T_PRESCALER               0x2B
#define RC522_REG_T_RELOAD_H                0x2C
#define RC522_REG_T_RELOAD_L                0x2D
#define RC522_REG_T_COUNTER_H               0x2E
#define RC522_REG_T_COUNTER_L               0x2F

/* Page 3 */
#define RC522_REG_TEST_SEL1                 0x31
#define RC522_REG_TEST_SEL2                 0x32
#define RC522_REG_TEST_PIN_EN               0x33
#define RC522_REG_TEST_PIN_VALUE            0x34
#define RC522_REG_TEST_BUS                  0x35
#define RC522_REG_AUTO_TEST                 0x36
#define RC522_REG_VERSION                   0x37
#define RC522_REG_ANALOG_TEST               0x38
#define RC522_REG_TEST_ADC1                 0x39
#define RC522_REG_TEST_ADC2                 0x3A
#define RC522_REG_TEST_ADC0                 0x3B

/*=============================================================================
 * Bit Definitions
 *===========================================================================*/

/* Status2Reg */
#define RC522_STATUS2_MF_CRYPTO1_ON         (1U << 3)

/* BitFramingReg */
#define RC522_BIT_FRAMING_START_SEND        (1U << 7)

/* ErrorReg */
#define RC522_ERROR_WR_ERR                  (1U << 7)
#define RC522_ERROR_TEMP_ERR                (1U << 6)
#define RC522_ERROR_BUFFER_OVFL             (1U << 4)
#define RC522_ERROR_COLLISION               (1U << 3)
#define RC522_ERROR_CRC                     (1U << 2)
#define RC522_ERROR_PARITY                  (1U << 1)
#define RC522_ERROR_PROTOCOL                (1U << 0)

/*=============================================================================
 * Misc
 *===========================================================================*/
#define RC522_FIFO_SIZE                     64

#define RC522_UID_SIZE_SINGLE               4
#define RC522_UID_SIZE_DOUBLE               7
#define RC522_UID_SIZE_TRIPLE               10

#define RC522_KEY_SIZE                      6
#define RC522_BLOCK_SIZE                    16

/* Default Key A (MIFARE Classic) */
#define RC522_DEFAULT_KEY                   \
{                                           \
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF           \
}

#ifdef __cplusplus
}
#endif

#endif /* RC522_REG_H */
