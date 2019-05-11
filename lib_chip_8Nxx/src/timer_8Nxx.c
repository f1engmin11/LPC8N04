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


#include "chip.h"

/* -------------------------------------------------------------------------
 * public functions
 * ------------------------------------------------------------------------- */

/* Initialize a timer */
#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
void Chip_TIMER_Init(LPC_TIMER_T *pTMR, CLOCK_PERIPHERAL_T clk)
{
    Chip_Clock_Peripheral_EnableClock(clk);
}

/*  Shutdown a timer */
#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
void Chip_TIMER_DeInit(LPC_TIMER_T *pTMR, CLOCK_PERIPHERAL_T clk)
{
    Chip_Clock_Peripheral_DisableClock(clk);
}

/* Resets the timer terminal and prescale counts to 0 */
void Chip_TIMER_Reset(LPC_TIMER_T *pTMR)
{
    uint32_t reg;

    /* Disable timer, set terminal count to non-0 */
    reg = pTMR->TCR;
    pTMR->TCR = 0;
    pTMR->TC = 1;

    /* Reset timer counter */
    pTMR->TCR = TIMER_RESET;

    while (pTMR->TC != 0) {
        /* Wait for terminal count to clear */
    }

    /* Restore timer state */
    pTMR->TCR = reg;
}

/* Sets external match control (MATn.matchnum) pin control */
void Chip_TIMER_ExtMatchControlSet(LPC_TIMER_T *pTMR, int8_t initial_state, TIMER_PIN_MATCH_STATE_T matchState,
                                   int8_t matchnum)
{
    uint32_t mask;
    uint32_t reg;

    /* Clear bits corresponding to selected match register */
    mask = (1u << matchnum) | (0x03u << (4 + (matchnum * 2)));
    reg = pTMR->EMR &= ~mask;

    /* Set new configuration for selected match register */
    pTMR->EMR = reg | (((uint32_t)initial_state) << matchnum) | (((uint32_t)matchState) << (4 + (matchnum * 2)));
}

/* Sets PWM mode of external match pin. */
void Chip_TIMER_SetMatchOutputMode(LPC_TIMER_T *pTMR, int matchnum, LPC_TIMER_MATCH_OUTPUT_MODE_T mode)
{
    if (TIMER_MATCH_OUTPUT_PWM == mode) {
        pTMR->PWMC |= (1u << (matchnum & 0x1)); /* Enable PWM */
    }
    else {
        pTMR->PWMC &= ~(1u << (matchnum & 0x1));
    }
}

/* Get external match output mode.  */
LPC_TIMER_MATCH_OUTPUT_MODE_T Chip_TIMER_GetMatchOutputMode(LPC_TIMER_T *pTMR, int matchnum)
{
    return ((pTMR->PWMC & (1u << (matchnum & 0x1))) != 0) ? TIMER_MATCH_OUTPUT_PWM : TIMER_MATCH_OUTPUT_EMC;
}
