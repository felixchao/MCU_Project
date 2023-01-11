#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
// using namespace std;
#define _XTAL_FREQ 4000000

char str[20];
int n = 6;
int count = 1;

void delay(unsigned int val){
    unsigned int i,j; //msec
    for(i=0;i<val;i++)
        for(j=0;j<165;j++);
}

void PWM_initialize(){
    /*******************     MOTOR    ************************/
    //interrupt setting
    RCONbits.IPEN = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.INT0IE = 1;
    INTCONbits.INT0IF = 0;
    INTCON2bits.RBPU = 0;           // PORTB pull-ups are enabled by individual port latch values
    INTCON2bits.INTEDG0 = 1;        // set external Interrupt #0 on falling edge
    
    // Timer2 -> On, prescaler -> 4
    T2CONbits.TMR2ON = 0b1;
    T2CONbits.T2CKPS = 0b01;

    // Internal Oscillator Frequency, Fosc = 125 kHz, Tosc = 1/Focs = 8 탎
    
    // PWM mode, P1A, P1C active-high; P1B, P1D active-high
    CCP1CONbits.CCP1M = 0b1100;
    
    // CCP1/RC2 -> Output
    TRISC = 0;
    LATC = 0;
    TRISBbits.RB0 = 1;

    PR2 = 0x9b;
    
    /**
     * Duty cycle
     * = (CCPR1L:CCP1CON<5:4>) * Tosc * (TMR2 prescaler)
     * = (0x0b*4 + 0b01) * 8탎 * 4
     * = 0.00144s ~= 1450탎
     */
    CCPR1L = 0x0b;
    CCP1CONbits.DC1B = 0b01;
    

}

void PWM(int degree){
     /**
     * Duty cycle
     * = (CCPR1L:CCP1CON<5:4>) * Tosc * (TMR2 prescaler)
     * = (0x0b*4 + 0b01) * 8탎 * 4
     * = 0.00144s ~= 1450탎
      * 2400 = 90degree
      * 500 = -90degree
     */
    OSCCONbits.IRCF = 0b001;
    CCP1CONbits.DC1B = (degree/32)%4;
    CCPR1L = (degree/128);
    __delay_ms(10);
}

void ADC(){
            //configure OSC and port
    OSCCONbits.IRCF = 0b001; //1MHz
    TRISAbits.RA0 = 1;       //analog input port
    
    //step1
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.PCFG = 0b1110; //AN0 ?analog input,???? digital
    ADCON0bits.CHS = 0b0000;  //AN0 ?? analog input
    ADCON2bits.ADCS = 0b100;  //????000(1Mhz < 2.86Mhz)
    ADCON2bits.ACQT = 0b010;  //Tad = 2 us acquisition time?2Tad = 4 > 2.4
    ADCON0bits.ADON = 1;
    ADCON2bits.ADFM = 1;    //left justified 
    
    
    //step2
    PIE1bits.ADIE = 1;
    PIR1bits.ADIF = 0;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    
    TRISB = 0;
    LATB = 0;


    //step3
    ADCON0bits.GO = 1;
}

void Timer1(){
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    IPR1bits.TMR1IP = 1;
    
    T1CON = 0x20;
    
    TMR1H = 0xFF - 0xF4;
    TMR1L = 0xFF - 0x24 + 1;
    
    T1CONbits.TMR1ON = 1;
}

void Timer2(){
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    IPR1bits.TMR2IP = 1;
    
    T2CON = 0x7B;
    
    PR2 = 244;
    TMR2 = 0;
    
    T2CONbits.TMR2ON = 1;
}


void Mode1(){   // Todo : Mode1 
    __delay_ms(10);
    TRISB = 0;
    PORTB = 3;
    TRISDbits.TRISD2 = 1;
    
    PWM_initialize();
    PWM(1450);
    int count = 1;
    int inter = 950/n;
    
    while(count <= num){
        if(PORTDbits.RD2 == 0){
           PWM(500+count*inter);
           count++;
        }
    }
    
    count = num - 1;
    
    while(count >= 0){
        if(PORTDbits.RD2 == 0){
           PWM(500+count*inter);
           count--;
        }
    }
    
    state = 0;
    
    ClearBuffer();

    return ;
}

void Mode2(){   // Todo : Mode2 
    __delay_ms(20);
    while(num == 0);
    __delay_ms(20);
    while(count <= num){
      TRISB = 0;
      
      PORTB = count;
    
     PWM_initialize();
     int inter = 950/num;
    PWM(1450);
    PWM(1450-inter*count);
    PWM(1450);
    PWM(1450+inter*count);
    PWM(1450);
    count++;
//    if(PORTDbits.RD2 == 0){
//          count++;
//    }

    }
    return ;
}


 
void main(void) 
{
    
    SYSTEM_Initialize();
    ClearBuffer();
    PWM_initialize();
    
    
    TRISD = 0;
    TRISBbits.RB1 = 1;
    PORTDbits.RD2 = 0;
    
    while(1){
        if(PORTBbits.RB1 == 0){
            PORTDbits.RD2 = ~PORTDbits.RD2;
            __delay_ms(100);
        }
    }
    
    //while(1);
    
    return;
    
    
    return;
}


void ADC_ISR(){
           //step4
    int valueh = ADRESH;
    int valuel = ADRESL;
    int value = (valueh*256+valuel)/180;
    count = value*400 + 500;
    PWM(count);

    if((value & 1) != 0 )
      LATBbits.LATB0 = 1;
    if((value & 2) != 0 )
      LATBbits.LATB1 = 1;
    if((value & 4) != 0 )
      LATBbits.LATB2 = 1;
    if((value & 8) != 0 )
      LATBbits.LATB3 = 1;

    //clear flag bit
    
    
    PIR1bits.ADIF = 0;
    ADCON0bits.GO = 1;

}

void Timer1_ISR(){
    char str[5];
    timer++;
    if(timer >= 10){
        sprintf(str, "\b\b%d", timer);
    }
    else{
        sprintf(str, "\b%d", timer);
    }
    UART_Write_Text(str);
    PWM(90);
    PIR1bits.TMR1IF = 0;
}
//void Timer2_ISR(){
//    if(x < 0){
//       PORTB = 0;
//       PIR1bits.TMR2IF = 1;
//       return;
//    }
//    if(t == 0){
//      PORTB = x;
//      x--;
//      t = 1;
//      PIR1bits.TMR2IF = 0;
//    }
//    else{
//        t--;
//        PIR1bits.TMR2IF = 0;
//    }
//}

void __interrupt(high_priority) Hi_ISR(void)
{
     if(INTCONbits.INT0IF == 1){
        UART_Write_Text("Pouring\r\n");
        /* rotate counter-clockwise */
        //CCPR1L = 0x04 --> 0x13;
        //CCP1CONbits.DC1B = 0b00 --> 0b11
        //d'4' -->  d'75'
        __delay_ms(10);
        OSCCONbits.IRCF = 0b001;
        CCPR1L = 0X09;
        num = num/10;
        
        for(int i = 4; i <= 35; i++){
            CCP1CONbits.DC1B += 0b01;
            if(CCP1CONbits.DC1B == 0b00){
                CCPR1L += 1;
            }
            
            delay(1);
            if(CCPR1L == (num + 0x09)){
                break;
            }
        }
        
        delay(10);

        /* set to initial clockwise  */
        //CCPR1L = 0x13 --> 0x04;
        //CCP1CONbits.DC1B = 0b11 --> 0b00
        // d'75' -->  d'4'
        for(int i = 4; i <= 35; i++){
            CCP1CONbits.DC1B -= 0b01;
            if(CCP1CONbits.DC1B == 0b11){
                CCPR1L -= 1;
            }
            delay(1);
            if(CCPR1L == 0x09 && CCP1CONbits.DC1B == 0b01){
                break;
            }
        }

        delay(10);
        INTCONbits.INT0IF = 0;
    }
    OSCCONbits.IRCF = 0b100;
    UART_Write_Text("Finish\r\n");
    return;
}