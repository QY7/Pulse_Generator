/*
 * global_defs.h
 *
 *  Created on: Jun 10, 2020
 *      Author: Felix
 */

#ifndef GLOBAL_DEFS_H_
#define GLOBAL_DEFS_H_

#include "system_state.h"

#define CLK_FREQ_MHz 90.0


extern const int x[5];

extern const char PULSE1_CONTROL_1;
extern const char PULSE1_CONTROL_2;
extern const char PULSE1_CONTROL_3;

extern const char PULSE2_CONTROL_1;
extern const char PULSE2_CONTROL_2;
extern const char PULSE2_CONTROL_3;

extern const char PHASE_CONTROL_1;
extern const char PHASE_CONTROL_2;
extern const char PHASE_CONTROL_3;

extern const char FREQUENCY_CONTROL_1;
extern const char FREQUENCY_CONTROL_2;
extern const char FREQUENCY_CONTROL_3;

extern const char MINIUTE_CONTROL;
extern const char SECOND_CONTROL;

extern const char D1_CONTROL;
extern const char D2_CONTROL;
extern const char TW_CONTROL;
extern const char TC_CONTROL;

extern const char M34_CONTROL;
extern const char PHASE2_CONTROL;

extern float D1,Tw,Tx,Tc;
extern float D2,M34,M34test,phase2;

extern const char INDEX_CONTROL;

extern char index;

extern float PWM_CLK_DIV;

//extern unsigned char cnt;

extern Uint32 pwm1_prd;
extern Uint32 pwm1_cmp;

extern Uint32 pwm2_pha;
extern Uint32 pwm2_cmp;
extern Uint32 pwm2_prd;

extern unsigned int period;
extern Uint32 low_freq_period;
extern unsigned int frequency[];
extern unsigned char low_freq_mode;
extern float width1_us[3];//����1�Ŀ���Ĭ��Ϊ5us
extern float width2_us[3];//����2�Ŀ���Ĭ��Ϊ1us
extern float phase_us[3];//��λĬ��Ϊ5us
extern float timer_minute;
extern float timer_second;
extern char change_flag;
extern unsigned int num1;
extern Uint32 num2;
extern enum sys_state state;
extern enum sys_state pre_state;
extern char rdataA[50];    // Received data for SCI-A
extern char rdataB[50];    // Received data for SCI-B
extern char stop_cmd_flag;
extern char one_pulse_flag;
extern unsigned char sample_finished_flag;
extern unsigned int voltage_max;
extern int sampled_voltage;
extern float sampled_voltage_conv;
//�жϺ���
extern __interrupt void cpu_timer0_isr(void);
extern __interrupt void cpu_timer1_isr(void);
extern __interrupt void sciaRxFifoIsr(void);
extern __interrupt void scibRxFifoIsr(void);
extern __interrupt void xint1_isr(void);
extern __interrupt void ecap2_isr(void);
extern __interrupt void  adc_isr(void);

#define M3_LOW GpioDataRegs.GPACLEAR.bit.GPIO0 = 1
#define M3_HIGH GpioDataRegs.GPASET.bit.GPIO0 = 1
#define M4_LOW GpioDataRegs.GPACLEAR.bit.GPIO2 = 1
#define M4_HIGH GpioDataRegs.GPASET.bit.GPIO2 = 1

#endif /* GLOBAL_DEFS_H_ */
