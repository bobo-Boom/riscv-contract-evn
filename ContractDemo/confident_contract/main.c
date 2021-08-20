#include "hal.h"
extern uint64_t ADDR_HEADER;
extern uint64_t mtd_param_count;
extern const funcp flist[4];


void main(uint64_t txdata_header, uint32_t mtd_index, uint32_t mpc)
{
    funcp contract_method;

    ADDR_HEADER = txdata_header;
    mtd_param_count = mpc;
    contract_method = flist[mtd_index];
    contract_method();


}