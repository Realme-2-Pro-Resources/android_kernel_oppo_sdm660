/*
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/gpio.h>
#include <soc/oppo/oppo_project.h>
#include "oppo_touchscreen/Synaptics/S3508/synaptics_s3508.h"
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"
#ifdef CONFIG_MACH_REALME_RMX1801
#include "touch.h"
#endif

#define MAX_LIMIT_DATA_LENGTH         100

#define S3508_FW_NAME "tp/16051/16051_FW_S3508_SYNAPTICS.img"
#define S3508_BASELINE_TEST_LIMIT_NAME "tp/16051/16051_Limit_data.img"
#define S3320_FW_NAME "tp/16103/16103_FW_S3320_JDI.img"
#define S3320_BASELINE_TEST_LIMIT_NAME "tp/16103/16103_Limit_data.img"
#define S3508_FW_NAME_16118 "tp/16118/16118_FW_S3508_SYNAPTICS.img"
#define S3508_BASELINE_TEST_LIMIT_NAME_16118 "tp/16118/16118_Limit_data.img"
#define S3508_FW_NAME_17011 "tp/17011/17011_FW_S3508_SYNAPTICS.img"
#define S3508_BASELINE_TEST_LIMIT_NAME_17011 "tp/17011/17011_Limit_data.img"
#define S3508_FW_NAME_17021 "tp/17021/17021_FW_S3508_SYNAPTICS.img"
#define S3508_BASELINE_TEST_LIMIT_NAME_17021 "tp/17021/17021_Limit_data.img"

#define NT36672_NF_CHIP_NAME "NT_NF36672"
#define HX83112A_NF_CHIP_NAME "HX_NF83112A"

struct tp_dev_name tp_dev_names[] = {
     {TP_OFILM, "OFILM"},
     {TP_BIEL, "BIEL"},
     {TP_TRULY, "TRULY"},
     {TP_BOE, "BOE"},
     {TP_G2Y, "G2Y"},
     {TP_TPK, "TPK"},
     {TP_JDI, "JDI"},
     {TP_TIANMA, "TIANMA"},
     {TP_SAMSUNG, "SAMSUNG"},
     {TP_DSJM, "DSJM"},
     {TP_BOE_B8, "BOEB8"},
     {TP_INNOLUX, "INNOLUX"},
     {TP_HIMAX_DPT, "DPT"},
     {TP_AUO, "AUO"},
     {TP_DEPUTE, "DEPUTE"},
     {TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

int g_tp_dev_vendor = TP_UNKNOWN;
char *g_tp_chip_name;

/*
 * Judge whether the ic driver should be loaded.
 * For incell modules the tp is defined by the lcd module,
 * so the tp type can be got from the lcd panel on the boot command line.
 */
bool __init tp_judge_ic_match(char *tp_ic_name)
{
    pr_info("[TP] tp_ic_name = %s\n", tp_ic_name);

    switch (get_project()) {
    case OPPO_18321:
        if (strstr(tp_ic_name, "nt36672") && strstr(boot_command_line, "tianma_nt36672")) {
            g_tp_dev_vendor = TP_TIANMA;
            g_tp_chip_name = NT36672_NF_CHIP_NAME;
            return true;
        }
        if (strstr(tp_ic_name, "hx83112a_nf") && strstr(boot_command_line, "himax_hx83112")) {
            g_tp_dev_vendor = TP_DSJM;
            g_tp_chip_name = HX83112A_NF_CHIP_NAME;
            return true;
        }
        if (strstr(tp_ic_name, "nt36672") && strstr(boot_command_line, "dpt_jdi_nt36672")) {
            g_tp_dev_vendor = TP_DEPUTE;
            g_tp_chip_name = NT36672_NF_CHIP_NAME;
            return true;
        }
        break;
    default:
        /* Non-incell projects have a single tp ic, always load it */
        return true;
    }

    pr_err("[TP] lcd module not matched for %s\n", tp_ic_name);
    return false;
}

#ifdef CONFIG_MACH_REALME_RMX1801
static int tp_util_get_vendor_18321(struct panel_info *panel_data)
{
    char *vendor;

    if (g_tp_chip_name != NULL)
        panel_data->chip_name = g_tp_chip_name;

    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL)
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");

    panel_data->extra = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->extra == NULL)
        pr_err("[TP]panel_data.extra kzalloc error\n");

    panel_data->tp_type = g_tp_dev_vendor;
    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);
    strcpy(panel_data->manufacture_info.manufacture, vendor);
    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            get_project(), panel_data->chip_name, vendor);

    if (panel_data->test_limit_name)
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                "tp/%d/LIMIT_%s_%s.img",
                get_project(), panel_data->chip_name, vendor);

    if (panel_data->extra)
        snprintf(panel_data->extra, MAX_LIMIT_DATA_LENGTH,
                "tp/%d/BOOT_FW_%s_%s.ihex",
                get_project(), panel_data->chip_name, vendor);

    switch (panel_data->tp_type) {
    case TP_TIANMA:
        memcpy(panel_data->manufacture_info.version, "0xBD1671", 8);
        panel_data->firmware_headfile.firmware_data = FW_18316_NT36672A_NF_TIANMA;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_18316_NT36672A_NF_TIANMA);
        break;
    case TP_DEPUTE:
        memcpy(panel_data->manufacture_info.version, "0xBD1672", 8);
        panel_data->firmware_headfile.firmware_data = FW_18316_NT36672A_NF_DEPUTE;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_18316_NT36672A_NF_DEPUTE);
        break;
    case TP_DSJM:
        memcpy(panel_data->manufacture_info.version, "0xBD1673", 8);
        panel_data->firmware_headfile.firmware_data = FW_18316_HX83112A_NF_DSJM;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_18316_HX83112A_NF_DSJM);
        break;
    default:
        panel_data->firmware_headfile.firmware_data = NULL;
        panel_data->firmware_headfile.firmware_size = 0;
        break;
    }

    pr_info("[TP]Vendor:%s\n", vendor);
    pr_info("[TP]Fw:%s\n", panel_data->fw_name);
    pr_info("[TP]Limit:%s\n", panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);
    pr_info("[TP]Extra:%s\n", panel_data->extra == NULL ? "NO Extra" : panel_data->extra);

    return 0;
}
#endif

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
#ifdef CONFIG_MACH_REALME_RMX1801
    if (is_project(OPPO_18321))
        return tp_util_get_vendor_18321(panel_data);
#endif

    if (is_project(OPPO_16051)) {
        panel_data->test_limit_name = kzalloc(sizeof(S3508_BASELINE_TEST_LIMIT_NAME), GFP_KERNEL);
        if (panel_data->test_limit_name == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
            return -1;
        }
        strcpy(panel_data->test_limit_name, S3508_BASELINE_TEST_LIMIT_NAME);
        strcpy(panel_data->fw_name, S3508_FW_NAME);
        pr_err("[TP]%s: fw_name = %s \n",__func__, panel_data->fw_name);
    } else if (is_project(OPPO_16103)) {
        panel_data->test_limit_name = kzalloc(sizeof(S3320_BASELINE_TEST_LIMIT_NAME), GFP_KERNEL);
        if (panel_data->test_limit_name == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
            return -1;
        }
        strcpy(panel_data->test_limit_name, S3320_BASELINE_TEST_LIMIT_NAME);
        strcpy(panel_data->fw_name, S3320_FW_NAME);
        pr_err("[TP]%s: fw_name = %s \n",__func__, panel_data->fw_name);
    } else if (is_project(OPPO_16118)) {
        panel_data->test_limit_name = kzalloc(sizeof(S3508_BASELINE_TEST_LIMIT_NAME_16118), GFP_KERNEL);
        if (panel_data->test_limit_name == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
            return -1;
        }
        strcpy(panel_data->test_limit_name, S3508_BASELINE_TEST_LIMIT_NAME_16118);
        strcpy(panel_data->fw_name, S3508_FW_NAME_16118);
        pr_err("[TP]%s: fw_name = %s \n",__func__, panel_data->fw_name);
    } else if (is_project(OPPO_17011)) {
        panel_data->test_limit_name = kzalloc(sizeof(S3508_BASELINE_TEST_LIMIT_NAME_17011), GFP_KERNEL);
        if (panel_data->test_limit_name == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
            return -1;
        }
        strcpy(panel_data->test_limit_name, S3508_BASELINE_TEST_LIMIT_NAME_17011);
        strcpy(panel_data->fw_name, S3508_FW_NAME_17011);
        pr_err("[TP]%s: fw_name = %s \n",__func__, panel_data->fw_name);
    } else if (is_project(OPPO_17021)) {
        panel_data->test_limit_name = kzalloc(sizeof(S3508_BASELINE_TEST_LIMIT_NAME_17021), GFP_KERNEL);
        if (panel_data->test_limit_name == NULL) {
            pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
            return -1;
        }
        strcpy(panel_data->test_limit_name, S3508_BASELINE_TEST_LIMIT_NAME_17021);
        strcpy(panel_data->fw_name, S3508_FW_NAME_17021);
        pr_err("[TP]%s: fw_name = %s \n",__func__, panel_data->fw_name);
    }
    strcpy(panel_data->manufacture_info.manufacture, "SAMSUNG");

    return 0;
}
