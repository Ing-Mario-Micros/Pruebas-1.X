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

void ImprimirDecimal (float); //Función para imprimir un valor decimal en el puerto serial

void Codificar_Dato (void); //Función para identificar las directivas enviadas desde dispositivo RS232
char b;
char BufferR2[Tam_Vec];
extern char Vector_Datos[];
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
             if(BufferR2[0]>=47 && BufferR2[0]<=67){
                Codificar_Dato();
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
/*  Funcion para imprimer el numero decimal  */
void ImprimirDecimal (float An){
    char Entero; //prueba de impresion decimal
    int Decimal,aux1,i;
    if(An<0){
        Transmitir('-');
        An=An*(-1);
    }
    Entero=(int)An;             //La Patre entera de la división se guarda en Entero
    An= An-Entero;              //Dejar solamente la parte decimal
    An= An*10000;               //Convierto los decimales a enteros para poder extraerlos
    Decimal = (int)An;          //La Parte decimal de la división se guarda en decimal
    //aux2 = Entero;
    //aux3 = Decimal;
    i=10;
    while(Entero%i!=Entero){
        i=i*10;
    }
    i=i/10;
    aux1=Entero;
    while(i>=1){
        Transmitir((aux1/i)+48);
        aux1=aux1%i;
        i=i/10;
    }
    Transmitir('.');
    i=1000;
    aux1=Decimal;
    while(i>=1){
        Transmitir((aux1/i)+48);
        aux1=aux1%i;
        i=i/10;
    }
    Transmitir(' ');
}
void Codificar_Dato (void){
    unsigned char Var_Codificada,i, j=1;
    for(i=1;BufferR2[i]!='\n';i++){
        Var_Codificada=Var_Codificada*j;
        Var_Codificada=Var_Codificada+(BufferR2[i]-48);
        j=j*10;
    }
    if(BufferR2[1]==47){
        //Transmitir_VDatos
    }
    else{
        Vector_Datos[BufferR2[0]-34]=Var_Codificada;
    }
}
#endif	/* RS232_H */

