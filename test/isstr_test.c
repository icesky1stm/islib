//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/isstr.h"

int main(){
    printf("version = [%s]\n", isstr_version());

    char isodate[20] = "   aa bb ";
    printf("isstr_trim = [%s] -> [%s]\n", isodate, isstr_trim(isodate));

    char line1[] = "a1111,bbbb,cccc,dddddddd,eee,fff,";
    char line2[] = "a1111,bbbb,cccc,dddddddd,eee,fff";
    char line3[] = "a11##bbb##c##dddddd##eeeew##fffffff";
    char value[20];
    printf("isstr_splite = [%s]\n", isstr_split(line1,",", 1, value));
    printf("isstr_splite = [%s]\n", isstr_split(line1,",", 3, value));
    printf("isstr_splite = [%s]\n", isstr_split(line1,",", 6, value));
    printf("isstr_splite = [%s]\n", isstr_split(line2,",", 6, value));
    printf("isstr_splite = [%s]\n", isstr_split(line3,"##", 1, value));
    printf("isstr_splite = [%s]\n", isstr_split(line3,"##", 3, value));
    printf("isstr_splite = [%s]\n", isstr_split(line3,"##", 6, value));
    printf("汉字的长度 = [%lu]\n", strlen("汉字"));

    return 0;
}

