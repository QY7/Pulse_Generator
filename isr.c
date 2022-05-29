/*
 * isr.c
 *
 *  Created on: Aug 18, 2020
 *      Author: Felix
 */

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "global_defs.h"
#include "sci_utils.h"
#include "APWM_utils.h"
#include "multiple_pulse.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
extern long cnt;
unsigned int received_len = 0;
//PC接管控制
unsigned char takeover_flag = 0;
extern char reset_flag;
//extern unsigned int cnt_Tx;
extern void stop_PWM();

__interrupt void sciaRxFifoIsr(void)
{
    static unsigned int received_len = 0;
    rdataA[received_len++]=SciaRegs.SCIRXBUF.all & 0xFF;  // Read data
    if(received_len>4 && rdataA[received_len-1]==0xFF && rdataA[received_len-2] == 0xFF && rdataA[received_len-3] == 0xFC && rdataA[received_len-4]== 0xFF){
        rdataA[received_len] = '\0';
        if(takeover_flag==0){
            process_cmd(rdataA,received_len,TOUCHPAD);
        }
        received_len = 0;
        wait_flag = 1;
    }

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack
}

__interrupt void scibRxFifoIsr(void)
{
    rdataB[received_len]=ScibRegs.SCIRXBUF.all & 0xFF;  // Read data
    if(rdataB[received_len]=='\n'){
        rdataB[received_len] = '\0';
        process_cmd(rdataB,received_len,PC);
        received_len = 0;
    }else{
        received_len++;
    }
    ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= 0x100;       // Issue PIE ack
}
__interrupt void cpu_timer0_isr(void)
{
   // Acknowledge this interrupt to receive more interrupts from group 1
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;
    static unsigned int data_cnt = 0;
    static unsigned int inner_cnt = 0;
    static unsigned int sci_idle_cnt = 0;
    if(state == OFF){
        // detecting sci idle,10us interrupt
        if(GpioDataRegs.GPADAT.bit.GPIO11 == 0){
            sci_idle_cnt++;
            if(sci_idle_cnt >=1000){
                // 10us*1000=10ms
                //set sci restart flag, it reset scib once pc is connected
                reset_flag = 1;
                sci_idle_cnt = 0;
                //scib is idle, let touchpad handle this situation
                takeover_flag = 0;
            }
        }else{
            sci_idle_cnt = 0;
            //GPIO11 is high level, meaning PC is connected
            takeover_flag = 1;
        }
    }
    if(data_cnt++==10){
        if(++inner_cnt == 5){
            inner_cnt = 0;
        }
        //Software trigger ADC
        AdcRegs.ADCSOCFRC1.all = 0x01;
        data_cnt = 0;
    }
    if(state == OFF){
//        stop_cmd_flag = 0;
        cnt = 0;
//        cnt_Tx = 0;
        return;
    }

    if(state == WAIT){
        if(stop_cmd_flag == 1){
            state = OFF;
            stop_cmd_flag = 0;
            icon_off_state();
        }
        if(cnt>=PERIOD_1MS/TIMER_PERIOD_US*D1){
            //如果状态是等待，且时间超过了等待时间，就开启，然后计数器清零
            state = CMD_ON;
            //开启了脉冲，计数器清零
            cnt = 0;
        }else{
            //如果状态是等待，则cnt+1计时，直到计数到为D1
            cnt++;
        }
    }
    else if(state == ON){
        if(cnt>=PERIOD_1MS/TIMER_PERIOD_US*Tc){
            //如果状态是开启的状态，然后时间超过了设定的时常限制就关掉

            cnt = 0;
            //TODO
            one_pulse_flag = 1;
            if(stop_cmd_flag == 1){
                state = OFF;
                stop_cmd_flag = 0;
                icon_off_state();
            }else{
                state = CMD_WAIT;
            }
        }else{
            //计时，直到计数为Tw
            cnt++;
            if(over_voltage_flag){
//                如果检测到电压到了，那么关闭M1和M2，然后发后面几路的Pulse
                state = CMD_WAIT;
                one_pulse_flag = 1;
                cnt = 0;
                over_voltage_flag = 0;
            }
        }
    }else{
        if(stop_cmd_flag == 1){
            state = OFF;
            stop_cmd_flag = 0;
            icon_off_state();
        }
    }
}

__interrupt void ecap2_isr(void){
//    EALLOW;
//    GpioDataRegs.GPATOGGLE.bit.GPIO0 = 1;
    if(one_pulse_flag == 1){
        stop_PWM();
        single_pulse_start();
        one_pulse_flag = 0;
    }
    ECap2Regs.CAP1 = ECap2Regs.CAP3;//手动赋值
    if(state == CMD_ON){
        state = ON;
        enableAPWMGPIO();
    }
    //以下的代码运行结果是不对的，但是顺序换一下，运行结果就是对的。很迷
    ECap2Regs.ECCLR.bit.CTR_EQ_CMP = 1;
    ECap2Regs.ECCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
//    EDIS;
}

__interrupt void xint1_isr(void)
{
    // Acknowledge this interrupt to get more from group 1
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;
    //当外部中断产生的时候，进入WAIT状态
    if(state == CMD_WAIT && pulse_source_index == 1){
        //如果信号源为外部，而且开启了接收，则进入等待脉冲源状态
        state = WAIT;
        stop_PWM();
    }
}

//TODO
//写大周期触发的功能
__interrupt void cpu_timer1_isr(void)     // INT13 or CPU-Timer1
{
    if(state == CMD_WAIT && pulse_source_index == 0){
        //如果信号源为内部，且开启，则进入等待脉冲源状态
        state = WAIT;
        stop_PWM();
//        cnt = 0;
    }
}
__interrupt void  adc_isr(void)
{
  AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;     //Clear ADCINT1 flag reinitialize for next SOC
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

  sample_finished_flag = 1;
  return;
}
