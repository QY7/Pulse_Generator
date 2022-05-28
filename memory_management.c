#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "memory_management.h"
#include "global_defs.h"

#define CLK_HIGH GpioDataRegs.GPADAT.bit.GPIO26 = 1;
#define CLK_LOW GpioDataRegs.GPADAT.bit.GPIO26 = 0;

#define MOSI_HIGH GpioDataRegs.GPADAT.bit.GPIO27 = 1;
#define MOSI_LOW GpioDataRegs.GPADAT.bit.GPIO27 = 0;

#define MISO_HIGH GpioDataRegs.GPADAT.bit.GPIO10 = 1;
#define MISO_LOW GpioDataRegs.GPADAT.bit.GPIO10 = 0;


#define CHIP_ENABLE GpioDataRegs.GPADAT.bit.GPIO16 = 0;
#define CHIP_DISABLE GpioDataRegs.GPADAT.bit.GPIO16 = 1;

unsigned char params_to_save[100] = {0};
unsigned char params_load[100] = {0};
unsigned int save_index = 0;
unsigned char params_cnt = 23;
unsigned int load_index = 0;
void add_to_list(unsigned int value){
    params_to_save[save_index*2] = value>>8&0xFF;
    params_to_save[save_index*2+1] = value&0xFF;
    save_index++;
}

void save_params(){
//   先擦除数据
    BSP_M25P16_ErasureSector(0);
    save_index = 0;
//    1-5
    add_to_list((unsigned int)D1);
    add_to_list((unsigned int)Tw);
    add_to_list((unsigned int)Tx);
    add_to_list((unsigned int)Tc);
    add_to_list((unsigned int)D2);
//    6-10
    add_to_list((unsigned int)M34);
    add_to_list((unsigned int)M34test);
    add_to_list((unsigned int)phase2);
    add_to_list((unsigned int)period);
    add_to_list((unsigned int)frequency[0]);
//    11-15
    add_to_list((unsigned int)frequency[1]);
    add_to_list((unsigned int)frequency[2]);
    add_to_list((unsigned int)width1_us[0]);
    add_to_list((unsigned int)width1_us[1]);
    add_to_list((unsigned int)width1_us[2]);
//    16-20
    add_to_list((unsigned int)width2_us[0]);
    add_to_list((unsigned int)width2_us[1]);
    add_to_list((unsigned int)width2_us[2]);
    add_to_list((unsigned int)phase_us[0]);
    add_to_list((unsigned int)phase_us[1]);
//    21-25
    add_to_list((unsigned int)phase_us[2]);
    add_to_list((unsigned int)timer_minute);
    add_to_list((unsigned int)timer_second);

//    unsigned int test_a = (unsigned int)Tw;

    while(BSP_M25P16_WriteData(0, 0, 0, params_cnt*2, params_to_save));
}
unsigned int load_single_param(){
    unsigned int data = params_load[load_index*2]<<8|params_load[load_index*2+1];
    load_index++;
    return data;
}
void load_params(){
//    载入存储数据
    while(BSP_M25P16_ReadData(0, 0, 0, params_cnt*2, params_load));

//    解析数据
    load_index = 0;
//    1-5
    D1 = load_single_param();
    Tw = load_single_param();
    Tx = load_single_param();
    Tc = load_single_param();
    D2 = load_single_param();
//    6-10
    M34 = load_single_param();
    M34test= load_single_param();
    phase2 = load_single_param();
    period = load_single_param();
    frequency[0] = load_single_param();
//    11-15
    frequency[1] = load_single_param();
    frequency[2] = load_single_param();
    width1_us[0] = load_single_param();
    width1_us[1] = load_single_param();
    width1_us[2] = load_single_param();
//    16-20
    width2_us[0] = load_single_param();
    width2_us[1] = load_single_param();
    width2_us[2] = load_single_param();
    phase_us[0] = load_single_param();
    phase_us[1] = load_single_param();
//    21-25
    phase_us[2] = load_single_param();
    timer_minute = load_single_param();
    timer_second = load_single_param();

}
void init_mem_gpio(){
    EALLOW;
//    DQ0 input of chip, so it should be set as output of dsp
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;

//    DQ1 output of chip, so it should be set as input of dsp
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;
//  CLK, output
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;
//    CSN, output
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;

    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;
    EDIS;

}

 /* SPI字节写 */
void spi_write_byte(unsigned char b)
{
    int i;
    for (i=7; i>=0; i--) {
        CLK_LOW;
        if(b&(1<<i)){
            MOSI_HIGH;
        }else{
            MOSI_LOW;
        }
        DELAY_US(50);
        CLK_HIGH;
        DELAY_US(50);
    }
}
/* SPI字节读 */
unsigned char spi_read_byte()
{
    int i;
    unsigned char r = 0;
    for (i=0; i<8; i++) {
        CLK_LOW;
        DELAY_US(50);       //延时
        CLK_HIGH;
        r = (r<<1)|(GpioDataRegs.GPADAT.bit.GPIO10);
//        r = (r <<1) | get_gpio_value(MISO);   //从高位7到低位0进行串行读出
        DELAY_US(50);
    }
    return r;
}
/*
 SPI写操作
 buf：写缓冲区
 len：写入字节的长度
*/
void spi_write (unsigned char* buf, int len)
{
    int i;
    CHIP_ENABLE;
    DELAY_US(50);        //延时
    //写入数据
    for (i=0; i<len; i++)
        spi_write_byte(buf[i]);
    DELAY_US(50);
//    CHIP_DISABLE;
}
/*
SPI读操作
buf：读缓冲区
len：读入字节的长度
*/
void spi_read(unsigned char* buf, int len)
{
    int i;
    CHIP_ENABLE;
    DELAY_US(50);        //延时
    //读入数据
    for (i=0; i<len; i++)
        buf[i] = spi_read_byte();
    DELAY_US(50);
//    CHIP_DISABLE;
}

/*
M25P16说明：
共2M*8bit空间
扇区：0-31
每个扇区内页地址：0-255
每页包含字节：0-255
*/

//接口函数-M25P16 10ms级延时
#define BSP_M25P16_Delay10ms_Port(ms)       DELAY_US(ms*10000)

//接口函数-M25P16 SPI读写一个字节
#define BSP_M25P16_Write_Port(data)     spi_write_byte(data)
#define BSP_M25P16_Read_Port()     spi_read_byte()

//接口函数-M25P16 片选使能
#define BSP_M25P16_NSS_EN       CHIP_ENABLE

//接口函数-M25P16 片选失能
#define BSP_M25P16_NSS_DN       CHIP_DISABLE

/*
 ************************************************************
 *  名称： BSP_M25P16_Init()
 *  功能： FLASH初始化
 *  输入：  无
 *  输出：  FLASH ID号
 ************************************************************
*/
void BSP_M25P16_Init(void)
{
    init_mem_gpio();
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ReadId()
 *  功能： M25P16 读ID号
 *  输入：  无
 *  输出：  无
 ************************************************************
*/
u32 BSP_M25P16_ReadId(void)
{
    u32 temp = 0;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(RDID_FLASH);

    temp |= ((u32)BSP_M25P16_Read_Port() << 16);
    temp |= ((u32)BSP_M25P16_Read_Port() << 8);
    temp |= BSP_M25P16_Read_Port();

    BSP_M25P16_NSS_DN;

    return temp;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ReadState()
 *  功能： M25P16 读状态寄存器
 *  输入：  无
 *  输出：  状态寄存器数值
 ************************************************************
*/
u8 BSP_M25P16_ReadState(void)
{
    u8 temp = 0;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(RDSR_FLASH);
    temp = BSP_M25P16_Read_Port();

    BSP_M25P16_NSS_DN;

    return temp;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_WriteEnable()
 *  功能： M25P16 写使能
 *  输入：  无
 *  输出：  1-成功  0-失败
 ************************************************************
*/
u8 BSP_M25P16_WriteEnable(void)
{
    BSP_M25P16_NSS_EN;
    BSP_M25P16_Write_Port(WREN_FLASH);
    BSP_M25P16_NSS_DN;

    u8 temp = BSP_M25P16_ReadState();

    if (temp & 0x02)
        return 1;
    else
        return 0;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_WriteState()
 *  功能： M25P16 写状态寄存器
 *  输入：  data-写入状态寄存器的值
 *  输出：  1-成功  0-失败
 ************************************************************
*/
u8 BSP_M25P16_WriteState(u8 data)
{
    if(0 == BSP_M25P16_WriteEnable()) return 0;

    BSP_M25P16_NSS_EN;
    BSP_M25P16_Write_Port(WRSR_FLASH);
    BSP_M25P16_Write_Port(data);
    BSP_M25P16_NSS_DN;

    return 1;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ReadBusy()
 *  功能： M25P16 读取忙状态
 *  输入：  无
 *  输出：  0-不忙  1-忙
 ************************************************************
*/
u8 BSP_M25P16_ReadBusy(void)
{
    u8 temp = BSP_M25P16_ReadState();
    if(temp & 0x01)   return 1;
    else return 0;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_WriteData()
 *  功能： M25P16 页写入程序 (需要先擦除，才能写入)
 *  输入：  sector_addr-扇区地址  page_addr-页地址  byte_addr-字节地址  num-写入个数  pointer-写入数据的指针
 *  输出：  0-写入成功  1-字节数为0  2-FLASH忙  3-FLASH使能失败
 ************************************************************
*/
u8 BSP_M25P16_WriteData(u8 sector_addr, u8 page_addr, u8 byte_addr, u16 num, u8 *pointer)
{
    u16 i;

    if (num == 0)   return 1;

    i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //延时10ms
        i--;
        if(i==0) return 2;
    }

    if (0 == BSP_M25P16_WriteEnable()) return 3;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(PP);
    BSP_M25P16_Write_Port(sector_addr);
    BSP_M25P16_Write_Port(page_addr);
    BSP_M25P16_Write_Port(byte_addr);
    for (i = 0; i < num; i++)
    {
    BSP_M25P16_Write_Port(*(pointer + i));
    }

    BSP_M25P16_NSS_DN;

    //  i=100;
    //  while(1)
    //  {
    //      if (0 == BSP_M25P16_ReadBusy()) break;
    //
    //      BSP_M25P16_Delay10ms_Port(1);   //延时10ms
    //      i--;
    //      if(i==0) return 4;
    //  }

    return 0;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ReadData()
 *  功能： M25P16 页读取
 *  输入：  sector_addr-扇区地址  page_addr-页地址  num-读取个数  p_back-读取缓冲区的首地址
 *  输出：  0-读取成功  1-字节数为0  2-FLASH忙
 ************************************************************
*/
u8 BSP_M25P16_ReadData(u8 sector_addr, u8 page_addr, u8 byte_addr ,u16 num, u8 *p_back)
{
    u16 i;

    if (num == 0)   return 1;

    i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //延时10ms
        i--;
        if(i==0) return 2;
    }

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(READ_FLASH);
    BSP_M25P16_Write_Port(sector_addr);
    BSP_M25P16_Write_Port(page_addr);
    BSP_M25P16_Write_Port(byte_addr);
    for (i = 0; i < num; i++)
    {
        *(p_back + i) = BSP_M25P16_Read_Port();
    }

    BSP_M25P16_NSS_DN;

    //  i=100;
    //  while(1)
    //  {
    //      if (0 == BSP_M25P16_ReadBusy()) break;
    //
    //      BSP_M25P16_Delay10ms_Port(1);   //延时10ms
    //      i--;
    //      if(i==0) return 3;
    //  }

    return 0;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ErasureSector()
 *  功能： M25P16 扇区擦除
 *  输入：  sector_addr-扇区地址
 *  输出：  0-擦除成功  1-FLASH忙  2-FLASH使能失败
 ************************************************************
*/
u8 BSP_M25P16_ErasureSector(u8 sector_addr)
{
    u8 i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //延时100ms
        i--;
        if(i==0) return 1;
    }

    if (0 == BSP_M25P16_WriteEnable()) return 2;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(SE);
    BSP_M25P16_Write_Port(sector_addr);
    BSP_M25P16_Read_Port();
    BSP_M25P16_Read_Port();

    BSP_M25P16_NSS_DN;

    i=40;       //最高擦除时间3S
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(10);  //延时100ms
        i--;
        if(i==0) return 3;
    }

    return 0;
}

/*
 ************************************************************
 *  名称： BSP_M25P16_ErasureAll()
 *  功能： M25P16 擦除所有数据
 *  输入：  sector_addr-扇区地址  page_addr-页地址  byte_addr-字节地址
 *  输出：  0-格式化成功  1-FLASH忙  2-FLASH使能失败
 ************************************************************
*/
u8 BSP_M25P16_ErasureAll(void)
{
    u8 i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //延时10ms
        i--;
        if(i==0) return 1;
    }

    if (0 == BSP_M25P16_WriteEnable()) return 2;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(BE);

    BSP_M25P16_NSS_DN;
    return 0;
}
