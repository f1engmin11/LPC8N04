;/*****************************************************************************
; * @file:    startup_LPC8Nxx.s
; * @purpose: CMSIS Cortex-M4/M0+ Core Device Startup File
; *           for the NXP LPC8Nxx Device Series (manually edited)
; * @version: V1.00
; * @date:    5. January 2018
; *----------------------------------------------------------------------------
; *
; * Copyright (C) 2018 ARM Limited. All rights reserved.
; *
; * ARM Limited (ARM) is supplying this software for use with Cortex-Mx
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler

        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     0
        DCD     0
        DCD     0
__vector_table_0x1c
        DCD     0                            ; Checksum of the first 7 words
        DCD     0
        DCD     0                            ; Enhanced image marker, set to 0x0 for legacy boot
        DCD     0                            ; Pointer to enhanced boot block, set to 0x0 for legacy boot
        DCD     SVC_Handler
        DCD     0
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     PIO0_0_IRQHandler       ; PIO INT0
        DCD     PIO0_1_IRQHandler       ; PIO INT1
        DCD     PIO0_2_IRQHandler       ; PIO INT2
        DCD     PIO0_3_IRQHandler       ; PIO INT3
        DCD     PIO0_4_IRQHandler       ; PIO INT4
        DCD     PIO0_5_IRQHandler       ; PIO INT5
        DCD     PIO0_6_IRQHandler       ; PIO INT6
        DCD     PIO0_7_IRQHandler       ; PIO INT7
        DCD     PIO0_8_IRQHandler       ; PIO INT8
        DCD     PIO0_9_IRQHandler       ; PIO INT9
        DCD     PIO0_10_IRQHandler      ; PIO INT10
        DCD     RFFIELD_IRQHandler      ; RF Field Wakeup
        DCD     RTCPWREQ_IRQHandler     ; RTC Wakeup
        DCD     NFC_IRQHandler          ; NFC
        DCD     RTC_IRQHandler          ; RTC event
        DCD     I2C0_IRQHandler         ; I2C
        DCD     CT16B0_IRQHandler       ; CT16B0
        DCD     PMUFLD_IRQHandler       ; Power from Field Detected
        DCD     CT32B0_IRQHandler       ; CT32B0
        DCD     PMUBOD_IRQHandler       ; BOD
        DCD     SSP0_IRQHandler         ; SSP0
        DCD     TSEN_IRQHandler         ; Temperature Sensor
        DCD     0                       ; Reserved
        DCD     0                       ; Reserved
        DCD     0                       ; Reserved
        DCD     0                       ; Reserved
        DCD     WDT_IRQHandler          ; WatchDog
        DCD     FLASH_IRQHandler        ; Flash
        DCD     EEPROM_IRQHandler       ; EEPROM
        DCD     0                       ; Reserved
        DCD     0                       ; Reserved
        DCD     PIO0_IRQHandler         ; Port 0 Interrupt

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;

        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
; Reset Handler
Reset_Handler

                LDR     r0, =SystemInit
                BLX     r0
                LDR     r0, =__iar_program_start
                BX      r0

; For cores with SystemInit() or __iar_program_start(), the code will sleep the MCU
        PUBWEAK SystemInit
        SECTION .text:CODE:REORDER:NOROOT(1)
SystemInit
        BX      LR

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B .

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B .

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B .

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B .

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B .

    PUBWEAK Reserved_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
Reserved_IRQHandler
        B .

        PUBWEAK     PIO0_0_IRQHandler      ; PIO INT0
        PUBWEAK     PIO0_1_IRQHandler      ; PIO INT1
        PUBWEAK     PIO0_2_IRQHandler      ; PIO INT2
        PUBWEAK     PIO0_3_IRQHandler      ; PIO INT3
        PUBWEAK     PIO0_4_IRQHandler      ; PIO INT4
        PUBWEAK     PIO0_5_IRQHandler      ; PIO INT5
        PUBWEAK     PIO0_6_IRQHandler      ; PIO INT6
        PUBWEAK     PIO0_7_IRQHandler      ; PIO INT7
        PUBWEAK     PIO0_8_IRQHandler      ; PIO INT8
        PUBWEAK     PIO0_9_IRQHandler      ; PIO INT9
        PUBWEAK     PIO0_10_IRQHandler     ; PIO INT10
        PUBWEAK     RFFIELD_IRQHandler     ; RF Field Wakeup
        PUBWEAK     RTCPWREQ_IRQHandler    ; RTC Wakeup
        PUBWEAK     NFC_IRQHandler         ; NFC
        PUBWEAK     RTC_IRQHandler         ; RTC event
        PUBWEAK     I2C0_IRQHandler        ; I2C
        PUBWEAK     CT16B0_IRQHandler      ; CT16B0
        PUBWEAK     PMUFLD_IRQHandler      ; Power from Field Detected
        PUBWEAK     CT32B0_IRQHandler      ; CT32B0
        PUBWEAK     PMUBOD_IRQHandler      ; BOD
        PUBWEAK     SSP0_IRQHandler        ; SSP0
        PUBWEAK     TSEN_IRQHandler        ; Temperature Sensor
        PUBWEAK     WDT_IRQHandler         ; WatchDog
        PUBWEAK     FLASH_IRQHandler       ; Flash
        PUBWEAK     EEPROM_IRQHandler      ; EEPROM
        PUBWEAK     PIO0_IRQHandler        ; Port 0 Interrupt

PIO0_0_IRQHandler      ; PIO INT0
PIO0_1_IRQHandler      ; PIO INT1
PIO0_2_IRQHandler      ; PIO INT2
PIO0_3_IRQHandler      ; PIO INT3
PIO0_4_IRQHandler      ; PIO INT4
PIO0_5_IRQHandler      ; PIO INT5
PIO0_6_IRQHandler      ; PIO INT6
PIO0_7_IRQHandler      ; PIO INT7
PIO0_8_IRQHandler      ; PIO INT8
PIO0_9_IRQHandler      ; PIO INT9
PIO0_10_IRQHandler     ; PIO INT10
RFFIELD_IRQHandler     ; RF Field Wakeup
RTCPWREQ_IRQHandler    ; RTC Wakeup
NFC_IRQHandler         ; NFC
RTC_IRQHandler         ; RTC event
I2C0_IRQHandler        ; I2C
CT16B0_IRQHandler      ; CT16B0
PMUFLD_IRQHandler      ; Power from Field Detected
CT32B0_IRQHandler      ; CT32B0
PMUBOD_IRQHandler      ; BOD
SSP0_IRQHandler        ; SSP0
TSEN_IRQHandler        ; Temperature Sensor
WDT_IRQHandler         ; WatchDog
FLASH_IRQHandler       ; Flash
EEPROM_IRQHandler      ; EEPROM
PIO0_IRQHandler        ; Port 0 Interrupt

Default_Handler:
        B .

        END
