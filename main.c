#include "project.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "LiquidCrystal_psoc.h"
#include "cabeceras.h"

int main(void)
{
    CyGlobalIntEnable; 
    //pantalla
    LCD_Init();
    LCD_Clear();
    LCD_Print("----iniciando---");
    LCD_SetCursor(0,1);
    LCD_Print("----Chistema----");
    CyDelay(2000);
    LCD_Clear();
    LCD_SetCursor(1,0);
    LCD_Print("---Zelaya 2025--");
    LCD_SetCursor(0,1);
    LCD_Print("--TP Micro1---");
    CyDelay(2000);
    
    //ADC
    ADC_SAR_1_Start();
    ADC_SAR_1_StartConvert();
    ADC_SAR_2_Start();
    ADC_SAR_2_StartConvert();
    
   
    //opamps
    Opamp_1_Start();
    Opamp_2_Start();
    
    //timer
    isr_Start();
    Timer_1_Start();
    
    //motor
    PWM_Start();
    PWM_WriteCompare(1500);
    CyDelay(100);
    
    //eeprom
    EEPROM_Start();
    EscrituraEeprom();
    ultimaPlanta();
    
    
    for(;;)
    {
        isr_StartEx(isr);
        char tecla = leer_teclado(); 
        menuFSM(tecla);
    }
}

