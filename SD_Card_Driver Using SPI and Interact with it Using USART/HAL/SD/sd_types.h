/*
 * sd_types.c
 *
 *  Created on: 12 Dec 2023
 *      Author: Abo Khalil
 */
#ifndef SD_TYPES_H
#define SD_TYPES_H




#define SD_ERROR 1
#define SD_SUCCESS 0
#define START_TOKEN 0xFE // start recieving data token
//R1  Bits
#define PARAM_ERROR(X)      X & 0b01000000
#define ADDR_ERROR(X)       X & 0b00100000
#define ERASE_SEQ_ERROR(X)  X & 0b00010000
#define CRC_ERROR(X)        X & 0b00001000
#define ILLEGAL_CMD(X)      X & 0b00000100
#define ERASE_RESET(X)      X & 0b00000010
#define IN_IDLE(X)          X & 0b00000001
// Response-7 Bits
#define CMD_VER(X)          ((X >> 4) & 0xF0)
#define VOL_ACC(X)          (X & 0x1F)

#define VOLTAGE_ACC_27_33   0b00000001
#define VOLTAGE_ACC_LOW     0b00000010
#define VOLTAGE_ACC_RES1    0b00000100
#define VOLTAGE_ACC_RES2    0b00001000
// Response-3 bits
#define POWER_UP_STATUS(X)  X & 0x40
#define CCS_VAL(X)          X & 0x40
#define VDD_2728(X)         X & 0b10000000
#define VDD_2829(X)         X & 0b00000001
#define VDD_2930(X)         X & 0b00000010
#define VDD_3031(X)         X & 0b00000100
#define VDD_3132(X)         X & 0b00001000
#define VDD_3233(X)         X & 0b00010000
#define VDD_3334(X)         X & 0b00100000
#define VDD_3435(X)         X & 0b01000000
#define VDD_3536(X)         X & 0b10000000
//CMD 0 Parameters
#define CMD0        0
#define CMD0_ARG    0x00000000
#define CMD0_CRC    0x94
//CMD8 Parameters
#define CMD8  8
#define CMD8_ARG 0x000001AA
#define CMD8_CRC  0x86 //(1000011b<<1)
// CMD 58 parameters
#define CMD58   58
#define CMD58_ARG  0x00000000
#define CMD58_CRC  0x0
//CMD55 parameters
#define CMD55 55
#define CMD55_ARG 0x00000000
#define CMD55_CRC 0x00
// ACMD41
#define ACMD41  41
#define ACMD41_ARG 0x40000000
#define ACMD41_CRC 0x00
//CMD 17

#define CMD17   17
#define CMD17_CRC 0x00

#define SD_TOKEN_OOR(X)     X & 0b00001000
#define SD_TOKEN_CECC(X)    X & 0b00000100
#define SD_TOKEN_CC(X)      X & 0b00000010
#define SD_TOKEN_ERROR(X)   X & 0b00000001
#define CMD24                   24
#define CMD24_CRC               0x00

#endif
