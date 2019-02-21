/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include <LPC17xx.h>
#include "stdio.h"
#include "GPIO.h"
#include "Driver.h"

#define ENTETE 0xA5
#define STOP 0x25
#define RESET 0x40
#define SCAN 0x20

char distance[360];

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern ARM_DRIVER_USART Driver_USART0;

void Reponse(void const * argument);
void LectureLidar(void const * argument);

osThreadId ID_Reponse;
osThreadId ID_Lecture;
osThreadDef (Reponse,osPriorityNormal, 1 , 0);
osThreadDef (LectureLidar, osPriorityNormal, 1, 0);
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	GLCD_Initialize();
	init_PWM6();
	Init_UART();
	Initialise_GPIO(); 
	
  // initialize peripherals here

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	
	ID_Reponse = osThreadCreate(osThread(Reponse), NULL);
	ID_Lecture = osThreadCreate(osThread(LectureLidar), NULL);
	
  osKernelStart ();                         // start thread execution 
	
	osDelay(osWaitForever);
}

void Reponse(void const * argument){
	uint8_t tabRx[7],tabTx[2], donnee[20];
	char d[8];
	int i=0,rx=0;
	
	osEvent result;
	
	tabTx[0] = ENTETE;
	
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
	GLCD_SetBackgroundColor  (GLCD_COLOR_WHITE);
	
	GLCD_DrawString(0,50,"INIT!");
	
	tabTx[1] = STOP;
	
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(tabTx,2);
	
	osDelay(3000);
	//for(i=0;i<100000;i++);
		tabTx[1] = SCAN;
	
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(tabTx,2);
	
	GLCD_DrawString(0,50,"SCAN!");
	

	
	while(1){
		Driver_USART0.Receive(tabRx,7);
		while(Driver_USART0.GetRxCount()<1);
		
		if(tabRx[0] == 0xA5 && tabRx[1] == 0x5A && tabRx[2] == 0x05 && tabRx[3] == 0x00 && tabRx[4] == 0x00 && tabRx[5] == 0x40 && tabRx[6] == 0x81)//Si la trame reçue correspond à ce qui est attendu
			{
				GLCD_DrawString(0,0,"Bingo");
			osSignalSet(ID_Lecture, 0x0001);
			osDelay(1);
			osSignalWait(0x0001, osWaitForever);
		}
		else {GLCD_DrawString(0,50,"ATTENTE");}
	
	
	}
}

void LectureLidar(void const * argument){
	osEvent result;
	while(1){
	osSignalWait(0x0001, osWaitForever);
	GLCD_DrawString(0,50,"Lecture");
	}
}