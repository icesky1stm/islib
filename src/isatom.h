//
// Created by suitm on 2021/1/5.
//

#ifndef ISLIB_ISATOM_H
#define ISLIB_ISATOM_H

#define ISATOM_VERSION_NO "21.01.1"

char * isatom_version();

#define isatom_incr(var,count) do { \
    __atomic_add_fetch(&var,(count),__ATOMIC_RELAXED); \
} while(0)

#define isatom_decr(var,count) do { \
    __atomic_sub_fetch(&var,(count),__ATOMIC_RELAXED); \
} while(0)

#define isatom_getincr(var,oldvalue_var,count) do { \
    oldvalue_var = __atomic_fetch_add(&var,(count),__ATOMIC_RELAXED); \
} while(0)

#define isatom_get(var,dstvar) do { \
    dstvar = __atomic_load_n(&var,__ATOMIC_RELAXED); \
} while(0)
#define isatom_set(var,value) do { \
    __atomic_store_n(&var,value,__ATOMIC_RELAXED); \
} while(0)


#endif //ISLIB_ISATOM_H
