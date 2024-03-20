// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <mapmem.h>
#include <fdt_support.h>

#include <earlycamera_starter.h>

#define SCRATCHPAD	1024	/* bytes of scratchpad memory */
#define LOG_TAG		"RVC"
/* coverity [misra_c_2012_rule_20_10_violation : FAIL] */
#define loge(fmt, args...)      {					\
		pr_err("[ERROR][%s] %s - " fmt, LOG_TAG, __func__, ##args); }
/* coverity [misra_c_2012_rule_20_10_violation : FAIL] */
#define logw(fmt, args...)      {					\
		pr_warn("[WARN][%s] %s - " fmt, LOG_TAG, __func__, ##args); }
/* coverity [misra_c_2012_rule_20_10_violation : FAIL] */
#define logd(fmt, args...)      {					\
		pr_info("[DEBUG][%s] %s - " fmt, LOG_TAG, __func__, ##args); }
/* coverity [misra_c_2012_rule_20_10_violation : FAIL] */
#define logi(fmt, args...)      {					\
		pr_info("[DEBUG][%s] %s - " fmt, LOG_TAG, __func__, ##args); }

static uintptr_t	addr;

static int get_node_offset(int *offset, uintptr_t paddr, const char *pathp);

static int setenv_fdt_addr(unsigned long p_addr)
{
	const struct fdt_header	*blob;
	int			ret;

	/*
	 * Set the address [and length] of the fdt.
	 */
	logd("dtb addr: 0x%08lx\n", p_addr);
	addr = p_addr;
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	blob = map_sysmem(p_addr, 0);
	if (blob == NULL) {
		loge("blob is NULL\n");
		ret = 1;
	} else {
		set_working_fdt_addr(p_addr);
		logd("setting working fdt addr\n");
		ret = 0;
	}
	return ret;
}

static int check_params(int argc, char *const argv[])
{
	int ret = (int)CMD_RET_SUCCESS;

	logd("%s - argc: %d\n", __func__, argc);
	logd("%s - argv[1]: %s\n", __func__, argv[1]);

	if ((argc < 2) || (strncmp(argv[1], "dtb", 3) != 0)) {
		ret = (int)CMD_RET_USAGE;
	}
	return ret;
}

struct pmap_info {
	unsigned int	base;
	unsigned int	size;
};

static void get_memory_info(struct pmap_info *pinfo,
	struct ecam_params *params, const fdt32_t *datap,
	int cell_size, uint32_t idx_path)
{
	u64 ret = 0;
	logd("cell size: %d\n", cell_size);
	logd("datap: 0x%lx\n", (uintptr_t)datap);
	logd("idx_path: 0x%08x\n", idx_path);

	if (cell_size == 1) {
		ret = fdt_read_number(datap, 1);
		datap++;
		if (ret < UINT_MAX) {
			pinfo[idx_path].base = (unsigned int)ret;
		}
		ret = fdt_read_number(datap, 1);
		if (ret < UINT_MAX) {
			pinfo[idx_path].size = (unsigned int)ret;
		}


	} else if (cell_size == 2) {
		datap++;
		ret = fdt_read_number(datap, 1);
		datap++;
		if (ret < UINT_MAX) {
			pinfo[idx_path].base = (unsigned int)ret;
		}
		datap++;
		ret = fdt_read_number(datap, 1);
		datap++;
		if (ret < UINT_MAX) {
			pinfo[idx_path].size = (unsigned int)ret;
		}
	} else {
		logw("Cell size is wrong. Skip to initialize pmap info\n");
	}

	if ((idx_path > 0U) && (pinfo[idx_path - 1U].size > 0U)) {
		pinfo[idx_path].base =
			pinfo[idx_path - 1U].base +
			pinfo[idx_path - 1U].size;
	}

	params->memory_addr[idx_path] = pinfo[idx_path].base;
}

static int fdt_extract_memory(struct ecam_params *params, int cell_size)
{
	static struct pmap_info pinfo[PMAP_TYPE_CAM_MAX];
	const char *prop = "reg";
	const char* const paths[PMAP_TYPE_CAM_MAX] = {
		"/reserved-memory/pmap_parking_gui",
		"/reserved-memory/pmap_rearcamera_viqe",
		"/reserved-memory/pmap_rearcamera"
	};

	int ret = 0;
	int len = 0;
	int offset;
	uint32_t idx_path;

	const fdt32_t	*datap;
	const char	*pathp;

	for (idx_path = 0; idx_path < (uint32_t)PMAP_TYPE_CAM_MAX; idx_path++) {
		pathp = paths[idx_path];

		logd("paths[%u] = %s\n", idx_path, pathp);

		if (get_node_offset(&offset, addr, pathp) == 0) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			datap = fdt_getprop((const void *)addr, offset, prop, &len);
			get_memory_info(&pinfo[0], params, datap,
				cell_size, idx_path);

		} else {
			ret = 1;
			break;
		}

		logd("base: 0x%08x, size: 0x%08x, name: %s\n",
		     pinfo[idx_path].base,
		     pinfo[idx_path].size, paths[idx_path]);
	}

	earlycamera_set_parameters(params);

	for (idx_path = 0U; idx_path < (uint32_t)PMAP_TYPE_CAM_MAX; idx_path++) {
		logd("memory_addr[%u] = 0x%lx\n",
			idx_path, params->memory_addr[idx_path]);
	}

	return ret;
}

static int check_and_set_fdt_addr(int argc, char *const argv[])
{
	/* coverity[misra_c_2012_rule_13_5_violation : FALSE] */
	return (int)((int)((check_params(argc, argv) == 0) &&
		(setenv_fdt_addr(simple_strtoul(argv[2], NULL, 16)) == 0)) == 0);
}

static int get_cell_size(int *cell_size, const fdt32_t *p_data)
{
	u64 ret = fdt_read_number(p_data++, 1);
	/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
	/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
	if (ret < INT_MAX) {
		*cell_size = (int)ret;
	}
	logd("cellsize: %d, p_data: 0x%lx\n", *cell_size, (uintptr_t)p_data);

	return (*cell_size == 0) ? 1 : 0;
}

static int fdt_read_cellsize(int *size, uintptr_t paddr,
	const char *pchar, const char *prop)
{
	int offset;
	int len = 0;
	int ret = 0;

	const fdt32_t *p_data;

	if (get_node_offset(&offset, paddr, pchar) == 0) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		p_data = fdt_getprop((const void *)paddr, offset, prop, &len);
		ret = get_cell_size(size, p_data);
	} else {
		ret = 1;
	}
	return ret;
}

/* coverity[misra_c_2012_rule_2_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int prepare_earlycamera(cmd_tbl_t *cmdtp, int flag,
	int argc, char *const argv[])
{
	struct ecam_params	params;

	const char		*p_cellsize	= "/reserved-memory/";
	const char		*prop_cellsize	= "#address-cells";
	int			cellsize	= 0;

	/* coverity[misra_c_2012_rule_13_5_violation : FALSE] */
	return (int)((int)((check_and_set_fdt_addr(argc, argv) == 0) &&
		(fdt_read_cellsize(&cellsize, addr,
			p_cellsize, prop_cellsize) == 0) &&
		(fdt_extract_memory(&params, cellsize) == 0)) == 0);
}

/* coverity[misra_c_2012_rule_2_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int start_earlycamera(cmd_tbl_t *cmdtp,
	int flag, int argc, char *const argv[])
{
	return startEarlyCamera();
}

/* coverity[misra_c_2012_rule_2_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int load_parking_guide_line(cmd_tbl_t *cmdtp,
	int flag, int argc, char *const argv[])
{
	return prepareParkingGuideLine();
}

static int map_pgl_header(const struct fdt_header *header, const uintptr_t paddr)
{
	int ret = 0;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	header = map_sysmem(paddr, 0);
	if (header == NULL) {
		loge("Failed to map fdt_header\n");
		ret = 1;
	}
	return ret;
}

static int get_node_offset(int *offset, uintptr_t paddr, const char *pathp)
{
	int ret = 0;

	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	*offset = fdt_path_offset((const void *)paddr, pathp);
	if (*offset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		loge("Failed to find node: %s\n", pathp);
		(void)fdt_strerror(*offset);
		ret = 1;
	}
	logd("pathp: %s, offset: %d\n", pathp, *offset);
	return ret;
}

static int fdt_pgl_setprop(uintptr_t paddr, int nodeoffset,
	const char *prop, int enable)
{
	int			datap;
	int			ret = 0;

	if ((enable == 0) || (enable == 1)) {
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		datap = fdt_setprop_cell((void *)paddr, nodeoffset,
			prop, (uint32_t)enable);
		if (datap < 0) {
			/* failed to set property */
			loge("ERROR: prop: %s is not found\n", prop);
			ret = 1;
		}
	} else {
		ret = 1;
	}

	return ret;
}

int fdt_pgl_set(int enable)
{
	const struct fdt_header	*blob;
	const char		*pathp;
	const char		*prop;
	int			nodeoffset;

	pathp	= "/videoinput0";
	prop	= "use_pgl";
	blob	= NULL;

	logd("%s - dtb addr: 0x%08lx\n", __func__, addr);

	set_working_fdt_addr(addr);

	/* coverity[misra_c_2012_rule_13_5_violation : FALSE] */
	return (int)((int)((map_pgl_header(blob, addr) == 0) &&
		(get_node_offset(&nodeoffset, addr, pathp) == 0) &&
		(fdt_pgl_setprop(addr, nodeoffset, prop, enable) == 0)) == 0);
}

static int do_earlycamera(cmd_tbl_t *cmdtp,
			  int flag, int argc, char *const argv[])
{
	(void)pr_force("Early-camera is running in legacy mode.\n");
	(void)pr_force("Please check the configuration\n");

	(void)prepare_earlycamera(cmdtp, flag, argc, argv);
	(void)load_parking_guide_line(cmdtp, flag, argc, argv);
	(void)start_earlycamera(cmdtp, flag, argc, argv);

	return (int)CMD_RET_SUCCESS;
}

/* coverity[cert_str30_c_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_CMD(
	prepare_earlycamera, 3, 1, prepare_earlycamera,
	"Telechips Earlycamera Solution",
	"prepare_earlycamera dtb ${pgl_addr}"
);

/* coverity[cert_str30_c_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_CMD(
	load_pgl, 3, 1, load_parking_guide_line,
	"Telechips Earlycamera Solution",
	"load_pgl dtb ${pgl_addr}"
);

/* coverity[cert_str30_c_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_CMD(
	start_earlycamera, 1, 1, start_earlycamera,
	"Telechips Earlycamera Solution",
	"start_earlycamera (no args)"
);

/* coverity[cert_str30_c_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_CMD(
	earlycamera, 3, 1, do_earlycamera,
	"Telechips Earlycamera Solution",
	" - This is a legacy command of earlycamera. Do not use this command."
);
