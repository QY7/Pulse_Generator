
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "sci_utils.h"
#include "global_defs.h"
#include "system_state.h"
#include "APWM_utils.h"
#include "memory_management.h"
#include "math.h"
void set_pwm_width(unsigned char channel,float duty);
void set_pwm_phase(float n_us);
void set_pwm_frequency(Uint32 period_in);
//void start_PWM_low();
void stop_PWM();
void init_ext_int();
void initScreenDate();
void initAPWM();

void enableOnePulseGPIO();
//inline void delay_us(float delay);
extern void init_operation(OPERATION op[]);
extern OPERATION operation[];
//#define _FLASH

unsigned char test_flag = 0;
//unsigned int frequency_Hz = 10000;//频锟斤拷默锟斤拷为10kHz

#ifdef _FLASH
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadSize;
extern Uint16 RamfuncsRunStart;
#endif

#define _SCIB
unsigned int test_data = 0;
extern unsigned int received_len;
//当等于1的时候，需要等待RX拉高，reset
char reset_flag = 0;


unsigned char data_flash[4] = {'a','b','c','d'};

unsigned char dataflash_back[4] = {0,0,0,0};

inline void delay_us(float delay){
    long delay_tmp = (long)(delay*18.018-1.8);
    DSP28x_usDelay(delay_tmp);
}

#define SAMPLE_BIAS 2000
#define SAMPLE_GAIN 2
void main(void)
{
//    Uint16 ReceivedChar;
    InitSysCtrl();
//    InitEPwm1Gpio();
//    InitEPwm2Gpio();
    InitSciaGpio();
#ifdef _SCIB
    InitScibGpio();
#endif
    DINT;
    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize);//锟斤拷锟斤拷ramfuncs装锟截达拷锟斤拷锟街凤拷锟斤拷锟斤拷莸锟斤拷锟斤拷写锟斤拷锟斤拷址
    InitFlash();                           //锟斤拷始锟斤拷Flash锟侥匡拷锟狡寄达拷锟斤拷
#endif


    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
#ifdef _SCIB
    PieVectTable.SCIRXINTB = &scibRxFifoIsr;
#endif
    PieVectTable.XINT1 = &xint1_isr;
    PieVectTable.TINT1 = &cpu_timer1_isr;
    PieVectTable.ECAP2_INT = &ecap2_isr;
    PieVectTable.ADCINT1 = &adc_isr;
//    PieVectTable.
//    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
    InitAdc();  // For this example, init the ADC
    scia_echoback_init();  // Initalize SCI for echoback
#ifdef _SCIB
    scib_echoback_init();  // Initalize SCI for echoback
#endif

    disableAPWMGPIO();//锟截憋拷APWM锟斤拷GPIO assign锟斤拷锟洁当锟节关憋拷锟斤拷锟�
    initAPWM();//锟斤拷始锟斤拷APWM
    enableOnePulseGPIO();//锟斤拷始锟斤拷OnePulse锟斤拷GPIO

    InitCpuTimers();   // For this example, only initialize the Cpu Timers
    //Timer0 10us enters interrupt
    ConfigCpuTimer(&CpuTimer0, 90, TIMER_PERIOD_US);//10us detect
    //Timer1锟斤拷实锟斤拷锟斤拷循锟斤拷锟斤拷锟节ｏ拷注锟斤拷C2000只锟斤拷锟斤拷锟斤拷Timer锟斤拷锟斤拷锟矫ｏ拷锟斤拷锟斤拷锟揭癸拷锟絋imer2锟侥伙拷锟斤拷要锟斤拷锟解处锟斤拷锟斤拷锟斤拷锟解，也锟斤拷锟斤拷通锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷实锟街★拷
    ConfigCpuTimer(&CpuTimer1, 90, Tw*1000000);//Tw

    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    init_ext_int();
    // to CPU-Timer 2:
    IER |= M_INT1;
    IER |= M_INT4; // Enable ECAP INT
    IER |= M_INT9; // Enable SCI INT
    IER |= M_INT13; // Enable SCI INT
 // Enable EPWM INTn in the PIE: Group 3 interrupt 1-3
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;  // Enable INT 1.1 in the PIE

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    // Enable TINT0 in the PIE: Group 1 interrupt 7
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;          // Enable PIE Group 1 INT4 XINT1
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    PieCtrlRegs.PIEIER4.bit.INTx2 = 1;//ENABLE ECAP2

    PieCtrlRegs.PIEIER9.bit.INTx1=1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx2=1;     // PIE Group 9, INT2
    PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT2

    // Enable global Interrupts and higher priority real-time debug events:
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;  // Enable non-overlap mode
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;    // ADCINT1 trips after AdcResults latch
    AdcRegs.INTSEL1N2.bit.INT1E     = 1;    // Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;    // Disable ADCINT1 Continuous mode
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 0;    // setup EOC0 to trigger ADCINT1 to fire
    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 0;    // set SOC0 channel select to ADCINA4
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 0;    // set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;    // set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    EDIS;
    DELAY_US(1000000);

    initScreenDate();
    state = OFF;
    add_data_channel();

    BSP_M25P16_Init();

//    delay_us(100000);
//    while(BSP_M25P16_WriteData(0, 0, 0, 1, data_flash));
//    save_params();
    delay_us(10000);
    load_params();
    for(;;)
    {
        if(test_flag == 1){
            update_channel(0);
            test_flag = 0;
        }else if(test_flag == 2){
            update_channel(1);
            test_flag = 0;
        }
        if(wait_flag == 1){
            wait_flag = 0;
            low_freq_mode = 1;
            low_freq_period = CLK_FREQ_MHz*1000000/frequency[index];
            num2 = (Uint32)((width1_us[index]+width2_us[index]+phase_us[index]+10)*CLK_FREQ_MHz);
            if(low_freq_period<num2){
                low_freq_period = num2;
                frequency[index] = CLK_FREQ_MHz*1000000/low_freq_period;
            }
            switch(index){
            case 0:
                update_touch_data(1,FREQUENCY_CONTROL_1, frequency[0],"%.0f");
//                update_PC_data(1,FREQUENCY_CONTROL_1,frequency[0]);
                break;
            case 1:
                update_touch_data(1,FREQUENCY_CONTROL_2, frequency[1],"%.0f");
//                update_PC_data(1,FREQUENCY_CONTROL_2,frequency[1]);
                break;
            case 2:
                update_touch_data(1,FREQUENCY_CONTROL_3, frequency[2],"%.0f");
//                update_PC_data(1,FREQUENCY_CONTROL_3,frequency[2]);
                break;
            default:break;
            }
        }
        if(state == START_BTN){
            init_operation(operation);
            set_pwm_frequency(low_freq_period);
            set_pwm_phase(phase_us[index]);
            //
            set_pwm_width(1,width1_us[index]);
            set_pwm_width(2,width2_us[index]);
//            delay_us(100000);/
            state = CMD_WAIT;
//            state = ON;
//            delay_us(1000000/frequency[index]);
//            enableAPWMGPIO();
        }else if(state == OFF){
//            stop_PWM_low();
            disableAPWMGPIO();
        }
        if(reset_flag && (GpioDataRegs.GPADAT.bit.GPIO11 == 1)){
            reset_flag = 0;
            ScibRegs.SCICTL1.bit.SWRESET = 0;
            delay_us(100);
            ScibRegs.SCICTL1.bit.SWRESET = 1;
            received_len = 0;
        }
        if(sample_finished_flag){
            sampled_voltage = AdcResult.ADCRESULT0;
//            这里的SAMPLE_BIAS和SAMPLE_GAIN与上位机保持一致
            sampled_voltage_conv = ((float)sampled_voltage-SAMPLE_BIAS)*SAMPLE_GAIN;
            sample_finished_flag = 0;
            char tmp[20];
            tmp[0] = 0x3A;
            tmp[1] = 0xBF;
            tmp[2] = 0x33;
            sprintf(tmp+3,"%d\n",sampled_voltage);
            sci_msg(tmp,strlen(tmp),PC);
          //  TODO
          //  发送采样数据到PC
            if(state == ON){
                add_data(sampled_voltage);
            }
//           最大是voltage_max,用转换过后的电压与voltage_max作除法，得到比例
//            由于voltage_max可能是负数，这里做了一次取绝对值
            char complete_ratio = abs((char)((sampled_voltage_conv)*100/voltage_max));
//            将完成比例上传到DSP
            update_progress(complete_ratio);
            if(voltage_max>0){
//          如果voltage_max是正压，那么超过正压的时候触发过压
                if(sampled_voltage_conv>voltage_max){
                    over_voltage_flag = 1;
                }
            }else{
//           如果voltage_max是负压，那么低于负压的时候触发过压
                if(sampled_voltage_conv<voltage_max){
                    over_voltage_flag = 1;
                }
            }
        }

    }
}

void set_pwm_width(unsigned char channel,float n_us){
    if(channel==1){
        ECap1Regs.CAP4 = ECap1Regs.CAP3 - (Uint32)(n_us*CLK_FREQ_MHz);;         // Set Compare value
    }else{
        ECap2Regs.CAP4 = (Uint32)(n_us*CLK_FREQ_MHz);         // Set Compare value
    }
}
void set_pwm_phase(float n_us){
    ECap2Regs.CTRPHS = ECap2Regs.CAP3-(Uint32)(n_us*CLK_FREQ_MHz);
}

void set_pwm_frequency(Uint32 period_in){
    ECap1Regs.CAP4 = period_in+1;
    ECap2Regs.CAP4 = 0;
    ECap1Regs.CAP3 = period_in;         // Set Period value
    ECap2Regs.CAP3 = period_in;         // Set Period value
    ECap1Regs.TSCTR = 0;
    ECap2Regs.TSCTR = 0;
}

void stop_PWM(){
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;     // Configure GPIO5 as CAP1
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;     // Configure GPIO5 as OUTPUT
    GpioDataRegs.GPADAT.bit.GPIO5 = 0;     // Configure GPIO5 as OUTPUT

    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;     // Configure GPIO5 as CAP1
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;     // Configure GPIO5 as OUTPUT
    GpioDataRegs.GPADAT.bit.GPIO7 = 0;     // Configure GPIO5 as OUTPUT
    EDIS;
}

void init_ext_int(){
 // GPIO0 is XINT1, GPIO1 is XINT2
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 1;
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0;

    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 12;   // XINT1 is GPIO0
//    GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 9;   // XINT2 is GPIO1
 // Configure XINT1 and XINT2
    XIntruptRegs.XINT1CR.bit.POLARITY = 1;      // Both Rising and Falling edge interrupt
 // Enable XINT1 and XINT2
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;        // Enable XINT1
//    XIntruptRegs.XINT2CR.bit.ENABLE = 1;        // Enable XINT2
    EDIS;
}

void enableOnePulseGPIO(){
    EALLOW;

    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
    delay_us(1);

    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO14 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO15 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO22 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO20 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;

    delay_us(1);
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;//output
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;//output
//    M3_LOW;
//    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;
//    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;//output
//    M4_LOW;
    EDIS;
}

void initScreenDate(){
    update_touch_data(1,PULSE1_CONTROL_1, width1_us[0],"%.1f");
    update_touch_data(1,PULSE1_CONTROL_2, width1_us[1],"%.1f");
    update_touch_data(1,PULSE1_CONTROL_3, width1_us[2],"%.1f");

    update_touch_data(1,PULSE2_CONTROL_1, width2_us[0],"%.1f");
    update_touch_data(1,PULSE2_CONTROL_2, width2_us[1],"%.1f");
    update_touch_data(1,PULSE2_CONTROL_3, width2_us[2],"%.1f");

    update_touch_data(1,D1_CONTROL, D1,"%.0f");
    update_touch_data(1,D2_CONTROL, D2,"%.0f");
    update_touch_data(1,PHASE2_CONTROL, phase2,"%.0f");
    update_touch_data(1,M34_CONTROL, M34,"%.0f");

    update_touch_data(1,PHASE_CONTROL_1, phase_us[0],"%.1f");
    update_touch_data(1,PHASE_CONTROL_2, phase_us[1],"%.1f");
    update_touch_data(1,PHASE_CONTROL_3, phase_us[2],"%.1f");

    update_touch_data(1,FREQUENCY_CONTROL_1, frequency[0],"%.0f");
    update_touch_data(1,FREQUENCY_CONTROL_2, frequency[1],"%.0f");
    update_touch_data(1,FREQUENCY_CONTROL_3, frequency[2],"%.0f");

    update_touch_data(0,MINIUTE_CONTROL, timer_minute,"%.0f");
    update_touch_data(0,SECOND_CONTROL, timer_second,"%.0f");

    update_touch_data(0,TC_CONTROL, Tc,"%.0f");
    update_touch_data(0,TW_CONTROL, Tw,"%.0f");

    send_two_byte_data(1,INDEX_CONTROL,index);

    icon_off_state();
}

void initAPWM(){

    //锟斤拷始锟斤拷Ecap锟斤拷锟斤拷锟揭匡拷锟斤拷锟叫讹拷
    ECap1Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags
    ECap1Regs.ECCTL2.bit.CAP_APWM = 1;   // Enable APWM mode
    ECap2Regs.ECCTL2.bit.CAP_APWM = 1;   // Enable APWM mode


    ECap2Regs.ECEINT.bit.CTR_EQ_CMP = 1; //Enable APWM interrupt when CTR equals CMP
//    ECap2Regs.ECEINT.bit.CEVT2 = 1;

    ECap1Regs.ECCLR.all = 0x0FF;         // Clear pending interrupts锟斤拷锟叫断ｏ拷锟斤拷锟叫讹拷锟斤拷freeze counter锟斤拷锟斤拷证锟杰癸拷锟截碉拷ecap

    ECap1Regs.ECCTL2.bit.APWMPOL = 1;
    ECap2Regs.ECCTL2.bit.APWMPOL = 0;
    ECap1Regs.CAP1 = pwm1_prd;         // Set Period value
    ECap1Regs.CAP2 = pwm1_cmp;         // Set Compare value
//
    ECap2Regs.CAP1 = pwm2_prd;         // Set Period value
    ECap2Regs.CAP2 = pwm2_cmp;         // Set Compare value
    // Start counters
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;
    ECap2Regs.ECCTL2.bit.TSCTRSTOP = 1;

    ECap1Regs.ECCTL2.bit.SYNCO_SEL = 1;
    ECap2Regs.ECCTL2.bit.SYNCI_EN = 1;
    ECap2Regs.CTRPHS  = pwm2_pha;         // Set Compare value
}
