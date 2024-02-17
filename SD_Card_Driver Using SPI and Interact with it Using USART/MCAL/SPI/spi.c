/*
 * spi.c
 *
 *  Created on: 5 Dec 2023
 *      Author: Abo Khalil
 */

#include"avr/io.h"
#include "spi.h"

void (*volatile STC_p2f)(void) = 0;

void SPI_SlaveInit(void) {
	/* Set MISO output, all others input */
	SET_BIT(SPI_DDR ,MISO_PIN);
	/* Enable SPI */
#if TRANSMITTED_BIT_FIRST == LSB

	SET_BIT(SPCR,DORD);
#elif TRANSMITTED_BIT_FIRST == MSB
	CLEAR_BIT(SPCR,DORD);
#else
#error "Wrong Configuration"
#endif
#if CLK_POL == IDLE_HIGH
	SET_BIT(SPCR,CPOL);
#elif CLK_POL == IDLE_LOW // SCK will be idle low
	CLEAR_BIT(SPCR,CPOL);
#else
#error "Wrong Configuration"
#endif
#if CLK_PHASE == SAMPLE_LEADING  // sample in the leading edge
	CLEAR_BIT(SPCR,CPHA);
#elif CLK_PHASE == SAMPLE_TRAILING
	SET_BIT(SPCR,CPHA);
#else
#error "Wrong Configuration"
#endif
	SPCR |= (SPI_CLK << 0); // adjust SCK value
#if DOUBLE_SPEED_SCK == ENABLED
SET_BIT(SPSR ,SPI2X) ;
#elif DOUBLE_SPEED_SCK == DISABLED
	CLEAR_BIT(SPSR ,SPI2X);
#else
#error "Wrong Configuration"
#endif
	SET_BIT(SPCR,SPE);

}

char SPI_SlaveReceive(void) {
	/* Wait for reception complete */
	while (!(SPSR & (1 << SPIF)))
		;
	/* Return data register */
	return SPDR;
}

void SPI_MasterInit(void) {
	/* Set MOSI and SCK output, all others input */
	SPI_DDR |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << CS_PIN);
	SPI_PORT |= (1 << CS_PIN);
	/* Enable SPI, Master, set clock rate fck/16 */
#if TRANSMITTED_BIT_FIRST == LSB
	SET_BIT(SPCR,DORD);
#elif TRANSMITTED_BIT_FIRST == MSB
	CLEAR_BIT(SPCR,DORD);
#else
#error "Wrong Configuration"
#endif
#if CLK_POL == IDLE_HIGH
	SET_BIT(SPCR,CPOL);
#elif CLK_POL == IDLE_LOW // SCK will be idle low
	CLEAR_BIT(SPCR,CPOL);
#else
#error "Wrong Configuration"
#endif
#if CLK_PHASE == SAMPLE_LEADING  // sample in the leading edge
	CLEAR_BIT(SPCR,CPHA);
#elif CLK_PHASE == SAMPLE_TRAILING
	SET_BIT(SPCR,CPHA);
#else
#error "Wrong Configuration"
#endif
	SPCR |= (SPI_CLK << 0); // adjust SCK value
#if DOUBLE_SPEED_SCK == ENABLED
SET_BIT(SPSR ,SPI2X) ;
#elif DOUBLE_SPEED_SCK == DISABLED
	CLEAR_BIT(SPSR ,SPI2X);
#else
#error "Wrong Configuration"
#endif
	SPCR |= (1 << SPE) | (1 << MSTR);
}

uint8 SPI_MasterTransmit(uint8 cData) {
	/* Start transmission */
	SPDR = (uint8) cData;
	/* Wait for transmission complete */
	while (!(SPSR & (1 << SPIF)))
		;
	return SPDR;
}

void SPI_Interrupt_EN(void) {
	SET_BIT(SPCR,SPIE);
}
void SPI_Interrupt_DI(void) {
	CLEAR_BIT(SPCR,SPIE);
}
ISR(SPI_STC_vect) {
	if (STC_p2f) {
		STC_p2f();
	}
}
void SPI_SetCallBackFn(void (*p2f)(void)) {
	if (p2f) {
		STC_p2f = p2f;
	}
}
