//
// Created by suitm on 2021/1/5.
//

#include "isatom.h"

char * isatom_version(){
    return ISATOM_VERSION_NO;
}

/***
ԭ�Ӷ�
type __atomic_load_n(type *ptr,int memmodel)

ԭ��д
void __atomic_store_n (type *ptr, type val, int memmodel)

ԭ�ӽ���
type __atomic_exchange_n (type *ptr, type val, int memmodel)

ԭ��CAS
bool __atomic_compare_exchange_n (type *ptr, type *expected, type desired, bool weak, int success_memmodel, int failure_memmodel)

ԭ�Ӽ�/��/��/��/���
type __atomic_op_fetch (type *ptr, type val, int memorder)
type __atomic_fetch_op (type *ptr, type val, int memorder)
add/sub/and/xor/or/nand

ԭ��TAS
bool __atomic_test_and_set (void *ptr, int memorder)

ԭ��Clear
void __atomic_clear (bool *ptr, int memorder)
 */
