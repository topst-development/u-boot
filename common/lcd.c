// SPDX-License-Identifier: GPL-2.0+
/*
 * Common LCD routines
 *
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
 */

/*
 * Modified by Telechips Inc. (date: 2020-04)
 */

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <env_callback.h>
#include <linux/types.h>
#include <stdio_dev.h>
#include <lcd.h>
#include <mapmem.h>
#include <watchdog.h>
#include <asm/unaligned.h>
#include <splash.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <video_font.h>

#ifdef CONFIG_LCD_LOGO
#include <bmp_logo.h>
#include <bmp_logo_data.h>
#if (CONSOLE_COLOR_WHITE >= BMP_LOGO_OFFSET) && (LCD_BPP != LCD_COLOR16)
#error Default Color Map overlaps with Logo Color Map
#endif
#endif

#ifndef CONFIG_LCD_ALIGNMENT
#define CONFIG_LCD_ALIGNMENT PAGE_SIZE
#endif

#if (LCD_BPP != LCD_COLOR8) && (LCD_BPP != LCD_COLOR16) && \
	(LCD_BPP != LCD_COLOR32)
#error Unsupported LCD BPP.
#endif

DECLARE_GLOBAL_DATA_PTR;

static int lcd_init(void *lcdbase);
static void lcd_logo(void);
static void lcd_setfgcolor(int color);
static void lcd_setbgcolor(int color);

static int lcd_color_fg;
static int lcd_color_bg;
int lcd_line_length;
char lcd_is_enabled = 0;
static void *lcd_base;			/* Start of framebuffer memory	*/
static char lcd_flush_dcache;	/* 1 to flush dcache after each lcd update */

extern struct vidinfo panel_info;

/* Flush LCD activity to the caches */
void lcd_sync(void)
{
/*
 * flush_dcache_range() is declared in common.h but it seems that some
 * architectures do not actually implement it. Is there a way to find
 * out whether it exists? For now, ARM is safe.
 */
#if defined(CONFIG_ARM) && !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	if (lcd_flush_dcache) {
		for (u8 ucCount = 0;
			ucCount < DISPLAY_CONTROLLER_MAX;
			ucCount++)
			lcd_panel_sync(ucCount);
		}
#else
	int line_length;

	if (lcd_flush_dcache)
		flush_dcache_range((ulong)lcd_base,
			(ulong)(lcd_base + lcd_get_size(&line_length)));
#endif
#endif
}

void lcd_set_flush_dcache(int flush)
{
	lcd_flush_dcache = (flush != 0);
}

static void lcd_stub_putc(struct stdio_dev *dev, const char c)
{
	lcd_putc(c);
}

static void lcd_stub_puts(struct stdio_dev *dev, const char *s)
{
	lcd_puts(s);
}

/* Small utility to check that you got the colours right */
#ifdef LCD_TEST_PATTERN

#if LCD_BPP == LCD_COLOR8
#define	N_BLK_VERT	2
#define	N_BLK_HOR	3

static int test_colors[N_BLK_HOR * N_BLK_VERT] = {
	CONSOLE_COLOR_RED,
	CONSOLE_COLOR_GREEN,
	CONSOLE_COLOR_YELLOW,
	CONSOLE_COLOR_BLUE,
	CONSOLE_COLOR_MAGENTA,
	CONSOLE_COLOR_CYAN,
}; /*LCD_BPP == LCD_COLOR8 */

#elif LCD_BPP == LCD_COLOR16
#define	N_BLK_VERT	2
#define	N_BLK_HOR	4

static int test_colors[N_BLK_HOR * N_BLK_VERT] = {
	CONSOLE_COLOR_RED,
	CONSOLE_COLOR_GREEN,
	CONSOLE_COLOR_YELLOW,
	CONSOLE_COLOR_BLUE,
	CONSOLE_COLOR_MAGENTA,
	CONSOLE_COLOR_CYAN,
	CONSOLE_COLOR_GREY,
	CONSOLE_COLOR_WHITE,
};
#endif /*LCD_BPP == LCD_COLOR16 */

static void test_pattern(void)
{
#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
{
	ushort v_max, h_max;
	ushort v_step, h_step;
#if LCD_BPP == LCD_COLOR8
	uchar *pix;
#elif LCD_BPP == LCD_COLOR16
	ushort *pix;
#else
	uint *pix;
#endif

	struct vidinfo	*pstPanel_Info;

	for (unsigned char ucCount = 0;
		ucCount < DISPLAY_CONTROLLER_MAX;
		ucCount++) {
		pstPanel_Info = lcd_get_panel_infor_path(ucCount);
		if (pstPanel_Info != NULL) {
#if LCD_BPP == LCD_COLOR8
			pix	= (uchar *)(pstPanel_Info->lcdbase +
				(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE * ucCount));
#elif LCD_BPP == LCD_COLOR16
			pix	= (ushort *)(pstPanel_Info->lcdbase +
				(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE * ucCount));
#else
			pix	= (uint *)(pstPanel_Info->lcdbase +
				(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE * ucCount));
#endif
			v_max = pstPanel_Info->vl_row;
			h_max = pstPanel_Info->vl_col;

			v_step = ((v_max + N_BLK_VERT - 1) / N_BLK_VERT);
			h_step = ((h_max + N_BLK_HOR  - 1) / N_BLK_HOR);

			for (ushort v = 0; v < v_max; ++v) {
				uchar iy = (v / v_step);

				for (ushort h = 0; h < h_max; ++h) {
					uchar ix = N_BLK_HOR * iy + h / h_step;
					*pix++ = test_colors[ix];
				}
			}
		}
	}
}
#else

	ushort v_max  = panel_info.vl_row;
	ushort h_max  = panel_info.vl_col;
	ushort v_step = (v_max + N_BLK_VERT - 1) / N_BLK_VERT;
	ushort h_step = (h_max + N_BLK_HOR  - 1) / N_BLK_HOR;
	ushort v, h;
#if LCD_BPP == LCD_COLOR8
	uchar *pix = (uchar *)lcd_base;
#elif LCD_BPP == LCD_COLOR16
	ushort *pix = (ushort *)lcd_base;
#endif

	pr_info("[LCD] Test Pattern: %d x %d [%d x %d]\n",
		h_max, v_max, h_step, v_step);

	for (v = 0; v < v_max; ++v) {
		uchar iy = v / v_step;

		for (h = 0; h < h_max; ++h) {
			uchar ix = N_BLK_HOR * iy + h / h_step;
			*pix++ = test_colors[ix];
		}
	}
#endif
}
#endif /* LCD_TEST_PATTERN */

/*
 * With most lcd drivers the line length is set up
 * by calculating it from panel_info parameters. Some
 * drivers need to calculate the line length differently,
 * so make the function weak to allow overriding it.
 */
__weak int lcd_get_size(int *line_length)
{
	*line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;
	return *line_length * panel_info.vl_row;
}

int drv_lcd_init(void)
{
	struct stdio_dev lcddev;
	int rc;

	lcd_base = map_sysmem(gd->fb_base, 0);

	lcd_init(lcd_base);

	/* Device initialization */
	memset(&lcddev, 0, sizeof(lcddev));

	strcpy(lcddev.name, "lcd");
	lcddev.ext   = 0;			/* No extensions */
	lcddev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	lcddev.putc  = lcd_stub_putc;		/* 'putc' function */
	lcddev.puts  = lcd_stub_puts;		/* 'puts' function */

	rc = stdio_register(&lcddev);

	return (rc == 0) ? 1 : rc;
}

void lcd_clear(void)
{
	int bg_color;
	__maybe_unused ulong addr;
	static int do_splash = 1;
#if LCD_BPP == LCD_COLOR8
	/* Setting the palette */
	lcd_setcolreg(CONSOLE_COLOR_BLACK, 0, 0, 0);
	lcd_setcolreg(CONSOLE_COLOR_RED, 0xFF, 0, 0);
	lcd_setcolreg(CONSOLE_COLOR_GREEN, 0, 0xFF, 0);
	lcd_setcolreg(CONSOLE_COLOR_YELLOW, 0xFF, 0xFF, 0);
	lcd_setcolreg(CONSOLE_COLOR_BLUE, 0, 0, 0xFF);
	lcd_setcolreg(CONSOLE_COLOR_MAGENTA, 0xFF, 0, 0xFF);
	lcd_setcolreg(CONSOLE_COLOR_CYAN, 0, 0xFF, 0xFF);
	lcd_setcolreg(CONSOLE_COLOR_GREY, 0xAA, 0xAA, 0xAA);
	lcd_setcolreg(CONSOLE_COLOR_WHITE, 0xFF, 0xFF, 0xFF);
#endif

#ifndef CONFIG_SYS_WHITE_ON_BLACK
	lcd_setfgcolor(CONSOLE_COLOR_BLACK);
	lcd_setbgcolor(CONSOLE_COLOR_WHITE);
	bg_color = CONSOLE_COLOR_WHITE;
#else
	lcd_setfgcolor(CONSOLE_COLOR_WHITE);
	lcd_setbgcolor(CONSOLE_COLOR_BLACK);
	bg_color = CONSOLE_COLOR_BLACK;
#endif	/* CONFIG_SYS_WHITE_ON_BLACK */

#ifdef	LCD_TEST_PATTERN
	test_pattern();
#else
	/* set framebuffer to background color */

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	lcd_panel_set_background_color(bg_color);
#else
#if (LCD_BPP != LCD_COLOR32)
	memset((char *)lcd_base, bg_color, lcd_line_length * panel_info.vl_row);
#else
	u32 *ppix = lcd_base;
	u32 i;

	for (i = 0;
		i < (lcd_line_length * panel_info.vl_row)/
			NBYTES(panel_info.vl_bpix);
		i++)
		*ppix++ = bg_color;

#endif
#endif	/* #if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH) */
#endif	/* #ifdef	LCD_TEST_PATTERN */

	/* setup text-console */
	debug("[LCD] setting up console...\n");
	lcd_init_console(lcd_base,
			 panel_info.vl_col,
			 panel_info.vl_row,
			 panel_info.vl_rot);
	/* Paint the logo and retrieve LCD base address */
	debug("[LCD] Drawing the logo...\n");

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_CMD_BMP)
	int iNumOfColByteAligned;
	int iLastNumOfColByteAligned;
	int iTotalNumOfPanelBytes;
	int iRetVal;
	void *pvLastLcd_BaseAddr;
	struct vidinfo	*pstPanel_Info, stLastPanel_Info;

	if (do_splash) {
		iLastNumOfColByteAligned = lcd_line_length;
		pvLastLcd_BaseAddr = lcd_base;

		memcpy(&stLastPanel_Info,
				&panel_info,
				sizeof(struct vidinfo));

		for (unsigned char ucCount = 0;
			ucCount < DISPLAY_CONTROLLER_MAX;
			ucCount++) {
			pstPanel_Info = lcd_get_panel_infor_path(ucCount);
			if (pstPanel_Info != NULL) {
				iTotalNumOfPanelBytes =
					lcd_get_size(&lcd_line_length);
				lcd_base = (void *)pstPanel_Info->lcdbase;
				memcpy(&panel_info,
						pstPanel_Info,
						sizeof(struct vidinfo));

				iRetVal = splash_display();
				if (iRetVal != 0)
					pr_err("[%s:%d]Error: from splash_display(%d)\n",
							__func__,
							__LINE__,
							ucCount);
			}
		}

		lcd_base = pvLastLcd_BaseAddr;
		lcd_line_length = iLastNumOfColByteAligned;

		memcpy(&panel_info, &stLastPanel_Info,    sizeof(vidinfo_t));

		if (iRetVal == 0) {
			do_splash = 0;
			lcd_sync();
			return;
		}
	}
#else
	if (do_splash) {
		if (splash_display() == 0) {
			do_splash = 0;
			lcd_sync();
			return;
		}
	}
#endif
#endif/* #if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH) */

	lcd_logo();
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	addr = (ulong)lcd_base + BMP_LOGO_HEIGHT * lcd_line_length;
	lcd_init_console((void *)addr, panel_info.vl_col,
			 panel_info.vl_row, panel_info.vl_rot);
#endif
	lcd_sync();
}

static int lcd_init(void *lcdbase)
{
	debug("[LCD] Initializing LCD frambuffer at %p\n", lcdbase);
	lcd_ctrl_init(lcdbase);

	/*
	 * lcd_ctrl_init() of some drivers (i.e. bcm2835 on rpi) ignores
	 * the 'lcdbase' argument and uses custom lcd base address
	 * by setting up gd->fb_base. Check for this condition and fixup
	 * 'lcd_base' address.
	 */
	if (map_to_sysmem(lcdbase) != gd->fb_base)
		lcd_base = map_sysmem(gd->fb_base, 0);

	debug("[LCD] Using LCD frambuffer at %p\n", lcd_base);

	lcd_get_size(&lcd_line_length);
	lcd_is_enabled = 1;
	lcd_clear();
	lcd_enable();

	/* Initialize the console */
	lcd_set_col(0);
#ifdef CONFIG_LCD_INFO_BELOW_LOGO
	lcd_set_row(7 + BMP_LOGO_HEIGHT / VIDEO_FONT_HEIGHT);
#else
	lcd_set_row(1);/* leave 1 blank line below logo */
#endif

	return 0;
}

/*
 * This is called early in the system initialization to grab memory
 * for the LCD controller.
 * Returns new address for monitor, after reserving LCD buffer memory
 *
 * Note that this is running from ROM, so no write access to global data.
 */
ulong lcd_setmem(ulong addr)
{
	ulong size;
	int line_length;

	debug("LCD panel info: %d x %d, %d bit/pix\n", panel_info.vl_col,
		panel_info.vl_row, NBITS(panel_info.vl_bpix));

	size = lcd_get_size(&line_length);

	/* Round up to nearest full page, or MMU section if defined */
	size = ALIGN(size, CONFIG_LCD_ALIGNMENT);
	addr = ALIGN(addr - CONFIG_LCD_ALIGNMENT + 1, CONFIG_LCD_ALIGNMENT);

	/* Allocate pages for the frame buffer. */
	addr -= size;

	debug("Reserving %ldk for LCD Framebuffer at: %08lx\n",
	      size >> 10, addr);

	return addr;
}

static void lcd_setfgcolor(int color)
{
	lcd_color_fg = color;
}

int lcd_getfgcolor(void)
{
	return lcd_color_fg;
}

static void lcd_setbgcolor(int color)
{
	lcd_color_bg = color;
}

int lcd_getbgcolor(void)
{
	return lcd_color_bg;
}

#ifdef CONFIG_LCD_LOGO
__weak void lcd_logo_set_cmap(void)
{
	int i;
	ushort *cmap = configuration_get_cmap();

	for (i = 0; i < ARRAY_SIZE(bmp_logo_palette); ++i)
		*cmap++ = bmp_logo_palette[i];
}

void lcd_logo_plot(int x, int y)
{
	ushort i, j;
	uchar *bmap = &bmp_logo_bitmap[0];
	unsigned int bpix = NBITS(panel_info.vl_bpix);
	uchar *fb = (uchar *)(lcd_base + y * lcd_line_length + x * bpix / 8);
	ushort *fb16;

	debug("Logo: width %d  height %d  colors %d\n",
	      BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, BMP_LOGO_COLORS);

	if (bpix < 12) {
		WATCHDOG_RESET();
		lcd_logo_set_cmap();
		WATCHDOG_RESET();

		for (i = 0; i < BMP_LOGO_HEIGHT; ++i) {
			memcpy(fb, bmap, BMP_LOGO_WIDTH);
			bmap += BMP_LOGO_WIDTH;
			fb += panel_info.vl_col;
		}
	} else { /* true color mode */
		u16 col16;

		fb16 = (ushort *)fb;
		for (i = 0; i < BMP_LOGO_HEIGHT; ++i) {
			for (j = 0; j < BMP_LOGO_WIDTH; j++) {
				col16 = bmp_logo_palette[(bmap[j]-16)];
				fb16[j] =
					((col16 & 0x000F) << 1) |
					((col16 & 0x00F0) << 3) |
					((col16 & 0x0F00) << 4);
				}
			bmap += BMP_LOGO_WIDTH;
			fb16 += panel_info.vl_col;
		}
	}

	WATCHDOG_RESET();
	lcd_sync();
}
#else
static inline void lcd_logo_plot(int x, int y) {}
#endif /* CONFIG_LCD_LOGO */

#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)
#ifdef CONFIG_SPLASH_SCREEN_ALIGN

static void splash_align_axis(int *axis, unsigned long panel_size,
					unsigned long picture_size)
{
	unsigned long panel_picture_delta = panel_size - picture_size;
	unsigned long axis_alignment;

	if (*axis == BMP_ALIGN_CENTER)
		axis_alignment = panel_picture_delta / 2;
	else if (*axis < 0)
		axis_alignment = panel_picture_delta + *axis + 1;
	else
		return;

	*axis = max(0, (int)axis_alignment);
}
#endif

#ifdef CONFIG_LCD_BMP_RLE8
#define BMP_RLE8_ESCAPE		0
#define BMP_RLE8_EOL		0
#define BMP_RLE8_EOBMP		1
#define BMP_RLE8_DELTA		2

static void draw_unencoded_bitmap(ushort **fbp, uchar *bmap, ushort *cmap,
				  int cnt)
{
	while (cnt > 0) {
		*(*fbp)++ = cmap[*bmap++];
		cnt--;
	}
}

static void draw_encoded_bitmap(ushort **fbp, ushort c, int cnt)
{
	ushort *fb = *fbp;
	int cnt_8copy = cnt >> 3;

	cnt -= cnt_8copy << 3;
	while (cnt_8copy > 0) {
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		*fb++ = c;
		cnt_8copy--;
	}
	while (cnt > 0) {
		*fb++ = c;
		cnt--;
	}
	*fbp = fb;
}

/*
 * Do not call this function directly, must be called from lcd_display_bitmap.
 */
static void lcd_display_rle8_bitmap(struct bmp_image *bmp, ushort *cmap,
				    uchar *fb, int x_off, int y_off)
{
	uchar *bmap;
	ulong width, height;
	ulong cnt, runlen;
	int x, y;
	int decode = 1;

	width = get_unaligned_le32(&bmp->header.width);
	height = get_unaligned_le32(&bmp->header.height);
	bmap = (uchar *)bmp + get_unaligned_le32(&bmp->header.data_offset);

	x = 0;
	y = height - 1;

	while (decode) {
		if (bmap[0] == BMP_RLE8_ESCAPE) {
			switch (bmap[1]) {
			case BMP_RLE8_EOL:
				/* end of line */
				bmap += 2;
				x = 0;
				y--;
				/* 16bpix, 2-byte per pixel, width should *2 */
				fb -= (width * 2 + lcd_line_length);
				break;
			case BMP_RLE8_EOBMP:
				/* end of bitmap */
				decode = 0;
				break;
			case BMP_RLE8_DELTA:
				/* delta run */
				x += bmap[2];
				y -= bmap[3];
				/* 16bpix, 2-byte per pixel, x should *2 */
				fb = (uchar *) (lcd_base + (y + y_off - 1)
					* lcd_line_length + (x + x_off) * 2);
				bmap += 4;
				break;
			default:
				/* unencoded run */
				runlen = bmap[1];
				bmap += 2;
				if (y < height) {
					if (x < width) {
						if (x + runlen > width)
							cnt = width - x;
						else
							cnt = runlen;
						draw_unencoded_bitmap(
							(ushort **)&fb,
							bmap, cmap, cnt);
					}
					x += runlen;
				}
				bmap += runlen;
				if (runlen & 1)
					bmap++;
			}
		} else {
			/* encoded run */
			if (y < height) {
				runlen = bmap[0];
				if (x < width) {
					/* aggregate the same code */
					while (bmap[0] == 0xff &&
					       bmap[2] != BMP_RLE8_ESCAPE &&
					       bmap[1] == bmap[3]) {
						runlen += bmap[2];
						bmap += 2;
					}
					if (x + runlen > width)
						cnt = width - x;
					else
						cnt = runlen;
					draw_encoded_bitmap((ushort **)&fb,
						cmap[bmap[1]], cnt);
				}
				x += runlen;
			}
			bmap += 2;
		}
	}
}
#endif

__weak void fb_put_byte(uchar **fb, uchar **from)
{
	*(*fb)++ = *(*from)++;
}

#if defined(CONFIG_BMP_16BPP)
__weak void fb_put_word(uchar **fb, uchar **from)
{
	*(*fb)++ = *(*from)++;
	*(*fb)++ = *(*from)++;
}
#endif /* CONFIG_BMP_16BPP */

__weak void lcd_set_cmap(struct bmp_image *bmp, unsigned int colors)
{
	int i;
	struct bmp_color_table_entry cte;
	ushort *cmap = configuration_get_cmap();

	for (i = 0; i < colors; ++i) {
		cte = bmp->color_table[i];
		*cmap = (((cte.red)   << 8) & 0xf800) |
			(((cte.green) << 3) & 0x07e0) |
			(((cte.blue)  >> 3) & 0x001f);
		cmap++;
	}
}

int lcd_display_bitmap(ulong bmp_image, int x, int y)
{
	ushort *cmap_base = NULL;
	ushort i, j;
	uchar *fb;
	uchar *bmap;
	ushort padded_width;
	unsigned long pwidth, width, height, byte_width;
	unsigned int colors, bpix, bmp_bpix;
	int hdr_size;
	unsigned int uiDiffWidth_Panel_Bmp = 0;

	struct bmp_image *bmp = (struct bmp_image *)map_sysmem(bmp_image, 0);
	struct bmp_color_table_entry *palette = bmp->color_table;

	if (!bmp || !(bmp->header.signature[0] == 'B' &&
		bmp->header.signature[1] == 'M')) {
		pr_err("Error: no valid bmp image at %lx\n", bmp_image);

		return 1;
	}

	width = get_unaligned_le32(&bmp->header.width);
	height = get_unaligned_le32(&bmp->header.height);
	bmp_bpix = get_unaligned_le16(&bmp->header.bit_count);
	hdr_size = get_unaligned_le16(&bmp->header.size);
	debug("hdr_size=%d, bmp_bpix=%d\n", hdr_size, bmp_bpix);

	colors = 1 << bmp_bpix;

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	int iLastNumOfColByteAligned, iTotalNumOfPanelBytes;
	ushort usLast_VLine, usLast_HLine;
	void *pvLastLcd_BaseAddr;

	usLast_VLine = panel_info.vl_col;
	usLast_HLine = panel_info.vl_row;

	iLastNumOfColByteAligned = lcd_line_length;
	pvLastLcd_BaseAddr = lcd_base;

	lcd_base = (void *)panel_info.lcdbase;

	if ((width > MAX_INDIVIDUAL_IMAGE_WIDTH) ||
		(height >  MAX_INDIVIDUAL_IMAGE_HEIGHT)) {
		pr_err("\n[%s:%d][Individual]Error: Log size(col '%d', row '%d') is larger than '1920 x 1080'\n",
				__func__, __LINE__,
				(u32)width, (u32)height);
		return 1;
	}

	if (panel_info.vl_col > MAX_INDIVIDUAL_IMAGE_WIDTH) {
		pr_info("\n[%s:%d][Individual]Panel width %d is larger than %d -> reduce to %d\n",
				__func__, __LINE__,
				(u32)panel_info.vl_col,
				MAX_INDIVIDUAL_IMAGE_WIDTH,
				MAX_INDIVIDUAL_IMAGE_WIDTH);

		panel_info.vl_col = MAX_INDIVIDUAL_IMAGE_WIDTH;
	}

	if (panel_info.vl_row > MAX_INDIVIDUAL_IMAGE_HEIGHT) {
		pr_info("\n[%s:%d][Individual]Panel height %d is larger than %d -> reduce to %d\n",
				__func__, __LINE__,
				(u32)panel_info.vl_row,
				MAX_INDIVIDUAL_IMAGE_HEIGHT,
				MAX_INDIVIDUAL_IMAGE_HEIGHT);

		panel_info.vl_row = MAX_INDIVIDUAL_IMAGE_HEIGHT;
	}

	iTotalNumOfPanelBytes = lcd_get_size(&lcd_line_length);

	pr_info("\n[%s:%d]BMP   -> Col(%d), Row(%d), BPP(%d), Color(0x%x)\n",
				__func__, __LINE__,
				(u32)width, (u32)height,
				(u32)bmp_bpix, (u32)colors);
	pr_info("[%s:%d]Pannel -> Col(%d), Row(%d), BPP(%d)\n",
				__func__, __LINE__,
				(u32)panel_info.vl_col,
				(u32)panel_info.vl_row,
				panel_info.vl_bpix);
	pr_info("[%s:%d]Total length(%d), Line(%d) <- V(%d) x BPP(%d) / 8 * H(%d)\n",
				__func__, __LINE__,
				(u32)iTotalNumOfPanelBytes,
				(u32)lcd_line_length,
				(u32)panel_info.vl_col,
				(u32)NBITS(panel_info.vl_bpix),
				(u32)panel_info.vl_row);
#endif

	pwidth = panel_info.vl_col;

	bpix = NBITS(panel_info.vl_bpix);

	if (bpix != 1 && bpix != 8 && bpix != 16 && bpix != 32) {
		pr_err("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
				bpix,
				bmp_bpix);
		return 1;
	}

	/*
	 * We support displaying 8bpp BMPs on 16bpp LCDs
	 * and displaying 24bpp BMPs on 32bpp LCDs
	 */
	if (bpix != bmp_bpix &&
	    !(bmp_bpix == 8 && bpix == 16) &&
	    !(bmp_bpix == 24 && bpix == 32)) {
		pr_err("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix, get_unaligned_le16(&bmp->header.bit_count));
		return 1;
	}

	debug("Display-bmp: %d x %d  with %d colors, display %d\n",
	      (int)width, (int)height, (int)colors, 1 << bpix);

	if (bmp_bpix == 8)
		lcd_set_cmap(bmp, colors);

	padded_width = (width & 0x3 ? (width & ~0x3) + 4 : width);

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	if (width > pwidth) {
		pr_err("[%s:%d]Error: Logo width(%d) is larger than pannel width(%d) -> it makes log cut right side\n",
				__func__, __LINE__,
				(u32)width,
				(unsigned int)pwidth);
		uiDiffWidth_Panel_Bmp = (width - pwidth);
		width = pwidth;
	}
	if (height > panel_info.vl_row) {
		pr_err("[%s:%d]Error: Logo height(%d) is larger than pannel height(%d) -> it makes log cut top side\n",
				__func__, __LINE__,
				(u32)height,
				(unsigned int)panel_info.vl_row);
		height = panel_info.vl_row;
	}

	splash_align_axis(&x, pwidth, width);
	splash_align_axis(&y, panel_info.vl_row, height);
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

	if ((x + width) > pwidth)
		width = pwidth - x;
	if ((y + height) > panel_info.vl_row)
		height = panel_info.vl_row - y;

	bmap = (uchar *)bmp + get_unaligned_le32(&bmp->header.data_offset);
	fb   = (uchar *)(lcd_base +
		(y + height - 1) * lcd_line_length + x * bpix / 8);

	switch (bmp_bpix) {
	case 1:
	case 8: {
		cmap_base = configuration_get_cmap();
#ifdef CONFIG_LCD_BMP_RLE8
		u32 compression = get_unaligned_le32(&bmp->header.compression);

		debug("compressed %d %d\n", compression, BMP_BI_RLE8);
		if (compression == BMP_BI_RLE8) {
			if (bpix != 16) {
				/* TODO implement render code for bpix != 16 */
				pr_err("Error: only support 16 bpix");
				return 1;
			}
			lcd_display_rle8_bitmap(bmp, cmap_base, fb, x, y);
			break;
		}
#endif

		if (bpix != 16)
			byte_width = width;
		else
			byte_width = width * 2;

		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++) {
				if (bpix != 16) {
					fb_put_byte(&fb, &bmap);
				} else {
					struct bmp_color_table_entry *entry;
					uint val;

					if (cmap_base) {
						val = cmap_base[*bmap];
					} else {
						entry = &palette[*bmap];
						val = entry->blue >> 3 |
							entry->green >> 2 << 5 |
							entry->red >> 3 << 11;
					}
					*(uint16_t *)fb = val;
					bmap++;
					fb += sizeof(uint16_t) / sizeof(*fb);
				}
			}
			bmap += (padded_width - width);
			fb -= byte_width + lcd_line_length;
		}
		break;
	}
#if defined(CONFIG_BMP_16BPP)
	case 16:
		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++)
				fb_put_word(&fb, &bmap);

			bmap += (padded_width - width) * 2;
			fb -= width * 2 + lcd_line_length;

			bmap += (uiDiffWidth_Panel_Bmp * 2);
		}
		break;
#endif /* CONFIG_BMP_16BPP */
#if defined(CONFIG_BMP_24BPP)
	case 24:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = 0;
			}
			fb -= lcd_line_length + width * (bpix / 8);

			bmap += (uiDiffWidth_Panel_Bmp * 3);
		}
		break;
#endif /* CONFIG_BMP_24BPP */
#if defined(CONFIG_BMP_32BPP)
	case 32:
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; j++) {
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
			}
			fb -= lcd_line_length + width * (bpix / 8);

			bmap += (uiDiffWidth_Panel_Bmp * 4);
		}
		break;
#endif /* CONFIG_BMP_32BPP */
	default:
		break;
	};

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	if (lcd_flush_dcache)
		lcd_panel_sync(panel_info.dev.uiDisplayPathNum);

	panel_info.vl_col = usLast_VLine;
	panel_info.vl_row = usLast_HLine;

	lcd_base = pvLastLcd_BaseAddr;
	lcd_line_length = iLastNumOfColByteAligned;
#else
	lcd_sync();
#endif

	return 0;
}
#endif

static void lcd_logo(void)
{
	lcd_logo_plot(0, 0);

#ifdef CONFIG_LCD_INFO
	lcd_set_col(LCD_INFO_X / VIDEO_FONT_WIDTH);
	lcd_set_row(LCD_INFO_Y / VIDEO_FONT_HEIGHT);
	lcd_show_board_info();
#endif /* CONFIG_LCD_INFO */
}

#ifdef CONFIG_SPLASHIMAGE_GUARD
static int on_splashimage(const char *name, const char *value, enum env_op op,
	int flags)
{
	ulong addr;
	int aligned;

	if (op == env_op_delete)
		return 0;

	addr = simple_strtoul(value, NULL, 16);
	/* See README.displaying-bmps */
	aligned = (addr % 4 == 2);
	if (!aligned) {
		pr_err("Invalid splashimage value. Value must be 16 bit aligned, but not 32 bit aligned\n");
		return -1;
	}

	return 0;
}

U_BOOT_ENV_CALLBACK(splashimage, on_splashimage);
#endif

int lcd_get_pixel_width(void)
{
	return panel_info.vl_col;
}

int lcd_get_pixel_height(void)
{
	return panel_info.vl_row;
}
