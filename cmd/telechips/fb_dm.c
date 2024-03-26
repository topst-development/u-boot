#include <common.h>
#include <command.h>
#include <dm.h>
#include <display.h>
#include <clk.h>
#include <mach/clock.h>
#include <fdt_support.h>
#include <video.h>
#include <lcd.h>
#include <mapmem.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <telechips/fb_dm.h>
#include <dt-bindings/display/telechips-common-fb_dm_display.h>
#ifdef CONFIG_TCC897X
#include <dt-bindings/display/tcc897x-vioc.h>
#include <dt-bindings/display/tcc897x-fb_dm_display.h>
#elif CONFIG_TCC805X
#include <dt-bindings/display/tcc805x-vioc.h>
#include <dt-bindings/display/tcc805x-fb_dm_display.h>
#elif CONFIG_TCC750X
#include <dt-bindings/display/tcc750x-vioc.h>
#include <dt-bindings/display/tcc750x-fb_dm_display.h>
#endif

#define CMD_FB_SHOW_ALL 0xFF

DECLARE_GLOBAL_DATA_PTR;
extern struct tcc_fb_dm_global_info tcc_dm_global;

//gd->fdt_blob
/*
enum {
	PANEL_DP0 = 0,
	PANEL_DP1,
	PANEL_DP2,
	PANEL_DP3,
	PANEL_DP_MAX,
	PANEL_LVDS_DUAL,
	PANEL_LVDS_SINGLE,
	PANEL_HDMI,
	PANEL_DSI,
	PANEL_DEVICE_MAX
};
#define FB_DRAW_TYPE_BITMAP_HEADER 0
#define FB_DRAW_TYPE_SPLASH 1
*/
const char *out_media_name[PANEL_DEVICE_MAX] = {
	"DP0",
	"DP1",
	"DP2",
	"DP3",
	"DPMAX",
	"LVDS_DUAL",
	"LVDS_SINGLE",
	"HDMI",
	"DSI"
};

const char *fb_status_name[TCC_FBDM_STAT_MAX] = {
	"DISABLED",
	"ENABLED",
	"FB PARSE ERROR",
	"REMOTE(OUTPUT MEDIA) ERROR",
	"FB BUF ERROR",
	"FB PERI ERROR"
};

static void show_fb_dm_info(int fb_id)
{
	const struct udevice *dev;
	struct uclass *uc;
	int show_flag = 0;

	printf("[%s]: version : %d.%d.%d %s\n",
			tcc_dm_global.tcc_fb_dm_version.name,
			tcc_dm_global.tcc_fb_dm_version.major,
			tcc_dm_global.tcc_fb_dm_version.minor,
			tcc_dm_global.tcc_fb_dm_version.patchlevel,
			tcc_dm_global.tcc_fb_dm_version.date);

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		if (fb_id == CMD_FB_SHOW_ALL) {
			printf("%-12s= %s %sactive\n", "Video", dev->name,
			       device_active(dev) ? "" : "in");
			show_flag = 1;
		} else if (device_active(dev)) {
			struct video_priv *uc_priv = dev_get_uclass_priv(dev);
			//struct video_uc_plat *plat = dev_get_uclass_plat(dev);
			struct tcc_fb_dm_priv *priv = dev_get_priv(dev);

			if(fb_id == priv->id) {
				printf("[%s: Video private data(video_priv)]\n", dev->name);
				printf("  %-12s= 0x%lx\n", "FB base", (ulong)uc_priv->fb);
				printf("  %-12s= %dx%dx%d\n", "FB resolution", uc_priv->xsize,
				       uc_priv->ysize, 1 << uc_priv->bpix);
				printf("  %-12s= 0x%x\n", "FB size", uc_priv->fb_size);
				printf("  %-12s= 0x%x\n", "FB line_length", uc_priv->line_length);

				/*
				   printf("\n[video_plat info]\n");
				   printf("%-12s= 0x%lx\n", "base", plat->base);
				   printf("%-12s= 0x%lx\n", "copy_base", plat->copy_base);
				   printf("%-12s= 0x%x\n", "align", plat->align);
				   printf("%-12s= 0x%x\n", "size", plat->size);
				*/

				printf("[%s: FB private data(tcc_fb_dm_priv)]\n", dev->name);
				printf("  %-12s= %s\n", "fb status",
				       (priv->fb_stat < TCC_FBDM_STAT_MAX) ?
				       fb_status_name[priv->fb_stat] : "invalid");
				printf("  %-12s= %s\n", "panel id",
				       (priv->media_id < PANEL_DEVICE_MAX) ?
				       out_media_name[priv->media_id] : "invalid");
				printf("  %-12s= %d\n", "interlaced", priv->interlaced);
				printf("  %-12s= %d\n", "pixel_ri", priv->pixel_repetition_input);
				printf("  %-12s= %d\n", "lcd_id[disp]", priv->lcd_id);
				printf("  %-12s= %d\n", "clk_div", priv->clk_div);
				printf("  %-12s= %d\n", "ovp", priv->ovp);
				printf("  %-12s= %s\n", "fb draw type", (priv->fb_draw_type == 0U) ? "bitmap" : "splash");
				printf("  %-12s= %d\n", "fb_buf_id", priv->fb_buf_id);
				printf("  %-12s= %d\n", "bpix", priv->target_bpix);

				printf("[%s: Image data(tcc_fb_dm_image_info)]\n", dev->name);
				printf("  %-12s= %d\n", "lcd layer", priv->image_info.lcd_layer);
				printf("  %-12s= %d\n", "enable", priv->image_info.enable);
				printf("  %-12s= %d\n", "Frame Width", priv->image_info.frame_width);
				printf("  %-12s= %d\n", "Frame Height", priv->image_info.frame_height);
				printf("  %-12s= %d\n", "Image Width", priv->image_info.image_width);
				printf("  %-12s= %d\n", "Image Height", priv->image_info.image_height);
				printf("  %-12s= %d\n", "X offset", priv->image_info.offset_x);
				printf("  %-12s= %d\n", "Y offset", priv->image_info.offset_y);
				printf("  %-12s= 0x%lx\n", "base addr0", (unsigned long)priv->image_info.buf_addr0);
				printf("  %-12s= 0x%lx\n", "base addr1", (unsigned long)priv->image_info.buf_addr1);
				printf("  %-12s= 0x%lx\n", "base addr2", (unsigned long)priv->image_info.buf_addr2);
				printf("  %-12s= %d\n", "fmt", (unsigned int)priv->image_info.fmt);
				printf("  %-12s= 0x%08x\n", "logo_base", priv->image_info.logo_base);

				printf("[%s: VIOC path setting])\n", dev->name);
				printf("  RDMA[%d]-WMIX[%d]-DISP[%d]-[%s]\n",
				       (priv->lcd_id * 4) + priv->image_info.lcd_layer,
				       priv->lcd_id,
				       priv->lcd_id,
				       (priv->media_id < PANEL_DEVICE_MAX) ?
				       out_media_name[priv->media_id] : "invalid");

				printf("\n");
				show_flag = 1;
			}
		} else {
			printf("fb%d is inactive\n", fb_id);
		}
	}

	if (show_flag == 0) {
		printf("fb%d not found\n", fb_id);
	}
}

static int do_cmd_fb_dm(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	const char * str_cmd;
	int temp_val;

	if (argc < 2) {
		return CMD_RET_USAGE;
	}

	str_cmd = argv[1];
	argc -= 2;
	argv += 2;

	if (!strcmp(str_cmd, "show")) {
		if(argc >= 1) {
			str_cmd = argv[0];
			temp_val = strlen(str_cmd);
			if((temp_val < 3) || (str_cmd[0] != 'f') || (str_cmd[1] != 'b')){
				return CMD_RET_USAGE;
			} else {
				if(str_cmd[2] < '0' || str_cmd[2] > '9') {
					return CMD_RET_USAGE;
				}
				temp_val = str_cmd[2] - '0';
			}
		} else {
			temp_val = CMD_FB_SHOW_ALL;
		}
		show_fb_dm_info(temp_val);
	} else {
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(
	   fb_dm, 6, 0, do_cmd_fb_dm,
	   "command of fb_dm",
	   //"dt fb[n] - print or set device tree of fb[n]\n"
	   //"dt set fb[n] [property name] [value] - modify fdt property values\n"
	   "show fb[n]- print fb structure info\n"
	   //"enable fb[n] - enable fb[n] including the connected encoder(LVDS, DSI, DP ..)\n"
	   //"disable fb[n] - disable fb[n] includeing the connected encoder\n"
	   //"test_pt fb[n] - To display test pattern"
	   );
