/*
 * spi.h
 *
 *  Created on: 5 Dec 2023
 *      Author: Abo Khalil
 */

#ifndef MCAL_SPI_SPI_H_
#define MCAL_SPI_SPI_H_


#include <avr/io.h>
#include "spi_cfg.h"
#include "../../std_types.h"
/*
#define CONC_PORT(REG,PORT)  REG##PORT
#define SPI_PORT_REG(REG,PORT)  CONC(REG,PORT)
*/
// MCU Specific
#if SPI_ON_PORT == SPI_PORT_A
#define SPI_DDR  DDRA
#define	SPI_PORT PORTA
#elif SPI_ON_PORT == SPI_PORT_B
#define SPI_DDR  DDRB
#define	SPI_PORT PORTB
#elif SPI_ON_PORT == SPI_PORT_C
#define SPI_DDR  DDRC
#define	SPI_PORT PORTC
#elif SPI_ON_PORT == SPI_PORT_D
#define SPI_DDR  DDRA
#define	SPI_PORT PORTA
#else
#error "Configuration Error "
#endif


#define  CS_ENABLE()  SPI_PORT &= ~(1<<CS_PIN)
#define  CS_DISABLE()  SPI_PORT |= (1<<CS_PIN)
#define SPI_HIGH_SPEED() \
	CS_DISABLE() ; \
	SPCR = 0 ;	\
	SPCR = (1 << SPE) | (1 << MSTR) ; \
	SET_BIT(SPSR ,SPI2X) ; \

#ifndef ISR
#define ISR(vector)            \
    void vector (void) __attribute__ ((signal)); \
    void vector (void)
#endif
void SPI_SlaveInit(void);
char SPI_SlaveReceive(void);
void SPI_MasterInit(void);
uint8 SPI_MasterTransmit(uint8 cData);
void SPI_Interrupt_EN(void) ;
void SPI_Interrupt_DI(void) ;
void SPI_SetCallBackFn(void(*p2f)(void)) ;

#endif /* MCAL_SPI_SPI_H_ */
