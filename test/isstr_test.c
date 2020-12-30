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
    printf("orig str = [%s]\n", isodate);
    printf("isstr_trim = [%s]\n", isstr_trim(isodate));

    return 0;
}

