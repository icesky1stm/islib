//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../src/isipc.h"

int main(){
    printf("version = [%s]\n", isipc_version());

    return 0;
}