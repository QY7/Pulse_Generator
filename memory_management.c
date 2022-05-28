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
//   �Ȳ�������
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
//    ����洢����
    while(BSP_M25P16_ReadData(0, 0, 0, params_cnt*2, params_load));

//    ��������
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

 /* SPI�ֽ�д */
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
/* SPI�ֽڶ� */
unsigned char spi_read_byte()
{
    int i;
    unsigned char r = 0;
    for (i=0; i<8; i++) {
        CLK_LOW;
        DELAY_US(50);       //��ʱ
        CLK_HIGH;
        r = (r<<1)|(GpioDataRegs.GPADAT.bit.GPIO10);
//        r = (r <<1) | get_gpio_value(MISO);   //�Ӹ�λ7����λ0���д��ж���
        DELAY_US(50);
    }
    return r;
}
/*
 SPIд����
 buf��д������
 len��д���ֽڵĳ���
*/
void spi_write (unsigned char* buf, int len)
{
    int i;
    CHIP_ENABLE;
    DELAY_US(50);        //��ʱ
    //д������
    for (i=0; i<len; i++)
        spi_write_byte(buf[i]);
    DELAY_US(50);
//    CHIP_DISABLE;
}
/*
SPI������
buf����������
len�������ֽڵĳ���
*/
void spi_read(unsigned char* buf, int len)
{
    int i;
    CHIP_ENABLE;
    DELAY_US(50);        //��ʱ
    //��������
    for (i=0; i<len; i++)
        buf[i] = spi_read_byte();
    DELAY_US(50);
//    CHIP_DISABLE;
}

/*
M25P16˵����
��2M*8bit�ռ�
������0-31
ÿ��������ҳ��ַ��0-255
ÿҳ�����ֽڣ�0-255
*/

//�ӿں���-M25P16 10ms����ʱ
#define BSP_M25P16_Delay10ms_Port(ms)       DELAY_US(ms*10000)

//�ӿں���-M25P16 SPI��дһ���ֽ�
#define BSP_M25P16_Write_Port(data)     spi_write_byte(data)
#define BSP_M25P16_Read_Port()     spi_read_byte()

//�ӿں���-M25P16 Ƭѡʹ��
#define BSP_M25P16_NSS_EN       CHIP_ENABLE

//�ӿں���-M25P16 Ƭѡʧ��
#define BSP_M25P16_NSS_DN       CHIP_DISABLE

/*
 ************************************************************
 *  ���ƣ� BSP_M25P16_Init()
 *  ���ܣ� FLASH��ʼ��
 *  ���룺  ��
 *  �����  FLASH ID��
 ************************************************************
*/
void BSP_M25P16_Init(void)
{
    init_mem_gpio();
}

/*
 ************************************************************
 *  ���ƣ� BSP_M25P16_ReadId()
 *  ���ܣ� M25P16 ��ID��
 *  ���룺  ��
 *  �����  ��
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
 *  ���ƣ� BSP_M25P16_ReadState()
 *  ���ܣ� M25P16 ��״̬�Ĵ���
 *  ���룺  ��
 *  �����  ״̬�Ĵ�����ֵ
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
 *  ���ƣ� BSP_M25P16_WriteEnable()
 *  ���ܣ� M25P16 дʹ��
 *  ���룺  ��
 *  �����  1-�ɹ�  0-ʧ��
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
 *  ���ƣ� BSP_M25P16_WriteState()
 *  ���ܣ� M25P16 д״̬�Ĵ���
 *  ���룺  data-д��״̬�Ĵ�����ֵ
 *  �����  1-�ɹ�  0-ʧ��
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
 *  ���ƣ� BSP_M25P16_ReadBusy()
 *  ���ܣ� M25P16 ��ȡæ״̬
 *  ���룺  ��
 *  �����  0-��æ  1-æ
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
 *  ���ƣ� BSP_M25P16_WriteData()
 *  ���ܣ� M25P16 ҳд����� (��Ҫ�Ȳ���������д��)
 *  ���룺  sector_addr-������ַ  page_addr-ҳ��ַ  byte_addr-�ֽڵ�ַ  num-д�����  pointer-д�����ݵ�ָ��
 *  �����  0-д��ɹ�  1-�ֽ���Ϊ0  2-FLASHæ  3-FLASHʹ��ʧ��
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

        BSP_M25P16_Delay10ms_Port(1);   //��ʱ10ms
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
    //      BSP_M25P16_Delay10ms_Port(1);   //��ʱ10ms
    //      i--;
    //      if(i==0) return 4;
    //  }

    return 0;
}

/*
 ************************************************************
 *  ���ƣ� BSP_M25P16_ReadData()
 *  ���ܣ� M25P16 ҳ��ȡ
 *  ���룺  sector_addr-������ַ  page_addr-ҳ��ַ  num-��ȡ����  p_back-��ȡ���������׵�ַ
 *  �����  0-��ȡ�ɹ�  1-�ֽ���Ϊ0  2-FLASHæ
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

        BSP_M25P16_Delay10ms_Port(1);   //��ʱ10ms
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
    //      BSP_M25P16_Delay10ms_Port(1);   //��ʱ10ms
    //      i--;
    //      if(i==0) return 3;
    //  }

    return 0;
}

/*
 ************************************************************
 *  ���ƣ� BSP_M25P16_ErasureSector()
 *  ���ܣ� M25P16 ��������
 *  ���룺  sector_addr-������ַ
 *  �����  0-�����ɹ�  1-FLASHæ  2-FLASHʹ��ʧ��
 ************************************************************
*/
u8 BSP_M25P16_ErasureSector(u8 sector_addr)
{
    u8 i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //��ʱ100ms
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

    i=40;       //��߲���ʱ��3S
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(10);  //��ʱ100ms
        i--;
        if(i==0) return 3;
    }

    return 0;
}

/*
 ************************************************************
 *  ���ƣ� BSP_M25P16_ErasureAll()
 *  ���ܣ� M25P16 ������������
 *  ���룺  sector_addr-������ַ  page_addr-ҳ��ַ  byte_addr-�ֽڵ�ַ
 *  �����  0-��ʽ���ɹ�  1-FLASHæ  2-FLASHʹ��ʧ��
 ************************************************************
*/
u8 BSP_M25P16_ErasureAll(void)
{
    u8 i=100;
    while(1)
    {
        if (0 == BSP_M25P16_ReadBusy()) break;

        BSP_M25P16_Delay10ms_Port(1);   //��ʱ10ms
        i--;
        if(i==0) return 1;
    }

    if (0 == BSP_M25P16_WriteEnable()) return 2;

    BSP_M25P16_NSS_EN;

    BSP_M25P16_Write_Port(BE);

    BSP_M25P16_NSS_DN;
    return 0;
}
