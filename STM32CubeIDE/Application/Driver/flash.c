/*
 * flash.c
 *
 *  Created on: Jul 11, 2026
 *      Author: lqdung
 */

#include "flash.h"
#include "stm32f4xx_hal.h"

#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;

static UserDB g_db;

static void EraseSector23(void)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t error;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_23;
    erase.NbSectors = 1;

    HAL_FLASHEx_Erase(&erase, &error);
}

void DB_Load(void)
{
    memcpy(&g_db, (void *)USER_DB_ADDR, sizeof(UserDB));

    if (g_db.magic != DB_MAGIC)
    {
        memset(&g_db, 0, sizeof(UserDB));

        g_db.magic = DB_MAGIC;
        g_db.version = DB_VERSION;

        DB_Save();
    }
}

void DB_Save(void)
{
    HAL_FLASH_Unlock();

    EraseSector23();

    uint32_t *src = (uint32_t *)&g_db;
    uint32_t addr = USER_DB_ADDR;

    for (uint32_t i = 0; i < sizeof(UserDB) / 4; i++)
    {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                addr,
                src[i]) != HAL_OK)
        {
            break;
        }

        addr += 4;
    }

    HAL_FLASH_Lock();
}


bool DB_AddUser(const uint8_t cardID[5], const char *name)
{
    if (g_db.count >= MAX_USER)
        return false;

    if (DB_Find(cardID) != NULL)
        return false;

    User *u = &g_db.users[g_db.count];

    memcpy(u->cardID, cardID, 5);

    strncpy(u->name, name, MAX_NAME_LEN - 1);
    u->name[MAX_NAME_LEN - 1] = '\0';

    g_db.count++;

    return true;
}

User *DB_Find(const uint8_t cardID[5])
{
    for (uint32_t i = 0; i < g_db.count; i++)
    {
        if (memcmp(g_db.users[i].cardID, cardID, 5) == 0)
        {
            return &g_db.users[i];
        }
    }

    return NULL;
}

bool DB_DeleteUser(const uint8_t cardID[5])
{
    for (uint32_t i = 0; i < g_db.count; i++)
    {
        if (memcmp(g_db.users[i].cardID, cardID, 5) == 0)
        {
            memmove(&g_db.users[i],
                    &g_db.users[i + 1],
                    (g_db.count - i - 1) * sizeof(User));

            g_db.count--;

            return true;
        }
    }

    return false;
}

UserDB *DB_Get(void)
{
    return &g_db;
}

void DB_InitTest(void)
{
    memset(&g_db, 0, sizeof(g_db));
    g_db.magic = DB_MAGIC;
    g_db.version = DB_VERSION;
    g_db.count = 3;

    memcpy(g_db.users[0].cardID,
           (uint8_t[]){0xE7, 0x28, 0x69, 0x06, 0xA0},
           5);
    strcpy(g_db.users[0].name, "Le Quang Dung");

    memcpy(g_db.users[1].cardID,
           (uint8_t[]){0x56, 0xE5, 0xF2, 0x06, 0x47},
           5);
    strcpy(g_db.users[1].name, "User 2");

    memcpy(g_db.users[2].cardID,
           (uint8_t[]){0xF3, 0x5D, 0x34, 0x14, 0x8E},
           5);
    strcpy(g_db.users[2].name, "User 3");

    DB_Save();
}

void DB_Print(void)
{
    char buf[128];

    int len = snprintf(buf, sizeof(buf),
                       "\r\n===== USER DATABASE =====\r\n"
                       "Magic   : 0x%08lX\r\n"
                       "Version : %lu\r\n"
                       "Count   : %lu\r\n\r\n",
                       g_db.magic,
                       g_db.version,
                       g_db.count);

    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);

    for (uint32_t i = 0; i < g_db.count; i++)
    {
        len = snprintf(buf, sizeof(buf),
                       "[%lu] %-16s  UID: %02X %02X %02X %02X %02X\r\n",
                       i,
                       g_db.users[i].name,
                       g_db.users[i].cardID[0],
                       g_db.users[i].cardID[1],
                       g_db.users[i].cardID[2],
                       g_db.users[i].cardID[3],
                       g_db.users[i].cardID[4]);

        HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    }

    HAL_UART_Transmit(&huart1,
                      (uint8_t *)"=========================\r\n",
                      27,
                      HAL_MAX_DELAY);
}
