/*
 * log.h
 *
 *  Created on: Jul 13, 2026
 *      Author: lqdung
 */

#ifndef LOG_H
#define LOG_H


#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"
#include <string.h>

extern Log_t app_log[LOG_LEN];
extern int log_counter;

void Log_save_record(char *id, char *name, char *time, char *alcohol);


#ifdef __cplusplus
}
#endif
#endif
