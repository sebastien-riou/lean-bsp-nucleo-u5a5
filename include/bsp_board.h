/* lean-bsp-nucleo-u5a5 internals shared between the generated main.c and src/; not part of the contract. */
#pragma once

void bsp_init(void);  /* called by main() once the vendor bring-up is done, IRQs masked */
void bsp_nmi_report(void);  /* called by NMI_Handler: one line on UART 0, best effort */
