/*
 * log.c
 *
 *  Created on: Jul 13, 2026
 *      Author: lqdung
 */

#include "log.h"

void Log_save_record(char *id, char *name, char *time, char *alcohol){
	if(LOG_LEN < 3) return;

	Log_t record;

	memcpy(record.alcohol, alcohol, sizeof(record.alcohol));
	memcpy(record.id, id, sizeof(record.id));
	memcpy(record.name, name, sizeof(record.name));
	memcpy(record.time, time, sizeof(record.time));

	if(log_counter < LOG_LEN){
		app_log[log_counter++] = record;
	}else{
		app_log[0] = app_log[LOG_LEN-2];
		app_log[1] = app_log[LOG_LEN-1];
		app_log[2] = record;
		log_counter = 3;
	}
}

Log_t *Log_get_three_last_record(){
	return app_log;
}

