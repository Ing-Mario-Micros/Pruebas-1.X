/*
 * File:   DSPIC_Main.c
 * Author: mario
 *
 * Created on 26 de julio de 2022, 01:07 PM
 */


#include <xc.h>
#include "RS232.h"
#include "I2C.h"
//Fosc = 7.37MHz Por Defecto
#define FCY 1842500
#include <libpic30.h>
#include <math.h>

// DSPIC30F4013 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FOSFPR = FRC             // Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define LED_CPU _LATC13

/*------------------------- Función de Interrupción Timer 1 ----------------*/
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void);

/*------------------------- Funciones de I2C -------------------------------*/
void __attribute__((interrupt,auto_psv)) _SI2CInterrupt(void);





extern char Error ;


extern char Vector_Datos[];

/*------------------------- Variables PWM --------------------------------*/
void Por_PWM (float);
void main(void) {
    /*------------------ Configuración de Pines Digital --------------------*/
    TRISD=0;
    LATD=0;
    _LATD9 = 1;
    TRISC=0;
    _CN5PUE=1;
    /*------------------ Configuración del Timer 1 -------------------------*/
    PR1=7196;
    TMR1=0;
    _T1IF=0;
    T1CON = 0x8020;
    /*------------------ Configuración de RS232 ---------------------------*/
    Activar_RS232();
    /*------------------- Configuración de PWM -----------------------------*/
    PR2=1842;
    TMR2=0;
    _T2IF=0;
    T2CON= 0x0000;
    OC1R=0;
    OC1RS=200;
    OC1CON= 0x0000;
    _OCM=0b110;
    T2CON= T2CON|0x8000;
    /*------------------ Configuración de Interrupciones -------------------*/
    /**** Interrupción Timer 1 ****/
    _T1IE = 1;  //Habilitación Interrupción Timer1
    _T1IP = 7;  //Definición de Prioridad del Timer1
    _T1IF = 0;  //Inicializar la bandera de interrupción en 0
    /**** Interrupción I2C ****/
    _SI2CIE = 1;
    
    /*------------------- CONFIGURACIÓN I2C --------------------------------*/
    I2CBRG=16;
    I2CCON=0x9100;
    /**** I2C Esclavo ****/
    I2CADD = 0x08;
    _SI2CIF=0;
    /** Prueba de reset **/
    __delay_ms(1000);
    _LATD9 = 0;
    Vector_Datos[0x0]=10;
    Vector_Datos[0x1]=20;
    Vector_Datos[0x2]=30;
    Vector_Datos[0x3]=40;
    Vector_Datos[14]=50;
    /*----------------------------- Funciones de PWM ---------------------------*/
    Por_PWM (0.0);
    float a = 0.0;
    while(1){
         _LATD9 = 1;
        MensajeRS232("Hola Mundo\n");
        MensajeRS232(BufferR2);
        Transmitir(Vector_Datos[14]);
        Transmitir('-');
        a=(Vector_Datos[14]*0.01); //Porque no puedo dividir?
        ImprimirDecimal(a);
        Transmitir('-');
        Por_PWM (Vector_Datos[14]*0.01);
        _LATD9 = 0;
        //Lectura_Dir();
        //OC1RS=10*Vector_Datos[0x11];
        __delay_ms(1000);
    }
}
/*----------------------------- Funciones de PWM ---------------------------*/
void Por_PWM (float PPWM){
    //char aux=0
    //aux =(PPWM*1842);
    ImprimirDecimal (PPWM);
    OC1RS = (int)( (1842+1)*PPWM );
}
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void){
    LED_CPU=LED_CPU^ 1; // Conmutar PinC13 LED CPU
    _T1IF=0;            // Reset de bandera de interrupción en Cero
}
void __attribute__((interrupt,auto_psv)) _U2RXInterrupt(void){
    Interrupcion_RS232();
}
void __attribute__((interrupt,auto_psv)) _SI2CInterrupt(void){
    Interrupcion_Recepcion_I2C();

}



