/*
 * Copyright (c), NXP Semiconductors
 * (C)NXP B.V. 2014-2017
 * All rights are reserved. Reproduction in whole or in part is prohibited without
 * the written consent of the copyright owner. NXP reserves the right to make
 * changes without notice at any time. NXP makes no warranty, expressed, implied or
 * statutory, including but not limited to any implied warranty of merchantability
 * or fitness for any particular purpose, or that the use will not infringe any
 * third party patent, copyright or trademark. NXP must not be liable for any loss
 * or damage arising from its use.
 */
 
#include <string.h>
#include "board.h"
#include "ndeft2t/ndeft2t.h"
#include "tmeas/tmeas.h"
#include "timer.h"
#include "app_sel.h"

#define SYSTICK_INTERVAL (1000)// ms
#define CT16B_INTERVAL (1000)// ms


static void Init(void);
/* -------------------------------------------------------------------------
 * variables
 * ------------------------------------------------------------------------- */

static void Init(void)
{
	LPC_TIMER_T CT16B_Struct;

    Chip_Clock_System_SetClockFreq(2 * 1000 * 1000);

	while( SysTick_Config(2 * 1000 * SYSTICK_INTERVAL) )
	{
		;
	}

	// RTC
	Chip_RTC_DeInit(LPC_RTC);
	Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_RTC);// 使能RTC时钟
	Chip_RTC_Int_SetEnabledMask(LPC_RTC, RTC_INT_NONE);// RTC中断配置

	// CT16B
	Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_16TIMER0);
	
//	CT16B_Struct.EMR = 0;
//	CT16B_Struct.IR = 0;
//	CT16B_Struct.MCR = 0;
//	CT16B_Struct.MR = 0;
//	CT16B_Struct.PC = 0;
//	CT16B_Struct.PR = 0;
//	CT16B_Struct.PWMC = 0;
//	CT16B_Struct.RESERVED0 = 0;
//	CT16B_Struct.RESERVED1 = 0;
//	CT16B_Struct.TC = 0;
//	CT16B_Struct.TCR = 0;
//	Chip_TIMER_DeInit(CT16B_Struct, LPC_TIMER16_0);
	//Chip_TIMER_Init(LPC_TIMER16_0, CT16B_INTERVAL);
	//NVIC_EnableIRQ(LPC_TIMER16_0);

	// GPIO
    Board_Init();

    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)0, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)1, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)2, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)4, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)5, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)6, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)7, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)8, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)9, IOCON_FUNC_0 | IOCON_RMODE_INACT);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)10, IOCON_FUNC_2 | IOCON_RMODE_PULLDOWN);
    Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)11, IOCON_FUNC_2 | IOCON_RMODE_PULLDOWN);

    LPC_GPIO->DATA[0xFFF] = 0;
    LPC_GPIO->DIR = (LPC_GPIO->DIR & 0xFFF) | 0x3FF;
}

uint16_t test_systick_cnt = 0;
int rtc_time = 0;

void SysTick_Handler(void)
{
	test_systick_cnt++;

	rtc_time = Chip_RTC_Time_GetValue(LPC_RTC);
	Chip_GPIO_SetPinToggle(LPC_GPIO, 0, 6);
}

void CT16B0_IRQHandler(void)
{
	
}

void CT32B0_IRQHandler(void)
{
	
}

uint16_t test_cnt = 0;
int main(void)
{
	Init();

	Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, false);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 2, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 4, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 5, true);
	
	while(1)
	{
		test_cnt++;
	}
	//return 0;
}
