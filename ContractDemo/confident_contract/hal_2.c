//
// Created by Boom on 2021/9/1.
//

#include <stdint.h>
#include "hal.h"

const int DATA_COUNT = 256;
const int KEY_MAX_LEN = 512;
const int VALUE_MAX_LEN = 512;
const int EXEC_RESULT_MAX_LEN = 1024;
const int EXEC_LOG_MAX_LEN = 200;

uint64_t ADDR_HEADER;
uint32_t mtd_param_count;

const uint32_t READ_FLAG = 0x40;
const uint32_t WRITE_FLAG = 0x10;

const uint32_t OOM_ERROR = 1001;
const uint32_t PARAM_LEN_ERROR = 3001;
const uint32_t PARAM_NOT_FOUND_ERROR = 3002;

const uint32_t TREE_IS_NULL_ERROR = 4001;
const uint32_t TREE_KEY_TOO_LONG_ERROR = 4002;


char *get_key(int tree_id, long page_index, char *key, uint32_t *key_len);

struct bplus_tree *HAL_GetTree(char *tree_boot_addr) {
    return get_tree(tree_boot_addr);
}

uint32_t HAL_GetPageIndex(bplus_tree *tree, char *arg_key, uint32_t arg_key_len, long *page_Index) {

    if (arg_key_len <= 0 || arg_key_len > KEY_MAX_LEN) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    if tree == NULL
    {
        HAL_SetErrCode(TREE_IS_NULL_ERROR);
        return TREE_IS_NULL_ERROR;
    }


    offt_t tree_type = tree->key_type;

    switch (tree_type) {
        case INT_TREE_TYPE:
            if (arg_key_len > sizeof(key_t)) {
                HAL_SetErrCode(TREE_KEY_TOO_LONG_ERROR);
                return TREE_KEY_TOO_LONG_ERROR;
            }
            *page_index = bplus_tree_get(tree, *((key_t *) arg_key));
        case STRING_TREE_TYPE:
            if (arg_key_len > sizeof(key_t_arr)) {
                HAL_SetErrCode(TREE_KEY_TOO_LONG_ERROR);
                return TREE_KEY_TOO_LONG_ERROR;
            }
            *page_Index = bplus_tree_get_str(tree, *((key_t_arr *) arg_key));
    }
    return 0;

}

uint32_t HAL_GetStateAt_v2(int tree_id, long page_index, char *v, uint32_t *vlen, uint32_t index) {


    volatile uint32_t *addr_offset = (uint32_t * )(ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t * )(
            (uint64_t) addr_offset + (sizeof(uint32_t) * 3 + KEY_MAX_LEN + VALUE_MAX_LEN) * mtd_param_count);
    int find = 0;
    volatile uint32_t *tx_arg_flag;
    volatile uint32_t *tx_arg_klen;
    volatile uint32_t *tx_arg_vlen;
    volatile char *tx_arg_key;
    volatile char *tx_arg_value;
    volatile uint32_t *empty_offset = (uint32_t * ) - 1;

    //get key
    char k[32] = {0};
    uint32_t klen = 0;
    get_key(tree_id, page_index, k, &klen);

    for (int i = mtd_param_count; i < DATA_COUNT; i++) {
        tx_arg_flag = (uint32_t *) addr_offset;
        addr_offset = (uint32_t * )((uint64_t) addr_offset + sizeof(uint32_t));    // flag_len
        tx_arg_klen = (uint32_t *) addr_offset;
        if (*tx_arg_klen < 0 || *tx_arg_klen > KEY_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        if (empty_offset == (uint32_t * ) - 1 && *tx_arg_klen == 0) {
            empty_offset = (uint32_t * )((uint64_t) addr_offset - sizeof(uint32_t));
        }
        addr_offset = (uint32_t * )((uint64_t) addr_offset + sizeof(uint32_t));  // klen_len
        tx_arg_vlen = (uint32_t *) addr_offset;
        if (*tx_arg_vlen < 0 || *tx_arg_vlen > VALUE_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        addr_offset = (uint32_t * )((uint64_t) addr_offset + sizeof(uint32_t));    // vlen_len
        tx_arg_key = (char *) addr_offset;
        addr_offset = (uint32_t * )((uint64_t) addr_offset + KEY_MAX_LEN);
        tx_arg_value = (char *) addr_offset;
        addr_offset = (uint32_t * )((uint64_t) addr_offset + VALUE_MAX_LEN);
        if ((0xff & *tx_arg_flag) == READ_FLAG && klen == *tx_arg_klen) {  //find key
            int flag = 0;
            for (int j = 0; j < klen; j++) {
                flag = *(unsigned char *) tx_arg_key - *(unsigned char *) k;
                if (flag) {
                    break;
                }
                tx_arg_key++;
                k++;
            }
            if (!flag && find++ == index) {
                *vlen = *tx_arg_vlen;
                for (int j = 0; j < *tx_arg_vlen; j++) {
                    *v = *tx_arg_value;
                    v++;
                    tx_arg_value++;
                }
                v -= *tx_arg_vlen;
                break;
            }
        }
    }
    if (!find) {
        if (empty_offset == (uint32_t * ) - 1) {
            HAL_SetErrCode(OOM_ERROR);
            return OOM_ERROR;
        }
        tx_arg_flag = (uint32_t *) empty_offset;
        *(uint32_t *) empty_offset = READ_FLAG | *tx_arg_flag;
        empty_offset = (uint32_t * )((uint64_t) empty_offset + sizeof(uint32_t));    // flag_len
        *(uint32_t *) empty_offset = klen;
        empty_offset = (uint32_t * )((uint64_t) empty_offset + sizeof(uint32_t));    // klen_len
        empty_offset = (uint32_t * )((uint64_t) empty_offset + sizeof(uint32_t));    // vlen_len
        for (int i = 0; i < klen; i++) {
            *(char *) empty_offset = *k;
            empty_offset = (uint32_t * )((uint64_t) empty_offset + 1);
            k++;
        }
        return 0;
    }
    return 0;
}


uint32_t HAL_GetState_v2(off_t tree_id, long page_index, char *v, uint32_t *vlen) {


    return HAL_GetStateAt_v2(tree_id, page_index, v, vlen, 0);
}

char *get_key(int tree_id, long page_index, char *key, uint32_t *key_len) {
    ltoa(tree_id, key, 10);
    //get length
    int i = 0;
    while (key[i] != '\0') {
        i++;
    }
    //put '_' in key
    key[i++] = '_';

    //cat page_index to key
    int k = 0;
    char temp[16] = {0};
    ltoa(page_index, temp, 10);
    while (temp[k] != '\0') {
        key[i] = temp[k];
        i++;
        k++;
    }
    *key_len = i;
    return key;
}

