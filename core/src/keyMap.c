#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyMap.h"

at_key_t gKeyLib[MAX_KEY_NUM]={
    [34] = {"s1","251"},    /*SOFT_KEY1*/
    [4] =  {"s2","252"},    /*SOFT_KEY2*/
    [44] = {"s3","253"},    /*SOFT_KEY3*/
    [24] = {"s4","254"},    /*SOFT_KEY4*/
    [14] = {"up","31"},     /*KEY_ARROW_UP*/
    [54] = {"dn","32"},     /*KEY_ARROW_DOWN*/
    [3]  = {"1","49"},      /*1*/
    [2]  = {"2","50"},      /*2*/
    [41] = {"3","51"},      /*3*/
    [33] = {"4","52"},      /*4*/
    [32] = {"5","53"},      /*5*/
    [31] = {"6","54"},      /*6*/
    [13] = {"7","55"},      /*7*/
    [12] = {"8","56"},      /*8*/
    [1]  = {"9","57"},      /*9*/
    [52] = {"0","48"},      /*0*/
    [53] = {"*","10"},      /* '*' */
    [51] = {"#","11"},      /* '#' */
    [22] = {"vd","47"},     /*KEY_ALL_VOL_DOWN*/
    [42] = {"me","64"},     /*KEY_MUTE*/
    [43] = {"vu","46"},     /*KEY_ALL_VOL_UP*/
    [11] = {"rl","63"},     /*KEY_REDIAL*/
    [25] = {"he","61"},     /*KEY_HANDFREE*/
    [15] = {"le1","151"},   /*LINE1*/
    [5]  = {"le2","152"},    /*LINE2*/ 
    [100] = {"offhook","100"},/*OFF HOOK*/
    [101] = {"onhook","101"}  /*ON HOOK*/
};
int32_t getKeyCode(const at_key_t *keyLib, const c8_t *str) 
{
	int32_t index;
	for (index = 0; index < MAX_KEY_NUM; index++) {
		if (strcmp(keyLib[index].keyName, str) == 0 || \
		    strcmp(keyLib[index].keyCode, str) == 0) {
			return index;
		}
	}
	return FAILED;
}

