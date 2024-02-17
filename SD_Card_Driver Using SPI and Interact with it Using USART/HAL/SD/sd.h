#ifndef SD_H
#define SD_H
#include<util/delay.h>
#include "../../MCAL/SPI/spi.h"
#include "../../MCAL/USART/USART.h"
#include "sd_types.h"


#define SD_MAX_READ_ATTEMPTS 1563
#define SD_MAX_WRITE_ATTEMPTS   3907
#define SD_BLOCK_LEN 512

/***************************************************************************/
uint8 SD_Init() ;
uint8_t SD_ReadSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token);
void SD_PrintDataErrToken(uint8_t token);
uint8 SD_WriteSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token) ;
#endif
