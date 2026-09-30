//main.c
#include "project.h"
#include "LiquidCrystal_psoc.h"

int main(void)
{
    CyGlobalIntEnable;

    LCD_Init();
    LCD_Print("Line 1");
    LCD_SetCursor(0,1);
    LCD_Print("Line 2");

    CyDelay(2000);

    LCD_Clear();
    LCD_Print("Screen cleared");

    for(;;) {
        // your loop here
    }
}