/*
 * app_config.h
 *
 *  Created on: Jul 12, 2026
 *      Author: lqdung
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define Max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_NAME_LEN    32

#define MQ3_VC       5.0f
#define ADC_SCALE 1.5
#define ADC_BUF_LEN      5
#define HALF_BUF_LEN     (ADC_BUF_LEN / 2)
#define LOG_LEN 10

#include "inttypes.h"

typedef struct {
    char id[15];
    char name[32];
    char time[24];
} RFID_Packet_t;

typedef struct {
	char id[15];
	char name[32];
	char time[24];
	char alcohol[20];
} Log_t;


typedef struct {
	uint16_t raw_voltage_x1000;  //  v x 1000
	uint16_t alcohol_x100; // mg/100ml x 100
} MQ3_result_t;

typedef enum {
    STATE_IDLE,              // 1. Chờ bấm nút kích hoạt
    STATE_WAIT_RFID,         // 2. Đang chờ quẹt thẻ RC522
    STATE_MEASURING,         // 3. Đang trong 5 giây thổi cồn
    STATE_SHOW_RESULT        // 4. Đã có kết quả, hiển thị lên màn hình
} SensorState_t;

enum {
    CMD_NONE = 0,
    CMD_GOTO_HISTORY,
    CMD_GOTO_MAIN,
    CMD_START_MEASURE_OK,
    CMD_WARN_ALCOHOL_HIGH,
	CMD_PROMPT_SCAN_CARD,
	CMD_MEASURE_DONE,
	CMD_SHOW_HIGH_ALCOHOL_DIALOG,
	CMD_OFF_HIGH_ALCOHOL_DIALOG
};


typedef struct
{
	uint8_t cmd;
    const char **texts;
    uint8_t count;
} UI_cmd_t;

typedef enum {
    BTN_NONE = 0,
    BTN_SHORT_PRESS,
    BTN_LONG_PRESS,
	BTN_LONG_LONG_PRESS
} ButtonEvent_t;

typedef enum {
    SCREEN_MAIN = 0,
    SCREEN_HISTORY
} ScreenState_t;

typedef enum {
	MQ3_CMD_MEASURE,
	MQ3_CMD_WAIT_READY
} MQ3_Command;

#ifdef __cplusplus
}
#endif
#endif


