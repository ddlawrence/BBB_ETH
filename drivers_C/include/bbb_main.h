//
// BeagleboneBlack Bare Metal  -  main header file
//
#include "string.h"
#include <stdint.h>

// gpio
extern uint32_t gpio_irq_count;
extern uint32_t gpio_init(uint32_t gpio_base_addr, uint32_t gpio_pins);
extern void gpio_on(uint32_t gpio_base_addr, uint32_t gpio_pins);
extern void gpio_off(uint32_t gpio_base_addr, uint32_t gpio_pins);
extern uint32_t gpio_read(uint32_t gpio_base_addr);
extern void blink32(uint32_t data);

// uart
#define CONF_UART_0_RXD            0x970
#define CONF_UART_0_TXD            0x974
extern uint32_t uart0_irq_count;
extern uint32_t uart0_rbuf;
extern uint32_t uart0_tbuf;
extern uint32_t uart0_init();
extern uint32_t uart_rx(uint32_t uart_base_addr);
extern void uart_tx(uint32_t uart_base_addr, uint32_t byte);
extern void uart_txi(uint32_t uart_base_addr);
extern void hexprint(uint32_t word);
extern void hexprintbyte(uint32_t word);
extern void pinmux(uint32_t pin, uint32_t val);
extern void nothing();
// extern void ConsolePrintf(const char *fmt, ...);
extern void ConsolePrintf(char *fmt, ...);

// i2c
#define CONF_I2C1_SDA              0x958
#define CONF_I2C1_SCL              0x95C
extern uint32_t i2c_init(uint32_t i2c_base_addr);
extern uint32_t i2c_read(uint32_t base_addr, uint32_t slave_addr, uint32_t buf_ptr, uint32_t len);
extern uint32_t i2c_write(uint32_t base_addr, uint32_t slave_addr, uint32_t data);
extern void poke(uint32_t base_addr, uint32_t offset, uint32_t value);

// pwm
#define GPIO_0_22                0x0820
#define GPIO_0_23                0x0824
#define GPIO_1_18                0x0848
#define GPIO_1_19                0x084C
#define GPIO_3_14                0x0990
#define GPIO_3_15                0x0994
extern uint32_t pwm_clk_init(uint32_t pwm_base_addr);
extern uint32_t pwm_init(uint32_t pwm_base_addr);
extern void pwm_write_A(uint32_t pwm_base_addr, uint32_t period);
extern void pwm_write_B(uint32_t pwm_base_addr, uint32_t period);
// PWM_PRESCALE of 224 = 14 * X  (X=1,2,4...128), see TBCTL Register, spruh73l 15.2.4.1
#define PWM_PRESCALE     224  // pwm clk divider (14*16) ~TBCLK
// TICKS_PER_MS * PWM_PERIOD_MS = period of pwm output in ticks = TBPRD (16 bits wide)
#define TICKS_PER_MS     446  // ticks per msec (pwm clock)
#define PWM_PERIOD_MS    20   // pwm output period in ms, which = 50Hz (RC-servo standard)
                              // NB need to recalibrate for mclk=1GHz - TODO
                              
// irq
extern void irq_init();
extern void irq_isr();
extern void und_isr();
extern uint32_t IntMasterModeGet();

// gpmc    - TODO
extern void gpmc_init();

// rtc
extern void rtc_init();
extern void rtc_irq();
extern uint32_t rtc_irq_count;
extern uint32_t year, month, day, hour, min, sec;

// timer
extern void tim_init();
extern void tim_delay(uint32_t msec);

// mmc
extern uint32_t mmc0_init();
extern void mmc0_irq_enab();
extern void mmc0_send_cmd(uint32_t cmd, uint32_t arg, uint32_t nblk);
extern void mmc0_set_dto(uint32_t ticks);
extern void mmc0_clear_status(uint32_t status_bits);
extern void mmc0_get_resp(unsigned int *response_addr);
// extern void mmc0_get_resp(uint32_t *response_addr);  // warning but still works - TODO

// eth
extern uint32_t eth_init();
extern uint32_t read_reg(uint32_t addr);
/* del these
extern uint32_t eth_rx_count;
extern uint32_t packet_len;
extern char *packet_ptr;
*/
extern void MACAddrGet(uint32_t mac_id, uint8_t *mac);

// mmu
extern uint32_t mmu_init();

// cp15
extern uint32_t CP15DCacheCleanBuff(uint32_t start_addr, uint32_t no_bytes);
extern uint32_t CP15DCacheFlushBuff(uint32_t start_addr, uint32_t no_bytes);

// cache
extern uint32_t cache_en();

// mclk
extern uint32_t mclk_1GHz();

// dma
extern void dma_clk_cfg();
