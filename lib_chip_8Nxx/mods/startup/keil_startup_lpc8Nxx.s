;/*
; * @brief LPC8Nxx startup code for Keil (Cortex-M0+)
; *
; * @note
; * Copyright(C) NXP Semiconductors, 2018
; * All rights reserved.
; *
; * @par
; * Software that is described herein is for illustrative purposes only
; * which provides customers with programming information regarding the
; * LPC products.  This software is supplied "AS IS" without any warranties of
; * any kind, and NXP Semiconductors and its licensor disclaim any and
; * all warranties, express or implied, including all implied warranties of
; * merchantability, fitness for a particular purpose and non-infringement of
; * intellectual property rights.  NXP Semiconductors assumes no responsibility
; * or liability for the use of the software, conveys no license or rights under any
; * patent, copyright, mask work right, or any other intellectual property rights in
; * or to any products. NXP Semiconductors reserves the right to make changes
; * in the software without notification. NXP Semiconductors also makes no
; * representation or warranty that such application will be suitable for the
; * specified use without further testing or modification.
; *
; * @par
; * Permission to use, copy, modify, and distribute this software and its
; * documentation is hereby granted, under NXP Semiconductors' and its
; * licensor's relevant copyrights in the software, without fee, provided that it
; * is used in conjunction with NXP Semiconductors microcontrollers.  This
; * copyright, permission, and disclaimer notice must appear in all copies of
; * this code.
; */

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000100

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler

                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     MemManage_Handler
                DCD     BusFault_Handler
                DCD     UsageFault_Handler
__vector_table_0x1c
                DCD     0                         ; Checksum of the first 7 words
                DCD     0
                DCD     0                         ; Enhanced image marker, set to 0x0 for legacy boot
                DCD     0                         ; Pointer to enhanced boot block, set to 0x0 for legacy boot
                DCD     SVC_Handler
                DCD     DebugMon_Handler
                DCD     0
                DCD     PendSV_Handler
                DCD     SysTick_Handler

                ; External Interrupts
                DCD     PIO0_0_IRQHandler        ; PIO INT0
                DCD     PIO0_1_IRQHandler        ; PIO INT1
                DCD     PIO0_2_IRQHandler        ; PIO INT2
                DCD     PIO0_3_IRQHandler        ; PIO INT3
                DCD     PIO0_4_IRQHandler       ; PIO INT4
                DCD     PIO0_5_IRQHandler       ; PIO INT5
                DCD     PIO0_6_IRQHandler       ; PIO INT6
                DCD     PIO0_7_IRQHandler       ; PIO INT7
                DCD     PIO0_8_IRQHandler          ; PIO INT8
                DCD     PIO0_9_IRQHandler            ; PIO INT9
                DCD     PIO0_10_IRQHandler         ; PIO INT10
                DCD     RFFIELD_IRQHandler         ; CT32B1
                DCD     RTCPWREQ_IRQHandler           ; Smart Counter Timer
                DCD     NFC_IRQHandler         ; CT32B3
                DCD     RTC_IRQHandler      ; FLEXCOMM0
                DCD     I2C0_IRQHandler      ; FLEXCOMM1
                DCD     CT16B0_IRQHandler      ; FLEXCOMM2
                DCD     PMUFLD_IRQHandler      ; FLEXCOMM3
                DCD     CT32B0_IRQHandler      ; FLEXCOMM4
                DCD     PMUBOD_IRQHandler      ; FLEXCOMM5
                DCD     SSP0_IRQHandler      ; FLEXCOMM6
                DCD     TSEN_IRQHandler      ; FLEXCOMM7
                DCD     C2D_IRQHandler       ; ADC0 A sequence (A/D Converter) interrupt
                DCD     0                   ; ADC0 B sequence (A/D Converter) interrupt
                DCD     I2D_IRQHandler      ; ADC THCMP and OVERRUN ORed
                DCD     ADC_IRQHandler           ; Digital MIC
                DCD     WDT_IRQHandler          ; Voice Activity detect
                DCD     FLASH_IRQHandler    ; USB NeedCLK
                DCD     EEPROM_IRQHandler            ; USB
                DCD     0            ; RTC Timer
                DCD     0            ; IOH Handler
                DCD     PIO0_IRQHandler        ; Mailbox

;//   <h> Code Read Protection level (CRP)
;//     <o>    CRP_Level:
;//                     <0xFFFFFFFF=> Disabled
;//                     <0x4E697370=> NO_ISP
;//                     <0x12345678=> CRP1
;//                     <0x87654321=> CRP2
;//                     <0x43218765=> CRP3 (Are you sure?)
;//   </h>
CRP_Level       EQU     0xFFFFFFFF

                IF      :LNOT::DEF:NO_CRP
                AREA    |.ARM.__at_0x02FC|, CODE, READONLY
CRP_Key         DCD     0xFFFFFFFF
                ENDIF

                AREA    |.text|, CODE, READONLY


; Reset Handler - shared for both cores
Reset_Handler   PROC
                EXPORT  Reset_Handler               [WEAK]
                EXPORT  SystemInit                  [WEAK]
                IMPORT  __main

                LDR     r0, =SystemInit
                BLX     r0
                LDR     r0, =__main
                BX      r0
                ENDP

; For cores with SystemInit() or __main(), the code will sleep the MCU
SystemInit      PROC
                EXPORT  SystemInit                [WEAK]
                BX    lr
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)
NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP

HardFault_Handler \
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP

MemManage_Handler     PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP

BusFault_Handler PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP

UsageFault_Handler PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP

SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP

DebugMon_Handler PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP

PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP

SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT     PIO0_0_IRQHandler        [WEAK] ; Watchdog Ored with BOD
                EXPORT     PIO0_1_IRQHandler            [WEAK] ; DMA Controller
                EXPORT     PIO0_2_IRQHandler          [WEAK] ; GPIO Group0 Interrupt
                EXPORT     PIO0_3_IRQHandler          [WEAK] ; GPIO Group1 Interrupt
                EXPORT     PIO0_4_IRQHandler       [WEAK] ; PIO INT0
                EXPORT     PIO0_5_IRQHandler       [WEAK] ; PIO INT1
                EXPORT     PIO0_6_IRQHandler       [WEAK] ; PIO INT2
                EXPORT     PIO0_7_IRQHandler       [WEAK] ; PIO INT3
                EXPORT     PIO0_8_IRQHandler          [WEAK] ; UTICK timer
                EXPORT     PIO0_9_IRQHandler            [WEAK] ; Multi-Rate Timer
                EXPORT     PIO0_10_IRQHandler         [WEAK] ; CT32B0
                EXPORT     RFFIELD_IRQHandler         [WEAK] ; CT32B1
                EXPORT     RTCPWREQ_IRQHandler           [WEAK] ; Smart Counter Timer
                EXPORT     NFC_IRQHandler         [WEAK] ; CT32B3
                EXPORT     RTC_IRQHandler      [WEAK] ; FLEXCOMM0
                EXPORT     I2C0_IRQHandler      [WEAK] ; FLEXCOMM1
                EXPORT     CT16B0_IRQHandler      [WEAK] ; FLEXCOMM2
                EXPORT     PMUFLD_IRQHandler      [WEAK] ; FLEXCOMM3
                EXPORT     CT32B0_IRQHandler      [WEAK] ; FLEXCOMM4
                EXPORT     PMUBOD_IRQHandler      [WEAK] ; FLEXCOMM5
                EXPORT     SSP0_IRQHandler      [WEAK] ; FLEXCOMM6
                EXPORT     TSEN_IRQHandler      [WEAK] ; FLEXCOMM7
                EXPORT     C2D_IRQHandler       [WEAK] ; ADC0 A sequence (A/D Converter) interrupt
                EXPORT     I2D_IRQHandler      [WEAK] ; ADC THCMP and OVERRUN ORed
                EXPORT     ADC_IRQHandler           [WEAK] ; Digital MIC
                EXPORT     WDT_IRQHandler          [WEAK] ; Voice Activity detect
                EXPORT     FLASH_IRQHandler    [WEAK] ; USB NeedCLK
                EXPORT     EEPROM_IRQHandler            [WEAK] ; USB
                EXPORT     PIO0_IRQHandler        [WEAK] ; Mailbox

PIO0_0_IRQHandler     ; Watchdog Ored with BOD
PIO0_1_IRQHandler         ; DMA Controller
PIO0_2_IRQHandler      ; GPIO Group0 Interrupt
PIO0_3_IRQHandler      ; GPIO Group1 Interrupt
PIO0_4_IRQHandler      ; PIO INT0
PIO0_5_IRQHandler      ; PIO INT1
PIO0_6_IRQHandler      ; PIO INT2
PIO0_7_IRQHandler      ; PIO INT3
PIO0_8_IRQHandler      ; UTICK timer
PIO0_9_IRQHandler      ; Multi-Rate Timer
PIO0_10_IRQHandler     ; CT32B0
RFFIELD_IRQHandler     ; CT32B1
RTCPWREQ_IRQHandler    ; Smart Counter Timer
NFC_IRQHandler         ; CT32B3
RTC_IRQHandler         ; FLEXCOMM0
I2C0_IRQHandler        ; FLEXCOMM1
CT16B0_IRQHandler      ; FLEXCOMM2
PMUFLD_IRQHandler      ; FLEXCOMM3
CT32B0_IRQHandler      ; FLEXCOMM4
PMUBOD_IRQHandler      ; FLEXCOMM5
SSP0_IRQHandler        ; FLEXCOMM6
TSEN_IRQHandler        ; FLEXCOMM7
C2D_IRQHandler         ; ADC0 A sequence (A/D Converter) interrupt
I2D_IRQHandler         ; ADC0 B sequence (A/D Converter) interrupt
ADC_IRQHandler         ; ADC THCMP and OVERRUN ORed
WDT_IRQHandler         ; Digital MIC
FLASH_IRQHandler       ; Voice Activity detect
EEPROM_IRQHandler      ; USB NeedCLK
PIO0_IRQHandler        ; USB

                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
