/*
 * spi_cfg.h
 *
 *  Created on:  5 Dec 2023
 *      Author: Abo Khalil
 */

#ifndef MCAL_SPI_SPI_CFG_H_
#define MCAL_SPI_SPI_CFG_H_
#include "../../Bit_math.h"
#define DISABLED 0
#define ENABLED 1
#define LSB 1
#define MSB 0
//**************************************************
#define TRANSMITTED_BIT_FIRST   MSB
//**************************************************
#define CLK_DIV4  0b00
#define CLK_DIV16  0b01
#define CLK_DIV64 0b10
#define CLK_DIV128 0b11
//**************************************************
#define SPI_CLK	CLK_DIV128     /* Configure Here*/
#define DOUBLE_SPEED_SCK DISABLED  /*Configure Here*/	// if this enabled the SCK will be (SPI_CLK/2)
//*************************************************
#define IDLE_HIGH 0
#define IDLE_LOW 1
//*************************************************
#define CLK_POL IDLE_LOW  /*Configure Here*/
//*************************************************
#define SAMPLE_LEADING 0
#define SAMPLE_TRAILING 1
//**********************************************
#define CLK_PHASE  SAMPLE_LEADING  /*Configure Here*/
//**********************************************

#define SPI_PORT_B      'B'
#define SPI_PORT_A      'A'
#define SPI_PORT_C      'C'
#define SPI_PORT_D      'D'

//*********************************************
#define MISO_PIN PB6
#define MOSI_PIN PB5
#define SCK_PIN PB7
#define CS_PIN  PB4
#define SPI_ON_PORT SPI_PORT_B

#endif /* MCAL_SPI_SPI_CFG_H_ */
