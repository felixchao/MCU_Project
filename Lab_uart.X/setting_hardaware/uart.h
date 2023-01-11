#ifndef _UART_H
#define _UART_H

int lenStr;
int num;
int state;
int timer;

      
void UART_Initialize(void);
char * GetString();
void UART_Write(unsigned char data);
void UART_Write_Text(char* text);
void ClearBuffer();
void MyusartRead();

#endif