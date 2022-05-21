/*
 * multiple_pulse.c
 *
 *  Created on: May 20, 2021
 *      Author: Felix
 */
#include "global_funcs.h"
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#include "global_defs.h"
#include "stdlib.h"
#define PIN_NUM 8

int a = 0;
OPERATION operation[PIN_NUM*2];

void copy_operation(OPERATION *src,OPERATION *dest);
void swap_operation(OPERATION *op1, OPERATION *op2);
void sort_operation(OPERATION op[]);

//ÓÃÓÚ´æ´¢Êä³öµÄPWM¶ÔÓ¦µÄGPIOÐòºÅ 21,20,22,24,14,13,15,0
unsigned char pin_array[PIN_NUM] = {21,20,22,24,14,13,15,0};

void GPA_SET(unsigned char pin){
    GpioDataRegs.GPASET.all |= ((long)1)<<pin;
}

void GPA_CLEAR(unsigned char pin){
    GpioDataRegs.GPACLEAR.all |= ((long)1)<<pin;
}
#define GPB_SET(pin) GpioDataRegs.GPBSET.all |= 1<<(pin-32);
#define GPB_CLEAR(pin) GpioDataRegs.GPBCLEAR.all |= 1<<(pin-32);

//inline void delay_us(float delay){
//    long delay_tmp = (long)(delay*18.018-1.8);
//    DSP28x_usDelay(delay_tmp);
//}
char writeConfigurationSet(const char *data, char dataSize, char useProgMem) {
    char *progBuffer, success, special;
    unsigned int i;
    // config set data is a long string of blocks with the following structure:
    // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
    char bank, offset, length;
    for (i = 0; i < dataSize;) {
            bank = data[i++];
            offset = data[i++];
            length = data[i++];

        // write data or perform special action
        if (length > 0) {
            // regular block of data to write
            progBuffer = (char*)data + i;

//            success = MPU6050_writeMemoryBlock(progBuffer, length, bank, offset, 1, 0);
            i += length;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.

            special = data[i++];
            if (special == 0x01) {
                // enable DMP-related interrupts

//                IICwriteByte(devAddr, MPU6050_RA_INT_ENABLE, 0x32);  // single operation

                success = 1;
            } else {
                // unknown special command
                success = 0;
            }
        }

        if (!success) {
            return 0; // uh oh
        }
    }
    return 1;
}
void readBlock(char *data, char dataSize, char bank, char address) {
    char chunkSize;
    unsigned int i;
//    MPU6050_setMemoryBank(bank,0,0);
//    MPU6050_setMemoryStartAddress(address);

    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = 10;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        // read the chunk of data as specified

        // increase byte index by [chunkSize]
        i += chunkSize;

        // charautomatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
//            MPU6050_setMemoryBank(bank,0,0);
//            MPU6050_setMemoryStartAddress(address);
        }
    }
}
//³õÊ¼»¯ËùÓÐ²Ù×÷£¬Éè¶¨Ã¿´Î²Ù×÷µÄÄÚÈÝÒÔ¼°ÑÓ³ÙÊ±¼ä
void init_operation(OPERATION op[]){
    op[0].dir = PON;
    //µÚÒ»¸öÂö³åµÈ´ýÊ±¼äÎªD2
    op[0].operation_time = D2;
    op[0].pin_number = pin_array[0];
    op[1].dir = POFF;
    op[1].operation_time = D2+M34;
    op[1].pin_number = pin_array[0];
    for(int i = 1; i< PIN_NUM;i++){
        op[i*2].dir = PON;
        op[i*2].operation_time = op[(i-1)*2].operation_time+phase2;
        op[i*2].pin_number = pin_array[i];

        op[i*2+1].dir = POFF;
        op[i*2+1].operation_time = op[i*2].operation_time+M34;
        op[i*2+1].pin_number = pin_array[i];
    }
    sort_operation(op);
}

//½»»»²Ù×÷£¬ÓÃÓÚÅÅÐòµÄÊ±ºòÊ¹ÓÃ
void swap_operation(OPERATION *op1, OPERATION *op2){
    OPERATION tmp;
    //op1=>tmp
    copy_operation(op1,&tmp);
    //op2=>op1
    copy_operation(op2,op1);
    //tmp=>op2
    copy_operation(&tmp,op2);
}

//¸´ÖÆ²Ù×÷
void copy_operation(OPERATION *src,OPERATION *dest){
    dest->dir = src->dir;
    dest->operation_time = src->operation_time;
    dest->pin_number = src->pin_number;
}

//Ã°ÅÝÅÅÐò
void sort_operation(OPERATION op[]){
    unsigned char flag = 0;
    for(int i = 0;i<PIN_NUM*2;i++){
        flag = 0;
        for(int j = i;j<PIN_NUM*2-1;j++){
            if(op[j].operation_time>op[j+1].operation_time){
                swap_operation(&op[j],&op[j+1]);
                flag = 1;
            }
        }
        if(flag == 0){
            break;
        }
    }
}

void single_pulse_start(){
    DINT;   // Enable Global interrupt INTM
    DRTM;   // Enable Global realtime interrupt DBGM
    //¹Ø±ÕÖÐ¶Ï
    ECap3Regs.TSCTR = 0;
    ECap3Regs.ECCTL2.bit.TSCTRSTOP = 1;
    for(int i = 0;i<PIN_NUM*2;i++){
        while(ECap3Regs.TSCTR <=operation[i].operation_time*90);
        if(operation[i].dir==PON){
            if(operation[i].pin_number > 31){
                GPB_SET(operation[i].pin_number);
            }else{
                GPA_SET(operation[i].pin_number);
            }
        }else{
            if(operation[i].pin_number > 31){
                GPB_CLEAR(operation[i].pin_number);
            }else{
                GPA_CLEAR(operation[i].pin_number);
            }
        }
    }
    //´ò¿ªÖÐ¶Ï
    EINT;
    ERTM;
}
