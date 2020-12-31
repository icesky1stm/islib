//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/islog.h"

int main(){
    printf("version = [%s]\n", islog_version());

    char isodate[20] = "   aa bb ";

    islog_debug("DEBUG--[%s]\n", isodate);
    islog_info("INFO--[%s]\n", isodate);
    islog_error("ERROR--[%s]\n", isodate);
    islog_warn("WARN--[%s]\n", isodate);

    return 0;
}
