//
// Created by suitm on 2020/12/28.
//

#include <string.h>
#include "isstr.h"

char * isstr_version(){
    return ISSTR_VERSION_NO;
}

/** 去除字符串的前后空格 **/
char *isstr_trim(char *str)
{
    int i;
    int b1, e1;

    if (strlen(str) == 0) return str;

    if (str)
    {
        for(i = 0; str[i] == ' '; i++);

        b1 = i;

        for(i = strlen(str) - 1; i >= b1 && str[i] == ' '; i--);

        e1 = i;

        if (e1 >= b1)
            memmove(str, str+b1, e1-b1+1);

        str[e1-b1+1] = 0;
        return str;
    }
    else return str;
}
