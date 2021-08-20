//
// Created by 35437 on 2021/7/7.
//

#ifndef HAL_HAL_H
#define HAL_HAL_H

#include <stdint.h>
typedef void (*funcp)(void);

uint32_t HAL_GetArgs(char* k, uint32_t klen, char* v, uint32_t* vlen);
uint32_t HAL_GetStateAt(char* k, uint32_t klen, char* v, uint32_t* vlen, uint32_t index);
uint32_t HAL_GetState(char* k, uint32_t klen, char* v, uint32_t* vlen);
uint32_t HAL_PutState(char* k, uint32_t klen, char* v, uint32_t vlen);
uint32_t HAL_SetErrCode(uint32_t err);
uint32_t HAL_SetStatusCode(uint32_t status);
uint32_t HAL_SetExecCount();
uint32_t HAL_SetResultInfo(char* info, uint32_t len);
uint32_t HAL_SetLogInfo(char* info, uint32_t len);

#endif //HAL_HAL_H
