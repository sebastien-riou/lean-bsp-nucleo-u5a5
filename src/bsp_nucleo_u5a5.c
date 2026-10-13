/* NUCLEO-U5A5ZJ-Q: the lean-bsp contract on top of the STM32U5 HAL and the Nucleo BSP.
 *
 * The vendor-generated main() (Core/Src/main.c) brings the board up and initialises the LEDs, the
 * user button and COM1, the ST-LINK virtual COM port; its USER CODE BSP section then calls
 * bsp_init() and the application. Heap tracking lives in Core/Src/sysmem.c, next to _sbrk. */
#include "main.h"
#include "bsp_api.h"
#include "bsp_board.h"

extern DCACHE_HandleTypeDef hdcache1;

static const bsp_capabilities_t capabilities = {
    .board_name = "nucleo-u5a5",
    .n_uarts = 1,
    .n_leds = 3,
    .n_buttons = 1,
    .has_cycle_counter = true,
    .n_nvm_regions = 0,
    .nvm_regions = NULL,
};

const bsp_capabilities_t *bsp_get_capabilities(void) { return &capabilities; }

/* ---- the application hooks' defaults ---- */

__attribute__((weak)) void app_init(void) {}

__attribute__((weak)) void app_tick(uint32_t tick_ms) { (void)tick_ms; }

__attribute__((weak)) void app_on_fault(uint32_t reason, uintptr_t pc) {
    (void)reason;
    (void)pc;
}

__attribute__((weak)) void app_on_exit(int code) { (void)code; }  /* main() then parks the CPU */

void bsp_init(void) {
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;  /* the cycle counter */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    bsp_heap_usage_reset();
}

/* ---- UART 0: COM1, the ST-LINK virtual COM port; stdin and stdout too ---- */

static UART_HandleTypeDef *uart0(void) { return &hcom_uart[COM1]; }

void bsp_uart_tx(unsigned port, const void *buf, size_t len) {
    if (port != 0) return;
    const uint8_t *p = buf;
    while (len) {
        uint16_t n = len > 0xFFFFu ? 0xFFFFu : (uint16_t)len;
        HAL_UART_Transmit(uart0(), (uint8_t *)p, n, HAL_MAX_DELAY);  /* bounded by the baud rate */
        p += n;
        len -= n;
    }
}

/* Polled: what arrives while nobody calls this can overrun the peripheral's FIFO; an interrupt-fed
 * ring buffer is the obvious next step once an application needs continuous input. */
size_t bsp_uart_rx(unsigned port, void *buf, size_t len) {
    if (port != 0) return 0;
    UART_HandleTypeDef *h = uart0();
    if (__HAL_UART_GET_FLAG(h, UART_FLAG_ORE)) __HAL_UART_CLEAR_FLAG(h, UART_CLEAR_OREF);
    uint8_t *p = buf;
    size_t n = 0;
    while (n < len && __HAL_UART_GET_FLAG(h, UART_FLAG_RXNE)) p[n++] = (uint8_t)(h->Instance->RDR & 0xFFu);
    return n;
}

/* newlib's stdout and stdin, overriding the weak ones of Core/Src/syscalls.c */
int _write(int file, char *ptr, int len) {
    (void)file;
    if (len > 0) bsp_uart_tx(0, ptr, (size_t)len);
    return len;
}

int _read(int file, char *ptr, int len) {
    (void)file;
    if (len <= 0) return 0;
    size_t n;
    while ((n = bsp_uart_rx(0, ptr, (size_t)len)) == 0) {
    }
    return (int)n;
}

/* ---- LEDs (green, blue, red) and the user button ---- */

static const Led_TypeDef leds[] = {LED_GREEN, LED_BLUE, LED_RED};

void bsp_led_set(unsigned id, bool on) {
    if (id >= sizeof leds / sizeof leds[0]) return;
    if (on) {
        BSP_LED_On(leds[id]);
    } else {
        BSP_LED_Off(leds[id]);
    }
}

bool bsp_button_get(unsigned id) { return id == 0 && BSP_PB_GetState(BUTTON_USER) != 0; }

/* ---- time: the DWT cycle counter, at the core clock ---- */

uint32_t bsp_cycle_count(void) { return DWT->CYCCNT; }

uint32_t bsp_cpu_freq_hz(void) { return SystemCoreClock; }

/* On the cycle counter rather than HAL_Delay: works with interrupts masked, and is bounded. */
void bsp_delay_ms(uint32_t ms) {
    const uint32_t per_ms = SystemCoreClock / 1000u;
    while (ms--) {
        const uint32_t start = DWT->CYCCNT;
        while (DWT->CYCCNT - start < per_ms) {
        }
    }
}

void bsp_reset(void) { NVIC_SystemReset(); }

/* The instruction cache, the data cache and the flash prefetch buffer, as crypto-benchmark-stm32u5
 * clears them before a measured run. */
void bsp_clear_caches(void) {
    __HAL_FLASH_PREFETCH_BUFFER_DISABLE();
    HAL_ICACHE_Invalidate();
    HAL_ICACHE_WaitForInvalidateComplete();
    HAL_DCACHE_Invalidate(&hdcache1);
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
}
