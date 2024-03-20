// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_EARLYCAMERA_INTERRUPT
#define TCC_EARLYCAMERA_INTERRUPT

struct earlycam_board_info {
	unsigned int gear;
	unsigned int flag;
	unsigned int ma_ctrl;
	unsigned int timer_ms;
};

int tcc_earlycamera_initialize_irq(void);
void tcc_earlycamera_deinitialize_irq(void);
#endif // TCC_EARLY_CAM_INTERRUPT
