/*
 * APWM_utils.c
 *
 *  Created on: May 20, 2021
 *      Author: Felix
 */

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

void enableAPWMGPIO(){
    EALLOW;
//    ECap1Regs.TSCTR = 0;
//    ECap2Regs.TSCTR = ECap2Regs.CTRPHS;
//    GpioCtrlRegs.GPAQSEL1.bit.GPIO5 = 0;     // Synch to SYSCLKOUT GPIO5 (CAP1)
//    |=(0x3<<10)|0x3<<14
//    GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;     // Synch to SYSCLKOUT GPIO7 (CAP1)
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 3;      // Configure GPIO5 as CAP1
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 3;      // Configure GPIO7 as CAP1

    //TODO 手动去装载CTR，手动同步
    EDIS;
}
void disableAPWMGPIO(){
    EALLOW;

    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;     // Configure GPIO5 as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;     // Configure GPIO5 as OUTPUT
    GpioDataRegs.GPADAT.bit.GPIO5 = 0;     // Configure GPIO5 as OUTPUT

    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;     // Configure GPIO7 as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;     // Configure GPIO7 as OUTPUT
    GpioDataRegs.GPADAT.bit.GPIO7 = 0;     // Configure GPIO7 as OUTPUT
    EDIS;
}
