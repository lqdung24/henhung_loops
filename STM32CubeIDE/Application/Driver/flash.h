/*
 * flash.h
 *
 *  Created on: Jul 11, 2026
 *      Author: lqdung
 */

#ifndef FLASH_H
#define FLASH_H

#include <stdbool.h>
#include <stdint.h>


#define USER_DB_ADDR   0x081E0000U

#define DB_MAGIC       0x12345678U
#define DB_VERSION     1U

#define MAX_NAME_LEN   32
#define MAX_USER       100

typedef struct
{
    uint8_t cardID[5];
    char name[MAX_NAME_LEN];

} User;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t count;

    User users[MAX_USER];

} UserDB;

/* Init */
void DB_Load(void);
void DB_Save(void);

/* User */
bool DB_AddUser(const uint8_t cardID[5], const char *name);
bool DB_DeleteUser(const uint8_t cardID[5]);
User *DB_Find(const uint8_t cardID[5]);

// debug
void DB_InitTest(void);
void DB_Print(void);

/* Access */
UserDB *DB_Get(void);

#endif
