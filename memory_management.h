/*
 * memory_management.h
 *
 *  Created on: 2022Äê5ÔÂ22ÈÕ
 *      Author: Felix
 */

#ifndef MEMORY_MANAGEMENT_H_
#define MEMORY_MANAGEMENT_H_


#define M25P16_MAXBYTE  2*1024*1024

#define     WREN_FLASH                  0x06
#define     WRDI_FLASH                  0x04
#define     RDID_FLASH                  0x9F
#define     RDSR_FLASH                  0x05
#define     WRSR_FLASH                  0X01
#define     READ_FLASH                  0X03
#define     FAST_READ_FLASH         0x0B
#define     PP                          0x02
#define     SE                                  0xD8
#define     BE                                  0xC7
#define     DP                                  0xB9
#define     RES                                 0xAB


typedef unsigned long u32;
typedef unsigned char u8;
typedef unsigned int u16;


void BSP_M25P16_Init_Port(void);
void BSP_M25P16_Default_Port(void);

void BSP_M25P16_Init(void);
void BSP_M25P16_Enable(void);
void BSP_M25P16_Default(void);

u32 BSP_M25P16_ReadId(void);
u8 BSP_M25P16_WriteData(u8 sector_addr, u8 page_addr, u8 byte_addr, u16 num, u8 *pointer);
u8 BSP_M25P16_ReadData(u8 sector_addr, u8 page_addr, u8 byte_addr ,u16 num, u8 *p_back);
u8 BSP_M25P16_ErasureSector(u8 sector_addr);
u8 BSP_M25P16_ErasureAll(void);
void save_params();
void load_params();
//enum PARAMS_INDEX = {
////                     1-5
//    D1_OFF;
//    TW_OFF;
//    Tx_OFF;
//    TC_OFF;
//    D2_OFF;
////    6-10
//    M34_OFF;
//    M34test;
//    PHASE2_OFF;
//    PERIOD_OFF;
//    FREQUENCY1_OFF;
////    11-15
//    FREQUENCY2_OFF;
//    FREQUENCY3_OFF;
//    WIDTH11_OFF;
//    WIDTH12_OFF;
//    WIDTH13_OFF;
////    16-20
//    WIDTH21_OFF;
//    WIDTH22_OFF;
//    WIDTH23_OFF;
//    PHASE11_OFF;
//    PHASE12_OFF;
////    21-25
//    PHASE13_OFF;
//    MINUTE_OFF;
//    SECOND_OFF
//};
#endif /* MEMORY_MANAGEMENT_H_ */
