#!/usr/bin/env python3
"""lean-bsp entry point of the NUCLEO-U5A5ZJ-Q BSP, made of lean-bsp's board helpers (leanbsp.backends).

build compiles the vendor-generated board support (Core/, Drivers/), src/ and the application with
the arm-none-eabi toolchain named by CROSS_COMPILE (default: arm-none-eabi- from PATH), with the
CPU flags of bsp.toml, and links with linker/default.ld (or the application's own script). load
and run go through lean-hwbridge; the board is the usbman nickname in HWBRIDGE_BOARD, default
nucleo-u5a5.
"""

import json
import sys
from pathlib import Path

try:
    from leanbsp.backends import board, gcc
except ImportError:
    print(json.dumps({'status': 'error', 'message': 'this BSP needs a lean-bsp with leanbsp.backends: update lean-bsp'}))
    sys.exit(1)

ROOT = Path(__file__).resolve().parent

HAL = [
    'cortex', '', 'i2c', 'i2c_ex', 'rcc', 'rcc_ex', 'flash', 'flash_ex', 'gpio', 'dma', 'dma_ex', 'exti', 'pwr',
    'pwr_ex', 'gtzc', 'icache', 'usart', 'usart_ex', 'uart_ex', 'uart', 'dcache',
]
SOURCES = [
    *sorted((ROOT / 'Core' / 'Src').glob('*.c')),
    ROOT / 'Drivers' / 'BSP' / 'STM32U5xx_Nucleo' / 'stm32u5xx_nucleo.c',
    *(ROOT / 'Drivers' / 'STM32U5xx_HAL_Driver' / 'Src' / f'stm32u5xx_hal{"_" + m if m else ""}.c' for m in HAL),
    *sorted((ROOT / 'src').glob('*.c')),
    ROOT / 'startup_stm32u5a5xx.s',
]
INCLUDE_DIRS = [
    ROOT / 'include',
    ROOT / 'Core' / 'Inc',
    ROOT / 'Drivers' / 'STM32U5xx_HAL_Driver' / 'Inc',
    ROOT / 'Drivers' / 'STM32U5xx_HAL_Driver' / 'Inc' / 'Legacy',
    ROOT / 'Drivers' / 'BSP' / 'STM32U5xx_Nucleo',
    ROOT / 'Drivers' / 'CMSIS' / 'Device' / 'ST' / 'STM32U5xx' / 'Include',
    ROOT / 'Drivers' / 'CMSIS' / 'Include',
]
DEFINES = ['USE_HAL_DRIVER', 'STM32U5A5xx']  # USE_NUCLEO_144 comes from stm32u5xx_nucleo_conf.h


def build(app: dict, target, bdir: Path) -> dict:
    return gcc.build(app, target, bdir, root=ROOT, sources=SOURCES, include_dirs=INCLUDE_DIRS, defines=DEFINES,
                     linker_script=ROOT / 'linker' / 'default.ld', link_flags=[f'-L{ROOT / "linker"}', *gcc.HEAP_WRAP_FLAGS])


BOARD = board.Board(
    root=ROOT,
    title='NUCLEO-U5A5ZJ-Q',
    flash_by_hand='STM32_Programmer_CLI --connect port=swd mode=UR --download {hex} -rst',
    run_by_hand='Press the black RESET button and read the ST-LINK virtual COM port',
)

if __name__ == '__main__':
    sys.exit(board.main(BOARD, build))
