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

/* -------------------------------------------------------------------------
 * defines
 * ------------------------------------------------------------------------- */

/**
 * Just assign a reasonable number. It must accommodate for all the overhead that comes with the complete ndef
 * message, plus it must be a multiple of 4.
 */
#define MAX_COMMAND_MESSAGE_SIZE 512

/**
 * Allow the - assumed - corresponding host to issue a first command in the given time window.
 * A few seconds also allows for easier grabbing hold of a debug session.
 */
#define FIRST_HOST_TIMEOUT 100

/**
 * We're assuming the host will continually exchange commands and responses. If after some timeout no command
 * has been received, we shut down abort communication. When a field is still present, we will automatically
 * wake up again from Deep Power Down and refresh the NFC shared memory with a new initial message.
 * This way we prevent a possible hang-up when both sides are waiting indefinitely for an NDEF message to be
 * written by the other side.
 * No need to set the timeout too strict: set it reasonably long enough to never hamper any execution flow,
 * while still being short enough to re-enable communication from a user perspective.
 */
#define HOST_TIMEOUT 100

/**
 * From what was observed, the power off/power on/(re-)select sequence takes place in the order of a few 100ms
 * at most. Waiting a full second seems more than enough to also take small changes in physical placement into
 * account.
 */
#define LAST_HOST_TIMEOUT 100



#define LED_REFRESH_RATE_MS 	(4U)
#define LED_DISP_REFRESH_STEPS	(5U)
#define MAX_DISP_CHARS			(130U)

#define EE_HEADER_SIZE		(4U)
#define EE_PAGE_SIZE		(64U)

typedef enum {
	EE_DISP_TEXT = 0xA1,
	EE_SCROLL_SPEED,
	EE_MUSIC_TONE,
	EE_TEMP_LOG,
	EE_LAST_RESERVED
}EE_TAG_t;

typedef struct {
	uint16_t note;
	uint16_t halfPeriodUs;
}notes_t;

/* -------------------------------------------------------------------------
 * function prototypes
 * ------------------------------------------------------------------------- */

static void Init(void);
static void DeInit(void);
static void stopMusic(void);

/** Application's main entry point. Declared here since it is referenced in ResetISR. */
int main(void);

/* -------------------------------------------------------------------------
 * variables
 * ------------------------------------------------------------------------- */

static volatile bool sTargetWritten = false;
static volatile bool nfcOn = false;
static volatile bool displayInProgress = false;
static volatile bool updateNFC = false;
static bool musicInProgress = false;
static bool musicDataValid = false;


__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
static uint8_t sData[MAX_COMMAND_MESSAGE_SIZE];

__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
static uint8_t sNdefInstance[NDEFT2T_INSTANCE_SIZE];

char g_displayText[MAX_DISP_CHARS] = "";
uint8_t g_displayTextLen;
char g_tempText[20] = " Temp is 00.0 F ";
volatile int32_t g_currentTemp = 0;
volatile bool g_updateTemp = false;
volatile uint8_t g_ledPattern[6*MAX_DISP_CHARS];
volatile uint8_t dispStep = 0;
volatile uint8_t scanCount = 0;
volatile uint32_t startPatternIndex = 0;
uint8_t desiredSpeed;
volatile uint32_t sizeLEDPattern;

volatile uint8_t switchDebounceCnt;
volatile bool switchPressed;
volatile bool changeFont;

const uint8_t asciiFont55[][6] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x2E, 0x00, 0x00, 0x00},
		{0x00, 0x06, 0x00, 0x06, 0x00, 0x00},
		{0x14, 0x3E, 0x14, 0x3E, 0x14, 0x00},
		{0x2C, 0x2A, 0x3E, 0x2A, 0x1A, 0x00},
		{0x22, 0x10, 0x08, 0x04, 0x22, 0x00},
		{0x18, 0x24, 0x2A, 0x36, 0x30, 0x00},
		{0x00, 0x00, 0x04, 0x02, 0x00, 0x00},
		{0x00, 0x00, 0x1C, 0x22, 0x00, 0x00},
		{0x00, 0x00, 0x22, 0x1C, 0x00, 0x00},
		{0x00, 0x2A, 0x1C, 0x1C, 0x2A, 0x00},
		{0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},
		{0x00, 0x00, 0x20, 0x10, 0x00, 0x00},
		{0x08, 0x08, 0x08, 0x08, 0x08, 0x00},
		{0x00, 0x00, 0x20, 0x00, 0x00, 0x00},
		{0x20, 0x10, 0x08, 0x04, 0x02, 0x00},
		{0x1C, 0x32, 0x2A, 0x26, 0x1C, 0x00},
		{0x00, 0x24, 0x3E, 0x20, 0x00, 0x00},
		{0x3A, 0x2A, 0x2A, 0x2A, 0x2E, 0x00},
		{0x22, 0x22, 0x2A, 0x2A, 0x3E, 0x00},
		{0x10, 0x18, 0x14, 0x3E, 0x10, 0x00},
		{0x2E, 0x2A, 0x2A, 0x2A, 0x12, 0x00},
		{0x3E, 0x2A, 0x2A, 0x2A, 0x3A, 0x00},
		{0x02, 0x02, 0x32, 0x0A, 0x06, 0x00},
		{0x3E, 0x2A, 0x2A, 0x2A, 0x3E, 0x00},
		{0x2E, 0x2A, 0x2A, 0x2A, 0x3E, 0x00},
		{0x00, 0x00, 0x00, 0x14, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x14, 0x00, 0x00},
		{0x00, 0x08, 0x14, 0x22, 0x00, 0x00},
		{0x00, 0x14, 0x14, 0x14, 0x14, 0x00},
		{0x00, 0x22, 0x14, 0x08, 0x00, 0x00},
		{0x00, 0x04, 0x02, 0x2A, 0x04, 0x00},
		{0x3E, 0x22, 0x22, 0x2A, 0x2E, 0x00},
		{0x38, 0x14, 0x12, 0x14, 0x38, 0x00},
		{0x3E, 0x2A, 0x2A, 0x2A, 0x14, 0x00},
		{0x1C, 0x22, 0x22, 0x22, 0x14, 0x00},
		{0x3E, 0x22, 0x22, 0x22, 0x1C, 0x00},
		{0x3E, 0x2A, 0x2A, 0x22, 0x22, 0x00},
		{0x3E, 0x0A, 0x0A, 0x02, 0x02, 0x00},
		{0x1C, 0x22, 0x22, 0x2A, 0x18, 0x00},
		{0x3E, 0x08, 0x08, 0x08, 0x3E, 0x00},
		{0x00, 0x22, 0x3E, 0x22, 0x00, 0x00},
		{0x12, 0x22, 0x22, 0x1E, 0x02, 0x00},
		{0x3E, 0x08, 0x0C, 0x14, 0x22, 0x00},
		{0x3E, 0x20, 0x20, 0x20, 0x20, 0x00},
		{0x3E, 0x04, 0x08, 0x04, 0x3E, 0x00},
		{0x3E, 0x04, 0x08, 0x10, 0x3E, 0x00},
		{0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00},
		{0x3E, 0x0A, 0x0A, 0x0A, 0x04, 0x00},
		{0x1C, 0x22, 0x2A, 0x12, 0x2C, 0x00},
		{0x3E, 0x0A, 0x0A, 0x1A, 0x2C, 0x00},
		{0x24, 0x2A, 0x2A, 0x2A, 0x12, 0x00},
		{0x02, 0x02, 0x3E, 0x02, 0x02, 0x00},
		{0x1E, 0x20, 0x20, 0x20, 0x1E, 0x00},
		{0x02, 0x0C, 0x30, 0x0C, 0x02, 0x00},
		{0x3E, 0x10, 0x08, 0x10, 0x3E, 0x00},
		{0x22, 0x14, 0x08, 0x14, 0x22, 0x00},
		{0x02, 0x04, 0x38, 0x04, 0x02, 0x00},
		{0x22, 0x32, 0x2A, 0x26, 0x22, 0x00},
		{0x00, 0x3E, 0x22, 0x22, 0x00, 0x00},
		{0x02, 0x04, 0x08, 0x10, 0x20, 0x00},
		{0x00, 0x00, 0x22, 0x22, 0x3E, 0x00},
		{0x08, 0x04, 0x02, 0x04, 0x08, 0x00},
		{0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
		{0x00, 0x00, 0x02, 0x04, 0x00, 0x00},
		{0x38, 0x14, 0x12, 0x14, 0x38, 0x00},
		{0x3E, 0x2A, 0x2A, 0x2A, 0x14, 0x00},
		{0x1C, 0x22, 0x22, 0x22, 0x14, 0x00},
		{0x3E, 0x22, 0x22, 0x22, 0x1C, 0x00},
		{0x3E, 0x2A, 0x2A, 0x22, 0x22, 0x00},
		{0x3E, 0x0A, 0x0A, 0x02, 0x02, 0x00},
		{0x1C, 0x22, 0x22, 0x2A, 0x18, 0x00},
		{0x3E, 0x08, 0x08, 0x08, 0x3E, 0x00},
		{0x00, 0x22, 0x3E, 0x22, 0x00, 0x00},
		{0x12, 0x22, 0x22, 0x1E, 0x02, 0x00},
		{0x3E, 0x08, 0x0C, 0x14, 0x22, 0x00},
		{0x3E, 0x20, 0x20, 0x20, 0x20, 0x00},
		{0x3E, 0x04, 0x08, 0x04, 0x3E, 0x00},
		{0x3E, 0x04, 0x08, 0x10, 0x3E, 0x00},
		{0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00},
		{0x3E, 0x0A, 0x0A, 0x0A, 0x04, 0x00},
		{0x1C, 0x22, 0x2A, 0x12, 0x2C, 0x00},
		{0x3E, 0x0A, 0x0A, 0x1A, 0x2C, 0x00},
		{0x24, 0x2A, 0x2A, 0x2A, 0x12, 0x00},
		{0x02, 0x02, 0x3E, 0x02, 0x02, 0x00},
		{0x1E, 0x20, 0x20, 0x20, 0x1E, 0x00},
		{0x02, 0x0C, 0x30, 0x0C, 0x02, 0x00},
		{0x3E, 0x10, 0x08, 0x10, 0x3E, 0x00},
		{0x22, 0x14, 0x08, 0x14, 0x22, 0x00},
		{0x02, 0x04, 0x38, 0x04, 0x02, 0x00},
		{0x22, 0x32, 0x2A, 0x26, 0x22, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

const uint8_t asciiFont57[][6] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x5F, 0x00, 0x00, 0x00},
		{0x00, 0x07, 0x00, 0x07, 0x00, 0x00},
		{0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00},
		{0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00},
		{0x23, 0x13, 0x08, 0x64, 0x62, 0x00},
		{0x36, 0x49, 0x55, 0x22, 0x50, 0x00},
		{0x00, 0x05, 0x03, 0x00, 0x00, 0x00},
		{0x00, 0x1C, 0x22, 0x41, 0x00, 0x00},
		{0x00, 0x41, 0x22, 0x1C, 0x00, 0x00},
		{0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00},
		{0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},
		{0x00, 0x50, 0x30, 0x00, 0x00, 0x00},
		{0x08, 0x08, 0x08, 0x08, 0x08, 0x00},
		{0x00, 0x60, 0x60, 0x00, 0x00, 0x00},
		{0x20, 0x10, 0x08, 0x04, 0x02, 0x00},
		{0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00},
		{0x00, 0x42, 0x7F, 0x40, 0x00, 0x00},
		{0x42, 0x61, 0x51, 0x49, 0x46, 0x00},
		{0x21, 0x41, 0x45, 0x4B, 0x31, 0x00},
		{0x18, 0x14, 0x12, 0x7F, 0x10, 0x00},
		{0x27, 0x45, 0x45, 0x45, 0x39, 0x00},
		{0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00},
		{0x01, 0x71, 0x09, 0x05, 0x03, 0x00},
		{0x36, 0x49, 0x49, 0x49, 0x36, 0x00},
		{0x06, 0x49, 0x49, 0x29, 0x1E, 0x00},
		{0x00, 0x36, 0x36, 0x00, 0x00, 0x00},
		{0x00, 0x56, 0x36, 0x00, 0x00, 0x00},
		{0x00, 0x08, 0x14, 0x22, 0x41, 0x00},
		{0x14, 0x14, 0x14, 0x14, 0x14, 0x00},
		{0x41, 0x22, 0x14, 0x08, 0x00, 0x00},
		{0x02, 0x01, 0x51, 0x09, 0x06, 0x00},
		{0x32, 0x49, 0x79, 0x41, 0x3E, 0x00},
		{0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00},
		{0x7F, 0x49, 0x49, 0x49, 0x36, 0x00},
		{0x3E, 0x41, 0x41, 0x41, 0x22, 0x00},
		{0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00},
		{0x7F, 0x49, 0x49, 0x49, 0x41, 0x00},
		{0x7F, 0x09, 0x09, 0x01, 0x01, 0x00},
		{0x3E, 0x41, 0x41, 0x51, 0x32, 0x00},
		{0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00},
		{0x00, 0x41, 0x7F, 0x41, 0x00, 0x00},
		{0x20, 0x40, 0x41, 0x3F, 0x01, 0x00},
		{0x7F, 0x08, 0x14, 0x22, 0x41, 0x00},
		{0x7F, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00},
		{0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00},
		{0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00},
		{0x7F, 0x09, 0x09, 0x09, 0x06, 0x00},
		{0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00},
		{0x7F, 0x09, 0x19, 0x29, 0x46, 0x00},
		{0x46, 0x49, 0x49, 0x49, 0x31, 0x00},
		{0x01, 0x01, 0x7F, 0x01, 0x01, 0x00},
		{0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00},
		{0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00},
		{0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00},
		{0x63, 0x14, 0x08, 0x14, 0x63, 0x00},
		{0x03, 0x04, 0x78, 0x04, 0x03, 0x00},
		{0x61, 0x51, 0x49, 0x45, 0x43, 0x00},
		{0x00, 0x00, 0x7F, 0x41, 0x41, 0x00},
		{0x02, 0x04, 0x08, 0x10, 0x20, 0x00},
		{0x41, 0x41, 0x7F, 0x00, 0x00, 0x00},
		{0x04, 0x02, 0x01, 0x02, 0x04, 0x00},
		{0x40, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x00, 0x01, 0x02, 0x04, 0x00, 0x00},
		{0x20, 0x54, 0x54, 0x54, 0x78, 0x00},
		{0x7F, 0x48, 0x44, 0x44, 0x38, 0x00},
		{0x38, 0x44, 0x44, 0x44, 0x20, 0x00},
		{0x38, 0x44, 0x44, 0x48, 0x7F, 0x00},
		{0x38, 0x54, 0x54, 0x54, 0x18, 0x00},
		{0x08, 0x7E, 0x09, 0x01, 0x02, 0x00},
		{0x08, 0x14, 0x54, 0x54, 0x3C, 0x00},
		{0x7F, 0x08, 0x04, 0x04, 0x78, 0x00},
		{0x00, 0x44, 0x7D, 0x40, 0x00, 0x00},
		{0x20, 0x40, 0x44, 0x3D, 0x00, 0x00},
		{0x00, 0x7F, 0x10, 0x28, 0x44, 0x00},
		{0x00, 0x41, 0x7F, 0x40, 0x00, 0x00},
		{0x7C, 0x04, 0x18, 0x04, 0x78, 0x00},
		{0x7C, 0x08, 0x04, 0x04, 0x78, 0x00},
		{0x38, 0x44, 0x44, 0x44, 0x38, 0x00},
		{0x7C, 0x14, 0x14, 0x14, 0x08, 0x00},
		{0x08, 0x14, 0x14, 0x18, 0x7C, 0x00},
		{0x7C, 0x08, 0x04, 0x04, 0x08, 0x00},
		{0x48, 0x54, 0x54, 0x54, 0x20, 0x00},
		{0x04, 0x3F, 0x44, 0x40, 0x20, 0x00},
		{0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00},
		{0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00},
		{0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00},
		{0x44, 0x28, 0x10, 0x28, 0x44, 0x00},
		{0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00},
		{0x44, 0x64, 0x54, 0x4C, 0x44, 0x00},
		{0x00, 0x08, 0x36, 0x41, 0x00, 0x00},
		{0x00, 0x00, 0x7F, 0x00, 0x00, 0x00},
		{0x00, 0x41, 0x36, 0x08, 0x00, 0x00},
		{0x10, 0x04, 0x08, 0x10, 0x08, 0x00},
		{0x08, 0x1C, 0x2A, 0x08, 0x08, 0x00}
};

const uint8_t initLedPattern[][5] =
{
		{0x00, 0x00, 0x01, 0x00, 0x00},
		{0x00, 0x00, 0x02, 0x00, 0x00},
		{0x00, 0x00, 0x04, 0x00, 0x00},
		{0x00, 0x00, 0x08, 0x00, 0x00},
		{0x00, 0x00, 0x10, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x00, 0x00},
		{0x00, 0x00, 0x40, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x00, 0x00},
		{0x00, 0x00, 0x10, 0x00, 0x00},
		{0x00, 0x00, 0x08, 0x00, 0x00},
		{0x00, 0x00, 0x04, 0x00, 0x00},
		{0x00, 0x00, 0x02, 0x00, 0x00},
};

uint8_t rowMap[7] = {2, 7, 8, 9, 1, 0, 3};
uint8_t colMap[5] = {11, 10, 4, 5, 6};
uint8_t (*activeFont)[6];

const notes_t notes[] = {
		{'c', 1915},
		{'d', 1700},
		{'e', 1519},
		{'f', 1432},
		{'s', 1352},
		{'g', 1275},
		{'a', 1136},
		{'v', 1073},
		{'b', 1014},
		{'C', 956},
		{'D', 852},
		{'E', 758},
		{'F', 593},
		{'G', 468},
		{'A', 346},
		{'B', 224},
		{'x', 655},
		{'y', 715}
};

uint16_t tempo;
uint8_t songLength;
char song[200] = {0,};
uint8_t beats[200] = {0,};


volatile uint32_t musicTones[400][2];
volatile uint16_t currMusicIndex;
volatile uint16_t musicSize;
uint16_t hostTimeout;
uint16_t hostTicks;

uint16_t eeTagAddr[EE_LAST_RESERVED - EE_DISP_TEXT + 1] = {0, 2 * EE_PAGE_SIZE, 3 * EE_PAGE_SIZE, 19 * EE_PAGE_SIZE, 60 * EE_PAGE_SIZE};
/* -------------------------------------------------------------------------
 * Private functions
 * ------------------------------------------------------------------------- */

//void ResetISR(void)
//{
//    /* Increasing the system clock as soon as possible to reduce the startup time.
//     * Setting the clock to 2MHz is the maximum: when
//     * - running without a battery
//     * - at 4MHz
//     * - when the field is provided by some phones (e.g. S5)
//     * a voltage drop to below 1.2V was observed - effectively resetting the chip.
//     */
//    Chip_Clock_System_SetClockFreq(2 * 1000 * 1000);
//    Startup_VarInit();
//    main();
//}

/** Called under interrupt. Refer #NDEFT2T_FIELD_STATUS_CB. */
void NDEFT2T_FieldStatus_Cb(bool status)
{
    /* A PCD can do very strange things, a.o. power off the field and immediately power on the field again, or 10+ times
     * select the same device as part of its illogic procedure to select a PICC and start communicating with it.
     * Instead of deciding to Power-off or go to Deep Power Down immediately, it is more robust to additionally check
     * if during a small interval no NFC field is started again.
     * The loop in ExecuteNfcMode() may thus not look at the NFC interrupt status, but at the result of the timer
     * interrupt.
     */
    if (status) {
        hostTimeout = HOST_TIMEOUT;
        hostTicks = 0;
        nfcOn = true;
    }
    else {
        hostTimeout = LAST_HOST_TIMEOUT;
        hostTicks = 0;
        nfcOn = false;
    }
}

/** Called under interrupt. @see #NDEFT2T_MSG_AVAILABLE_CB. */
void NDEFT2T_MsgAvailable_Cb(void)
{
    sTargetWritten = true;
    hostTimeout = HOST_TIMEOUT;
    hostTicks = 0;
}

/**
 * @see TMEAS_CB
 * @see pTMeas_Cb_t
 */
#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
void App_TmeasCb(TSEN_RESOLUTION_T resolution, TMEAS_FORMAT_T format, int value, uint32_t context)
{
    ASSERT(format == TMEAS_FORMAT_FAHRENHEIT);
    if (value < -1210) {
        value = -1211;
    }
    if (1850 < value) {
        value = 1851;
    }

    if(context == 0) {
    	g_currentTemp = value;
    }

}

static void Init(void)
{
	/* Increasing the system clock as soon as possible to reduce the startup time.
	 * Setting the clock to 2MHz is the maximum: when
	 * - running without a battery
	 * - at 4MHz
	 * - when the field is provided by some phones (e.g. S5)
	 * a voltage drop to below 1.2V was observed - effectively resetting the chip.
	 */    
    Chip_Clock_System_SetClockFreq(2 * 1000 * 1000);
    /* First initialize the absolute minimum.
     * Avoid accessing the RTC and the PMU as these API calls are slow. Prepare an NFC message ASAP.
     * Only after that we can complete the initialization.
     */
    Board_Init();
    TMeas_Measure(TSEN_10BITS, TMEAS_FORMAT_FAHRENHEIT, false, 0 /* Value used in App_TmeasCb */);
    Chip_NFC_Init(LPC_NFC);
    NDEFT2T_Init();
    Chip_EEPROM_Init(LPC_EEPROM);
    Timer_Init();
    dispStep = 0;
    scanCount = 0;
    startPatternIndex = 0;
    nfcOn = false;
    displayInProgress = false;
    updateNFC = false;
    g_updateTemp = false;
    musicInProgress = false;

    while (Chip_TSen_ReadStatus(LPC_TSEN, NULL) & TSEN_STATUS_SENSOR_IN_OPERATION) {
        ; /* Wait until the temperature has become available. */
    }
    NDEFT2T_CreateMessage(sNdefInstance, sData, sizeof(sData), true);
    NDEFT2T_CommitMessage(sNdefInstance);

    /* Reduce power consumption by adding a pull-down. The default register values after a reset do
     * not have enabled these pulls. The functionality of the SWD pins are kept.
     */
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

/**
 * Cleanly closes everything down, decides which low power state to go to - Deep Power Down or Power-off, and enters
 * that state using the correctly determined parameters.
 * @note Wake-up conditions other than a reset pulse or the presence of an NFC field - both of which cannot be disabled
 *  nor require configuration - must have been set beforehand.
 * @param waitBeforeDisconnect Present to aid the SW developer. If @c true, it will wait - not sleep - for a couple of
 *  seconds before disconnecting the battery. The argument has no effect if Deep Power Down mode is selected.
 *  The extra time window allows for easier breaking in using SWD, allowing time to halt the core and flash new
 *  firmware. However, it will @c not touch any PIO, or ensure that SWD functionality is offered.
 *  The default value should be @c false, i.e. go to Power-off state without delay: typical user behavior is to bring
 *  the IC in and out the NFC field quickly, before stabilizing on a correct position. Having a time penalty of several
 *  seconds - during which the host SW may already have made several decisions about the use and state of the IC -
 *  diminishes the user experience.
 */
static void DeInit(void)
{
	bool bod;

    NDEFT2T_DeInit();
    NVIC_DisableIRQ(CT32B0_IRQn);
    stopMusic();
    LPC_GPIO->DATA[0xFFF] = 0;

	Chip_PMU_SetBODEnabled(true);
	bod = ((Chip_PMU_GetStatus() & PMU_STATUS_BROWNOUT) != 0);
	Chip_PMU_SetBODEnabled(false);

	Timer_StartMeasurementTimeout(2);
	Chip_PMU_PowerMode_EnterDeepPowerDown(bod);
	//Chip_PMU_Switch_OpenVDDBat();
	/* Normally, this function never returns. However, when providing power via SWD or any other PIO this will not
	 * work - power is flowing through the bondpad ring via the SWD pin, still powering a small part of the VDD_ALON
	 * domain.
	 * This situation is not covered by HW design: we can't rely on anything being functional or even harmless.
	 * Just ensure nothing happens, and wait until all power is gone.
	 */
    for(;;);
}

static uint32_t convertStr2LEDPattern(char *str, volatile uint8_t *pattern)
{
	uint8_t (*table)[6];
	uint32_t index = 0;
	uint8_t i;
	char offset;

	while(str && *str) {
		table = 0;
		if(*str >= ' ') {
			offset = ' ';
			table =  (uint8_t (*)[6])activeFont;
		}
		for(i = 0; table && (i < 6); i++) {
			pattern[index++] = table[*str - offset][i];
		}
		str++;
	}
	return index;
}
static void StartDisplayTimer(void)
{
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, 1000*LED_REFRESH_RATE_MS + Chip_TIMER_ReadCount(LPC_TIMER32_0));
	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER32_0, 0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER32_0, 0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
}

static void playTone(uint32_t halfPeriodUs)
{
	Chip_TIMER_Disable(LPC_TIMER16_0);
	Chip_TIMER_Reset(LPC_TIMER16_0);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, halfPeriodUs);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_0, 0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER16_0, 0);
	Chip_TIMER_MatchDisableInt(LPC_TIMER16_0, 0);
	Chip_TIMER_SetMatchOutputMode(LPC_TIMER16_0, 0, TIMER_MATCH_OUTPUT_EMC);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 0, TIMER_EXTMATCH_TOGGLE, 0);
	Chip_TIMER_Enable(LPC_TIMER16_0);
}
static void setToneDelay(uint32_t delayUs)
{
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, delayUs + Chip_TIMER_ReadCount(LPC_TIMER32_0));
	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER32_0, 1);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER32_0, 1);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
}

#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
static void startMusic(void)
{
	uint8_t i = 0, j;
	for( i = 0; i < songLength; i++) {
		if(song[i] == ' ') {
			musicTones[i*2][0] = 0;
		}
		else {
			for(j = 0; j < sizeof(notes)/sizeof(notes[0]); j++) {
				if(notes[j].note == song[i]) {
					musicTones[i*2][0] = notes[j].halfPeriodUs;
					break;
				}
			}
		}
		musicTones[i*2][1] = (uint32_t)(beats[i] * tempo * 1000U);
		musicTones[i*2+1][0] = 0;
		musicTones[i*2+1][1] = (tempo * 1000U) >> 1;
	}
	musicSize = (uint16_t)((uint16_t)i << 1);
	currMusicIndex = 0;

	Chip_TIMER16_0_Init();
	Chip_TIMER_PrescaleSet(LPC_TIMER16_0, 1);
	Chip_TIMER_Reset(LPC_TIMER16_0);
	NVIC_DisableIRQ(CT16B0_IRQn);
	Chip_TIMER_Disable(LPC_TIMER16_0);
	if(musicTones[currMusicIndex][0] != 0) {
		playTone(musicTones[currMusicIndex][0]);
	}
	setToneDelay(musicTones[currMusicIndex][1]);
	currMusicIndex++;
}

static void stopMusic(void)
{
	Chip_TIMER_Disable(LPC_TIMER16_0);
	Chip_TIMER_Reset(LPC_TIMER16_0);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 0, TIMER_EXTMATCH_TOGGLE, 0);

	Chip_TIMER_MatchDisableInt(LPC_TIMER32_0, 1);
}

static uint16_t eepromReadTag(EE_TAG_t tag, uint8_t *pBuff, uint16_t buffLen)
{
	uint8_t header[EE_HEADER_SIZE];
	uint16_t size;

	Chip_EEPROM_Read(LPC_EEPROM, eeTagAddr[tag - EE_DISP_TEXT], header, EE_HEADER_SIZE);
	size = (uint16_t)((((uint16_t)header[2]) << 8) | header[1]);
	if((header[0] == tag) && (size <= (eeTagAddr[tag - EE_DISP_TEXT + 1] - eeTagAddr[tag - EE_DISP_TEXT] - (uint16_t)EE_HEADER_SIZE))) {
		if(size && (size <= buffLen)) {
			Chip_EEPROM_Read(LPC_EEPROM, (int)(eeTagAddr[tag - EE_DISP_TEXT] + EE_HEADER_SIZE), pBuff, size);
		}
	}
	else {
		size = 0;
	}
	return size;
}

#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wconversion"
#endif
static void eepromReadMusic(void)
{
	uint8_t header[EE_HEADER_SIZE];
	uint16_t size;
	uint16_t currOffset;

	musicDataValid = false;
	Chip_EEPROM_Read(LPC_EEPROM, eeTagAddr[EE_MUSIC_TONE - EE_DISP_TEXT], header, EE_HEADER_SIZE);
	size = (uint16_t)((((uint16_t)header[2]) << 8) | header[1]);
	if((header[0] == EE_MUSIC_TONE) && (size) && (size <= (eeTagAddr[EE_MUSIC_TONE - EE_DISP_TEXT + 1] - eeTagAddr[EE_MUSIC_TONE - EE_DISP_TEXT] - (uint16_t)EE_HEADER_SIZE))) {
		currOffset = (uint16_t)(eeTagAddr[EE_MUSIC_TONE - EE_DISP_TEXT] + EE_HEADER_SIZE);
		Chip_EEPROM_Read(LPC_EEPROM, (int)currOffset, &tempo, sizeof(tempo));
		currOffset += 2;
		Chip_EEPROM_Read(LPC_EEPROM, (int)currOffset, &songLength, sizeof(songLength));
		currOffset += 1;
		Chip_EEPROM_Read(LPC_EEPROM, (int)currOffset, song, songLength);
		song[songLength] = 0;
		currOffset += (uint16_t)songLength;
		Chip_EEPROM_Read(LPC_EEPROM, (int)currOffset, beats, songLength);
		currOffset += (uint16_t)songLength;
		if(tempo && songLength) {
			musicDataValid = true;
		}
	}
}

static void eepromWriteTag(EE_TAG_t tag, uint8_t *pBuff, uint16_t size)
{
	uint8_t header[EE_HEADER_SIZE];

	if((eeTagAddr[tag - EE_DISP_TEXT] + EE_HEADER_SIZE + size) <= eeTagAddr[tag - EE_DISP_TEXT + 1]) {
		header[0] = tag;
		header[1] = size & 0xffUL;
		header[2] = (size >> 8) & 0xffUL;
		header[3] = 0;

		Chip_EEPROM_Write(LPC_EEPROM, eeTagAddr[tag - EE_DISP_TEXT], header, EE_HEADER_SIZE);
		Chip_EEPROM_Write(LPC_EEPROM, (int)(eeTagAddr[tag - EE_DISP_TEXT] + EE_HEADER_SIZE), pBuff, size);
		Chip_EEPROM_Flush(LPC_EEPROM, true);
	}
}

static void startLEDDisplay(bool addTemp)
{
	int temp;
	uint16_t decPosition =1000, digit, prevDigit = 0, index = 9;
	Chip_TIMER_MatchDisableInt(LPC_TIMER32_0, 0);
	/*Clear all LEDS */
	LPC_GPIO->DATA[0xFFF] = 0;
	if(addTemp) {
		temp = g_currentTemp;

		strcpy(g_tempText, " Temp is 00.0 F ");
		if(temp < 0) {
			temp = -temp;
			g_tempText[index++] = '-';
		}
		while(decPosition) {
			digit = (uint16_t)(((uint32_t)temp)/decPosition);
			if(digit) {
				if(decPosition == 1) {
					if(prevDigit == 0) {
						g_tempText[index++] = '0';
					}
					g_tempText[index++] = '.';
				}
				g_tempText[index++] = (char)(digit - prevDigit + '0');
				prevDigit = (uint16_t)(digit *10);
			}
			decPosition /= 10;
		}
		g_tempText[index++] = ' ';
		g_tempText[index++] = 'F';
		g_tempText[index++] = ' ';
		g_tempText[index] = 0;
		strcat(g_displayText, g_tempText);
	}
	sizeLEDPattern = convertStr2LEDPattern(g_displayText, g_ledPattern);
	dispStep = 0;
	scanCount = 0;
	startPatternIndex = 0;
	if(!displayInProgress) {
		displayInProgress = true;
		StartDisplayTimer();
	}
	else {
		Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, 1000*LED_REFRESH_RATE_MS + Chip_TIMER_ReadCount(LPC_TIMER32_0));
		Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
	}
}

#ifdef __REDLIB__
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
static bool writeNFCDisplayMsg(char *str, uint32_t len)
{
	NDEFT2T_CREATE_RECORD_INFO_T recordInfo;
	char lang[] = "en";
	char mime[] = "application/octet-stream";
	bool success = true;
	uint16_t delay;
	uint32_t ledIndex;
	uint8_t mimeResponse[9];

	NDEFT2T_CreateMessage(sNdefInstance, sData, sizeof(sData), false);

	recordInfo.shortRecord = true;
	recordInfo.pString = (uint8_t *)lang;
	success &= NDEFT2T_CreateTextRecord(sNdefInstance, &recordInfo);
	if (success) {
		success = NDEFT2T_WriteRecordPayload(sNdefInstance, str, (int)len);
		if (success) {
			NDEFT2T_CommitRecord(sNdefInstance);
		}
	}

	recordInfo.shortRecord = true;
	recordInfo.pString = (uint8_t *)mime;
	success &= NDEFT2T_CreateMimeRecord(sNdefInstance, &recordInfo);
	if (success) {
		mimeResponse[0] = 0x52;
		delay = (uint16_t)(((desiredSpeed - scanCount) * LED_REFRESH_RATE_MS * LED_DISP_REFRESH_STEPS) - (dispStep * LED_REFRESH_RATE_MS));
		ledIndex = startPatternIndex + 5;
		if(ledIndex >= sizeLEDPattern) {
			ledIndex -= sizeLEDPattern;
		}
		mimeResponse[1] = (uint8_t)(ledIndex/6);
		mimeResponse[2] = (uint8_t)(ledIndex % 6);
		mimeResponse[3] = (uint8_t)(delay & 0xffU);
		mimeResponse[4] = (uint8_t)((delay >> 8) & 0xffU);
		delay = (uint16_t)(desiredSpeed * LED_REFRESH_RATE_MS * LED_DISP_REFRESH_STEPS);
		mimeResponse[5] = (uint8_t)(delay & 0xffU);
		mimeResponse[6] = (uint8_t)((delay >> 8) & 0xffU);
		mimeResponse[7] = (uint8_t)(desiredSpeed - 5);
		if(activeFont == ((uint8_t (*)[6])asciiFont57)) {
			mimeResponse[8] = (uint8_t)0x57;
		}
		else {
			mimeResponse[8] = (uint8_t)0x55;
		}


		success = NDEFT2T_WriteRecordPayload(sNdefInstance, mimeResponse, 9);
		if (success) {
			NDEFT2T_CommitRecord(sNdefInstance);
		}
	}

	if (success) {
		NDEFT2T_CommitMessage(sNdefInstance);
	}

	return success;
}

static bool takeMemSemaphore(void)
{
	bool status = false;
	uint32_t *nfcMem = (uint32_t *)LPC_NFC->BUF;

	if(nfcMem[1] == 0x000002FD) {
		nfcMem[0] = 0x000102FD;
		if((nfcMem[0] == 0x000102FD) && (nfcMem[1] == 0x000002FD)) {
			status = true;
		}
		else {
			nfcMem[0] = 0x000002FD;
		}
	}
	return status;
}

static void releaseMemSemaphore(void)
{
	uint32_t *nfcMem = (uint32_t *)LPC_NFC->BUF;

	nfcMem[0] = 0x000002FD;
}

static void extractMusic(uint8_t *data)
{
	uint8_t i;
	musicDataValid = false;
	tempo = (uint16_t)(((uint16_t)*(data +1) << 8) | *data);
	data += 2;
	songLength = *data++;
	for(i= 0; i < songLength; i++) {
		song[i] = *data++;
	}
	song[i] = 0;
	for(uint8_t i = 0; i < songLength; i++) {
		beats[i] = *data++;
	}
	if(tempo && songLength) {
		musicDataValid = true;
	}
}

int CT32B0_Cnt;
void CT32B0_IRQHandler(void)
{
	uint32_t index;
	uint8_t i;
	CT32B0_Cnt++;
    if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 0)) {
    	LPC_GPIO->DATA[1] = 0;
    	LPC_GPIO->DIR = (LPC_GPIO->DIR & 0xFFF) & ~(0x1U);
        Chip_TIMER_ClearMatch(LPC_TIMER32_0, 0);
        Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, 1000*LED_REFRESH_RATE_MS + Chip_TIMER_ReadCount(LPC_TIMER32_0));

        /* P0_0 switch press detection */
        if(LPC_GPIO->DATA[1]) {
        	if(switchDebounceCnt < 10) {
        		switchDebounceCnt++;
        	}
        	if(!switchPressed && (switchDebounceCnt >= 10)) {
        		switchPressed = true;
        		changeFont = true;
        	}

        }
        else {
        	if(switchDebounceCnt > 0) {
        		switchDebounceCnt--;
        	}
        	if(switchPressed && (switchDebounceCnt == 0)) {
				switchPressed = false;
			}
        }
        LPC_GPIO->DIR = (LPC_GPIO->DIR & 0xFFF) | 0x1U;

        /* LED Display */
		LPC_GPIO->DATA[0xFFF] = 0xFFFU;
		if(displayInProgress) {
			for(i = 0; i < 7; i++) {
				index = startPatternIndex + dispStep;
				if(index >= sizeLEDPattern) {
					index -= sizeLEDPattern;
				}
				LPC_GPIO->DATA[1UL << rowMap[i]] = (g_ledPattern[index] & (1 << (6-i))) ? (1UL << rowMap[i]) : 0;
			}
		}
		else {
			for(i = 0; i < 7; i++) {
				LPC_GPIO->DATA[1UL << rowMap[i]] = (initLedPattern[startPatternIndex][dispStep] & (1 << (6-i))) ? (1UL << rowMap[i]) : 0;
			}
		}
		LPC_GPIO->DATA[1UL << colMap[dispStep]] = 0;

        if(++dispStep == LED_DISP_REFRESH_STEPS) {
        	dispStep = 0;
        	if(++scanCount >= desiredSpeed) {
        		scanCount = 0;
        		if(++startPatternIndex == sizeLEDPattern) {
        			startPatternIndex = 0;
        			g_updateTemp = true;
        		}
        	}
        }
        updateNFC = true;
    }
    if(Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) {
    	Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
    	Chip_TIMER_Disable(LPC_TIMER16_0);
    	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 0, TIMER_EXTMATCH_TOGGLE, 0);
    	if(musicTones[currMusicIndex][0] != 0) {
			playTone(musicTones[currMusicIndex][0]);
		}
		setToneDelay(musicTones[currMusicIndex][1]);
		currMusicIndex++;
		if(currMusicIndex >= musicSize) {
			currMusicIndex = 0;
		}
    }
}
/* -------------------------------------------------------------------------------- */
int cnt, System_ClockDiv, System_ClockFreq;
PMU_DPD_WAKEUPREASON_T wakeupReason_test;

int main(void)
{
	int temp;
	uint16_t decPosition, digit, prevDigit, index, textSize;
	uint32_t tempSpeed;
	bool initDispStarted = false;
	PMU_DPD_WAKEUPREASON_T wakeupReason;
    Init();
    wakeupReason = Chip_PMU_PowerMode_GetDPDWakeupReason();
	wakeupReason_test = wakeupReason;
    if(wakeupReason == PMU_DPD_WAKEUPREASON_RTC) {
    	/* Blink LED for second */
    	LPC_GPIO->DATA[0xFFF] = 0xE60U;
    	Chip_TIMER_SetMatch(LPC_TIMER32_0, 2, 1000*100 + Chip_TIMER_ReadCount(LPC_TIMER32_0));
    	Chip_TIMER_ResetOnMatchDisable(LPC_TIMER32_0, 2);
    	Chip_TIMER_StopOnMatchDisable(LPC_TIMER32_0, 2);
    	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 2);
    	__WFI();
    }
    else {
    	switchDebounceCnt = 0;
		switchPressed = false;
		changeFont = false;
		activeFont = (uint8_t (*)[6])asciiFont57;

		textSize = eepromReadTag(EE_DISP_TEXT, (uint8_t *)g_displayText, MAX_DISP_CHARS-20);
		if((textSize == 0) || (strlen(g_displayText) == 0)) {
			strcpy(g_displayText, "The LPC Experience; Your World Reimagined");
		}
		g_displayTextLen = (uint8_t)strlen(g_displayText);
		eepromReadMusic();

		/* Safest is to just try to communicate. It will be stopped or restarted using the callbacks provided by the
		 * NDEFT2T module. By using Timer_CheckHostTimeout in the while loop below, the while loop will be
		 * stopped when the field has been removed.
		 */
		hostTimeout = FIRST_HOST_TIMEOUT;
		hostTicks = 0;

		Chip_TIMER32_0_Init();
		Chip_TIMER_PrescaleSet(LPC_TIMER32_0, 1);
		Chip_TIMER_Reset(LPC_TIMER32_0);
		NVIC_EnableIRQ(CT32B0_IRQn);
		Chip_TIMER_Enable(LPC_TIMER32_0);
		
		System_ClockDiv = Chip_Clock_System_GetClockDiv();
		System_ClockFreq = Chip_Clock_System_GetClockFreq();

		/* Wait for a command. Send responses based on these commands. */
		while (hostTicks < hostTimeout) {
			cnt++;
			if(nfcOn && !displayInProgress) {
				desiredSpeed = 20;
				if(eepromReadTag(EE_SCROLL_SPEED, (uint8_t *)&tempSpeed, sizeof(tempSpeed))) {
					desiredSpeed = (tempSpeed & 0xffU) + 5;
					if((desiredSpeed < 5) || (desiredSpeed > 30)) {
						desiredSpeed = 20;
					}
				}
				updateNFC = false;
				g_updateTemp = false;
				Timer_StartMeasurementTimeout(1);
				Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_0 | IOCON_RMODE_INACT);
				Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)10, IOCON_FUNC_0 | IOCON_RMODE_INACT);
				Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)11, IOCON_FUNC_0 | IOCON_RMODE_INACT);

				LPC_GPIO->DATA[0xFFF] = 0;
				LPC_GPIO->DIR = (LPC_GPIO->DIR & 0xFFF) | 0xFFF;
				startLEDDisplay(true);
				if(((const uint8_t (*)[6])activeFont == asciiFont55) && musicDataValid) {
					musicInProgress = true;
					Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_1 | IOCON_RMODE_INACT);
					startMusic();
				}
			}
			if(!displayInProgress && !initDispStarted) {
				initDispStarted = true;
				Chip_TIMER_MatchDisableInt(LPC_TIMER32_0, 0);
				/*Clear all LEDS */
				LPC_GPIO->DATA[0xFFF] = 0;
				sizeLEDPattern = 12;
				dispStep = 0;
				scanCount = 0;
				startPatternIndex = 0;
				desiredSpeed = 5;
				StartDisplayTimer();
			}
			if(changeFont) {
				changeFont = false;
				if(activeFont == ((uint8_t (*)[6])asciiFont57)) {
					activeFont = (uint8_t (*)[6])asciiFont55;
				}
				else {
					activeFont = (uint8_t (*)[6])asciiFont57;
				}
				if(displayInProgress) {
					startLEDDisplay(false);
					if((const uint8_t (*)[6])activeFont == asciiFont55) {
						if(musicDataValid) {
							musicInProgress = true;
							Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_1 | IOCON_RMODE_INACT);
							startMusic();
						}
					}
					else {
						if(musicInProgress) {
							stopMusic();
							Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_0 | IOCON_RMODE_INACT);
							musicInProgress = false;
						}

					}
				}
				else {
					if((const uint8_t (*)[6])activeFont == asciiFont55) {
						stopMusic();
						Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_1 | IOCON_RMODE_INACT);
					}
					else {
						Chip_IOCON_SetPinConfig(LPC_IOCON, (IOCON_PIN_T)3, IOCON_FUNC_0 | IOCON_RMODE_INACT);
					}
				}
			}

			if(displayInProgress && g_updateTemp) {
				g_updateTemp = false;
				temp = g_currentTemp;

				index = (uint16_t)(g_displayTextLen + 9U);
				strcpy(&g_displayText[g_displayTextLen], " Temp is 00.0 F ");
				if(temp < 0) {
					temp = -temp;
					g_displayText[index++] = '-';
				}
				decPosition = 1000;
				prevDigit = 0;
				while(decPosition) {
					digit = (uint16_t)(((uint32_t)temp)/decPosition);
					if(digit) {
						if(decPosition == 1) {
							if(prevDigit == 0) {
								g_displayText[index++] = '0';
							}
							g_displayText[index++] = '.';
						}
						g_displayText[index++] = (char)(digit - prevDigit + '0');
						prevDigit = (uint16_t)(digit *10);
					}
					decPosition /= 10;
				}
				g_displayText[index++] = ' ';
				g_displayText[index++] = 'F';
				g_displayText[index++] = ' ';
				g_displayText[index] = 0;

				sizeLEDPattern = convertStr2LEDPattern(g_displayText, g_ledPattern);

			}
			if ((sTargetWritten) && takeMemSemaphore())  {
				sTargetWritten = false;
				if (NDEFT2T_GetMessage(sNdefInstance, sData, sizeof(sData))) {
					char * data;
					uint8_t *binData;
					int length;
					NDEFT2T_PARSE_RECORD_INFO_T recordInfo;
					while (NDEFT2T_GetNextRecord(sNdefInstance, &recordInfo)) {
						if ((recordInfo.type == NDEFT2T_RECORD_TYPE_TEXT) && (strncmp((char *)recordInfo.pString, "en", 2) == 0)) {
							data = NDEFT2T_GetRecordPayload(sNdefInstance, &length);
							strncpy(g_displayText, data, (size_t)length);
							g_displayText[length] = 0;
							g_displayTextLen = (uint8_t)length;
							eepromWriteTag(EE_DISP_TEXT, (uint8_t *)g_displayText, (uint16_t)(((uint16_t)length+4) & 0xFFFC));
							startLEDDisplay(true);
						}
						else if((recordInfo.type == NDEFT2T_RECORD_TYPE_MIME) && (strncmp((char *)recordInfo.pString, "application/octet-stream", 24) == 0)) {
							binData = NDEFT2T_GetRecordPayload(sNdefInstance, &length);
							if(binData[0] == 0x53) {
								extractMusic(&binData[1]);
								eepromWriteTag(EE_MUSIC_TONE, (uint8_t *)&binData[1], (uint16_t)(((uint16_t)length+2) & 0xFFFC));
								if(musicInProgress) {
									stopMusic();
									startMusic();
								}
							}
							else if(binData[0] == 0x51) {
								Chip_TIMER_MatchDisableInt(LPC_TIMER32_0, 0);
								desiredSpeed = (uint8_t)(binData[1] + 5U);
								if((desiredSpeed < 5) || (desiredSpeed > 30)) {
									desiredSpeed = 20;
								}
								Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, 1000*LED_REFRESH_RATE_MS + Chip_TIMER_ReadCount(LPC_TIMER32_0));
								Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
								eepromWriteTag(EE_SCROLL_SPEED, (uint8_t *)&binData[1], (uint16_t)(((uint16_t)length+3) & 0xFFFC));
							}
						}
					}
				}
				releaseMemSemaphore();
			}

			if(displayInProgress && !sTargetWritten && updateNFC && takeMemSemaphore()) {
				updateNFC = false;
				writeNFCDisplayMsg(g_displayText, strlen(g_displayText));
				releaseMemSemaphore();
			}

			if (Timer_CheckMeasurementTimeout()) {
				if(displayInProgress) {
					TMeas_Measure(TSEN_10BITS, TMEAS_FORMAT_FAHRENHEIT, false, 0 /* Value used in App_TmeasCb */);
				}
				hostTicks++;
				Timer_StartMeasurementTimeout(1);

			}
			__WFI();
		}
    }

    DeInit(); /* Does not return. */
    return 0;
}
