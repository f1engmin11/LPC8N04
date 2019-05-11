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

static void Init(void);
/* -------------------------------------------------------------------------
 * variables
 * ------------------------------------------------------------------------- */

static void Init(void)
{
    Chip_Clock_System_SetClockFreq(2 * 1000 * 1000);
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

uint16_t test_cnt = 0;
int main(void)
{
	Init();
	while(1)
	{
		test_cnt++;
	}
    return 0;
}
