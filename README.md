# lean-bsp-nucleo-u5a5

The [lean-bsp](https://github.com/sebastien-riou/lean-bsp) BSP for the ST NUCLEO-U5A5ZJ-Q board
(STM32U5A5ZJ, Cortex-M33 at 160 MHz). Any lean-bsp application builds and runs on it:

````
lbsp build-load-run --bsp nucleo-u5a5 --until 'hello-world done' --timeout 60
````

## What it needs

- The arm-none-eabi toolchain. `CROSS_COMPILE` selects it, e.g.
  `CROSS_COMPILE=/opt/toolchains/arm-none-eabi-gcc/xpack-arm-none-eabi-gcc-14.2.1-1.1/bin/arm-none-eabi-`;
  without it, `arm-none-eabi-gcc` is taken from `PATH`.
- For `load` and `run`, [lean-hwbridge](https://github.com/sebastien-riou/lean-hwbridge) serving the
  board: `hwbridge` on `PATH`, or the command in `HWBRIDGE` (e.g. `HWBRIDGE="python3 -m hwbridge"`).
  The board is its usbman nickname, `HWBRIDGE_BOARD`, by default `nucleo-u5a5`. Without a bridge,
  `load` prints the STM32CubeProgrammer command to flash the board by hand.

## How the contract maps to the board

| Primitive | On NUCLEO-U5A5ZJ-Q |
|---|---|
| UART 0 (and `printf`, `stdin`) | COM1, the ST-LINK virtual COM port, 115200 8N1. Reception is polled: bytes arriving while nobody reads can overrun. |
| LEDs | 0 green, 1 blue, 2 red |
| Button | 0 is the blue user button |
| Cycle counter, CPU frequency | DWT `CYCCNT`, at `SystemCoreClock` (160 MHz) |
| Delay | busy loop on the cycle counter: works with interrupts masked |
| Cache clear | ICACHE and DCACHE1 invalidated, flash prefetch buffer toggled |
| Cycle timer | TIM5 at the CPU clock (one tick per cycle), one-pulse; its interrupt reaches the handler through a RAM copy of the vector table. SysTick and the button interrupt are held off while it runs. |
| Heap usage | growth of the heap end since the last reset, tracked in `_sbrk` (`Core/Src/sysmem.c`) |
| Reset | `NVIC_SystemReset()` |
| `app_tick` | from `SysTick_Handler`, every millisecond |
| `app_on_fault` | from the HardFault, MemManage, BusFault and UsageFault handlers (`pc` is 0) |
| `app_on_exit` | default does nothing: `main()` then parks the CPU |
| Flash/NVM | not yet |

## Layout

The board support is an STM32CubeMX project (`nucleo-u5a5.ioc`), kept regeneration-safe: lean-bsp
code is only in `USER CODE` sections of the generated files (the glue in `main()`, `app_tick` and
`app_on_fault` in `stm32u5xx_it.c`) and in files CubeMX does not generate.

- `Core/`, `Drivers/`, `startup_stm32u5a5xx.s`: the vendor-generated bring-up (HAL, CMSIS, Nucleo BSP;
  device headers for this part only).
- `src/bsp_nucleo_u5a5.c`: the primitives, `bsp_init()` and the default application hooks.
- `include/bsp_api.h`: the contract header, identical to lean-bsp's canonical copy (`lbsp check` verifies it).
- `linker/`: `memory.ld` (this board's memory map), the Cortex-M `sections_head/data/tail.ld`, and
  `default.ld` which includes them. An application needing its own sections copies `default.ld`.
- `bsp.toml`, `bsp_cli.py`: what lean-bsp reads and runs. `bsp_cli.py` is this board's data on top of lean-bsp's
  board helpers (`leanbsp.backends`), which lean-bsp runs it with.

To open it in STM32CubeIDE, regenerate an IDE project from `nucleo-u5a5.ioc` with STM32CubeMX.
