// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <test/ut.h>
#include <test/test.h>
#include <common.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <asm/arch/serial.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <watchdog.h>
#include <dm/platform_data/serial_pl01x.h>
#include "../../drivers/serial/serial_pl01x_internal.h"
#include <cpu_func.h>

#define ENABLE 1

#define PL011_BASE_REG 0x16600000
#define PL081_BASE_REG 0x16680000

#define IO_HRSTEN2 0x16051014

#define UART_PORT_CONF_DONE 0x1
#define UART_BAUD_CONF_DONE 0x2

#if defined(CONFIG_TCC803X)
#define TCC_UART_PORT_CFG 0x16684000
#elif defined(CONFIG_TCC805X)
#define TCC_UART_PORT_CFG 0x16689000
#endif

//TCC8059
//#define CTS_TEST_GPIO TCC_GPB(17)
//#define RTS_TEST_GPIO TCC_GPB(18)

//TCC8050
#define CTS_TEST_GPIO TCC_GPA(22)
#define RTS_TEST_GPIO TCC_GPA(23)

//Portmux test. Only choose one and at least, one should be chosen.
#define MAINCORE
//#define MAINCORE_CH0
//#define SUBCORE
//#define SUBCORE_CH4

extern void gpio_set(unsigned n, unsigned on);

int result=0;
char test_c = 'a';
char test_C = 'A';
int num_con=0;
unsigned int  cur_baudrate;

struct board_serial_data_test {
	unsigned int con;
	unsigned int cfg_id;
	unsigned int port_tx;
	unsigned int port_rx;
	unsigned int port_rts;
	unsigned int port_cts;
	unsigned int port_fn;
	unsigned int use_rtscts;
};

static struct board_serial_data_test portmux_test_serial[] = {
#if defined(MAINCORE)||defined(MAINCORE_CH0)
        {0, 0, TCC_GPSD0(11), TCC_GPSD0(12), TCC_GPSD0(13), TCC_GPSD0(14), GPIO_FN(7), 1},
        {0, 1, TCC_GPSD1(0), TCC_GPSD1(1), TCC_GPSD1(2), TCC_GPSD1(3), GPIO_FN(7), 1},
        {0, 2, TCC_GPSD1(6), TCC_GPSD1(7), TCC_GPSD1(8), TCC_GPSD1(9), GPIO_FN(7), 1},
        {0, 3, TCC_GPSD2(0), TCC_GPSD2(1), TCC_GPSD2(2), TCC_GPSD2(3), GPIO_FN(7), 1},
        {0, 4, TCC_GPSD2(6), TCC_GPSD2(7), TCC_GPSD2(8), TCC_GPSD2(9), GPIO_FN(7), 1},
        {0, 5, TCC_GPA(0), TCC_GPA(1), TCC_GPA(2), TCC_GPA(3), GPIO_FN(7), 1},
        {0, 6, TCC_GPA(6), TCC_GPA(7), TCC_GPA(8), TCC_GPA(9), GPIO_FN(7), 1},
        {0, 7, TCC_GPA(12), TCC_GPA(13), TCC_GPA(14), TCC_GPA(15), GPIO_FN(7), 1},
        {0, 8, TCC_GPA(18), TCC_GPA(19), TCC_GPA(20), TCC_GPA(21), GPIO_FN(7), 1},
        {0, 9, TCC_GPA(24), TCC_GPA(25), TCC_GPA(26), TCC_GPA(27), GPIO_FN(7), 1}, //run test on this port from main core platform. main core platform uses port 18 for debug console.
        {0, 10, TCC_GPB(0), TCC_GPB(1), TCC_GPB(2), TCC_GPB(3), GPIO_FN(7), 1},
        {0, 11, TCC_GPB(6), TCC_GPB(7), TCC_GPB(8), TCC_GPB(9), GPIO_FN(7), 1},
        {0, 12, TCC_GPB(12), TCC_GPB(13), TCC_GPB(14), TCC_GPB(15), GPIO_FN(7), 1},
	{0, 13, TCC_GPB(19), TCC_GPB(20), TCC_GPB(21), TCC_GPB(22), GPIO_FN(7), 1},
        {0, 14, TCC_GPB(25), TCC_GPB(26), TCC_GPB(27), TCC_GPB(28), GPIO_FN(7), 1},
        {0, 15, TCC_GPC(0), TCC_GPC(1), TCC_GPC(2), TCC_GPC(3), GPIO_FN(7), 1},
        {0, 16, TCC_GPC(4), TCC_GPC(5), TCC_GPC(6), TCC_GPC(7), GPIO_FN(7), 1},
        {0, 17, TCC_GPC(10), TCC_GPC(11), TCC_GPC(8), TCC_GPC(9), GPIO_FN(7), 1},
#endif
#if defined(SUBCORE)||defined(SUBCORE_CH4)
        {0, 18, TCC_GPC(10), TCC_GPC(11), TCC_GPC(12), TCC_GPC(13), GPIO_FN(8), 1}, //run test on this port from subcore platfrom. subcore platfrom uses port 9 for debug console.
#endif
#if defined(MAINCORE)||defined(MAINCORE_CH0)
        {0, 19, TCC_GPC(12), TCC_GPC(13), TCC_GPC(14), TCC_GPC(15), GPIO_FN(7), 1},
        {0, 20, TCC_GPC(16), TCC_GPC(17), TCC_GPC(18), TCC_GPC(19), GPIO_FN(7), 1},
        {0, 21, TCC_GPC(20), TCC_GPC(21), TCC_GPC(22), TCC_GPC(23), GPIO_FN(7), 1},
        {0, 22, TCC_GPC(26), TCC_GPC(27), TCC_GPC(28), TCC_GPC(29), GPIO_FN(7), 1},
        {0, 23, TCC_GPG(0), TCC_GPG(1), TCC_GPG(2), TCC_GPG(3), GPIO_FN(7), 1},
        {0, 24, TCC_GPG(7), TCC_GPG(8), TCC_GPG(9), TCC_GPG(10), GPIO_FN(7), 1},
        {0, 25, TCC_GPE(5), TCC_GPE(6), TCC_GPE(7), TCC_GPE(8), GPIO_FN(7), 1},
        {0, 26, TCC_GPE(11), TCC_GPE(12), TCC_GPE(13), TCC_GPE(14), GPIO_FN(7), 1},
        {0, 27, TCC_GPE(16), TCC_GPE(17), TCC_GPE(18), TCC_GPE(19), GPIO_FN(7), 1},
        {0, 28, TCC_GPH(4), TCC_GPH(5), TCC_GPH(6), TCC_GPH(7), GPIO_FN(7), 1},
        {0, 29, TCC_GPH(6), TCC_GPH(7), TCC_GPH(8), TCC_GPH(9), GPIO_FN(8), 1}, // FUNCTION WEIRD
        {0, 30, TCC_GPH(0), TCC_GPH(1), TCC_GPH(2), TCC_GPH(3), GPIO_FN(7), 1},
        {0, 31, TCC_GPMA(0), TCC_GPMA(1), TCC_GPMA(2), TCC_GPMA(3), GPIO_FN(7), 1},
        {0, 32, TCC_GPMA(6), TCC_GPMA(7), TCC_GPMA(8), TCC_GPMA(9), GPIO_FN(7), 1},
        {0, 33, TCC_GPMA(12), TCC_GPMA(13), TCC_GPMA(14), TCC_GPMA(15), GPIO_FN(7), 1},
        {0, 34, TCC_GPMA(18), TCC_GPMA(19), TCC_GPMA(20), TCC_GPMA(21), GPIO_FN(7), 1},
        {0, 35, TCC_GPMA(24), TCC_GPMA(25), TCC_GPMA(26), TCC_GPMA(27), GPIO_FN(7), 1},
        //GPMA_24, GPMA_25 should be assigned to AP by port selection register(0x1B936160)
        //{0, 36, TCC_GPK(9), TCC_GPK(10), 0, 0, GPIO_FN(2), 0}, // only used by MICOM(R5)
	{0, 37, TCC_GPMB(0), TCC_GPMB(1), TCC_GPMB(2), TCC_GPMB(3), GPIO_FN(7), 1},
	{0, 38, TCC_GPMB(6), TCC_GPMB(7), TCC_GPMB(8), TCC_GPMB(9), GPIO_FN(7), 1},
        {0, 39, TCC_GPMB(12), TCC_GPMB(13), TCC_GPMB(14), TCC_GPMB(15), GPIO_FN(7), 1},
	{0, 40, TCC_GPMB(18), TCC_GPMB(19), TCC_GPMB(20), TCC_GPMB(21), GPIO_FN(7), 1},
	{0, 41, TCC_GPMB(24), TCC_GPMB(25), TCC_GPMB(26), TCC_GPMB(27), GPIO_FN(7), 1},
	{0, 42, TCC_GPMC(0), TCC_GPMC(1), TCC_GPMC(2), TCC_GPMC(3), GPIO_FN(7), 1},
        {0, 43, TCC_GPMC(6), TCC_GPMC(7), TCC_GPMC(8), TCC_GPMC(9), GPIO_FN(7), 1},
        {0, 44, TCC_GPMC(12), TCC_GPMC(13), TCC_GPMC(14), TCC_GPMC(15), GPIO_FN(7), 1},
        {0, 45, TCC_GPMC(18), TCC_GPMC(19), TCC_GPMC(20), TCC_GPMC(21), GPIO_FN(7), 1},
        {0, 46, TCC_GPMC(24), TCC_GPMC(25), TCC_GPMC(26), TCC_GPMC(27), GPIO_FN(7), 1},
#endif

};

static struct board_serial_data_test board_test_serial[] = {


#if defined(CONFIG_TCC805X)||defined(CONFIG_TCC803X)
#if 0
	{0, 0, TCC_GPSD0(11), TCC_GPSD0(12), TCC_GPSD0(13), TCC_GPSD0(14), GPIO_FN(7), 1},
	{0, 1, TCC_GPSD1(0), TCC_GPSD1(1), TCC_GPSD1(2), TCC_GPSD1(3), GPIO_FN(7), 1},
	{0, 2, TCC_GPSD1(6), TCC_GPSD1(7), TCC_GPSD1(8), TCC_GPSD1(9), GPIO_FN(7), 1},

	{0, 3, TCC_GPSD2(0), TCC_GPSD2(1), TCC_GPSD2(2), TCC_GPSD2(3), GPIO_FN(7), 1},
	{0, 4, TCC_GPSD2(6), TCC_GPSD2(7), TCC_GPSD2(8), TCC_GPSD2(9), GPIO_FN(7), 1},

	{0, 5, TCC_GPA(0), TCC_GPA(1), TCC_GPA(2), TCC_GPA(3), GPIO_FN(7), 1},
	{0, 6, TCC_GPA(6), TCC_GPA(7), TCC_GPA(8), TCC_GPA(9), GPIO_FN(7), 1},
	{0, 7, TCC_GPA(12), TCC_GPA(13), TCC_GPA(14), TCC_GPA(15), GPIO_FN(7), 1},
	{0, 8, TCC_GPA(18), TCC_GPA(19), TCC_GPA(20), TCC_GPA(21), GPIO_FN(7), 1},
	{0, 9, TCC_GPA(24), TCC_GPA(25), TCC_GPA(26), TCC_GPA(27), GPIO_FN(7), 1},

	{0, 10, TCC_GPB(0), TCC_GPB(1), TCC_GPB(2), TCC_GPB(3), GPIO_FN(7), 1},
	{0, 11, TCC_GPB(6), TCC_GPB(7), TCC_GPB(8), TCC_GPB(9), GPIO_FN(7), 1},
	{0, 12, TCC_GPB(12), TCC_GPB(13), TCC_GPB(14), TCC_GPB(15), GPIO_FN(7), 1},
#endif
#if 1
	{0, 13, TCC_GPB(19), TCC_GPB(20), TCC_GPB(21), TCC_GPB(22), GPIO_FN(7), 1},
#endif
#if 0
	{0, 14, TCC_GPB(25), TCC_GPB(26), TCC_GPB(27), TCC_GPB(28), GPIO_FN(7), 1},

	{0, 15, TCC_GPC(0), TCC_GPC(1), TCC_GPC(2), TCC_GPC(3), GPIO_FN(7), 1},
	{0, 16, TCC_GPC(4), TCC_GPC(5), TCC_GPC(6), TCC_GPC(7), GPIO_FN(7), 1},
	{0, 17, TCC_GPC(10), TCC_GPC(11), TCC_GPC(8), TCC_GPC(9), GPIO_FN(7), 1},
	{0, 18, TCC_GPC(10), TCC_GPC(11), TCC_GPC(12), TCC_GPC(13), GPIO_FN(8), 1},
	{0, 19, TCC_GPC(12), TCC_GPC(13), TCC_GPC(14), TCC_GPC(15), GPIO_FN(7), 1},
	{0, 20, TCC_GPC(16), TCC_GPC(17), TCC_GPC(18), TCC_GPC(19), GPIO_FN(7), 1},
	{0, 21, TCC_GPC(20), TCC_GPC(21), TCC_GPC(22), TCC_GPC(23), GPIO_FN(7), 1},
	{0, 22, TCC_GPC(26), TCC_GPC(27), TCC_GPC(28), TCC_GPC(29), GPIO_FN(7), 1},

	{0, 23, TCC_GPG(0), TCC_GPG(1), TCC_GPG(2), TCC_GPG(3), GPIO_FN(7), 1},
	{0, 24, TCC_GPG(7), TCC_GPG(8), TCC_GPG(9), TCC_GPG(10), GPIO_FN(7), 1},

	{0, 25, TCC_GPE(5), TCC_GPE(6), TCC_GPE(7), TCC_GPE(8), GPIO_FN(7), 1},
	{0, 26, TCC_GPE(11), TCC_GPE(12), TCC_GPE(13), TCC_GPE(14), GPIO_FN(7), 1},
	{0, 27, TCC_GPE(16), TCC_GPE(17), TCC_GPE(18), TCC_GPE(19), GPIO_FN(7), 1},
	{0, 28, TCC_GPH(4), TCC_GPH(5), 0, 0, GPIO_FN(7), 0},
	{0, 29, TCC_GPH(6), TCC_GPH(7), 0, 0, GPIO_FN(7), 0}, // FUNCTION WEIRD
	{0, 30, TCC_GPH(0), TCC_GPH(1), TCC_GPH(2), TCC_GPH(3), GPIO_FN(7), 1},

	{0, 31, TCC_GPMA(0), TCC_GPMA(1), TCC_GPMA(2), TCC_GPMA(3), GPIO_FN(7), 1},
	{0, 32, TCC_GPMA(6), TCC_GPMA(7), TCC_GPMA(8), TCC_GPMA(9), GPIO_FN(7), 1},
	{0, 33, TCC_GPMA(12), TCC_GPMA(13), TCC_GPMA(14), TCC_GPMA(15), GPIO_FN(7), 1},
	{0, 34, TCC_GPMA(18), TCC_GPMA(19), TCC_GPMA(20), TCC_GPMA(21), GPIO_FN(7), 1},
	{0, 35, TCC_GPMA(24), TCC_GPMA(25), TCC_GPMA(26), TCC_GPMA(27), GPIO_FN(7), 1}, 
	//GPMA_24, GPMA_25 should be assigned to AP by port selection register(0x1B936160)
	{0, 36, TCC_GPK(9), TCC_GPK(10), TCC_GPK(11), TCC_GPK(12), GPIO_FN(2), 1},
#endif
#endif

};

static void tcc_test_serial_set_gpio(int con, struct board_serial_data_test *info)
{

	unsigned int portcfg_val;
#if defined(CONFIG_TCC805X)
	void __iomem *portcfg = (void __iomem *)TCC_UART_PORT_CFG;
#elif defined(CONFIG_TCC803X)
	void __iomem *portcfg = (void __iomem *)TCC_UART_PORT_CFG;
#endif

	if(info->cfg_id==0){
#if defined(MAINCORE)
		portcfg_val = 0xFFFFFF00|(readl(portcfg)&0xFF); //for main core and channels except 0
#elif defined(MAINCORE_CH0)
		portcfg_val = 0xFFFF00FF|(readl(portcfg)&0xFF00); // for main core and channel 0
#endif
#if defined(MAINCORE)||defined(MAINCORE_CH0)
		writel(portcfg_val, portcfg); // for main core
		writel(0xFFFFFFFF, portcfg+0x4); // for main core
#endif

#if defined(SUBCORE)
		portcfg_val = 0xFFFFFF00|(readl(portcfg+0x4)&0xFF); // for subcore and channels except 4
#elif defined(SUBCORE_CH4)
		portcfg_val = 0xFFFF00FF|(readl(portcfg+0x4)&0xFF00); // for subcore and channel 4
#endif
#if defined(SUBCORE)||defined(SUBCORE_CH4)
		writel(portcfg_val, portcfg+0x4); // for subcore
		writel(0xFFFFFFFF, portcfg); // for subcore
#endif

		writel(0xFFFFFFFF, portcfg+0x8);

	}

	if(con < 4) {
		portcfg_val = readl(portcfg);
		writel((portcfg_val & ~(0xFF<<(con*8)))
				| ((info->cfg_id&0xFF) << (con*8)) , portcfg);
	}
	else if (con < 8) {
		portcfg_val = readl(portcfg+0x4);
		writel((portcfg_val & ~(0xFF<<((con%4)*8)))
				| ((info->cfg_id&0xFF) << ((con%4)*8)) , portcfg+0x4);
	}

	gpio_config(info->port_tx, info->port_fn);
	if(info->cfg_id == 29)
		gpio_config(info->port_rx, GPIO_FN(8));
	else
		gpio_config(info->port_rx, info->port_fn);

#if defined(CONFIG_TCC803X)
	if(info->cfg_id == 35)
		writel(readl(0x1B936160)|0x3000000, 0x1B936160);
#endif

	if(info->use_rtscts ==1){
		gpio_config(info->port_rts, info->port_fn);
		gpio_config(info->port_cts, info->port_fn);
	}
}

struct board_serial_data_test *board_test_serial_info(unsigned int idx);

void serial_set_baud(int num)
{
	unsigned int fraction;
	unsigned int divider;
	unsigned int base_regs;
	int baudrate;
	char baudrate_index;


	printf("\nbaud rate config\n"
			"0 : 9600\n"
			"1 : 38400\n"
			"2 : 115200\n");
	printf("put baudrate index : ");

	baudrate_index = getc();

	printf("\n baudrate : %c\n", baudrate_index);

	switch(baudrate_index){
		case '0' :
			cur_baudrate = 9600;
			break;
		case '1' :
			cur_baudrate = 38400;
			break;
		case '2' :
			cur_baudrate = 115200;
			break;
		default :
			cur_baudrate = 115200;
			printf("\nno baudrate mathced. default : 115200\n");
	}

	baudrate = cur_baudrate;


	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	divider = 48000000/(16*baudrate);
	fraction = (((8*(48000000%(16*baudrate)))/baudrate) >> 1) + (((8*(48000000%(16*baudrate)))/baudrate) & 1);

	writel(divider, base_regs+0x24);
	writel(fraction, base_regs+0x28);


	printf("base_regs : %x baudrate : %d\n", base_regs, baudrate);



}

int tcc_serial_port_conf(int index){


	struct board_serial_data_test *info = NULL;
	char con_num;


	printf("\n port config test\n");
	printf("put channel : ");

	con_num = getc();

	printf("\n channel : %c\n", con_num);

	num_con = con_num - '0';

	if(num_con == 0){
		printf("\nDEBUG channel\n");
		return -1;
	}
#if defined(CONFIG_TCC803X)
	if((num_con <0)&&(num_con>4)){
		printf("\nno channel mathced\n");
		return -1;
	}
#elif defined(CONFIG_TCC805X)
	if((num_con <0)&&(num_con>7)){
		printf("\nno channel mathced\n");
	}
#endif





	info = &board_test_serial[index];
	if(info->cfg_id==17 || info->cfg_id==18){
		printf("\nDEBUG PORT\n");

	}else{

		printf("\nchannel : %d uart_index : %d\n", num_con, index);

		if(info){

			tcc_test_serial_set_gpio(num_con, info);
			tcc_set_iobus_swreset(IOBUS_UART0 + num_con, 1);
			tcc_set_iobus_pwdn(IOBUS_UART0 + num_con, 1);
			tcc_set_iobus_pwdn(IOBUS_UART0 + num_con, 0);
			tcc_set_iobus_swreset(IOBUS_UART0 + num_con, 0);

			tcc_set_peri(PERI_UART0 + num_con, ENABLE, 48000000, 0);
		}
	}

	return 0;

}


void delay_ms(int msec){

	int i;
	for(i=0 ; i < msec; i++)
		udelay(1000);

}

void serial_line_control_test(int num){

	unsigned int base_regs, test_val=0, i=0;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %x\n", base_regs);
	printf("Before beginning the test, Rx/Tx between PC <-> board and CTS/RTS on the board should be physically connected.\n");

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS, base_regs+0x30);

	writel(readl(base_regs+0x30) | UART_PL011_CR_CTSEN, base_regs+0x30);

	gpio_config(CTS_TEST_GPIO, GPIO_FN(0)|GPIO_OUTPUT);
	gpio_set(CTS_TEST_GPIO, 1);

	for(i=0; i<100; i++){
		writel(test_c, base_regs);

		if(test_c=='a'+3)
			test_c='a';
		else
			++test_c;
	}

	printf("\ncharacter mode test\n");
	printf("insert any keys to start character mode test.\n"
			"check only two characters are received from PC terminal\n");

	getc();

	gpio_set(CTS_TEST_GPIO, 0);

	delay_ms(100);

	writel('\n', base_regs);

	gpio_set(CTS_TEST_GPIO, 1);

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	for(i=0; i<100; i++){
		writel(test_c, base_regs);
		if(test_c=='a'+3)
			test_c='a';
		else
			++test_c;


	}

	printf("\n FIFO mode test\n");
	printf("insert any keys to start FIFO mode test.\n"
			"check multiple characters are received from PC terminal\n");

	getc();

	gpio_set(CTS_TEST_GPIO, 0);

	delay_ms(100);

	writel('\n', base_regs);

	printf("\n7-bit data test\n");
	printf("transfer 7-bit data to test port(PC terminal) and insert any keys to debug port(board terminal) to start word length test.\n"
			"check 5 characters are received from PC terminal\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_7 | UART_PL011_LCRH_FEN, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	writel('\n', base_regs);
	for(i=0; i<5; i++){
		writel(test_c, base_regs);
		if(test_c=='a'+3)
			test_c='a';
		else
			++test_c;

	}

	printf("\n8-bit data test\n");
	printf("transfer 8-bit data to test port(PC terminal) and insert any keys to debug port(board terminal) to start word length test.\n"
			"check 5 characters are recieved from PC terminal\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	writel('\n', base_regs);
	for(i=0; i<5; i++){
		writel(test_c, base_regs);
		if(test_c=='a'+3)
			test_c='a';
		else
			++test_c;

	}

	printf("\nodd parity test\n");
	printf("1. transfer 'a' with 'even' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal)\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(test_val)
		printf("> odd parity error test succeeded\n");
	else
		printf("> odd parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("2. transfer 'a' with 'odd' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal)\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(!test_val)
		printf("> odd parity error test succeeded\n");
	else
		printf("> odd parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("\neven parity test\n");
	printf("1. transfer 'a' with 'odd' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start even parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_EPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(test_val)
		printf("> even parity error test succeeded\n");
	else
		printf("> even parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("2. transfer 'a' with 'even' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start even parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_EPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(!test_val)
		printf("> even parity error test succeeded\n");
	else
		printf("> even parity error test failed\n");

	writel(0xf, base_regs+0x4);


	printf("\nmark parity test\n");
	printf("1. transfer 'a' with 'space' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start mark parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_SPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(test_val)
		printf("> mark parity error test succeeded\n");
	else
		printf("> mark parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("2. transfer 'a' with 'mark' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start mark parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_SPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(!test_val)
		printf("> mark parity error test succeeded\n");
	else
		printf("> mark parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("\nspace parity test\n");
	printf("1. transfer 'a' with 'mark' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start space parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_SPS | UART_PL011_LCRH_EPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(test_val)
		printf("> space parity error test succeeded\n");
	else
		printf("> space parity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("2. transfer 'a' with 'space' parity to test port(PC terminal) and"
			" insert any keys to debug port(board terminal) to start space parity test\n");

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_SPS | UART_PL011_LCRH_EPS, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(!test_val)
		printf("> space parity error test succeeded\n");
	else
		printf("> space parity error test failed\n");

	writel(0xf, base_regs+0x4);


	printf("\ntwo stop bits test\n"
			"check oscilloscope for one or two stop bits\n");


	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	printf("\n1. one stop bit transfer start(insert any keys)\n");
	getc();

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE, base_regs+0x30);

	writel('c', base_regs);
	writel('c', base_regs);

	printf("\n insert any keys to continue\n");
	getc();

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	printf("\n2. two stop bit transfer start(insert any keys)\n");
	getc();

	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_STP2, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE, base_regs+0x30);

	writel('c', base_regs);
	writel('c', base_regs);


	printf("\nbreak test\n"
			"insert any keys to debug port(board terminal) to start break test\n"
			"check oscilloscope for break signal\n");

	getc();

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN | UART_PL011_LCRH_BRK, base_regs+0x2C);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	printf("\nafter checking oscilloscope, insert any keys to debug port(board terminal) to finish break test\n");

	getc();



	if(test_c=='A'+3)
		test_c='A';
	else
		++test_c;



}


void serial_flag_register_test(int num){

	unsigned int base_regs, test_val=0, i=0;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %x\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS, base_regs+0x30);

	test_val = readl(base_regs+0x18);
	printf("\n flag reg val : 0x%x\n", test_val);

	test_val=readl(base_regs+0x18) & UART_PL01x_FR_TXFE;

	if(test_val)
		printf("\nTx FIFO empty test succeeded\n");
	else
		printf("\nTx FIFO empty test failed\n");

	test_val=readl(base_regs+0x18) & UART_PL01x_FR_RXFE;

	if(test_val)
		printf("\nRx FIFO empty test succeeded\n");
	else
		printf("\nRx FIFO empty test failed\n");

	for(i=0; i<100; i++){
		writel(test_c, base_regs);
	}

	delay_ms(100);

	test_val=readl(base_regs+0x18) & UART_PL01x_FR_TXFF;

	if(test_val)
		printf("\nTx FIFO full test succeeded\n");
	else
		printf("\nTx FIFO full test failed\n");

	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN | UART_PL011_CR_LPE, base_regs+0x30);

	delay_ms(100);

	test_val=readl(base_regs+0x18) & UART_PL01x_FR_RXFF;

	if(test_val)
		printf("\nRx FIFO full test succeeded\n");
	else
		printf("\nRx FIFO full test failed\n");

}

void serial_receive_status_test(int num)
{
	unsigned int base_regs, test_val=0;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %x\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_PEN, base_regs+0x2C); //odd parity setting
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS, base_regs+0x30);

	printf("\nreceive status test\n"
			"Rx/Tx between PC<->board should be physically connected before test\n");

	printf("\nparity error test\n"
			"transfer any non-parity packet to test port(PC terminal) and insert any key to debug port(board terminal)\n");

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_PE;

	if(test_val)
		printf("\nparity error test succeeded\n");
	else
		printf("\nparity error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("\nframe error test\n"
			"transfer any packet to test port(PC terminal) by baudrate which is not %d and insert any key to debug port(board terminal)\n", cur_baudrate);

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_FE;

	if(test_val)
		printf("\nframe error test succeeded\n");
	else
		printf("\nframe error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("\noverrun error test\n"
			"transfer more than 20 characters to test port(PC terminal) and insert any key to debug port(board terminal)\n");

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_OE;

	if(test_val)
		printf("\noverrun error test succeeded\n");
	else
		printf("\noverrun error test failed\n");

	writel(0xf, base_regs+0x4);

	printf("\nbreak error test\n"
			"send break to test port(PC terminal) and insert any key to debug port(board terminal)\n");

	getc();

	readl(base_regs);

	test_val = readl(base_regs+0x4) & UART_PL01x_RSR_BE;

	if(test_val)
		printf("\nbreak error test succeeded\n");
	else
		printf("\nbreak error test failed\n");




}
#define DMA_BUFF_RECEIVE_LLI 0x60000000
static char dma_buff_send_lli[5]={'A', 'B', 'C', 'D', 'E'};
unsigned char *dma_buff_receive_lli = DMA_BUFF_RECEIVE_LLI; // char type should be used for writeb/readb

struct lli {
	unsigned int src;
	unsigned int det;
	unsigned int next_lli;
	unsigned int con;
};

struct lli lli_test[4] = {

	{
		0,
		DMA_BUFF_RECEIVE_LLI+1,
		0,
		0x10012001
	},
	{
		0,
		DMA_BUFF_RECEIVE_LLI+2,
		0,
		0x10012001
	},
	{
		0,
		DMA_BUFF_RECEIVE_LLI+3,
		0,
		0x10012001
	},
	{
		0,
		DMA_BUFF_RECEIVE_LLI+4,
		0,
		0x10012001
	}
};

void serial_dma_lli_test(int num){

	int i = 0;
	unsigned int base_regs_pl081=0;
	char result_lli[5] = {0,};

	lli_test[0].src = &dma_buff_send_lli[1];
	lli_test[1].src = &dma_buff_send_lli[2];
	lli_test[2].src = &dma_buff_send_lli[3];
	lli_test[3].src = &dma_buff_send_lli[4];

	lli_test[0].next_lli = &lli_test[1];
	lli_test[1].next_lli = &lli_test[2];
	lli_test[2].next_lli = &lli_test[3];

	for(i = 0 ; i < 5 ; i++)
		writeb(0x0, dma_buff_receive_lli+i);

	//flush_dcache_all(); // flush_dcache_all for variables

	base_regs_pl081=PL081_BASE_REG+(0x1000*num);

	printf("base_regs_pl081 : 0x%x\n", base_regs_pl081);
	printf("lli : 0x%p, send lli : 0x%p, receive lli : 0x%p\n", (unsigned int)lli_test,(unsigned int)&dma_buff_send_lli, dma_buff_receive_lli);

	writel(0x1, base_regs_pl081+0x30); //UDMA enable

	writel((unsigned int)&dma_buff_send_lli, base_regs_pl081+0x100); //DMA source addr
	writel((unsigned int*)dma_buff_receive_lli, base_regs_pl081+0x104); //DMA destination addr
	writel((unsigned int)lli_test, base_regs_pl081+0x108);
	writel(0x19012001, base_regs_pl081+0x10c); // DMA control setting : protect(Privileged mode), destination increment, destination burst(8),
	// source burst(8), transfer size(1)
	writel(0xc007, base_regs_pl081+0x110); // channel configuration : enable (1), src peripheral(0x3), flow control(Memory-to-memory, 0x0)
	// IE(1), ITC(1)
	writel(0x8, base_regs_pl081+0x20); //DMA burst request for ch2

	delay_ms(4); // wait for DMA transfer

	for(i = 0 ; i < 5 ; i++){

		result_lli[i] = readb(dma_buff_receive_lli+i);
	}

	printf("\n\nsend lli : %s, receive lli : %s\n", dma_buff_send_lli, result_lli);


	writel(0x0, base_regs_pl081+0x30); // DMA disable


}


#if 0  // for UDMA secure wrapper test
void serial_rx_tx_test_dma(int num)

{
	unsigned int base_regs=0;
	unsigned int base_regs_pl081=0;
	char dma_buff_send=test_C;
	char dma_buff_receive='F';
	char data;

	flush_dcache_all(); // flush_dcache_all for variables

	base_regs_pl081=PL081_BASE_REG+(0x1000*num);

	if(num==0)
		base_regs=PL011_BASE_REG+(0x10000); // for UDMA channel 0 test, UART channel 1 is used
	else if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); // UART channel 8 addr : 0x16690000
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs_pl081 : %x %x\n", base_regs_pl081, base_regs);
	printf("send buff addr : %p, receive buffer addr %p\n", (unsigned int)&dma_buff_send, (unsigned int)&dma_buff_receive);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS | UART_PL011_CR_LPE, base_regs+0x30);

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);



	writel(0x1, base_regs_pl081+0x30); //UDMA enable

#if 0
	//1. UDMA : memory_1 to peri.

	//UDMA ch1 setting

	writel((unsigned int)&dma_buff_send, base_regs_pl081+0x120); //DMA source addr
	writel(base_regs, base_regs_pl081+0x124); //DMA destination addr

	writel(0x9601B001, base_regs_pl081+0x12c); //DMA control setting : terminal interrupt enable(not used),
	// protect(Privileged mode), source increment, destination burst(16), 
	// source burst(16), transfer size(1)
	writel(0xc881, base_regs_pl081+0x130); //channel configuration : enable(1), dst peripheral(0x2), flow control(Memory-to-peripheral, 0x1)
	//IE(1), ITC(1)

	writel(0x4, base_regs_pl081+0x20); //DMA burst request for ch1

	delay_ms(4); // wait time for DMA transfer


	data = readl(base_regs);
	printf("DMA sent data : %c\n", data); // loopback test : memory_1 -> peri Tx -> phy. loop back -> peri Rx.
#endif


	//2. UDMA : peri to memory_2.

	//UDMA ch2 setting

	//writel(base_regs, base_regs_pl081+0x100); //DMA source addr
	writel((unsigned int)&dma_buff_send, base_regs_pl081+0x100); //DMA source addr
	writel((unsigned int)&dma_buff_receive, base_regs_pl081+0x104); //DMA destination addr
	writel(0x19012001, base_regs_pl081+0x10c); // DMA control setting : protect(Privileged mode), destination increment, destination burst(8),
	// source burst(8), transfer size(1)
	writel(0xd007, base_regs_pl081+0x110); // channel configuration : enable (1), src peripheral(0x3), flow control(Peripheral-to-memory, 0x2)
	// IE(1), ITC(1)
	writel(0x8, base_regs_pl081+0x20); //DMA burst request for ch2

	delay_ms(4); // wait for DMA transfer


#if 0
	//3. UDMA : memory_2 to peri.

	//UDMA ch1 setting

	writel((unsigned int)&dma_buff_receive, base_regs_pl081+0x120); //DMA source addr
	writel(0x9601B001, base_regs_pl081+0x12c); //DMA control setting : terminal interrupt enable(not used),
	// protect(Privileged mode), source increment, destination burst(16),
	// source burst(16), transfer size(1)
	writel(0xc881, base_regs_pl081+0x130); //channel configuration : enable(1), dst peripheral(0x2), flow control(Memory-to-peripheral, 0x1)
	//IE(1), ITC(1)
	writel(0x4, base_regs_pl081+0x20); //DMA burst request for ch1

	delay_ms(4); // wait time for DMA transfer

	data = readl(base_regs);
	printf("DMA receive data : %c\n", data);
#endif
	if(test_C=='A'+3)
		test_C='A';
	else
		++test_C;

	writel(0x0, base_regs_pl081+0x30); // DMA disable



}

#else


void serial_rx_tx_test_dma(int num)

{
	unsigned int base_regs=0;
	unsigned int base_regs_pl081=0;
	char dma_buff_send=test_C;
	char dma_buff_receive='F';
	char data;

	flush_dcache_all(); // flush_dcache_all for variables

	base_regs_pl081=PL081_BASE_REG+(0x1000*num);

	if(num==0)
		base_regs=PL011_BASE_REG+(0x10000); // for UDMA channel 0 test, UART channel 1 is used
	else if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000); // UART channel 8 addr : 0x16690000
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs_pl081 : %x %x\n", base_regs_pl081, base_regs);
	printf("send buff addr : %p, receive buffer addr %p\n", (unsigned int)&dma_buff_send, (unsigned int)&dma_buff_receive);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS | UART_PL011_CR_LPE, base_regs+0x30);

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);



	writel(0x1, base_regs_pl081+0x30); //UDMA enable

	//1. UDMA : memory_1 to peri.

	//UDMA ch1 setting

	writel((unsigned int)&dma_buff_send, base_regs_pl081+0x120); //DMA source addr
	writel(base_regs, base_regs_pl081+0x124); //DMA destination addr

	writel(0x9601B001, base_regs_pl081+0x12c); //DMA control setting : terminal interrupt enable(not used),
	// protect(Privileged mode), source increment, destination burst(16), 
	// source burst(16), transfer size(1)
	writel(0xc881, base_regs_pl081+0x130); //channel configuration : enable(1), dst peripheral(0x2), flow control(Memory-to-peripheral, 0x1)
	//IE(1), ITC(1)

	writel(0x4, base_regs_pl081+0x20); //DMA burst request for ch1

	delay_ms(4); // wait time for DMA transfer


	data = readl(base_regs);
	printf("DMA sent data : %c\n", data); // loopback test : memory_1 -> peri Tx -> phy. loop back -> peri Rx.


	//2. UDMA : peri to memory_2.

	//UDMA ch2 setting

	writel(base_regs, base_regs_pl081+0x100); //DMA source addr
	writel((unsigned int)&dma_buff_receive, base_regs_pl081+0x104); //DMA destination addr
	writel(0x19012001, base_regs_pl081+0x10c); // DMA control setting : protect(Privileged mode), destination increment, destination burst(8),
	// source burst(8), transfer size(1)
	writel(0xd007, base_regs_pl081+0x110); // channel configuration : enable (1), src peripheral(0x3), flow control(Peripheral-to-memory, 0x2)
	// IE(1), ITC(1)
	writel(0x8, base_regs_pl081+0x20); //DMA burst request for ch2

	delay_ms(4); // wait for DMA transfer


	//3. UDMA : memory_2 to peri.

	//UDMA ch1 setting

	writel((unsigned int)&dma_buff_receive, base_regs_pl081+0x120); //DMA source addr
	writel(0x9601B001, base_regs_pl081+0x12c); //DMA control setting : terminal interrupt enable(not used),
	// protect(Privileged mode), source increment, destination burst(16),
	// source burst(16), transfer size(1)
	writel(0xc881, base_regs_pl081+0x130); //channel configuration : enable(1), dst peripheral(0x2), flow control(Memory-to-peripheral, 0x1)
	//IE(1), ITC(1)
	writel(0x4, base_regs_pl081+0x20); //DMA burst request for ch1

	delay_ms(4); // wait time for DMA transfer

	data = readl(base_regs);
	printf("DMA receive data : %c\n", data);
	if(test_C=='A'+3)
		test_C='A';
	else
		++test_C;

	writel(0x0, base_regs_pl081+0x30); // DMA disable



}

#endif

#define DMA_ACCESS_ADDR_S(c,n) *(volatile unsigned int*)(c*0x20+(0x16689020+(n*0x8)))
#define DMA_ACCESS_ADDR_L(c,n) *(volatile unsigned int*)(c*0x20+(0x16689024+(n*0x8)))

//#define DMA_ACCESS_DST 0xC0000010 // SRAM : non-cachable area. When printing result value of DMA transfer, the value in cached variable mapped to DRAM cannot be printed because value in cached variable would not updated after DMA transfer.

#define DMA_ACCESS_DST 0x60000100
#define DST_MARGIN		0x6

char dma_buff_send_access[32]={'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd', 'a', 'b', 'c', 'd'};
unsigned char *dma_buff_receive_access = DMA_ACCESS_DST; // char type should be used for writeb/readb

void serial_dma_access_test(int num)
{
	int i=0, j=0;
	unsigned int base_regs_pl081=0;
	char result_val[5] = {0,}; //should be char, not unsigned char for readb()
	int dst_margin; //default 5

	printf("\ntest channel : %d\n", num);

	printf("\ninsert destination margin : ");

	dst_margin = (getc()-'0');

	//src limit range
	DMA_ACCESS_ADDR_S(num, 0) = dma_buff_send_access;
	DMA_ACCESS_ADDR_L(num, 0) = dma_buff_send_access+0x20;

	for(i=1 ; i < 4 ; i++)
	{
		printf("\ntest access controller number : %d\n", i);

		DMA_ACCESS_ADDR_S(num, 1) = 0;
		DMA_ACCESS_ADDR_L(num, 1) = 0;

		DMA_ACCESS_ADDR_S(num, 2) = 0;
		DMA_ACCESS_ADDR_L(num, 2) = 0;

		DMA_ACCESS_ADDR_S(num, 3) = 0;
		DMA_ACCESS_ADDR_L(num, 3) = 0;

		//dst limit range
		DMA_ACCESS_ADDR_S(num, i) = DMA_ACCESS_DST+0x0;
		DMA_ACCESS_ADDR_L(num, i) = DMA_ACCESS_DST+dst_margin;

		//flush_dcache_all(); // flush_dcache_all for variables

		for(j = 0 ; j < 5 ; j++){
			writeb(0x0, dma_buff_receive_access + j);
			result_val[j] = 0;
		}


		base_regs_pl081=PL081_BASE_REG+(0x1000*num);

		writel(0x1, base_regs_pl081+0x30); //UDMA enable

		writel((unsigned int)&dma_buff_send_access, base_regs_pl081+0x100); //DMA source addr
		writel((unsigned int*)dma_buff_receive_access, base_regs_pl081+0x104); //DMA destination addr
		writel(0x1D03f020, base_regs_pl081+0x10c); // DMA control setting : protect(Privileged mode), destination increment, D/S burst(256), //D/SWidth(8), transfer size(32)
		writel(0xc007, base_regs_pl081+0x110); // channel configuration : enable (1), src peripheral(0x3), flow control(Memory-to-memory, 0x0)
		// IE(1), ITC(1)

		delay_ms(4); // wait for DMA transfer

		//flush_dcache_all();

		for(j = 0 ; j < dst_margin ; j++){
			result_val[j] = readb(dma_buff_receive_access + j);
		}

		printf("\nresult : %s\n", result_val);

		writel(0x0, base_regs_pl081+0x30); // DMA disable

	}

		DMA_ACCESS_ADDR_S(num, 0) = 0;
		DMA_ACCESS_ADDR_L(num, 0) = 0xffffffff;

		DMA_ACCESS_ADDR_S(num, 1) = 0;
		DMA_ACCESS_ADDR_L(num, 1) = 0;

		DMA_ACCESS_ADDR_S(num, 2) = 0;
		DMA_ACCESS_ADDR_L(num, 2) = 0;

		DMA_ACCESS_ADDR_S(num, 3) = 0;
		DMA_ACCESS_ADDR_L(num, 3) = 0;



}

unsigned int reset_test_val[] = {

	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000091,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000300,
	0x00000012,
	0x00000000,
	0x0000000d,
	0x00000000,
	0x00000000,
	0x00000000,

};

char serial_register_name[][100] = {

	"DR register\n",
	"Receive Status Register/Error Clear Register\n",
	"null\n",
	"null\n",
	"null\n",
	"null\n",
	"Flag Register\n",
	"null\n",
	"IrDA Low-Power Counter Register\n",
	"Integer Baud Rate Register\n",
	"Fractional Baud Rate Register\n",
	"Line Control Register\n",
	"Control Register\n",
	"Interrupt FIFO Level Select Register\n",
	"Interrupt Mask Set/Clear Register\n",
	"Raw Interrupt Status Register\n",
	"Masked Interrupt Status Register\n",
	"Interrupt Clear Register\n",
	"DMA Control Register\n",

};


void serial_reset_test(int num)
{
	unsigned int base_regs=0, io_hrsten2=0, i=0, j=0;

	if(num==8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	io_hrsten2 = IO_HRSTEN2;


	for(i = 0; i < sizeof(reset_test_val)/sizeof(unsigned int); i++)
	{
		writel(0xffffffff, base_regs+(i*4));
	}

	writel(0xFFFFFFFF&~(1<<num), io_hrsten2);

	delay_ms(100);

	writel(0xFFFFFFFF, io_hrsten2);

	for(i = 0; i < sizeof(reset_test_val)/sizeof(unsigned int); i++)
	{
		if(readl(base_regs+(i*4)) == reset_test_val[i]){
			j++;
		}
		else
			printf("\nserial reset test fail : %s\n", serial_register_name[i]);

	}

	if(j==sizeof(reset_test_val)/sizeof(unsigned int))
		printf("\nserial reset test success\n");

}


void serial_rx_tx_test_fc(int num)
{
	char data;
	unsigned int base_regs;
	int i=0;

	if(num==8)
		base_regs=PL011_BASE_REG+(0x90000); //UART0
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %x\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS, base_regs+0x30);

	gpio_config(CTS_TEST_GPIO, GPIO_FN(0)|GPIO_OUTPUT);
	gpio_set(CTS_TEST_GPIO, 1);

	writel(readl(base_regs+0x30) | UART_PL011_CR_CTSEN, base_regs+0x30);

	printf("\nFlow control test\n"
			"CTS<->RTS and Rx<->Tx are physically connected for loop back test\n");

	getc();

	writel(test_c, base_regs);

	printf("\nCTS low sent character : %c\n", test_c);

	data = readl(base_regs);
	readl(base_regs);
	printf("\nCTS low received character : %c\n", data);

	if(data==0){
		printf("\n                                            CTS low test Succeeded\n");
	}else{
		printf("\n                                            CTS low test Failed\n");
		result+=1;
	}

	gpio_set(CTS_TEST_GPIO, 0);

	writel(test_c, base_regs);
	printf("\nCTS high sent character : %c\n", test_c);
	data = readl(base_regs);
	printf("\nCTS high received character : %c\n", data);

	if(data==0){
		printf("\n                                            CTS high test failed\n");
		result+=1;
	}else{
		printf("\n                                            CTS high test Succeeded\n");
	}

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_CTSEN, base_regs+0x30);

	gpio_config(RTS_TEST_GPIO, GPIO_FN(0)|GPIO_INPUT);

	for(i=0; i<100; i++){
		writel(test_c, base_regs);
	}

	delay_ms(100);

	if(gpio_get(RTS_TEST_GPIO))
	{
		printf("\n                                            Disabled RTS failed\n");
	}else{
		printf("\n                                            Disabled RTS Succeeded\n");
	}

	writel(readl(base_regs+0x30) | UART_PL011_CR_RTSEN, base_regs+0x30);

	delay_ms(100);

	if(gpio_get(RTS_TEST_GPIO))
	{
		printf("\n                                            Enabled RTS Succeeded\n");
	}else{
		printf("\n                                            Enabled RTS failed\n");
	}



	if(test_c=='a'+3)
		test_c='a';
	else
		++test_c;




}


void serial_SIR_rx_tx_test(int num){

	char data;
	unsigned int base_regs;

	if(num==8)
		base_regs=PL011_BASE_REG+(0x90000);
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %d\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(0x4, base_regs+0x80);

	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS | UART_PL011_CR_SIREN | UART_PL011_CR_LPE, base_regs+0x30);

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	writel(test_c, base_regs);
	printf("sent character : %c\n", test_c);

	data = readl(base_regs);
	printf("received character : %c\n", data);

	printf("\ncompare sent : %c received : %c\n\n", test_c, data);
	if(test_c != data){
		printf("\n                                            echo fail\n");
		result+=1;
	}else{
		printf("\n                                            echo Success\n");
	}

	if(test_c=='a'+3)
		test_c='a';
	else
		++test_c;


}


void serial_DMA_request_test(int num){

	char data;
	unsigned int base_regs, test_val;
	int i;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000);
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %d\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(0x3, base_regs+0x48); //DMA request enable

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_RXE | UART_PL011_CR_RTS , base_regs+0x30);


	test_val = readl(base_regs+0x88);
	if(!(test_val&0x8000))
	{
		printf("DMA TX signle request bit is '0' : okay\n");
	}else
	{
		printf("DMA TX signle request bit isn't '0' : failed\n");
	}

	printf("TXE bit in control register is set\n");
	writel(readl(base_regs+0x30)|UART_PL011_CR_TXE, base_regs+0x30);

	delay_ms(500);

	test_val = readl(base_regs+0x88);
	if(test_val&0x8000)
	{
		printf("DMA TX signle request bit is '1' : okay\n\n");
	}else
	{
		printf("DMA TX signle request bit isn't '1' : failed\n\n");
	}

	test_val = readl(base_regs+0x88);
	if(!(test_val&0x4000))
	{
		printf("DMA TX bust request bit is '0' : okay\n");
	}else
	{
		printf("DMA TX bust request bit isn't '0' : failed\n");
	}

	printf("FEN bit in line control register is set\n");
	writel(readl(base_regs+0x2C)|UART_PL011_LCRH_FEN, base_regs+0x2C);

	delay_ms(500);

	test_val = readl(base_regs+0x88);
	if(test_val&0x4000)
	{
		printf("DMA TX bust request bit is '1' : okay\n\n");
	}else
	{
		printf("DMA TX bust request bit isn't '1' : failed\n\n");
	}

	writel(readl(base_regs+0x30)|UART_PL011_CR_LPE, base_regs+0x30); // loop back enable

	test_val = readl(base_regs+0x88);
	if(!(test_val&0x2000))
	{
		printf("DMA RX signle request bit is '0' : okay\n");
	}else
	{
		printf("DMA RX signle request bit isn't '0' : failed\n");
	}

	printf("store a character in Rx FIFO\n");
	writel(test_c, base_regs);

	delay_ms(500);

	test_val = readl(base_regs+0x88);
	if(test_val&0x2000)
	{
		printf("DMA RX signle request bit is '1' : okay\n\n");
	}else
	{
		printf("DMA RX signle request bit isn't '1' : failed\n\n");
	}


	test_val = readl(base_regs+0x88);
	if(!(test_val&0x1000))
	{
		printf("DMA RX bust request bit is '0' : okay\n");
	}else
	{
		printf("DMA RX bust request bit isn't '0' : failed\n");
	}

	printf("store 20 characters in Rx FIFO\n");
	for(i=0; i<20; i++){
		writel(test_c, base_regs);
	}

	delay_ms(500);

	test_val = readl(base_regs+0x88);
	if(test_val&0x1000)
	{
		printf("DMA RX bust request bit is '1' : okay\n\n");
	}else
	{
		printf("DMA RX bust request bit isn't '1' : failed\n\n");
	}


}

void serial_int_sig_test(int num){

	char data;
	unsigned int base_regs, test_val;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000);
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : 0x%x\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);
	writel(0x10, base_regs+0x38);

	writel(UART_PL011_LCRH_WLEN_8, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS | UART_PL011_CR_LPE , base_regs+0x30);

	test_val = readl(base_regs+0x88);
	if(!(test_val&0x40))
	{
		printf("INT sig bit is '0' : okay\n");
	}else
	{
		printf("INT sig bit isn't '0' : failed\n");
	}

	printf("store a character in Rx FIFO\n");
	writel(test_c, base_regs);

	delay_ms(500);

	test_val = readl(base_regs+0x88);
	if(test_val&0x40)
	{
		printf("INT sig bit is '1' : okay\n");
	}else
	{
		printf("INT sig bit isn't '1' : failed\n");
	}

}


void serial_rx_tx_test(int num){

	char data;
	unsigned int base_regs;

	if(num == 8)
		base_regs=PL011_BASE_REG+(0x90000);
	else
		base_regs=PL011_BASE_REG+(0x10000*num);

	printf("base_regs : %d\n", base_regs);

	writel(0x0, base_regs+0x30);
	writel(0x0, base_regs+0x2C);

	writel(UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN, base_regs+0x2C);
	writel(UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE | UART_PL011_CR_RTS | UART_PL011_CR_LPE, base_regs+0x30);

	writel(readl(base_regs+0x30) & ~UART_PL011_CR_UARTEN, base_regs+0x30);
	writel(readl(base_regs+0x30) | UART_PL011_CR_UARTEN, base_regs+0x30);

	writel(test_c, base_regs);
	printf("sent character : %c\n", test_c);

	data = readl(base_regs);
	printf("received character : %c\n", data);

	printf("\ncompare sent : %c received : %c\n\n", test_c, data);
	if(test_c != data){
		printf("\n                                            echo fail\n");
		result+=1;
	}else{
		printf("\n                                            echo Success\n");
	}

	if(test_c=='a'+3)
		test_c='a';
	else
		++test_c;


}

#define GPIO_VIRT_BASE		0x14200400
#define TCC_GPIO_GROUP(x)	(((x)&GPIO_REGMASK) >> GPIO_REG_SHIFT)
#define TCC_GPIO_NUM(x)		((x)&GPIO_BITMASK)
#define TCC_GPIO_BIT(x)		(1<<TCC_GPIO_NUM(x))
#define VIR_OEN			0x0
#define VIR_IDAT		0x4
#define VIR_OEN			0x8
#define VIR_IEN			0xc
#define VIR_MDO			0x10
#define VIR_MOEN		0x14
struct gpio_virt_addr {
	void __iomem *v;
};

struct gpio_virt_addr gpio_v[] = {
	{(void __iomem *)GPIO_VIRT_BASE+0x0},	//a
	{(void __iomem *)GPIO_VIRT_BASE+0x20},	//b
	{(void __iomem *)GPIO_VIRT_BASE+0x40},	//c
	{(void __iomem *)GPIO_VIRT_BASE+0x80},	//e
	{(void __iomem *)GPIO_VIRT_BASE+0xC0},	//g
	{(void __iomem *)GPIO_VIRT_BASE+0x160},	//h
	{(void __iomem *)GPIO_VIRT_BASE+0},	//k
	{(void __iomem *)GPIO_VIRT_BASE+0x100},	//sd0
	{(void __iomem *)GPIO_VIRT_BASE+0x120},	//sd1
	{(void __iomem *)GPIO_VIRT_BASE+0x140},	//sd2
	{(void __iomem *)GPIO_VIRT_BASE+0x1A0},	//ma
	{(void __iomem *)GPIO_VIRT_BASE+0x1C0},	//mb
	{(void __iomem *)GPIO_VIRT_BASE+0x1E0},	//mc
	{(void __iomem *)GPIO_VIRT_BASE+0x3C0},	//md
};

void serial_portmux_test(void){

	unsigned int base_regs;
	unsigned int gpio_gr;
	unsigned int gpio_bit;
	unsigned int check_data;
	struct board_serial_data_test *info = NULL;
	void __iomem *v_reg_base = (void __iomem *)GPIO_VIRT_BASE;

#if defined(MAINCORE)
	for(int i = 1 ; i < 8 ; i++){
                for(int j = 0 ; j < 44 ; j++)
                {
#elif defined(MAINCORE_CH0)
	for(int i = 0 ; i < 1 ; i++){ // for channel 0
                for(int j = 0 ; j < 44 ; j++)
                {
#endif
#if defined(SUBCORE)
	for(int i = 0 ; i < 8 ; i++){
		if(i == 4)
			continue;
		int j=0;
                {

#elif defined(SUBCORE_CH4)
	int i = 4;
	{
		int j=0;
                {
#endif
			info = &portmux_test_serial[j];
#if defined(MAINCORE)||defined(MAINCORE_CH0)
			if(info->cfg_id==17 || info->cfg_id==18){
				printf("\nDEBUG PORT\n");
			}else{
#else
			{
#endif
				delay_ms(10);
				check_data = 0;

				printf("\n\n\n\\\\\\\\ch : %d port : %d\\\\\\\\\n", i, info->cfg_id);

				base_regs=PL011_BASE_REG+(0x10000*i);

				tcc_test_serial_set_gpio(i, info);

				//get gpio group index from port_rx
				gpio_gr = TCC_GPIO_GROUP(info->port_rx);
				//get bit of gpio pin number from port_rx
				gpio_bit = TCC_GPIO_BIT(info->port_rx);
				//set Rx pin to '1' in virtual input enable and virtual input data registers.
				writel(readl(gpio_v[gpio_gr].v+VIR_IEN)|gpio_bit, gpio_v[gpio_gr].v+VIR_IEN);
				writel(readl(gpio_v[gpio_gr].v+VIR_IDAT)|gpio_bit, gpio_v[gpio_gr].v+VIR_IDAT);

				//get gpio group index from port_cts
				gpio_gr = TCC_GPIO_GROUP(info->port_cts);
				//get bit of gpio pin number from port_cts
				gpio_bit = TCC_GPIO_BIT(info->port_cts);
				//set CTS pin to '1' in virtual input enable and virtual input data registers.
				writel(readl(gpio_v[gpio_gr].v+VIR_IEN)|gpio_bit, gpio_v[gpio_gr].v+VIR_IEN);
				writel(readl(gpio_v[gpio_gr].v+VIR_IDAT)|gpio_bit, gpio_v[gpio_gr].v+VIR_IDAT);

				tcc_set_iobus_swreset(IOBUS_UART0 + i, 1);
				tcc_set_iobus_pwdn(IOBUS_UART0 + i, 1);
				delay_ms(10);
				tcc_set_iobus_pwdn(IOBUS_UART0 + i, 0);
				tcc_set_iobus_swreset(IOBUS_UART0 + i, 0);
				tcc_set_peri(PERI_UART0 + i, ENABLE, 48000000, 0);

				//set IBRD(0x24) and FBRD(0x28) register to '1'.
				writel(0x1, base_regs+0x24);
				writel(0x1, base_regs+0x28);
				//set LCR_H(0x2c) register to '1'.
				writel(0x1, base_regs+0x2c);
				//set CR(0x30) register to '0x301'.
				writel(0x301, base_regs+0x30);

				//get gpio group index from port_tx
				gpio_gr = TCC_GPIO_GROUP(info->port_tx);
				//get bit of gpio pin number from port_tx
				gpio_bit = TCC_GPIO_BIT(info->port_tx);

				delay_ms(100); // << This delay is necessary. If there is no delay,
						// UART is not ready from reset and Tx test will be failed since
						// Tx has not yet dropped to '0'.

				//Tx test//

				//check if Tx pin is '0' in virtual output monitor register
				check_data = (readl(gpio_v[gpio_gr].v+VIR_MDO)&gpio_bit);
				if(check_data)
				{
					printf("\nTx pin in virtual output monitor register is not '0' : fail\n");
				}else{
					//toggle BRK bit in LCR_H(0x2C) from '1' to '0'
					writel((readl(base_regs+0x2c)&~0x1), base_regs+0x2c);

					delay_ms(10);
					//check if Tx pin is '1' in virtual output monitor register
					check_data = (readl(gpio_v[gpio_gr].v+VIR_MDO)&gpio_bit);
					if(check_data)
					{
						printf("\nTx pin in virtual output monitor register is not '1' : success\n");
					}else{
						printf("\nTx pin in virtual output monitor register is not '0' : fail\n");
					}
				}

				//Rx test//

				//get gpio group index from port_rx
				gpio_gr = TCC_GPIO_GROUP(info->port_rx);
				//get bit of gpio pin number from port_rx
				gpio_bit = TCC_GPIO_BIT(info->port_rx);
				//unset Rx pin in virtual input data register
				writel(readl(gpio_v[gpio_gr].v+VIR_IDAT)&~gpio_bit, gpio_v[gpio_gr].v+VIR_IDAT);

				delay_ms(10);

				//check if BERIS is set in RIS(0x3C).
				check_data = readl(base_regs+0x3c)&(1<<9);

				if(check_data)
				{
					printf("\nBERIS is set : success\n");
				}else{
					printf("\nBERIS is not set : fail\n");
				}

				//RTS test//

				//get gpio group index from port_rts
				gpio_gr = TCC_GPIO_GROUP(info->port_rts);
				//get bit of gpio pin number from port_rts
				gpio_bit = TCC_GPIO_BIT(info->port_rts);

				//check if RTS pin is '1' in virtual output monitor register.
				check_data = (readl(gpio_v[gpio_gr].v+VIR_MDO)&gpio_bit);
				if(check_data)
				{
					//set RTS bit to '1' in CR(0x30).
					writel((readl(base_regs+0x30)|(1<<11)), base_regs+0x30);

					delay_ms(10);

					//check if RTS pin is '0' in virtual output monitor register.
					check_data = (readl(gpio_v[gpio_gr].v+VIR_MDO)&gpio_bit);
					if(check_data)
					{
						printf("\nRTS pin in virtual output monitor register is not '1' : fail\n");
					}else{
						printf("\nRTS pin in virtual output monitor register is not '0' : success\n");
					}
				}else{
					printf("\nRTS pin in virtual output monitor register is not '1' : fail\n");
				}

				//CTS test//

				//check if CTS bit is set to '0' in FR(0x18)
				check_data = readl(base_regs+0x18)&(1<<0);
				if(check_data){
					printf("\nCTS bit in flag register is '1' : fail\n");
				}else{
					//get gpio group index from port_cts
					gpio_gr = TCC_GPIO_GROUP(info->port_cts);
					//get bit of gpio pin number from port_cts
					gpio_bit = TCC_GPIO_BIT(info->port_cts);
					//unset CTS pin in virtual input data register.
					writel(readl(gpio_v[gpio_gr].v+VIR_IDAT)&~gpio_bit, gpio_v[gpio_gr].v+VIR_IDAT);

					delay_ms(1);

					//check if CTS bit is set to '1' in FR(0x18)
					check_data = readl(base_regs+0x18)&(1<<0);
					if(check_data){
						printf("\nCTS bit in flag register is '1' : success\n");
					}else{
						printf("\nCTS bit in flag register is '0' : fail\n");
					}

				}

			}
		}
	}

}



static int tcc_test_serial_init(void)
{
	char test_num;
	int test_done=0;

	dcache_disable();

	while(1){

		result = 0;
		printf("\n\n0. port config test & baud rate config test\n"
				"1. Rx/Tx test\n"
				"2. DMA test\n"
				"3. DMA lli test\n"
				"4. DMA access test\n"
				"5. flag register test\n"
				"6. serial SIR test\n"
				"7. serial DMA request test\n"
				"8. serial inerrupt signal test\n"
				"9. flow control test -> physical connection needed\n"
				"a. receive status -> physical connection needed\n"
				"b. line control test -> physical connection needed\n"
				"c. reset test\n"
				"d. test all\n"
				"e. portmux test\n"
				"\n\n");
		printf("put test num : ");
		test_num = getc();
		printf("\n test : %c\n", test_num);

		switch(test_num){

			case '0' :
				if(tcc_serial_port_conf(0))
					break;
				serial_set_baud(num_con);

				test_done=UART_PORT_CONF_DONE;
				break;

			case '1' :
				printf("\nRx/Tx test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_rx_tx_test(num_con);

				test_done=0;
				break;

			case '2' :
				printf("\nDMA test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				printf("\nport 0 test : 0\n"
						"\nelse : 1\n\n");
				test_num = getc();

				printf("\ninserted : %d\n", test_num);

				if(test_num == '0')
					serial_rx_tx_test_dma(0);
				else if(test_num == '1')
					serial_rx_tx_test_dma(num_con);
				else
					printf("\ninsert '0' or '1'\n\n");

				test_done=0;
				break;

			case '3' :
				printf("\nDMA lli test\n");
				printf("\ninsert controller number\n");
				num_con = getc();

				num_con -= '0';

				if((num_con > 7)||(num_con<0))
				{
					printf("\nchannel range is from 0 to 7\n");
					break;
				}

				serial_dma_lli_test(num_con);

				test_done=0;
				break;

			case '4' :
				printf("\nDMA access test\n");
				printf("\ninsert controller number\n");
				num_con = getc();

				num_con -= '0';

				if((num_con > 7)||(num_con<0))
				{
					printf("\nchannel range is from 0 to 7\n");
					break;
				}

				serial_dma_access_test(num_con);

				test_done=0;
				break;

			case '5' :
				printf("\nflag register test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_flag_register_test(num_con);

				test_done=0;
				break;

			case '6' :
				printf("\nserial SIR test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_SIR_rx_tx_test(num_con);

				test_done=0;
				break;
			case '7' :
				printf("\nserial DMA request test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_DMA_request_test(num_con);

				test_done=0;
				break;

			case '8' :
				printf("\ninterrupt signal test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_int_sig_test(num_con);

				test_done=0;
				break;

			case '9' :
				printf("\nflow control test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_rx_tx_test_fc(num_con);

				test_done=0;
				break;

			case 'a' :
				printf("\nreceive status test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_receive_status_test(num_con);

				test_done=0;
				break;

			case 'b' :
				printf("\nline control test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_line_control_test(num_con);

				test_done=0;
				break;

			case 'c' :
				printf("\nreset test\n");
				if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_reset_test(num_con);

				test_done=0;
				break;
			case 'd' :
				printf("\nall\n");
                                if(!(test_done&UART_PORT_CONF_DONE))
				{
					printf("\n port conf test should be done before this test\n");
					break;
				}

				serial_rx_tx_test(num_con);
				serial_rx_tx_test_dma(num_con);
				serial_dma_lli_test(num_con);
				serial_dma_access_test(num_con);
				serial_flag_register_test(num_con);
				serial_SIR_rx_tx_test(num_con);
				serial_DMA_request_test(num_con);
				serial_int_sig_test(num_con);
				serial_rx_tx_test_fc(num_con);
				serial_receive_status_test(num_con);
				serial_line_control_test(num_con);
				serial_reset_test(num_con);

				break;
			case 'e' :
				printf("\nportmux test\n");

				serial_portmux_test();

				break;

			default:
				return 0;
		}

		if(result > 0)
			return -1;
	}
}



static int driver_test_uart(struct unit_test_state *uts)
{
	ut_assertok(tcc_test_serial_init());

	return 0;
}

UNIT_TEST(driver_test_uart, 0, driver_test);
