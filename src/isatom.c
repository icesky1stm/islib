//
// Created by suitm on 2021/1/5.
//

#include "isatom.h"

char * isatom_version(){
    return ISATOM_VERSION_NO;
}

/***
原子读
type __atomic_load_n(type *ptr,int memmodel)

原子写
void __atomic_store_n (type *ptr, type val, int memmodel)

原子交换
type __atomic_exchange_n (type *ptr, type val, int memmodel)

原子CAS
bool __atomic_compare_exchange_n (type *ptr, type *expected, type desired, bool weak, int success_memmodel, int failure_memmodel)

原子加/减/与/或/异或
type __atomic_op_fetch (type *ptr, type val, int memorder)
type __atomic_fetch_op (type *ptr, type val, int memorder)
add/sub/and/xor/or/nand

原子TAS
bool __atomic_test_and_set (void *ptr, int memorder)

原子Clear
void __atomic_clear (bool *ptr, int memorder)
 */
