//
//  DMA Events  -  prototypes
//  

#ifndef _DMAEVENT_H
#define _DMAEVENT_H

#include "hw_types.h"

/********************* Direct Mapped Events ********************************/
/* Events for McASP 1*/
#define DMA_CHA_MCASP1_TX               (10u)
#define DMA_CHA_MCASP1_RX               (11u)

/* MCSPI0 Channel 0 transmit event */
#define DMA_CHA_MCSPI0_CH0_TX           (16u)
/* MCSPI0 Channel 0 receive event */
#define DMA_CHA_MCSPI0_CH0_RX           (17u)
/* MCSPI0 Channel 1 transmit event */
#define DMA_CHA_MCSPI0_CH1_TX           (18u)
/* MCSPI0 Channel 1 receive event */
#define DMA_CHA_MCSPI0_CH1_RX           (19u)

/* MMC0 transmit event. */
#define DMA_CHA_MMC0_TX               (24u)
/* MMC0 receive event. */
#define DMA_CHA_MMC0_RX               (25u)

/* UART0 Transmit Event. */
#define DMA_CHA_UART0_TX                (26u)
/* UART0 Receive Event. */
#define DMA_CHA_UART0_RX                (27u)

/* UART1 Transmit Event. */
#define DMA_CHA_UART1_TX                (28u)
/* UART1 Receive Event. */
#define DMA_CHA_UART1_RX                (29u)

/* UART2 Transmit Event. */
#define DMA_CHA_UART2_TX                (30u)
/* UART2 Receive Event. */
#define DMA_CHA_UART2_RX                (31u)

/* MCSPI1 Channel 0 transmit event */
#define DMA_CHA_MCSPI1_CH0_TX           (42u)
/* MCSPI1 Channel 0 receive event */
#define DMA_CHA_MCSPI1_CH0_RX           (43u)
/* MCSPI1 Channel 1 transmit event */
#define DMA_CHA_MCSPI1_CH1_TX           (44u)
/* MCSPI1 Channel 1 receive event */
#define DMA_CHA_MCSPI1_CH1_RX           (45u)
/* I2C0 Transmit Event */
#define DMA_CHA_I2C0_TX                 (58u)
/* I2C0 Receive Event */
#define DMA_CHA_I2C0_RX                 (59u)
/* I2C1 Receive Event */
#define DMA_CHA_I2C1_TX                 (60u)
/* I2C1 Transmit Event */
#define DMA_CHA_I2C1_RX                 (61u)



/********************** Crossbar Mapped Events ********************************/
/* I2C2 Receive Event */
#define DMA_CHA_HSI2C2_RX               (4u)
/* I2C2 Transmit Event */
#define DMA_CHA_HSI2C2_TX               (3u)

/* UART3 Transmit Event. */
#define DMA_CHA_UART3_TX                (7u)
/* UART3 Receive Event. */
#define DMA_CHA_UART3_RX                (8u)

/* UART4 Transmit Event. */
#define DMA_CHA_UART4_TX                (9u)
/* UART4 Receive Event. */
#define DMA_CHA_UART4_RX                (10u)

/* UART5 Transmit Event. */
#define DMA_CHA_UART5_TX                (11u)
/* UART5 Receive Event. */
#define DMA_CHA_UART5_RX                (12u)

#endif

