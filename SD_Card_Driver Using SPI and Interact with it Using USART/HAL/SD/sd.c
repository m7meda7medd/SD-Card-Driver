/*
 * sd.c
 *
 *  Created on: 12 Dec 2023
 *      Author: Abo Khalil
 */

#include "sd.h"

/* Static Functions Declarations*/
static void SD_PowerUpSeq();
static void SD_SendCmd(uint8 cmd, uint32 arg, uint8 crc);
static void SD_ReadRes7(uint8 *res);
static uint8 SD_GoIdleState();
static uint8 SD_ReadRes1();
static void SD_SendIfCond(uint8 *res);
static void SD_PrintR1(uint8 res);
static void SD_PrintR3(uint8 *res);
static void SD_ReadOCR(uint8 *res);
static void SD_ReadRes3(uint8 *res3);
static void SD_PrintR7(uint8 *res);
static uint8 SD_SendOpCond();
static uint8 SD_SendApp();
static uint8 SD_GetStatus();
/*****************************************/

uint8 SD_Init() {
	uint8 res[5], cmdAttempts = 0;

	SD_PowerUpSeq(); // Power Up Sequence

	// command card to idle
	while ((res[0] = SD_GoIdleState()) != 0x01) {
		cmdAttempts++;
		if (cmdAttempts > 10)
			return SD_ERROR;
	}

	// send interface conditions
	SD_SendIfCond(res);
	if (res[0] != 0x01) {
		return SD_ERROR;
	}

	// check echo pattern
	if (res[4] != 0xAA) {
		return SD_ERROR;
	}

	// attempt to initialize card
	cmdAttempts = 0;
	do {
		if (cmdAttempts > 100)
			return SD_ERROR;

		// send app cmd
		res[0] = SD_SendApp();

		// if no error in response
		if (res[0] < 2) {
			res[0] = SD_SendOpCond();
		}

		// wait
		_delay_ms(10);

		cmdAttempts++;
	} while (res[0] != SD_SUCCESS);

	// read OCR
	SD_ReadOCR(res);

	// check card is ready
	if (!(res[1] & 0x80))
		return SD_ERROR;

	return SD_SUCCESS;
}

uint8_t SD_ReadSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token) {
	uint8_t res1, read;
	uint16_t readAttempts;

	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

	// send CMD17
	SD_SendCmd(CMD17, addr, CMD17_CRC);

	// read R1
	res1 = SD_ReadRes1();

	// if response received from card
	if (res1 != 0xFF) {
		// wait for a response token (timeout = 100ms)
		readAttempts = 0;
		while (++readAttempts != SD_MAX_READ_ATTEMPTS)
			if ((read = SPI_MasterTransmit(0xFF)) != 0xFF)
				break;
	}
	// if response token is 0xFE
	if (read == 0xFE) {
		// read 512 byte block
		for (uint16_t i = 0; i < 512; i++)
			*buf++ = SPI_MasterTransmit(0xFF);

		// read 16-bit CRC
		SPI_MasterTransmit(0xFF);
		SPI_MasterTransmit(0xFF);
	}

	// set token to card response
	*token = read;

	// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);

	return res1;
}

uint8 SD_WriteSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token) {
	uint8_t writeattempts, read;
	uint8 res[5];
	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

	// send CMD24
	SD_SendCmd(CMD24, addr, CMD24_CRC);

	// read response
	res[0] = SD_ReadRes1();

	// if no error
	if (res[0] == 0x00) {
		// send start token
		*token = SPI_MasterTransmit(0xff);

		// write buffer to card
		for (uint16_t i = 0; i < SD_BLOCK_LEN; i++)
			SPI_MasterTransmit(buf[i]);

		// wait for a response (timeout = 250ms)
		read = 0;
		while (++writeattempts != SD_MAX_WRITE_ATTEMPTS)
			if ((read = SPI_MasterTransmit(0xFF)) != 0xFF) {
				*token = 0xFF;
				break;
			}

		// if data accepted
		if ((read & 0x1F) == 0x05) {
			// set token to data accepted
			*token = 0x05;

			// wait for write to finish (timeout = 250ms)
			writeattempts = 0;
			while (SPI_MasterTransmit(0xFF) == 0x00)
				if (++writeattempts == SD_MAX_WRITE_ATTEMPTS) {
					*token = 0x00;
					break;
				}
		}
	}
	// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);

	return res[0];
}
void SD_PrintDataErrToken(uint8_t token) {
	if (SD_TOKEN_OOR(token))
		UART_SendString("Data out of range\n");
	if (SD_TOKEN_CECC(token))
		UART_SendString("Card ECC failed\n");
	if (SD_TOKEN_CC(token))
		UART_SendString("CC Error\n");
	if (SD_TOKEN_ERROR(token))
		UART_SendString("Error\n");
}

static void SD_PowerUpSeq() {
	// deselected
	CS_DISABLE();
	// give SD card time to power up
	_delay_ms(1);

	// send 80 clock cycles to synchronize
	for (uint8 i = 0; i < 10; i++)
		SPI_MasterTransmit(0xFF);

	// deselect SD card
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);
}

static void SD_SendCmd(uint8 cmd, uint32 arg, uint8 crc) {
	// transmit command to sd card
	SPI_MasterTransmit(cmd | 0x40); //0b 0 1 00 cmdindex

	// transmit argument
	SPI_MasterTransmit((uint8) (arg >> 24));
	SPI_MasterTransmit((uint8) (arg >> 16));
	SPI_MasterTransmit((uint8) (arg >> 8));
	SPI_MasterTransmit((uint8) (arg));

	// transmit crc
	SPI_MasterTransmit(crc | 0x01);
}
static uint8 SD_ReadRes1() {
	uint8 i = 0, res1;

	// keep polling until actual data received
	while ((res1 = SPI_MasterTransmit(0xFF)) == 0xFF) {
		i++;

		// if no data received for 8 bytes, break
		if (i > 8)
			break;
	}

	return res1;
}
static uint8 SD_GoIdleState() {
	// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

	// send CMD0
	SD_SendCmd(CMD0, CMD0_ARG, CMD0_CRC);
	_delay_ms(50);
	// read response
	uint8 res1 = SD_ReadRes1();

	// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);

	return res1;
}
static void SD_ReadRes7(uint8 *res) {
	// read response 1 in R7
	res[0] = SD_ReadRes1();

	// if error reading R1, return
	if (res[0] > 1)
		return;

	// read remaining bytes
	res[1] = SPI_MasterTransmit(0xFF);
	res[2] = SPI_MasterTransmit(0xFF);
	res[3] = SPI_MasterTransmit(0xFF);
	res[4] = SPI_MasterTransmit(0xFF);
}
static void SD_SendIfCond(uint8 *res) {
	// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

	// send CMD8
	SD_SendCmd(CMD8, CMD8_ARG, CMD8_CRC);

	// read response
	SD_ReadRes7(res);

	// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);
}

static void SD_PrintR1(uint8_t res) {
	if (res & 0b10000000) {
		UART_SendString("Error: MSB = 1\r\n");
		return;
	}
	if (res == 0) {
		UART_SendString("Card Ready\r\n");
		return;
	}
	if (PARAM_ERROR(res))
		UART_SendString("Parameter Error\r\n");
	if (ADDR_ERROR(res))
		UART_SendString("Address Error\r\n");
	if (ERASE_SEQ_ERROR(res))
		UART_SendString("\tErase Sequence Error\r\n");
	if (CRC_ERROR(res))
		UART_SendString("CRC Error\r\n");
	if (ILLEGAL_CMD(res))
		UART_SendString("Illegal Command\r\n");
	if (ERASE_RESET(res))
		UART_SendString("\tErase Reset Error\r\n");
	if (IN_IDLE(res))
		UART_SendString("In Idle State\r\n");
}
static void SD_PrintR7(uint8_t *res) {
	SD_PrintR1(res[0]);

	if (res[0] > 1)
		return;

	UART_SendString("Command Version: ");
	UART_SendString(CMD_VER(res[1]));
	UART_SendString("\r\n");

	UART_SendString("Voltage Accepted: ");
	if (VOL_ACC(res[3]) == VOLTAGE_ACC_27_33)
		UART_SendString("2.7-3.6V\r\n");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_LOW)
		UART_SendString("LOW VOLTAGE\r\n");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_RES1)
		UART_SendString("RESERVED\r\n");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_RES2)
		UART_SendString("RESERVED\r\n");
	else
		UART_SendString("NOT DEFINED\r\n");

	UART_SendString("Echo: ");
	UART_SendHex(res[4]);
	UART_SendString("\r\n");
}
static void SD_ReadRes3(uint8_t *res3) {
	// read R1
	res3[0] = SD_ReadRes1();

	// if error reading R1, return
	if (res3[0] > 1)
		return;

	// read remaining bytes
	res3[1] = SPI_MasterTransmit(0xFF);
	res3[2] = SPI_MasterTransmit(0xFF);
	res3[3] = SPI_MasterTransmit(0xFF);
	res3[4] = SPI_MasterTransmit(0xFF);
}
static void SD_ReadOCR(uint8_t *res) {
	// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);
	_delay_ms(100);
	// send CMD58
	SD_SendCmd(CMD58, CMD58_ARG, CMD58_CRC);
	_delay_ms(100);
	// read response
	SD_ReadRes3(res);

	// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);
}
static void SD_PrintR3(uint8_t *res) {
	SD_PrintR1(res[0]);

	if (res[0] > 1)
		return;

	UART_SendString("Card Power Up Status: ");
	if (POWER_UP_STATUS(res[1])) {
		UART_SendString("READY\r\n");
		UART_SendString("CCS Status: ");
		if (CCS_VAL(res[1])) {
			UART_SendString("1\r\n");
		} else
			UART_SendString("0\r\n");
	} else {
		UART_SendString("BUSY\r\n");
	}

	UART_SendString("VDD Window: ");
	if (VDD_2728(res[3]))
		UART_SendString("2.7-2.8, ");
	if (VDD_2829(res[2]))
		UART_SendString("2.8-2.9, ");
	if (VDD_2930(res[2]))
		UART_SendString("2.9-3.0, ");
	if (VDD_3031(res[2]))
		UART_SendString("3.0-3.1, ");
	if (VDD_3132(res[2]))
		UART_SendString("3.1-3.2, ");
	if (VDD_3233(res[2]))
		UART_SendString("3.2-3.3, ");
	if (VDD_3334(res[2]))
		UART_SendString("3.3-3.4, ");
	if (VDD_3435(res[2]))
		UART_SendString("3.4-3.5, ");
	if (VDD_3536(res[2]))
		UART_SendString("3.5-3.6");
	UART_SendString("\r\n");
}
static uint8 SD_SendOpCond() {
	uint8 res1;
// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

// send CMD0
	SD_SendCmd(ACMD41, ACMD41_ARG, ACMD41_CRC);

// read response
	res1 = SD_ReadRes1();

// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);

	return res1;
}
static uint8 SD_SendApp() { //send Application CMD
	uint8 res1;
// assert chip select
	SPI_MasterTransmit(0xFF);
	CS_ENABLE();
	SPI_MasterTransmit(0xFF);

// send CMD0
	SD_SendCmd(CMD55, CMD55_ARG, CMD55_CRC);

// read response
	res1 = SD_ReadRes1();

// deassert chip select
	SPI_MasterTransmit(0xFF);
	CS_DISABLE();
	SPI_MasterTransmit(0xFF);

	return res1;
}

static uint8 SD_GetStatus() {
	SD_SendCmd(13, 0x00, 0x00);
	return SPI_MasterTransmit(0xFF);
}

