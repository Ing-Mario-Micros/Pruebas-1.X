/* 
 * File:   RS232.h
 * Author: mario
 *
 * Created on 24 de agosto de 2022, 04:56 PM
 */

#ifndef RS232_H
#define	RS232_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif
#include <xc.h>

#define Tam_Vec 20              //Define el tamaño del Vector de recepción RS232

void Activar_RS232(void);
void Interrupcion_RS232 (void);
/* Función imprimir Cadena de caracteres */
void MensajeRS232(char *);    //Función para imprimir cadena de Caracteres
void Transmitir(unsigned char BufferT);

char b;
char BufferR2[Tam_Vec];
void Activar_RS232(void){
    U2MODE=0x0000;
    U2STA=0x0000;
    U2BRG=11;
    U2MODE=U2MODE|0x8000;
    U2STA=U2STA|0x0400;
    /***Interrupción Recepción de datos***/
    _U2RXIE=1;
    _U2RXIP=6;
    _U2RXIF=0;
}
/***-----------------------------Funciones RS232--------------------------***/
void Interrupcion_RS232 (void){
     _U2RXIF=0;
         BufferR2[b]=U2RXREG;
         b++;
         if(BufferR2[b-1]=='\n'||b==Tam_Vec){ 
             while(b!=Tam_Vec){
                 BufferR2[b]=0;
                 b++;
             }
             b=0;
         }
}
void MensajeRS232(char* a){
//Función que escribe una cadena de caracteres variable en la pantalla
//a es una cadena de caracteres guardada en una variable *char
//Ejemplo: char aux[4]="Hola"; MensajeLCD_Var(aux);	
	while (*a != '\0'){
		Transmitir(*a); 
        /*if(*a != '\n'){
          Transmitir(*a);  //En el caso de no querer enviar el salto de linea
                           //Se puede usar esta función
        }/**/
		a++;
	}		
}
void Transmitir(unsigned char BufferT){
    
        while((U2STA&0x0100)==0);
        U2TXREG=BufferT;
        //__delay_us(800);
    
}
#endif	/* RS232_H */

