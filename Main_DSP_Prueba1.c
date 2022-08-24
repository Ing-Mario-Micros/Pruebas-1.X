/*
 * File:   DSPIC_Main.c
 * Author: mario
 *
 * Created on 26 de julio de 2022, 01:07 PM
 */


#include <xc.h>
#include "RS232.h"
//Fosc = 7.37MHz Por Defecto
#define FCY 1842500
#include <libpic30.h>

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

#define LED_CPU _LATD3

/*------------------------- Función de Interrupción Timer 1 ----------------*/
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void);

/*------------------------- Funciones de I2C -------------------------------*/
void __attribute__((interrupt,auto_psv)) _SI2CInterrupt(void);


void Start (void);
void Restart (void);
void Stop (void);
void ACK_NACK (void);
void Lectura_Dir (void);
char Error ;
char contador=0;
char recepcion =0;
/**Variables de comunicación Esclavo**/
unsigned char Tipo=0, Fase=0;
unsigned char Registro_Temp, Dato_Temp;
char Vector_Datos[20];

/*------------------------- Variables PWM --------------------------------*/
void Por_PWM (float);
void main(void) {
    /*------------------ Configuración de Pines Digital --------------------*/
    TRISD=0;
    LATD=0;
    _LATD9 = 1;
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
    /*----------------------------- Funciones de PWM ---------------------------*/
    
    while(1){
         _LATD9 = 1;
        MensajeRS232("Hola Mundo\n");
        MensajeRS232(BufferR2);
        _LATD9 = 0;
        //Lectura_Dir();
        OC1RS=10*Vector_Datos[0x11];
        __delay_ms(1000);
    }
}
/*----------------------------- Funciones de PWM ---------------------------*/
void Por_PWM (float PPWM){
    //char aux=0
    //aux =(PPWM*1842);
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
    char aux = 0;
    if(_R_W == 0){
        if(_D_A == 1){
            switch(Tipo){
                case 0:
                    if(I2CRCV>=0 && I2CRCV<=13){
                        Tipo=1;
                    }
                    else{
                        Tipo=2;
                    }
                    Registro_Temp=I2CRCV;
                    Fase=0;
                    break;
                case 2:
                    if(Fase==0){
                        Dato_Temp=I2CRCV;
                        Fase=1;
                    }
                    else{
                        if(Fase==2){
                            if(I2CRCV==2){
                                Vector_Datos[Registro_Temp]=Dato_Temp;
                                Fase=3;
                                Tipo=0;
                            }
                            else{
                                Error = Error & 0x08;
                            }
                        }
                    }
                    break;
            }
        }
        else{
            aux=I2CRCV;
        }
    }
    else{
        if(_SCLREL==0){
            if(Tipo==1){
                if(Fase==0){
                    I2CTRN=Vector_Datos[Registro_Temp];
                    while(_TBF==0);
                    _SCLREL=1;
                    Fase=1;
                }
                else{
                    if(Fase==1){
                        I2CTRN=Registro_Temp;
                        while(_TBF==0);
                        _SCLREL=1;
                        Fase=2;
                        Tipo=0;
                    }
                }
            }
            else{
                if(Tipo==2){
                    I2CTRN=Registro_Temp;
                    while(_TBF==0);
                    _SCLREL=1;
                    Fase=2;
                }
            }
        }
        aux = I2CRCV;
    }

    _SI2CIF=0;
}
void Lectura_Dir (void){
    Start();
    /** Envio de Direccíon modo Escritura hacia el esclavo **/
    I2CTRN=0x0A;
    while(_TRSTAT == 1 && _TBF ==1);
    if(_ACKSTAT == 1){
        Error = Error|0x01;
    }
    /** Envio de Dato hacia el esclavo **/
    I2CTRN=contador;
    _MI2CIF = 0;
    while(_TRSTAT == 1 && _TBF == 1);
    __delay_us(1);
    if(_ACKSTAT == 1){
        Error = Error | 0x02;
    }
    /** Directiva de Reinicio **/
    Restart();
    /** Envio de Direccíon modo lectura hacia el esclavo **/   
    I2CTRN=0x0B;
    _MI2CIF = 0;
    while(_TRSTAT == 1 && _TBF == 1);
    __delay_us(1);
    if(_ACKSTAT == 1){
        Error = Error | 0x04;
    }
    /** Recepción de dato desde el esclavo **/
    _RCEN = 1;
    while(_RCEN == 1);
    _ACKDT = 1; //NACK
    _ACKEN = 1;
    _MI2CIF = 0;
    while(_ACKEN == 1);
    recepcion = I2CRCV;
    /** Directiva para terminar Comunicación **/
    Stop();
}
void Stop (void){
    _PEN = 1;
    while(_PEN == 1);
}
void Start (void){
    if(_P ==1 || _RF3 == 1){
        _SEN = 1;
        while(_SEN==1);
    }
}
void Restart (void){
    _RSEN=1;
    while(_RSEN == 1);
}