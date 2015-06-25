#ifndef __KEYMAP_H__
#define __KEYMAP_H__
#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include "at_common.h"
#include "autoTest.h"
#include "at_network.h"
#include "keyMap.h"
#define KEY_PRESS         "p"
#define KEY_LONG_PRESS    "l"

#define MAX_KEY_NUM       255
#define MIN_KEY_NUM       0
#define MAX_KEY_NAME      10
#define MAX_ACT_LENTH     5
#define MAX_KEY_CODE      10
#define MAX_KEY_LENTH     5

typedef struct at_key_t{
    char keyName[MAX_KEY_NAME];
    char keyCode[MAX_KEY_LENTH];
}at_key_t;
int32_t getKeyCode(const at_key_t *keyLib, const c8_t *str);
#endif
