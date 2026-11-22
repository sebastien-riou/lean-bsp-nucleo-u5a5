/* NUCLEO-U5A5ZJ-Q memory map (STM32U5A5ZJ), from the STM32CubeMX-generated STM32U5A5xx_FLASH.ld.
 * The one board-specific fragment: see default.ld for how the fragments fit together. An
 * application's own linker script may override the sizes below by assigning them again after
 * INCLUDE memory.ld. */

ENTRY(Reset_Handler)

MEMORY
{
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 2496K
  SRAM4 (xrw) : ORIGIN = 0x28000000, LENGTH = 16K
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K  /* bank 1: the image */
  NVM0  (rw)  : ORIGIN = 0x08200000, LENGTH = 2048K  /* bank 2: NVM region 0, never loaded (src/bsp_nvm.c) */
}

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size = 0x2000;    /* required amount of heap */
_Min_Stack_Size = 0x40000;  /* required amount of stack: generous, RAM is plentiful on this part */
