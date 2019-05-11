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


#include "board.h"
#include "led/led.h"

void Board_Init(void)
{
    /* There is no pin muxing to do; the hardware defaults are ok.
     * Individual drivers will configure pins if they need them.
     */

    Chip_IOCON_Init(LPC_IOCON);/* 使能CON时钟 */
    Chip_GPIO_Init(LPC_GPIO);/* 使能GPIO时钟 */
    //LED_Init();
}
