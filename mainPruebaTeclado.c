/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_loc_Start();
    char uwu[200];

    for(;;)
    {
        int8 escribe, lee=0;
        for (int f = 1; f <= 8; f <<= 1) 
        {
            filas_Write(f);
            if ((lee = botonPulsado_Read())){
                sprintf(uwu,"filas: %d // columnas: %d \n\r",f,lee);
                

        }
        escribe=botonPulsado_1_Read();
        //sprintf(uwu,"filas: %d // columnas: %d \n\r",f,lee);
        UART_loc_PutString(uwu);
        /* Place your application code here. */
    }
    }
}
/* [] END OF FILE */
