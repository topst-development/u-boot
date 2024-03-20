// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <test/ut.h>
#include <test/test.h>
#include <asm/io.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/gpio.h>
//#include <asm/telechips/irqs.h>
#include <irq_func.h>

#define ENABLE 1
#define DISABLE 0

unsigned int pll_3_val = 0;
unsigned int pll_3_sel = 0;

void ictc_interrupt_enable(int type);
void ictc_interrupt_disable(void);

#define test_mem 0x60000000


unsigned int ictc_vals[20][12] =
{
        //off
        {
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
                0x00000000,
        },
	//f-chg-int
	{
		0x80290000,
		0x203C0036,
		0x10000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//r-edge-int
	{
		0x802A0000,
		0x203C0036,
		0x40000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//f-edge-int
	{
		0x80290000,
		0x203C0036,
		0x80000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//df-cnt-full-int
	{
		0x802B0000,
		0x203C0036,
		0x20000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
        //d-chg-int
        {
                0x80290000,
                0x203D0036,
                0x08000000,
                0x00000000,
                0x00000064,
                0x00000064,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },

	//e-cnt-full-int
	{
		0x802A0000,
		0x203C0036,
		0x04000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x0000FFFF, //edge counter matching value defines how many count will generate edge counter full interrupt.
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//to-cnt-full-int
	{
		0x802C0000,
		0x203C0036,
		0x02000000,
		0x005B8D81, // for 1 sec
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//nf-cnt-full-int
	{
		0x802A0000,
		0x203C0036,
		0x01000000,
		0x00000000,
		0x0FFFFFFF,
		0x0FFFFFFF,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
	//time-stamp-cnt
	{
		0x80390000,
		0x203C0036,
		0x00000000,
		0x00000000,
		0x00000064,
		0x00000064,
		0x0FFFFFF0,
		0x0FFFFFF0,
		0x00000064,
		0x00000000,
		0x00000000,
		0x00000000,
	},
        //op-mode-test, filter-hold-mode
        {
                0x80290000,
                0x20140036,
                0x10000000,
                0x00000000,
                0x000004B0,
                0x000004B0,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
        //op-mode-test, filter-up-mode
        {
                0x80290000,
                0x20280036,
                0x10000000,
                0x00000000,
                0x00000708,
                0x00000258,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
        //op-mode-test, filter-down-mode
        {
                0x80290000,
                0x20280036,
                0x10000000,
                0x00000000,
                0x00000258,
                0x00000708,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
        //op-mode-test, filter-reset-mode
        {
                0x80290000,
                0x20000036,
                0x10000000,
                0x00000000,
                0x00000100,
                0x00000001,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
        //op-mode-test, filter-immediate-mode
        {
                0x80290000,
                0x203C0036,
                0x10000000,
                0x00000000,
                0x00000123,
                0x00000123,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
#if 1
        //d-chg-int_test
        {
                0x80290000,
                0x203F0036,
                0x18000000,
                0x00000000,
                0x00000064,
                0x00000064,
                0x0FFFFFF0,
                0x0FFFFFF0,
                0x00000064,
                0x00000000,
                0x00000000,
                0x00000000,
        },
#endif
	//dma_test
	{
		0x803B0000,
		0x203F0036,
		0xD8000000,
		0x00000000,
		0x00000064,
        0x00000064,
        0x0FFFFFF0,
        0x0FFFFFF0,
        0x00000064,
        0x00000000,
        0x00000000,
		0x00000000,
	},
	//hreset_test
	{
		0x803F001F,
		0x37FFF000,
		0xFF00FF00,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
	},



};

enum decimal_place{ d_1=1, d_2=10, d_3=100 };
int decimal[3] = {d_1, d_2, d_3};

volatile int get_value_from_user_999(unsigned int over_range){


	int val_conf[3], cnt, fail, val, i;


	val=0;
	fail=0;
	cnt=0;
	val_conf[0]=0;
	val_conf[1]=0;
	val_conf[2]=0;



	while(1){

		val = getc();

		printf("%c", val);

		if(val==0xd)
			break;

		if((val>=48)&&(val<=57)){
			val_conf[cnt]=(val-48);
			cnt++;
		}
		else
		{
			printf("invalid number\n");
			fail=1;
			return -1;
		}



		if(cnt>3)
			break;

	}


	if(fail==1)
		return -1;


	if(cnt==0){
		printf("\ninput a number\n");
		return -1;
	}

	val=0;

	for(i=0; i<cnt; i++){
		val+=val_conf[cnt-i-1]*decimal[i];
	}


	if(val>over_range-1){
		printf("\n over range\n");
		return -1;
	}

	return val;


}

int initial_pdm=0;

#define PDM_INFINITE_LOOP 0

#define PDM_1_HZ 0x5B8D80
#define PDM_10_HZ 0x927C0
#define PDM_50_HZ 0x1D4C0
#define PDM_100_HZ 0xEA60
#define PDM_500_HZ 0x2EE0
#define PDM_1_KHZ 0x1770
#define PDM_5_KHZ 0x4B0
#define PDM_10_KHZ 0x258
#define PDM_100_KHZ 0x3C
#define PDM_1_MHZ 0x4
#define PDM_22_P_369_SEC 0x10000000
#define PDM_3000_CNT_1K 0xBB8
#define PDM_9000_CNT_1K 0x2328

void pdm_high(void){

    long gpio_b_func_2 = 0x14200078;
    long pdm_reg = 0x16030000;
	long pdm_clock = 0x1400072C;
	long pdm_port_conf = 0x16051094;

	//PDM initalizae
	writel(readl(gpio_b_func_2)|0xA000000, gpio_b_func_2);
	/////////////////////////////////////////////////////////////

	if(pll_3_sel != 3){
		if(pll_3_sel == 1)
			tcc_set_pll(PLL_3,  ENABLE,  400000000, 0);
		else if(pll_3_sel == 2)
			tcc_set_pll(PLL_3,  ENABLE,  600000000, 0);

		writel(0x63000000, pdm_clock);
	} else {
		writel(0x65000000, pdm_clock);
	}

	/////////////////////////////////////////////////////////////
	writel(0x2B, pdm_port_conf);
	writel(0x100001, pdm_reg+0x8);
	writel(0x10, pdm_reg+0x4);

}

void pdm_increase_10_ictc_duty(int period){

	int i=0, usr_val_r=0, usr_val_f=0;
	long cur_duty_cnt = 0;
	long gpio_b_func_2 = 0x14200078;
	long ictc_addr = 0x16055000;
	long pdm_reg = 0x16030000;
        long pdm_clock = 0x1400072C;
	long pdm_port_conf = 0x16051094;


	//PDM initalizae
	writel(readl(gpio_b_func_2)|0xA000000, gpio_b_func_2);

//////////////////////////////////////////////////////////////
        if(pll_3_sel != 3){
                if(pll_3_sel == 1)
                        tcc_set_pll(PLL_3,  ENABLE,  400000000, 0);
                else if(pll_3_sel == 2)
                        tcc_set_pll(PLL_3,  ENABLE,  600000000, 0);

                writel(0x63000000, pdm_clock);
        } else {
                writel(0x65000000, pdm_clock);
        }

//////////////////////////////////////////////////////////////
	writel(0x2B, pdm_port_conf);
	writel(0x100001, pdm_reg+0x8);

	writel(0, pdm_reg+0xC);

	if(period<0x100){
		printf("period should be over 100");
		return;
	}


        writel(period, pdm_reg+0x10);
        writel(period, pdm_reg+0x14);

        writel(0x11, pdm_reg+0x4);

	mdelay(1000);

        writel(0x10001, pdm_reg+0x4);

	cur_duty_cnt = readl(ictc_addr+0x38);
	printf("current duty count : 0x%x\n", cur_duty_cnt);

	ictc_interrupt_enable(5); //P_CHG_INT

	for(i=1; i<0x10; i++){

	writel(period+i, pdm_reg+0x10);
	writel(period+i, pdm_reg+0x14);

	writel(0x11, pdm_reg+0x4);

	mdelay(300);

	writel(0x10001, pdm_reg+0x4);

        cur_duty_cnt = readl(ictc_addr+0x38);
        printf("current duty count : 0x%x\n", cur_duty_cnt);

	}

}


void pdm_setting(int loop, int period_r, int period_f, int initialized){

	int usr_val_r=0, usr_val_f=0, finite=0;
	long gpio_b_func_2 = 0x14200078;
	long pdm_reg = 0x16030000;
	long pdm_clock = 0x1400072C;
	long pdm_port_conf = 0x16051094;

	//PDM pin : GPB22, PDM number : 0x2B(43)

	//PDM initalizae
	writel(readl(gpio_b_func_2)|0xA000000, gpio_b_func_2);

	//////////////////
        if(pll_3_sel != 3){
                if(pll_3_sel == 1)
                        tcc_set_pll(PLL_3,  ENABLE,  400000000, 0);
                else if(pll_3_sel == 2)
                        tcc_set_pll(PLL_3,  ENABLE,  600000000, 0);

                writel(0x63000000, pdm_clock);
        } else {
                writel(0x65000000, pdm_clock);
        }

	//////////////////
	writel(0x2B, pdm_port_conf);
	writel(0x10001, pdm_reg+0x8);

	if(loop>0)
		finite=1;

	writel(loop, pdm_reg+0xC);

	writel(period_r, pdm_reg+0x10);
	writel(period_f, pdm_reg+0x14);

	if(finite){
		writel(0x10, pdm_reg+0x4);

		mdelay(1000);

		writel(0x10001, pdm_reg+0x4);
	}else{
		writel(0x11, pdm_reg+0x4);

		if(initialized == 0){
			mdelay(1000);
			writel(0x10001, pdm_reg+0x4);
		}
	}


}

void pdm_off(void){

	volatile int time = 5000;
	long pdm_reg = 0x16030000;
	long io_hreset = 0x1605100C;

	writel(0x0, pdm_reg+0x4);
	writel(0x0, pdm_reg+0x8);
	writel(0x0, pdm_reg+0xC);
	writel(0x0, pdm_reg+0x10);
	writel(0x0, pdm_reg+0x14);

	writel(0xFFFFEFFF, io_hreset);

        while(time){
                time-=1;
        }

	writel(0xFFFFFFFF, io_hreset);

	if(pll_3_sel != 3)
	tcc_set_pll(PLL_3,  ENABLE,  pll_3_val, 0);

}

#define CUR_P_VAL	0
#define PRE_P_VAL	1
#define CUR_D_VAL	2
#define PRE_D_VAL	3
#define CUR_F_E_VAL	4
#define PRE_F_E_VAL	5
#define R_E_T_VAL	6
#define F_E_T_VAL	7

#define DMA_CUR_P_VAL (0x8<<12)
#define DMA_PRE_P_VAL (0x9<<12)
#define DMA_CUR_D_VAL (0xA<<12)
#define DMA_PRE_D_VAL (0xB<<12)
#define DMA_CUR_F_E_VAL (0xC<<12)
#define DMA_PRE_F_E_VAL (0xD<<12)
#define DMA_R_E_T_VAL (0xE<<12)
#define DMA_F_E_T_VAL (0xF<<12)


unsigned int ictc_counter[8] = {

	0x16055030, //Counted previous period value
	0x16055034, //Counted 1 cycle before-previous period value
	0x16055038, //Counted previous duty value
	0x1605503C, //Counted 1 cycle before-previous duty value
	0x16055040, //Counted current falling edge count value
	0x16055044, //Counted previous falling edge count value
	0x16055048, //Counted rising edge timestamp count value
	0x1605504C //Counted falling edge timestamp count value


};

void ut_dma_setting(int type){

	long gdma1_src = 0x16020000;
	long gdma1_dst = 0x16020010;
	long gdma1_hcnt = 0x16020020;
	long gdma1_ctrl = 0x16020024;
	long gdma1_rsel = 0x1602002C;

	writel(ictc_counter[type], gdma1_src);
	writel(test_mem, gdma1_dst);
	writel(0x1, gdma1_hcnt);
	writel(0x3133, gdma1_ctrl);
	writel(0x80000000, gdma1_rsel);


}



#define ICTC_IRQ_NUM 129

#define ICTC_OFF 0
#define P_CHG_INT 1
#define R_EDGE_INT 2
#define F_EDGE_INT 3
#define D_P_CNT_FULL_INT 4
#define D_CHG_INT 5
#define E_CNT_FULL_INT 6
#define TO_CNT_FULL_INT 7
#define NF_CNT_FULL_INT 8
#define TIME_STAMP_CNT 9
#define FILTER_HOLD_MODE 10
#define FILTER_UP_MODE 11
#define FILTER_DOWN_MODE 12
#define FILTER_RESET_MODE 13
#define FILTER_IMMEDIATE_MODE 14
#define D_CHG_INT_TEST 15
#define DMA_TEST 16
#define HRESET_TEST 17

#define F_EDGE (1<<7)
#define R_EDGE (1<<6)
#define D_F_FULL (1<<5)
#define P_CHG (1<<4)
#define D_CHG (1<<3)
#define E_FULL (1<<2)
#define TIME_OUT (1<<1)
#define N_E_FULL (1<<0)




static void tcc_ictc_handler(void *data)
{
	long ictc_addr = 0x16055000;
	long ictc_int_addr = 0x16055008;
	unsigned int value = 0;

	value = readl(ictc_int_addr);
	printf("\n########################interrupt val before handling : 0x%x########################\n", value);

	value = readl(ictc_int_addr);
	printf("interrupt val read again : 0x%x\n", value);
	value &= 0xff;

        if(value&F_EDGE)
                printf("Falling edge interrupt!!\n");
        if(value&R_EDGE)
                printf("Rising edge interrupt!!\n");
        if(value&D_F_FULL)
                printf("Duty and period comparing counter full interrupt!!\n");
        if(value&P_CHG){

                printf("Period changing interrupt!!\n");
		printf("cur period : 0x%x, old period : 0x%x, cur duty 0x%x, old duty 0x%x\n", readl(ictc_addr+0x30), readl(ictc_addr+0x34), readl(ictc_addr+0x38), readl(ictc_addr+0x3C));
	}
        if(value&D_CHG){

                printf("Duty changing interrupt!!\n");
		printf("cur period : 0x%x, old period : 0x%x, cur duty 0x%x, old duty 0x%x\n", readl(ictc_addr+0x30), readl(ictc_addr+0x34), readl(ictc_addr+0x38), readl(ictc_addr+0x3C));
	}
        if(value&E_FULL)
                printf("Edge counter full interrupt!!\n");
        if(value&TIME_OUT)
                printf("Time-out counter full interrupt!!\n");
        if(value&N_E_FULL)
                printf("Noise-filter and edge detection counter full interrupt!!\n");

	value = readl(ictc_int_addr)|0x0000ff00;
	writel(value, ictc_int_addr);
	value = readl(ictc_int_addr)& ~0x0000ff00;
	writel(value, ictc_int_addr);


        value = readl(ictc_int_addr);
        printf("########################interrupt val after handling : 0x%x########################\n", value);

}

void ictc_interrupt_enable(int type){

        long ictc_addr = 0x16055000;

        irq_install_handler(ICTC_IRQ_NUM, tcc_ictc_handler, NULL);
        irq_unmask(ICTC_IRQ_NUM);
        writel(ictc_vals[type][2], ictc_addr+(0x8));

}

void ictc_interrupt_disable(void){

        long ictc_addr = 0x16055000;
        volatile int time = 5000;
        unsigned int value=0;
        long ictc_int_addr = 0x16055008;

	irq_free_handler(ICTC_IRQ_NUM);
	irq_mask(ICTC_IRQ_NUM);
	value = readl(ictc_int_addr)|0x0000ff00;
	writel(value, ictc_int_addr);
	while(time){
		time-=1;
	}
	value = readl(ictc_int_addr)& ~0x0000ff00;
	writel(value, ictc_int_addr);
	writel(0x0, ictc_addr+(0x8));
}



void ut_ictc_config_type(int type){

	unsigned int value=0;
	long ictc_clock = 0x140007A0;
	long ictc_addr = 0x16055000;
	long gpio_addr = 0x14200000;
        int i = 0;

        //irq_install_handler(ICTC_IRQ_NUM, tcc_ictc_handler, NULL);
        //irq_unmask(ICTC_IRQ_NUM);

	//ictc GPIO : GPB21
//////////////////////////////////////
        if(pll_3_sel != 3){
                if(pll_3_sel == 1)
                        tcc_set_pll(PLL_3,  ENABLE,  400000000, 0);
                else if(pll_3_sel == 2)
                        tcc_set_pll(PLL_3,  ENABLE,  600000000, 0);

                writel(0x63000000, ictc_clock);
        } else {
                writel(0x65000000, ictc_clock);
        }

//////////////////////////////////////

        value = 0;
        value=readl(gpio_addr+0x78)&~(0x00F00000);
        writel(value, gpio_addr+0x78);
        //printf("\nset ictc gpio 0x%lx val : 0x%x\n", gpio_addr+0x78, value);
        value=readl(gpio_addr+0x44)&~(0x00200000);
        writel(value, gpio_addr+0x44);
        //printf("set ictc gpio 0x%lx val : 0x%x\n", gpio_addr+0x44, value);
        value=readl(gpio_addr+0x64)|0x200000;
        writel(value, gpio_addr+0x64);
        //printf("set ictc gpio 0x%lx val : 0x%x\n", gpio_addr+0xa4, value);

        for(i=1; i<12; i++){
		if(i==2)
			continue;
                writel(ictc_vals[type][i], ictc_addr+(i*0x4));
                value=readl(ictc_addr+(i*0x4));
                //printf("set 0x%lx val : 0x%x\n", ictc_addr+(i*0x4), value);
        }

        writel(ictc_vals[type][0], ictc_addr);
        value=readl(ictc_addr);
        //printf("set 0x%lx val : 0x%x\n", ictc_addr, value);

	//printf("\n###################################################\n\n");



}

void ictc_off(void){


	printf("\n####################RESET##########################\n");
	ut_ictc_config_type(ICTC_OFF);
	ictc_interrupt_disable();

	initial_pdm=0;

	if(pll_3_sel != 3)
        tcc_set_pll(PLL_3,  ENABLE,  pll_3_val, 0);
}


void ut_ictc_opmode_test(void){

	long cur_duty_cnt = 0;
	long cur_period_cnt = 0;
	long ictc_addr = 0x16055000;

        printf("\n>>>>>>>>>>>>>>>>>starting absolute bit test\n\n");

        pdm_setting(PDM_INFINITE_LOOP, PDM_500_HZ, PDM_500_HZ, 0);
	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(D_CHG_INT);

	printf("\nabsolute bit : 1\n");

	mdelay(1000);

	pdm_increase_10_ictc_duty(0x505);

        ictc_off();
        pdm_off();

	mdelay(500);

        pdm_setting(PDM_INFINITE_LOOP, PDM_500_HZ, PDM_500_HZ, 0);
	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(D_CHG_INT);

	writel(readl(ictc_addr+0x4)&~(1<<29), ictc_addr+0x4);

	printf("\nabsolute bit : 0\n");

	mdelay(1000);

	pdm_increase_10_ictc_duty(0x505);

        ictc_off();
        pdm_off();

	printf("\n>>>>>>>>>>>>>>>>>starting hold mode filter test\n");

	mdelay(1000);

	printf("\n######################SET##########################\n\n");
	ut_ictc_config_type(FILTER_HOLD_MODE);
	pdm_setting(PDM_INFINITE_LOOP, PDM_10_KHZ, PDM_10_KHZ, 0);

	mdelay(1000);

	cur_duty_cnt = readl(ictc_addr+0x38);
	cur_period_cnt = readl(ictc_addr+0x30);
	cur_period_cnt -= cur_duty_cnt;

	printf("current duty count : 0x%x\n", cur_duty_cnt);
	printf("current period count : 0x%x\n", cur_period_cnt);

	printf("expected result =>\n"
		"current duty is about 0x708\n"
		"current period is about 0x708\n\n");

	mdelay(1000);

	pdm_off();
	ictc_off();

        printf("\n>>>>>>>>>>>>>>>>>starting up mode filter test\n\n");

	mdelay(1000);

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(FILTER_UP_MODE);
        pdm_setting(PDM_INFINITE_LOOP, PDM_5_KHZ, PDM_10_KHZ, 0);

	mdelay(1000);

        cur_duty_cnt = readl(ictc_addr+0x38);
        cur_period_cnt = readl(ictc_addr+0x30);
	cur_period_cnt -= cur_duty_cnt;

        printf("current duty count : 0x%x\n", cur_duty_cnt);
        printf("current period count : 0x%x\n", cur_period_cnt);

        printf("expected result =>\n"
                "current duty is about 0x258\n"
                "current period is about 0xbb8\n\n");

	mdelay(1000);

        pdm_off();
        ictc_off();

        printf("\n>>>>>>>>>>>>>>>>>starting down mode filter test\n\n");

	mdelay(1000);

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(FILTER_DOWN_MODE);
        pdm_setting(PDM_INFINITE_LOOP, PDM_10_KHZ, PDM_5_KHZ, 0);

	mdelay(1000);

        cur_duty_cnt = readl(ictc_addr+0x38);
        cur_period_cnt = readl(ictc_addr+0x30);
        cur_period_cnt -= cur_duty_cnt;

        printf("current duty count : 0x%x\n", cur_duty_cnt);
        printf("current period count : 0x%x\n", cur_period_cnt);

        printf("expected result =>\n"
                "current duty is about 0xbb8\n"
                "current period is about 0x258\n\n");

	mdelay(1000);

        pdm_off();
        ictc_off();

        printf("\n>>>>>>>>>>>>>>>>>starting reset mode filter test\n\n");

	mdelay(1000);

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(FILTER_RESET_MODE);
        pdm_setting(PDM_INFINITE_LOOP, PDM_500_HZ, PDM_500_HZ, 0);

	mdelay(1000);

        cur_duty_cnt = readl(ictc_addr+0x38);
        cur_period_cnt = readl(ictc_addr+0x30);
        cur_period_cnt -= cur_duty_cnt;

        printf("current duty count : 0x%x\n", cur_duty_cnt);
        printf("current period count : 0x%x\n", cur_period_cnt);

        printf("expected result =>\n"
                "current duty is about 0x2de1\n"
                "current period is about 0x2fdf\n\n");

	mdelay(1000);

        pdm_off();
        ictc_off();

        printf("\n>>>>>>>>>>>>>>>>>starting immediate mode filter test\n\n");

	mdelay(1000);

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(FILTER_IMMEDIATE_MODE);
        pdm_setting(PDM_INFINITE_LOOP, PDM_500_HZ, PDM_500_HZ, 0);

	mdelay(1000);

        cur_duty_cnt = readl(ictc_addr+0x38);
        cur_period_cnt = readl(ictc_addr+0x30);
        cur_period_cnt -= cur_duty_cnt;

        printf("current duty count : 0x%x\n", cur_duty_cnt);
        printf("current period count : 0x%x\n", cur_period_cnt);

        printf("expected result =>\n"
                "current duty is about 0x2ee0\n"
                "current period is about 0x2ee0\n\n");

	mdelay(1000);

        pdm_off();
        ictc_off();



}

void ut_p_chg_test(void){

	printf("\n>>>>>>>>>>>>>>>>>starting period change interrupt test\n\n");

	pdm_setting(PDM_INFINITE_LOOP, PDM_500_HZ, PDM_500_HZ, 0);

	printf("\n######################SET##########################\n\n");
	ut_ictc_config_type(P_CHG_INT);

	printf("\ngenerating interrupt\n");

	mdelay(1000);

	ictc_interrupt_enable(P_CHG_INT);

	pdm_setting(PDM_INFINITE_LOOP, PDM_1_KHZ, PDM_1_KHZ, 1);

	mdelay(1000);

	ictc_off();
	pdm_off();


}

void ut_r_edge_test(void){

        printf("\n>>>>>>>>>>>>>>>>>starting rising edge interrupt test\n\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(R_EDGE_INT);

	pdm_high();

	printf("\ngenerating interrupt\n");

	mdelay(1000);

	ictc_interrupt_enable(R_EDGE_INT);

        pdm_setting(3, PDM_1_HZ, PDM_1_HZ, 0);

	mdelay(500); //wait for rising edge of 1Hz PWM

	ictc_off();
        pdm_off();


}

void ut_f_edge_test(void){

        printf("\n>>>>>>>>>>>>>>>>>starting falling edge interrupt test\n\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(F_EDGE_INT);

	pdm_high();

	mdelay(1000);

	printf("\ngenerating interrupt\n");

	ictc_interrupt_enable(F_EDGE_INT);

        pdm_setting(3, PDM_1_HZ, PDM_1_HZ, 0);

	mdelay(200); //wait for falling edge of 1Hz PWM

        ictc_off();
        pdm_off();


}

void ut_d_p_full_test(void){


	int i = 0;
	//wait for about 22.269621.. that is calculated according to the formula, 268435455 * 0.0000000833333333.
	//0xFFFFFFF = 268435455 is maximum count of Period/duty compare counter
	//1/12000000 = 0.00000008333333333 is period of 12MHz.
	//12MHz is tclk of ICTC.

        printf("\n>>>>>>>>>>>>>>>>>starting duty and period comparing counter full interrupt test\n\n");

	printf("wait for about 22 secs\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(D_P_CNT_FULL_INT);

	mdelay(1000);

	ictc_interrupt_enable(D_P_CNT_FULL_INT);

        pdm_setting(3, PDM_22_P_369_SEC, PDM_22_P_369_SEC, 0);

	for(i=0;i<23;i++)
	mdelay(1000);

        ictc_off();
        pdm_off();


}

void ut_d_chg_test(void){

	int test_freq=0;
	int i =0;
	int h_cnt=0, l_cnt=0;

	test_freq=PDM_1_KHZ;

	printf("\n>>>>>>>>>>>>>>>>>starting duty change interrupt test\n\n"
			"test_freq : 0x%x, high count : 0x%x, low count : 0x%x", test_freq*2, h_cnt, l_cnt);

	printf("\nPDM initialize : 50%% duty\n");

	pdm_setting(PDM_INFINITE_LOOP, test_freq, test_freq, 0);

	printf("\n######################SET##########################\n\n");
	ut_ictc_config_type(D_CHG_INT_TEST);

	printf("\nictc interrupt enbale\n");

	ictc_interrupt_enable(D_CHG_INT_TEST);

	mdelay(3000);

	printf("\n10 %% linear increase mode test\n");


	for(i = 1; i<10; i++)
	{
		h_cnt=(test_freq*2)-(((test_freq*2)*i)/10);
		l_cnt=(test_freq*2)-h_cnt;

		mdelay(3000);

		printf("\n%d %% high : 0x%x low : 0x%x\n",i*10, h_cnt, l_cnt);

		pdm_setting(PDM_INFINITE_LOOP, h_cnt, l_cnt, 1);

	}

	ictc_off();
	pdm_off();


}

void ut_edge_full_test(void){

	//edge counter matching value defines how many count will generate edge counter full interrupt.
	//if edge counter matching value is set to 0xFFFF then edge counter full interrupt will be generated after about 13.107 secs.
	//the '13.107' is calculated with equation, 65535 * 0.0002.
	//65535 = 0xFFFF
	//0.0002 = 1/5000. 5KHz

	int i = 0;

        printf("\n>>>>>>>>>>>>>>>>>starting edge counter full interrupt test\n\n");

	printf("wait for about 13 secs\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(E_CNT_FULL_INT);

	mdelay(1000);

	ictc_interrupt_enable(E_CNT_FULL_INT);

        pdm_setting(PDM_INFINITE_LOOP, PDM_5_KHZ, PDM_5_KHZ, 0);

	for(i=0;i<14;i++)
	mdelay(1000);

        ictc_off();
        pdm_off();


}

void ut_to_full_test(void){


	int i = 0;

        printf("\n>>>>>>>>>>>>>>>>>starting time-out counter full interrupt test\n\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(TO_CNT_FULL_INT);

	mdelay(1000);

	ictc_interrupt_enable(TO_CNT_FULL_INT);

        pdm_setting(PDM_INFINITE_LOOP, PDM_1_HZ, PDM_1_HZ, 0);

        ictc_off();
        pdm_off();


}

void ut_noise_filter_full_test(void){

	//test enviroment : IMM mode, keeping both high level and low level of PWM signal for 22.269621.. which is enough time that the filter counter reaches its maximum count.
        //22.269621.. is calculated according to the formula, 268435455 * 0.0000000833333333.
        //0xFFFFFFF = 268435455 is maximum count of filter counter
        //1/12000000 = 0.00000008333333333 is period of 12MHz.
        //12MHz is tclk of ICTC.
	//
	//todo : this test should be done with hold and up/down mode.

	int i = 0;

        printf("\n>>>>>>>>>>>>>>>>>starting noise-filter counter full interrupt test\n\n");

	printf("wait for about 22 secs\n");

	printf("\n######################SET##########################\n\n");
        ut_ictc_config_type(NF_CNT_FULL_INT);

	mdelay(1000);

	ictc_interrupt_enable(NF_CNT_FULL_INT);

	pdm_setting(PDM_INFINITE_LOOP, PDM_22_P_369_SEC, PDM_22_P_369_SEC, 0);

	for(i=0;i<22;i++)
	mdelay(1000);

        ictc_off();
        pdm_off();


}

void ut_time_stamp_test(void){

	long value_f=0, value_r = 0, value=0;
	long ictc_addr = 0x16055000;

        printf("\n>>>>>>>>>>>>>>>>>starting time stamp test\n\n");

        ut_ictc_config_type(TIME_STAMP_CNT);


        pdm_setting(PDM_INFINITE_LOOP, PDM_1_KHZ, PDM_1_KHZ, 0);


	mdelay(1000);

	value_r = readl(ictc_addr+0x48);
	value_f = readl(ictc_addr+0x4C);
	if(value_r>value_f){
	value = value_r-value_f;
	printf("Rising time stamp - falling time stamp = 0x%x\n", value);
	value = readl(ictc_addr+0x38);
	printf("duty val : 0x%x\n",value);
	}else if(value_r<value_f){
        value = value_f-value_r;
	printf("Falling time stamp - rising time stamp = 0x%x\n", value);
	value = readl(ictc_addr+0x38);
        printf("duty val : 0x%x\n",value);
	}

        ictc_off();
        pdm_off();



}

void ut_dma_test(void){

	int i = 0;
	long ictc_addr = 0x16055000;
	long ictc_ctrl = ictc_addr+0x4;

	printf("\n>>>>>>>>>>>>>>>>>starting dma test\n\n");

	dcache_disable();
// Counted previous period value
	ut_dma_setting(CUR_P_VAL);
	ut_ictc_config_type(DMA_TEST);
	//pdm_setting(5, PDM_1_KHZ, PDM_1_KHZ, 0);
	//mdelay(100);
	writel(readl(ictc_ctrl)|DMA_CUR_P_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(3, PDM_5_KHZ, PDM_5_KHZ, 0);
	mdelay(1000);

	printf("current period val : 0x%x, DMA val : 0x%x", readl(ictc_counter[CUR_P_VAL]), readl(test_mem));

    ictc_off();
    pdm_off();
// Counted 1 cycle before-previous period value
	ut_dma_setting(PRE_P_VAL);
	ut_ictc_config_type(DMA_TEST);
	//pdm_setting(5, PDM_5_KHZ, PDM_5_KHZ, 0);
	//mdelay(100);
	writel(readl(ictc_ctrl)|DMA_PRE_P_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(3, PDM_1_KHZ, PDM_1_KHZ, 0);
	mdelay(1000);

	printf("previous period val : 0x%x, DMA val : 0x%x", readl(ictc_counter[PRE_P_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();
#if 0 // DMA duty mode is not valid anymore.
// Counted previous duty value
	ut_dma_setting(CUR_D_VAL);
	ut_ictc_config_type(DMA_TEST);
	//pdm_setting(5, PDM_1_KHZ, PDM_1_KHZ, 0);
	//mdelay(100);
	writel(readl(ictc_ctrl)|DMA_CUR_D_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(4, PDM_10_KHZ, PDM_10_KHZ, 0);
	mdelay(1000);

	printf("current duty val : 0x%x, DMA val : 0x%x", readl(ictc_counter[CUR_D_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();

// Counted 1 cycle before-previous duty value
	ut_dma_setting(PRE_D_VAL);
	ut_ictc_config_type(DMA_TEST);
	//pdm_setting(5, PDM_1_KHZ, PDM_1_KHZ, 0);
	//mdelay(100);
	writel(readl(ictc_ctrl)|DMA_PRE_D_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(4, PDM_10_KHZ, PDM_10_KHZ, 0);
	mdelay(1000);

	printf("previous duty val : 0x%x, DMA val : 0x%x", readl(ictc_counter[PRE_D_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();
#endif
// Counted current falling edge count value

	ut_dma_setting(CUR_F_E_VAL);
	ut_ictc_config_type(DMA_TEST);
	writel(readl(ictc_ctrl)|DMA_CUR_F_E_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(5, PDM_100_KHZ, PDM_100_KHZ, 0);
	mdelay(1000);

	printf("current falling edge count : 0x%x, DMA val : 0x%x", readl(ictc_counter[CUR_F_E_VAL]), readl(test_mem));


	ictc_off();
	pdm_off();
// Counted previous falling edge count value

	ut_dma_setting(PRE_F_E_VAL);
	ut_ictc_config_type(DMA_TEST);
	writel(readl(ictc_ctrl)|DMA_PRE_F_E_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(5, PDM_100_KHZ, PDM_100_KHZ, 0);
	mdelay(1000);

	printf("previous falling edge count : 0x%x, DMA val : 0x%x", readl(ictc_counter[PRE_F_E_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();
// Counted rising edge timestamp count value

	ut_dma_setting(R_E_T_VAL);
	ut_ictc_config_type(DMA_TEST);
	writel(readl(ictc_ctrl)|DMA_R_E_T_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(1, PDM_100_KHZ, PDM_100_KHZ, 0);
	mdelay(1000);

	printf("rising edge time stamp : 0x%x, DMA val : 0x%x", readl(ictc_counter[R_E_T_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();
// Counted falling edge timestamp count value

	ut_dma_setting(F_E_T_VAL);
	ut_ictc_config_type(DMA_TEST);
	writel(readl(ictc_ctrl)|DMA_F_E_T_VAL, ictc_ctrl);

	mdelay(1000);
	pdm_setting(1, PDM_100_KHZ, PDM_100_KHZ, 0);
	mdelay(1000);

	printf("falling edge time stamp : 0x%x, DMA val : 0x%x", readl(ictc_counter[F_E_T_VAL]), readl(test_mem));

	ictc_off();
	pdm_off();
}

#define ICTC_HRESET_STATE	0xFFFFFFFE
#define ICTC_HRESET_BYPASS	0xFFFFFFFF

unsigned int ictc_hreset_val[9] = {

	0x00000000,
	0x00020000,
	0xFF000000,
	0x0FFFFFFF,
	0x0FFFFFFF,
	0x0FFFFFFF,
	0x0FFFFFFF,
	0x0FFFFFFF,
	0x0000FFFF,

};

char ictc_hreset_string[][100] = {

	"\nHRESET test : Operation enable register okay\n",
	"\nHRESET test : Operation mode control register okay\n",
	"\nHRESET test : Interrupt control and status register okay\n",
	"\nHRESET test : Time-out value configuration register okay\n",
	"\nHRESET test : Rising edge matching value configuration register okay\n",
	"\nHRESET test : Falling edge matching value configuration register okay\n",
	"\nHRESET test : Period compare rounding value configuration register okay\n",
	"\nHRESET test : Duty compare rounding value configuration register okay\n",
	"\nHRESET test : Edge counter matching value configuration register okay\n",

};

void ut_hreset_test(){

	int i = 0;
	long ictc_reg = 0x16055000;
	long hrsten0 = 0x1605100C;
	long ictc_test_val = 0;


	printf("\n>>>>>>>>>>>>>>>>>starting hreset test\n\n");

	ut_ictc_config_type(HRESET_TEST);
	mdelay(1000);
	writel(0xFFFFFFFE, hrsten0);
	mdelay(1000);
	writel(0xFFFFFFFF, hrsten0);

	for(i = 0 ; i < sizeof(ictc_hreset_val)/sizeof(unsigned int) ; i++)
	{
		ictc_test_val = readl(ictc_reg + (0x4 * i));
		if(ictc_hreset_val[i]  == ictc_test_val)
			printf("%s", ictc_hreset_string[i]);
		else
			printf("\nICTC HRESET test fail\n");
		ictc_test_val = 0;
	}

	ictc_off();

}



void ut_ictc_selection(void){


	unsigned int value=0;

	/*Do not use mdelay() which resets TCFG1 and TCFG3*/

	pll_3_val = tcc_get_pll(PLL_3);

	printf("\nPLL 3 : %d\n", pll_3_val);


	printf("PLL sel\n"
		"PLL 200Mhz : 1\n"
		"PLL 300Mhz : 2\n"
		"PLL 24Mhz  : 3\n"
		"caution : The values used in this test are based on 24Mhz.\n"
		"	  If the test runs on other frequency, the result can be different\n");


	pll_3_sel = getc();

	pll_3_sel -= '0';

	if((pll_3_sel<0)||(pll_3_sel>3))
	{
		printf("\npll sel range : 1~3\n");
		return -1;
	}

	printf("PLL sel : %d\n", pll_3_sel);


	printf(
			"filter mode test : 0\n"
			"period change interrupt test : 1\n"
			"rising edge interrupt test : 2\n"
			"falling edge interrupt test : 3\n"
			"duty and period counter full interrupt test : 4\n"
			"duty change interrupt test : 5\n"
			"edge counter full interrupt test : 6\n"
			"timeout counter full interrupt : 7\n"
			"noise filter and edge detection counter full interrupt test : 8\n"
			"time stamp counter test : 9\n"
			"DMA test : 10\n"
			"HRESET test : 11\n"
			"full test : 12\n"
	      );



	printf("insert number : ");

	value = get_value_from_user_999(100);
	printk("\ninserted : %d\n", value);

	ictc_interrupt_disable();

	switch(value){

		case 0:
			ut_ictc_opmode_test();
			break;
		case 1:
			ut_p_chg_test();
			break;
		case 2:
			ut_r_edge_test();
			break;
		case 3:
			ut_f_edge_test();
			break;
		case 4:
			ut_d_p_full_test();
			break;
		case 5:
			ut_d_chg_test();
			break;
		case 6:
			ut_edge_full_test();
			break;
		case 7:
			ut_to_full_test();
			break;
		case 8:
			ut_noise_filter_full_test();
			break;
		case 9:
			ut_time_stamp_test();
			break;
		case 10:
			ut_dma_test();
			break;
		case 11:
			ut_hreset_test();
			break;
		case 12:
			ut_ictc_opmode_test();
			ut_p_chg_test();
			ut_r_edge_test();
			ut_f_edge_test();
			ut_d_p_full_test();
			ut_d_chg_test();
			ut_edge_full_test();
			ut_to_full_test();
			ut_noise_filter_full_test();
			ut_time_stamp_test();
			ut_dma_test();
			ut_hreset_test();
			break;

		default :
			break;
	}



}

static int ut_tcc_ictc(void)
{
	int finite_mode=0;
	int usr_val=0;
	char ch = 0;

	/*Do not use mdelay() which resets TCFG1 and TCFG3*/

	ut_ictc_selection();


	return 0;
}

static int driver_test_ictc(struct unit_test_state *uts)
{

	ut_assertok(ut_tcc_ictc());

	return 0;
}

UNIT_TEST(driver_test_ictc, 0, driver_test);
