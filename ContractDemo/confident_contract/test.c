#include "hal.h"

const uint32_t STATUS_CODE_SUCCESS = 0;
const uint32_t STATUS_CODE_FAILURE = 1;

const uint64_t MONEY_UNIT_Y = 10000000000;
const uint64_t MONEY_UNIT_W = 1000000;
// 东城区、西城区、朝阳区、丰台区、石景山区、海淀区、顺义区、通州区、大兴区、房山区、门头沟区、昌平区、平谷区、密云区、怀柔区、延庆区
const char BEIJING_DISTRICT[16][14] = {"dongchengqu", "xichengqu", "chaoyangqu", "fengtaiqu", "shijingshanqu", "haidianqu",
                               "shunyiqu", "tongzhouqu", "daxingqu", "fangshanqu", "mentougouqu", "changpingqu",
                               "pingguqu", "miyunqu", "huairouqu", "yanqingqu"};
const uint64_t BEIJING_DISTRICT_PRICE[16] = {1000000, 1000000, 1000000, 1000000,
                                             2000000, 2000000, 2000000, 2000000,
                                             3000000, 3000000, 3000000, 3000000,
                                             4000000, 4000000, 4000000, 4000000};


void num2str(uint64_t num, char *out, uint32_t *out_len)
{
    char nums[] = "0123456789";
    int i=0, j, k = 0;
    char swap_temp;
    if (num < 0) {
        out[i++] = '-';
        k = 1;
    }
    do {
        out[i++] = nums[num % 10];
        num /= 10;

    } while(num);
    *out_len = i;
    for(j= k; j<= (i-1)/2; j++) {
        swap_temp = out[j];
        out[j] = out[i - 1 + k - j];
        out[i-1+k-j] = swap_temp;
    }
}

void assemble_sql(const char *table, uint32_t table_len, const char *cond_col, uint32_t cond_col_len, const char *cond_val,
               uint32_t cond_val_len, char *sel_col, uint32_t sel_col_len, char *sql, uint32_t *sql_len)
{
    for (int i = 0; i < table_len; i++) {
        sql[i] = table[i];
    }
    sql[table_len] = ':';
    sql[table_len + 1] = ':';
    for (int i = 0; i < cond_col_len; i++) {
        sql[table_len + 2 + i] = cond_col[i];
    }
    sql[table_len + 2 + cond_col_len] = '=';
    for (int i = 0; i < cond_val_len; i++) {
        sql[table_len + 2 + cond_col_len + 1 + i] = cond_val[i];
    }
    sql[table_len + 2 + cond_col_len + 1 + cond_val_len] = ':';
    sql[table_len + 2 + cond_col_len + 1 + cond_val_len + 1] = '$';
    sql[table_len + 2 + cond_col_len + 1 + cond_val_len + 2] = ':';
    for (int i = 0; i < sel_col_len; i++) {
        sql[table_len + 2 + cond_col_len + 1 + cond_val_len + 3 + i] = sel_col[i];
    }
    *sql_len = table_len + 2 + cond_col_len + 1 + cond_val_len + 3 + sel_col_len;
}

uint32_t is_exception(char *certno, uint32_t certno_len)
{
    char isvalid[1];
    char is_dissent[1];
    char is_restrict[1];
    uint32_t len;
    char sql[512];
    uint32_t sql_len;

    assemble_sql("estate", 6, "certno", 6, certno, certno_len, "isvalid", 7, sql, &sql_len);
    uint32_t ret = HAL_GetState(sql, sql_len, isvalid, &len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get isvalid fail", 16);
        return ret;
    }
    assemble_sql("estate", 6, "certno", 6, certno, certno_len, "is_dissent", 10, sql, &sql_len);
    ret = HAL_GetState(sql, sql_len, is_dissent, &len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get is_dissent fail", 16);
        return ret;
    }
    assemble_sql("estate", 6, "certno", 6, certno, certno_len, "is_restrict", 11, sql, &sql_len);
    ret = HAL_GetState(sql, sql_len, is_restrict, &len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get is_restrict fail", 16);
        return ret;
    }
    return isvalid[0] == '0' || is_dissent[0] == '1' || is_restrict[0] == '1';
}

int BAEC_strcmp(const char *s1, const char *s2, uint32_t len)
{
   volatile char c1, c2;

    for (int i = 0; i < len; ++i) {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 != c2) {
            return (c1 < c2) ? c1 : c2;
        }
        return 0;
    }

    return 0;
}

uint64_t get_price(char *district, uint32_t len)
{
    for (int i = 0; i < 16; i++) {
        if (!BAEC_strcmp(BEIJING_DISTRICT[i], district, len))
            return BEIJING_DISTRICT_PRICE[i];
    }
    return 0;
}

uint32_t is_mortagage(char *certno, uint32_t certno_len)
{
    char is_mortgage[1];
    char is_mortforecast[1];
    uint32_t len;
    char sql[512];
    uint32_t sql_len;
    assemble_sql("estate", 6, "certno", 6, certno, certno_len, "is_mortgage", 11, sql, &sql_len);
    uint32_t ret = HAL_GetState(sql, sql_len, is_mortgage, &len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get is_mortgage fail", 20);
        return ret;
    }
    assemble_sql("estate", 6, "certno", 6, certno, certno_len, "is_mortforecast", 15, sql, &sql_len);
    ret = HAL_GetState(sql, sql_len, is_mortforecast, &len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get is_mortforecast fail", 24);
        return ret;
    }
    return is_mortgage[0] == '1' || is_mortforecast[0] == '1';
}

uint32_t classify(uint64_t totle_asset_val)
{
    // 房产总价值分档：500万以下以50万为单位，分成10档,；500万-2000万以100万为单位，分成15档；2000万-1亿以500万为单位，分成16档；>1亿为1档。
    uint64_t grade_line = 50 * MONEY_UNIT_W;
    if (totle_asset_val <= 500 * MONEY_UNIT_W) {
        for (int i = 1; i < 10; i++) {
            if (totle_asset_val <= grade_line) {
                return i;
            }
            grade_line += 50 * MONEY_UNIT_W;
        }
    }
    grade_line = 500 * MONEY_UNIT_W;
    if (totle_asset_val <= 2000 * MONEY_UNIT_W) {
        for (int i = 10; i < 25; i++) {
            if (totle_asset_val <= grade_line) {
                return i;
            }
            grade_line += 100 * MONEY_UNIT_W;
        }
    }
    grade_line = 2000 * MONEY_UNIT_W;
    if (totle_asset_val <= 2000 * MONEY_UNIT_W) {
        for (int i = 25; i < 41; i++) {
            if (totle_asset_val <= grade_line) {
                return i;
            }
            grade_line += 500 * MONEY_UNIT_W;
        }
    }
    return totle_asset_val <= MONEY_UNIT_Y ? 41 : 42;
}

void f0(void)
{
    char qlrzjh[300];
    int vlen;
    char count_result[300];
    int stlen;
    char count_sql[512];
    uint32_t * addr_debug = (uint32_t *)0x140000000;
    *addr_debug = 0x55aa55aa;
    uint32_t ret = HAL_GetArgs("qlrzjh", 6, qlrzjh, &vlen);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get arg fail", 12);
        return;
    }
    *(addr_debug + 1) = 0x66bb66bb;
    for(int i = 0; i < 15; i++)
    {
        count_sql[i] = "estate::qlrzjh="[i];
    }
    for (int i = 15; i < 15 + vlen; i++) {
        count_sql[i] = qlrzjh[i - 15];
    }
    for (int i = 15 + vlen; i < 15 + vlen + 11; i++) {
        count_sql[i] = ":$:count(*)"[i - 15 - vlen];
    }
    ret = HAL_GetState(count_sql, 15 + vlen + 11, count_result, &stlen);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get count_result fail", 21);
        return;
    }
    *(addr_debug + 2) = 0x77cc77cc;
    HAL_SetStatusCode(STATUS_CODE_SUCCESS);
    HAL_SetResultInfo(count_result, stlen);
    *(addr_debug + 3) = 0x88dd88dd;
}

void f1(void)
{
	volatile char field[300];
	volatile uint32_t field_len;
	volatile char sql[512];
	volatile uint32_t sql_len;
	volatile char count_result[4];
	volatile uint32_t count_result_len;
    int count = 0;
    char certno_var[10] = "$$certno_";
    uint32_t certno_var_len = 9;
    volatile uint64_t estate_price;
    volatile uint64_t estate_price_sum1 = 0;
    volatile uint64_t estate_price_sum2 = 0;
    volatile char estate_estimate_1[2];
    volatile uint32_t estate_estimate_1_len;
    volatile char estate_estimate_2[2];
    volatile uint32_t estate_estimate_2_len;
    volatile char index[4];
    volatile uint32_t index_length;
    volatile uint32_t area = 0;
    volatile char result_info[1024];

    uint32_t ret = HAL_GetArgs("qlrzjh", 6, field, &field_len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get arg fail", 12);
        return;
    }
    assemble_sql("estate", 6, "qlrzjh", 6, field, field_len, "count(*)", 8, sql, &sql_len);
    ret = HAL_GetState(sql, sql_len, count_result, &count_result_len);
    if (ret != 0) {
        HAL_SetStatusCode(STATUS_CODE_FAILURE);
        HAL_SetResultInfo("get count_result fail", 21);
        return;
    }
    for (int i = 0; i < count_result_len; i++) {
        count = count * 10 + count_result[i] - '0';
    }
    for (int i = 0; i < count; i++) {
        assemble_sql("estate", 6, "qlrzjh", 6, field, field_len, "certno", 6, sql, &sql_len);
        ret = HAL_GetStateAt(sql, sql_len, field, &field_len, i);
        if (ret != 0) {
            HAL_SetStatusCode(STATUS_CODE_FAILURE);
            HAL_SetResultInfo("get certno fail", 15);
            return;
        }
        num2str(i, index, &index_length);
        for (int j = 0; j < index_length; j++) {
            certno_var[certno_var_len++] = index[j];
        }
        if (is_exception(certno_var, certno_var_len)) {
            continue;
        }
        assemble_sql("estate", 6, "certno", 6, certno_var, certno_var_len, "zl", 2, sql, &sql_len);
        ret = HAL_GetState(sql, sql_len, field, &field_len);
        if (ret != 0) {
            HAL_SetStatusCode(STATUS_CODE_FAILURE);
            HAL_SetResultInfo("get district fail", 17);
            return;
        }
        estate_price = get_price(field, field_len);
        assemble_sql("estate", 6, "certno", 6, certno_var, certno_var_len, "jzmj", 4, sql, &sql_len);
        ret = HAL_GetState(sql, sql_len, field, &field_len);
        if (ret != 0) {
            HAL_SetStatusCode(STATUS_CODE_FAILURE);
            HAL_SetResultInfo("get area_result fail", 20);
            return;
        }
        for (int j = 0; j < field_len; j++) {
            area = area * 10 + field[j] - '0';
        }
        if (is_mortagage(certno_var, certno_var_len)) {
            estate_price_sum2 += area * estate_price;
        } else {
            estate_price_sum1 += area * estate_price;
        }
    }
    num2str(classify(estate_price_sum1), estate_estimate_1, &estate_estimate_1_len);
    num2str(classify(estate_price_sum2), estate_estimate_2, &estate_estimate_2_len);
    for (int i = 0; i < 28; i++) {
        result_info[i] = "estate estimate range 1 is: "[i];
    }
    for (int i = 0; i < estate_estimate_1_len; i++) {
        result_info[28 + i] = estate_estimate_1[i];
    }
    for (int i = 0; i < 30; i++) {
        result_info[28 + estate_estimate_1_len + i] = ", estate estimate range 2 is: "[i];
    }
    for (int i = 0; i <estate_estimate_2_len; i++) {
        result_info[28 + estate_estimate_1_len + 30 + i] = estate_estimate_2[i];
    }
    HAL_SetResultInfo(result_info, 28 + estate_estimate_1_len + 30 + estate_estimate_2_len);
    HAL_SetStatusCode(STATUS_CODE_SUCCESS);
}

const funcp flist[4] = {
        f0,
        f1,
        0,
        0,
};