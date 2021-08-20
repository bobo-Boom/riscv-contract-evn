#include <stdint.h>

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

uint32_t HAL_GetArgs(char *k, uint32_t klen, char *v, uint32_t *vlen);
uint32_t HAL_GetStateAt(char *k, uint32_t klen, char *v, uint32_t *vlen, uint32_t index);
uint32_t HAL_GetState(char *k, uint32_t klen, char *v, uint32_t *vlen);
uint32_t HAL_PutState(char *k, uint32_t klen, char *v, uint32_t vlen);
uint32_t HAL_SetErrCode(uint32_t err);
uint32_t HAL_SetStatusCode(uint32_t status);
uint32_t HAL_SetExecCount();
uint32_t HAL_SetResultInfo(char *info, uint32_t len);
uint32_t HAL_SetLogInfo(char *info, uint32_t len);


uint32_t HAL_GetArgs(char *k, uint32_t klen, char *v, uint32_t *vlen)
{
    if (klen <= 0 || klen > KEY_MAX_LEN) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    volatile uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));  // tx_id
    int find = 0;
    volatile uint32_t *tx_arg_klen;
    volatile uint32_t *tx_arg_vlen;
    volatile char *tx_arg_key;
    volatile char *tx_arg_value;
    for (int i = 0; i < mtd_param_count; i++) {
//        uint32_t *tx_arg_flag = (uint32_t *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // flag_len
        tx_arg_klen = (uint32_t *)addr_offset;
        if (*tx_arg_klen < 0 || *tx_arg_klen > KEY_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // klen_len
        tx_arg_vlen = (uint32_t *)addr_offset;
        if (*tx_arg_vlen < 0 || *tx_arg_vlen > VALUE_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // vlen_len
        tx_arg_key = (char *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + KEY_MAX_LEN);
        tx_arg_value = (char *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + VALUE_MAX_LEN);
        if (klen == *tx_arg_klen) {
            int flag = 0;
            for (int j = 0; j < klen; j++) {
                flag = *(unsigned char*)tx_arg_key - *(unsigned char*)k;
                if (flag) {
                    break;
                }
                tx_arg_key++;
                k++;
            }
            if (!flag) {
                *vlen = *tx_arg_vlen;
                for (int j = 0; j < *tx_arg_vlen; j++) {
                    *v = *tx_arg_value;
                    v++;
                    tx_arg_value++;
                }
                v -= *tx_arg_vlen;
                find = 1;
                break;
            }
        }
    }
    if (!find) {
        HAL_SetErrCode(PARAM_NOT_FOUND_ERROR);
        return PARAM_NOT_FOUND_ERROR;
    }
    return 0;
}


uint32_t HAL_GetStateAt(char *k, uint32_t klen, char *v, uint32_t *vlen, uint32_t index)
{
    if (klen <= 0 || klen > KEY_MAX_LEN || index < 0 ) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    volatile uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t)*3 + KEY_MAX_LEN + VALUE_MAX_LEN) * mtd_param_count);
    int find = 0;
    volatile uint32_t *tx_arg_flag;
    volatile uint32_t *tx_arg_klen;
    volatile uint32_t *tx_arg_vlen;
    volatile char *tx_arg_key;
    volatile char *tx_arg_value;
    volatile uint32_t *empty_offset = (uint32_t *) -1;
    for (int i = mtd_param_count; i < DATA_COUNT; i++) {
        tx_arg_flag = (uint32_t *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // flag_len
        tx_arg_klen = (uint32_t *)addr_offset;
        if (*tx_arg_klen < 0 || *tx_arg_klen > KEY_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        if (empty_offset == (uint32_t *)-1 && *tx_arg_klen == 0) {
            empty_offset = (uint32_t *)((uint64_t)addr_offset - sizeof(uint32_t));
        }
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));  // klen_len
        tx_arg_vlen = (uint32_t *)addr_offset;
        if (*tx_arg_vlen < 0 || *tx_arg_vlen > VALUE_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // vlen_len
        tx_arg_key = (char *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + KEY_MAX_LEN);
        tx_arg_value = (char *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + VALUE_MAX_LEN);
        if ((0xff & *tx_arg_flag) == READ_FLAG && klen == *tx_arg_klen) {
            int flag = 0;
            for (int j = 0; j < klen; j++) {
                flag = *(unsigned char*)tx_arg_key - *(unsigned char*)k;
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
        if (empty_offset == (uint32_t *)-1) {
            HAL_SetErrCode(OOM_ERROR);
            return OOM_ERROR;
        }
        tx_arg_flag = (uint32_t *)empty_offset;
        *(uint32_t *)empty_offset = READ_FLAG | *tx_arg_flag;
        empty_offset = (uint32_t  *)((uint64_t)empty_offset + sizeof(uint32_t));    // flag_len
        *(uint32_t *)empty_offset = klen;
        empty_offset = (uint32_t  *)((uint64_t)empty_offset + sizeof(uint32_t));    // klen_len
        empty_offset = (uint32_t  *)((uint64_t)empty_offset + sizeof(uint32_t));    // vlen_len
        for (int i = 0; i < klen; i++) {
            *(char *)empty_offset = *k;
            empty_offset = (uint32_t  *)((uint64_t)empty_offset + 1);
            k++;
        }
        return 0;
    }
    return 0;
}

uint32_t HAL_GetState(char *k, uint32_t klen, char *v, uint32_t *vlen)
{
    return HAL_GetStateAt(k, klen, v, vlen, 0);
}


uint32_t HAL_PutState(char *k, uint32_t klen, char *v, uint32_t vlen)
{
    if (klen <= 0 || klen > KEY_MAX_LEN || vlen <= 0 || vlen > VALUE_MAX_LEN) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    volatile uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) *3 + KEY_MAX_LEN + VALUE_MAX_LEN) * mtd_param_count);
    int find = 0;
    volatile uint32_t *tx_arg_flag;
    volatile uint32_t *tx_arg_klen;
//    uint32_t *tx_arg_vlen;
    volatile char *tx_arg_key;
//    char *tx_arg_value;
    volatile uint32_t *empty_offset = (uint32_t *) -1;
    for (int i = mtd_param_count; i < DATA_COUNT; i++) {
        tx_arg_flag = (uint32_t *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // flag_len
        tx_arg_klen = (uint32_t *)addr_offset;
        if (*tx_arg_klen < 0 || *tx_arg_klen > KEY_MAX_LEN) {
            HAL_SetErrCode(PARAM_LEN_ERROR);
            return PARAM_LEN_ERROR;
        }
        if (empty_offset == (uint32_t *)-1 && *tx_arg_klen == 0) {
            empty_offset = (uint32_t *)((uint64_t)addr_offset - sizeof(uint32_t));
        }

        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // klen_len
//        tx_arg_vlen = (uint32_t *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + sizeof(uint32_t));    // vlen_len
        tx_arg_key = (char *)addr_offset;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + KEY_MAX_LEN);
//        tx_arg_value = (char *)addr_offset;
        if (klen == *tx_arg_klen) {
            int flag = 0;
            for (int j = 0; j < klen; j++) {
                flag = *(unsigned char*)tx_arg_key - *(unsigned char*)k;
                if (flag) {
                    break;
                }
                tx_arg_key++;
                k++;
            }
            if (!flag) {
                *(uint32_t *)((uint64_t)addr_offset - KEY_MAX_LEN - sizeof(uint32_t)*3) = WRITE_FLAG | *tx_arg_flag;
                *(uint32_t *)((uint64_t)addr_offset - KEY_MAX_LEN - sizeof(uint32_t)) = vlen;
                for (int j = 0; j < vlen; j++) {
                    *(char *)addr_offset = *v;
                    addr_offset = (uint32_t *)((uint64_t)addr_offset + 1);
                    v++;
                }
                find = 1;
                break;
            }
        }
        addr_offset = (uint32_t *)((uint64_t)addr_offset + VALUE_MAX_LEN);
    }
    if (!find) {
        if (empty_offset == (uint32_t *)-1) {
            HAL_SetErrCode(OOM_ERROR);
            return OOM_ERROR;
        }
        tx_arg_flag = (uint32_t *)empty_offset;
        *(uint32_t *)empty_offset = WRITE_FLAG | *tx_arg_flag;
        empty_offset = (uint32_t *)((uint64_t)empty_offset + sizeof(uint32_t));    // flag_len
        *(uint32_t *)empty_offset = klen;
        empty_offset = (uint32_t *)((uint64_t)empty_offset + sizeof(uint32_t));    // klen_len
        *(uint32_t *)empty_offset = vlen;
        empty_offset = (uint32_t *)((uint64_t)empty_offset + sizeof(uint32_t));    // vlen_len
        for (int i = 0; i < klen; i++) {
            *(char *)empty_offset = *k;
            empty_offset = (uint32_t *)((uint64_t)empty_offset + 1);
            k++;
        }
        empty_offset = (uint32_t *)((uint64_t)empty_offset + KEY_MAX_LEN);
        for (int i = 0; i < vlen; i++) {
            *(char *)empty_offset = *v;
            empty_offset = (uint32_t *)((uint64_t)empty_offset + 1);
            v++;
        }
        return 0;
    }
    return 0;
}



uint32_t HAL_SetErrCode(uint32_t err)
{
    uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) *3 + KEY_MAX_LEN + VALUE_MAX_LEN) * DATA_COUNT + sizeof(uint64_t) + sizeof(uint32_t));
    *(uint32_t *)addr_offset = err;
    return 0;
}


uint32_t HAL_SetStatusCode(uint32_t status) {
    uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) *3 + KEY_MAX_LEN + VALUE_MAX_LEN) * DATA_COUNT
                               + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t));
    *(uint32_t *)addr_offset = status;
    return 0;
}


uint32_t HAL_SetExecCount() {
    uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) *3 + KEY_MAX_LEN + VALUE_MAX_LEN) * DATA_COUNT
                               + sizeof(uint64_t));
    (*(uint32_t *)addr_offset)++;
    return 0;
}


uint32_t HAL_SetResultInfo(char *info, uint32_t len) {
    if (len > EXEC_RESULT_MAX_LEN - sizeof(uint32_t)) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) *3 + KEY_MAX_LEN + VALUE_MAX_LEN) * DATA_COUNT
                   + sizeof(uint64_t) + sizeof(uint32_t)*3);
    *(uint32_t *)addr_offset = len;
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t)));
    for (int i = 0; i < len; i++) {
        *(char *)addr_offset = *info;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + 1);
        info++;
    }
    return 0;
}


uint32_t HAL_SetLogInfo(char *info, uint32_t len) {
    if (len <= 0 || len > EXEC_LOG_MAX_LEN ) {
        HAL_SetErrCode(PARAM_LEN_ERROR);
        return PARAM_LEN_ERROR;
    }
    uint32_t *addr_offset = (uint32_t *) (ADDR_HEADER + sizeof(uint32_t));
    addr_offset = (uint32_t *)((uint64_t)addr_offset + (sizeof(uint32_t) * 3 + KEY_MAX_LEN + VALUE_MAX_LEN) * DATA_COUNT + sizeof(uint64_t)
                               + sizeof(uint32_t) * 3 + EXEC_RESULT_MAX_LEN);
    for (int i = 0; i < len; i++) {
        *(char *)addr_offset = *info;
        addr_offset = (uint32_t *)((uint64_t)addr_offset + 1);
        info++;
    }
    return 0;
}