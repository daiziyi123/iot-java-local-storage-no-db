/* startup_gd32f450_470.S - GNU ARM Assembler format
 * For GD32F470ZK Cortex-M4
 */
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.global g_pfnVectors
.global Default_Handler
.global fault_info

.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss

.equ Stack_Size, 0x400
.equ Heap_Size,  0x200

    .section .stack, "w", %nobits
    .align 3
    .globl __stack
__stack:
    .space Stack_Size

    .section .heap, "w", %nobits
    .align 3
    .globl __heap_base
__heap_base:
    .space Heap_Size
    .globl __heap_limit
__heap_limit:

    .section .isr_vector, "a", %progbits
    .align 2
    .globl g_pfnVectors
g_pfnVectors:
    .word __stack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler
    /* External interrupts */
    .word WWDGT_IRQHandler
    .word LVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCU_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA0_Channel0_IRQHandler
    .word DMA0_Channel1_IRQHandler
    .word DMA0_Channel2_IRQHandler
    .word DMA0_Channel3_IRQHandler
    .word DMA0_Channel4_IRQHandler
    .word DMA0_Channel5_IRQHandler
    .word DMA0_Channel6_IRQHandler
    .word ADC0_1_IRQHandler
    .word USART0_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word TI2_IRQHandler
    .word TI3_IRQHandler
    .word TI4_IRQHandler
    .word I2C0_EV_IRQHandler
    .word I2C0_ER_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word SPI0_IRQHandler
    .word SPI1_IRQHandler
    .word USART3_IRQHandler
    .word EXTI5_9_IRQHandler
    .word EXTI10_15_IRQHandler
    .word RTC_Alarm_IRQHandler
    .word USBFS_Wakeup_IRQHandler
    .word TIMER0_BRK_IRQHandler
    .word TIMER0_UP_IRQHandler
    .word TIMER0_TRG_CMT_IRQHandler
    .word TIMER0_Channel_IRQHandler
    .word TIMER7_BRK_IRQHandler
    .word TIMER7_UP_IRQHandler
    .word TIMER7_TRG_CMT_IRQHandler
    .word TIMER7_Channel_IRQHandler
    .word DMA0_Channel7_IRQHandler
    .word EXTI16_IRQHandler
    .word EXTI17_IRQHandler
    .word EXTI18_IRQHandler
    .word EXTI19_IRQHandler
    .word CMP0_IRQHandler
    .word CMP1_IRQHandler
    .word CMP2_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI2_IRQHandler
    .word USART4_IRQHandler
    .word USART5_IRQHandler
    .word TIMER1_BRK_IRQHandler
    .word TIMER1_UP_IRQHandler
    .word TIMER1_TRG_CMT_IRQHandler
    .word TIMER1_Channel_IRQHandler
    .word TIMER8_BRK_IRQHandler
    .word TIMER8_UP_IRQHandler
    .word TIMER8_TRG_CMT_IRQHandler
    .word TIMER8_Channel_IRQHandler
    .word DMA1_Channel0_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler
    .word USART6_IRQHandler
    .word USART7_IRQHandler
    .word USART8_IRQHandler
    .word TIMER9_IRQHandler
    .word TIMER10_IRQHandler
    .word TIMER11_IRQHandler
    .word TIMER12_IRQHandler
    .word TIMER13_IRQHandler
    .word TIMER14_IRQHandler
    .word SPI3_IRQHandler
    .word I2C3_EV_IRQHandler
    .word I2C3_ER_IRQHandler
    .word SPI4_IRQHandler
    .word TIMER15_IRQHandler
    .word TIMER16_IRQHandler
    .word TIMER17_IRQHandler
    .word TIMER18_IRQHandler
    .word USART9_IRQHandler
    .word USART10_IRQHandler
    .word USART11_IRQHandler
    .word USART12_IRQHandler
    .word USART13_IRQHandler
    .word USART14_IRQHandler
    .word I2C4_EV_IRQHandler
    .word I2C4_ER_IRQHandler
    .word I2C5_EV_IRQHandler
    .word I2C5_ER_IRQHandler

    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* [修复] 设置 VTOR(向量表偏移寄存器) 为 0x08000000。
     * GD32F470 启动时 VTOR=0，指向 BootROM 而非 Flash。
     * 不加此行，第一个中断(SysTick/USART/TIMER)会跳到错误的向量表 → HardFault。 */
    ldr r0, =0xE000ED08
    ldr r1, =g_pfnVectors
    str r1, [r0]

    /* [修复] 在 data/bss 拷贝前关闭中断，在 SystemInit + main 之后再打开。
     * 防止任何中断在 C 运行时环境就绪之前触发。 */
    cpsid i

    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit

    ldr r2, =_sbss
    ldr r4, =_ebss
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str r3, [r2]
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss

    bl SystemInit
    cpsie i
    bl main
LoopForever:
    b LoopForever
.size Reset_Handler, .-Reset_Handler

    .section .text.Default_Handler
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b .
.size Default_Handler, .-Default_Handler

    .section .text.HardFault_Handler, "ax", %progbits
    .globl HardFault_Handler
    .type HardFault_Handler, %function
HardFault_Handler:
    ldr r0, =fault_info
    stm r0, {r4-r7}
    ldr r1, =0xE000ED04
    ldr r2, [r1]
    str r2, [r0, #32]
    ldr r1, =0xE000ED28
    ldr r2, [r1]
    str r2, [r0, #36]
    ldr r1, =0xE000ED2C
    ldr r2, [r1]
    str r2, [r0, #40]
    ldr r1, =0xE000ED34
    ldr r2, [r1]
    str r2, [r0, #44]
    ldr r1, =0xE000ED38
    ldr r2, [r1]
    str r2, [r0, #48]
FaultLoop:
    b FaultLoop
.size HardFault_Handler, .-HardFault_Handler

    .section .bss.fault_info, "w", %nobits
    .globl fault_info
    .align 4
fault_info:
    .space 64

    .macro IRQ handler_name
    .section .text.\handler_name
    .weak \handler_name
    .set \handler_name, Default_Handler
    .endm

    IRQ NMI_Handler
    IRQ MemManage_Handler
    IRQ BusFault_Handler
    IRQ UsageFault_Handler
    IRQ SVC_Handler
    IRQ DebugMon_Handler
    IRQ PendSV_Handler
    IRQ SysTick_Handler
    IRQ WWDGT_IRQHandler
    IRQ LVD_IRQHandler
    IRQ TAMPER_IRQHandler
    IRQ RTC_IRQHandler
    IRQ FLASH_IRQHandler
    IRQ RCU_IRQHandler
    IRQ EXTI0_IRQHandler
    IRQ EXTI1_IRQHandler
    IRQ EXTI2_IRQHandler
    IRQ EXTI3_IRQHandler
    IRQ EXTI4_IRQHandler
    IRQ DMA0_Channel0_IRQHandler
    IRQ DMA0_Channel1_IRQHandler
    IRQ DMA0_Channel2_IRQHandler
    IRQ DMA0_Channel3_IRQHandler
    IRQ DMA0_Channel4_IRQHandler
    IRQ DMA0_Channel5_IRQHandler
    IRQ DMA0_Channel6_IRQHandler
    IRQ ADC0_1_IRQHandler
    IRQ USART0_IRQHandler
    IRQ USART1_IRQHandler
    IRQ USART2_IRQHandler
    IRQ TI2_IRQHandler
    IRQ TI3_IRQHandler
    IRQ TI4_IRQHandler
    IRQ I2C0_EV_IRQHandler
    IRQ I2C0_ER_IRQHandler
    IRQ I2C1_EV_IRQHandler
    IRQ I2C1_ER_IRQHandler
    IRQ SPI0_IRQHandler
    IRQ SPI1_IRQHandler
    IRQ USART3_IRQHandler
    IRQ EXTI5_9_IRQHandler
    IRQ EXTI10_15_IRQHandler
    IRQ RTC_Alarm_IRQHandler
    IRQ USBFS_Wakeup_IRQHandler
    IRQ TIMER0_BRK_IRQHandler
    IRQ TIMER0_UP_IRQHandler
    IRQ TIMER0_TRG_CMT_IRQHandler
    IRQ TIMER0_Channel_IRQHandler
    IRQ TIMER7_BRK_IRQHandler
    IRQ TIMER7_UP_IRQHandler
    IRQ TIMER7_TRG_CMT_IRQHandler
    IRQ TIMER7_Channel_IRQHandler
    IRQ DMA0_Channel7_IRQHandler
    IRQ EXTI16_IRQHandler
    IRQ EXTI17_IRQHandler
    IRQ EXTI18_IRQHandler
    IRQ EXTI19_IRQHandler
    IRQ CMP0_IRQHandler
    IRQ CMP1_IRQHandler
    IRQ CMP2_IRQHandler
    IRQ I2C2_EV_IRQHandler
    IRQ I2C2_ER_IRQHandler
    IRQ SPI2_IRQHandler
    IRQ USART4_IRQHandler
    IRQ USART5_IRQHandler
    IRQ TIMER1_BRK_IRQHandler
    IRQ TIMER1_UP_IRQHandler
    IRQ TIMER1_TRG_CMT_IRQHandler
    IRQ TIMER1_Channel_IRQHandler
    IRQ TIMER8_BRK_IRQHandler
    IRQ TIMER8_UP_IRQHandler
    IRQ TIMER8_TRG_CMT_IRQHandler
    IRQ TIMER8_Channel_IRQHandler
    IRQ DMA1_Channel0_IRQHandler
    IRQ DMA1_Channel1_IRQHandler
    IRQ DMA1_Channel2_IRQHandler
    IRQ DMA1_Channel3_IRQHandler
    IRQ DMA1_Channel4_IRQHandler
    IRQ DMA1_Channel5_IRQHandler
    IRQ DMA1_Channel6_IRQHandler
    IRQ DMA1_Channel7_IRQHandler
    IRQ USART6_IRQHandler
    IRQ USART7_IRQHandler
    IRQ USART8_IRQHandler
    IRQ TIMER9_IRQHandler
    IRQ TIMER10_IRQHandler
    IRQ TIMER11_IRQHandler
    IRQ TIMER12_IRQHandler
    IRQ TIMER13_IRQHandler
    IRQ TIMER14_IRQHandler
    IRQ SPI3_IRQHandler
    IRQ I2C3_EV_IRQHandler
    IRQ I2C3_ER_IRQHandler
    IRQ SPI4_IRQHandler
    IRQ TIMER15_IRQHandler
    IRQ TIMER16_IRQHandler
    IRQ TIMER17_IRQHandler
    IRQ TIMER18_IRQHandler
    IRQ USART9_IRQHandler
    IRQ USART10_IRQHandler
    IRQ USART11_IRQHandler
    IRQ USART12_IRQHandler
    IRQ USART13_IRQHandler
    IRQ USART14_IRQHandler
    IRQ I2C4_EV_IRQHandler
    IRQ I2C4_ER_IRQHandler
    IRQ I2C5_EV_IRQHandler
    IRQ I2C5_ER_IRQHandler
