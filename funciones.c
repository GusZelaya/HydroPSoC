

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
#include <stdlib.h>
#include "LiquidCrystal_psoc.h"

//globales locura 
float ph_actual=0;
float Temp_actual=0;
volatile int tiempo=0;
volatile int indiceLoc=0;
#define LOCURA_SIZE 4
char locura[LOCURA_SIZE] = "";
volatile uint8_t indice = 0;
#define MAGIC_ADDR   4*sizeof(struct planta)
#define MAGIC_VALUE  0xBB
#define DATA_START   0

struct planta{
    char nombre;
    float phmaxx;
    float phminn;
}plntlist[4]={{'1',6.50,5.5},{'2',0.0,0.0},{'3',0.0,0.0},{'4',6.0,8.0}};
struct planta plantaActual;

void escribirEstructuraEnEEPROM(uint16_t base, struct planta *p)
{
    uint8_t *bytes = (uint8_t*)p;

    for (uint8_t i = 0; i < sizeof(struct planta); i++)
    {
        EEPROM_WriteByte(bytes[i], base + i);
    }
}

void leerEstructuraDesdeEEPROM(uint16_t base, struct planta *dest)
{
    uint8_t *bytes = (uint8_t*)dest;

    for (uint8_t i = 0; i < sizeof(struct planta); i++)
    {
        bytes[i] = EEPROM_ReadByte(base + i);
    }
}

void ultimaPlanta()
{
    leerEstructuraDesdeEEPROM(3*sizeof(struct planta),&plantaActual);
}

void cargarDatosIniciales()
{
    uint16_t base = DATA_START;

    for (uint8_t i = 0; i < 4; i++)
    {
        escribirEstructuraEnEEPROM(base, &plntlist[i]);
        base += sizeof(struct planta);
    }
}

void EscrituraEeprom(){
   if (EEPROM_ReadByte(MAGIC_ADDR) != MAGIC_VALUE)
    {
        cargarDatosIniciales();
        EEPROM_WriteByte(MAGIC_VALUE, MAGIC_ADDR);
    }
}

uint8_t leer_teclado(void) {
    uint8_t fila, tecla = 0xFF;
    
    const uint8_t mapa[4][4] = {
        { '1','2','3','A' },
        { '4','5','6','B' },
        { '7','8','9','C' },
        { '*','0','#','D' }
    };
    
    for (fila = 0; fila < 4; fila++) {
        filas_Write(0);
        
        switch(fila) {
            case 0: filas_Write(1); 
            CyDelay(50);
            break;
            case 1: filas_Write(2); 
            CyDelay(50);
            break;
            case 2: filas_Write(4); 
            CyDelay(50);
            break;
            case 3: filas_Write(8); 
            CyDelay(50);
            break;

        }
        CyDelayUs(50);
        if (columna_Read() == 1) { tecla = mapa[fila][0]; }
        if (columna_Read() == 2) { tecla = mapa[fila][1]; }
        if (columna_Read() == 4) { tecla = mapa[fila][2]; }
        if (columna_Read() == 8) { tecla = mapa[fila][3]; }
        if (tecla != 0xFF) break;
    }
    filas_Write(0);
    
    return tecla;
}


void EdicionPH(int cual) 
{
    
    LCD_SetCursor(0,1);
    LCD_Print("*.Acep/#.Can");
    int index=0;
    char buffer[4]="";
    while(1)
    {
        char tecla=leer_teclado();
        CyDelay(100);
        if((tecla >= '0' && tecla <= '9') && index<4)
        {
            buffer[index]=tecla;
            LCD_SetCursor(index++,0);
            LCD_WriteChar(tecla);
        }
        if(tecla=='#') break;
        if(tecla=='*')
        {
            int base=0;
            if(cual==1)plantaActual.phmaxx=(atof(buffer))/100;
            else plantaActual.phminn=(atof(buffer))/100;
            plntlist[3]=plantaActual;
            int temp=plantaActual.nombre - '0' - 1;
            plntlist[temp].phmaxx=plantaActual.phmaxx;
            plntlist[temp].phminn=plantaActual.phminn;
            for (uint8_t i = 0; i < 4; i++)
            {
                escribirEstructuraEnEEPROM(base, &plntlist[i]);
                base += sizeof(struct planta);
            }
            break;
        }
    }
}

float leerph()
{
    uint16 Num=0;
    float voltaje=0;
    float ban=0;
    for (int i=0;i<40;i++){
    Num=ADC_SAR_1_GetResult16();
    voltaje=ADC_SAR_1_CountsTo_Volts(Num);
    voltaje=-4.51 * voltaje + 19.53;
    ban+=voltaje;
    }
    ban=ban/40;
    return ban;
}

float ControlTemp()
{
    uint16 bitTemp;
    float tempF;
    float ban=0;
    for (int i=0;i<40;i++){
    bitTemp=ADC_SAR_2_GetResult16();
    tempF=ADC_SAR_2_CountsTo_Volts(bitTemp);
    tempF=37.3*tempF - 108.75;
    ban+=tempF;
    }
    ban=ban/40;
    return ban;
}

void ControlMotor()
{
    //motor
    PWM_Start();
    if(plantaActual.phmaxx>=ph_actual)
    {
        PWM_WriteCompare(2500);
        CyDelay(100);
    }else if(plantaActual.phminn<=ph_actual)
    {
        PWM_WriteCompare(500);
        CyDelay(100);
    }else {PWM_WriteCompare(1500);}
    
    PWM_Stop();
}

CY_ISR(isr) 
{
    tiempo=1;
    Timer_1_ReadStatusRegister();
    ph_actual=leerph();
    ControlMotor();
    Temp_actual=ControlTemp();
/* ISR Code here */ 
}


void ftoa(float num, char *buffer, int decimals) {
    int intPart = (int)num;                   // parte entera
    float frac = num - (float)intPart;        // parte decimal
    int i = 0;

    // manejar números negativos
    if (num < 0) {
        buffer[i++] = '-';
        intPart = -intPart;
        frac = -frac;
    }

    // convertir parte entera a string
    sprintf(&buffer[i], "%d", intPart);
    i = strlen(buffer);

    // si no se quieren decimales -> fin
    if (decimals <= 0) return;

    buffer[i++] = '.';   // agregar punto decimal

    // convertir parte fraccionaria
    for (int j = 0; j < decimals; j++) {
        frac *= 10.0f;
        int digit = (int)frac;
        buffer[i++] = '0' + digit;
        frac -= digit;
    }

    buffer[i] = '\0';  // fin de string
}

typedef enum {
    ESTADO_REPOSO,
    MENU_PRINCIPAL,
    MENU_ESTADO_SENSORES,
    MENU_PERFILES,
    MENU_PERFIL_SELECCIONAR,
    MENU_CAMBIAR_PH,
    MENU_CAMBIAR_MENOR,
    MENU_CAMBIAR_MAYOR,
    MOSTRAR_PH,
    MENU_PERFIL_CARGAR,
    MENU_PERFIL_BORRAR,          // editando pH máximo    
} EstadoMenu;

void menuFSM(char tecla)
{
    static EstadoMenu estado_actual = ESTADO_REPOSO;
    static EstadoMenu estado_anterior = -1;
    static char Phlocura[10];
    static char Templocura[10];
    char buffer[64];
    if (tecla != 0) {

        if (estado_actual == ESTADO_REPOSO && tecla == '5') 
        {
            estado_actual = MENU_PRINCIPAL;
        } else {
            switch (estado_actual)
            {
                case MENU_PRINCIPAL:
                    if (tecla == '1') estado_actual = MENU_ESTADO_SENSORES;
                    else if (tecla == '4') estado_actual = MENU_PERFILES;
                    else if (tecla == 'A') estado_actual = ESTADO_REPOSO;
                    break;

                case MENU_PERFILES:
                    if (tecla == '1') estado_actual = MENU_PERFIL_SELECCIONAR;
                    else if (tecla == '2') estado_actual = MENU_PERFIL_CARGAR;
                    else if (tecla == '3') estado_actual = MENU_PERFIL_BORRAR;
                    else if (tecla == '0') estado_actual = MENU_PRINCIPAL;
                    break;
                    
                case MENU_PERFIL_SELECCIONAR:
                    if (tecla=='0') estado_actual=MENU_PERFILES;
                    switch(tecla){
                        case ('1'):
                            leerEstructuraDesdeEEPROM(0,&plantaActual);
                            escribirEstructuraEnEEPROM(3*sizeof(struct planta),&plantaActual);
                            estado_actual=MOSTRAR_PH;
                            break;
                        case('2'):
                            leerEstructuraDesdeEEPROM(sizeof(struct planta),&plantaActual);
                            escribirEstructuraEnEEPROM(3*sizeof(struct planta),&plantaActual);
                            estado_actual=MOSTRAR_PH;
                            break;
                        case('3'):
                            leerEstructuraDesdeEEPROM(2*sizeof(struct planta),&plantaActual);
                            escribirEstructuraEnEEPROM(3*sizeof(struct planta),&plantaActual);
                            estado_actual=MOSTRAR_PH;
                            break;
                        default:
                            plantaActual=plantaActual;
                            break;
                        }
                    break;
                    
                case(MENU_PERFIL_CARGAR):
                if(tecla=='5')estado_actual=MENU_CAMBIAR_PH;
                break;
                    
                case MENU_CAMBIAR_PH:
                    if (tecla=='0') estado_actual=MENU_PERFIL_SELECCIONAR;
                    else if(tecla=='A') estado_actual=MENU_CAMBIAR_MENOR;
                    else if(tecla=='B') estado_actual=MENU_CAMBIAR_MAYOR;
                    break;
                    
                case MENU_CAMBIAR_MENOR:
                    if(tecla=='*') {
                        estado_actual = MOSTRAR_PH;
                    } else if(tecla=='#') {
                        estado_actual = MENU_PERFIL_SELECCIONAR;
                    }
                    break;

                case MENU_CAMBIAR_MAYOR:
                    if(tecla=='*') {
                        estado_actual = MOSTRAR_PH;
                    } else if(tecla=='#') {
                        estado_actual = MENU_PERFIL_SELECCIONAR;
                    }
                    break;
                case MOSTRAR_PH:
                    if (tecla=='5')estado_actual=MENU_PERFILES;
                    break;
                case MENU_PERFIL_BORRAR:
                    if (tecla=='5'){
                        plantaActual.phmaxx=plantaActual.phminn=0.0;
                        plntlist[3]=plantaActual;
                        int base=0;
                        int temp=plantaActual.nombre - '0' - 1;
                        plntlist[temp].phmaxx=plantaActual.phmaxx;
                        plntlist[temp].phminn=plantaActual.phminn;
                        for (uint8_t i = 0; i < 4; i++)
                        {
                            escribirEstructuraEnEEPROM(base, &plntlist[i]);
                            base += sizeof(struct planta);
                        }
                        LCD_Clear();
                        LCD_SetCursor(1,0);
                        LCD_Print("perfil borrado");
                        LCD_SetCursor(0,1);
                        LCD_Print("0. Volver");
                    }
                    if(tecla=='0')estado_actual=MENU_PERFILES;
                    break;
                default:
                    if (tecla == '0') estado_actual = MENU_PRINCIPAL;
                    break;
            }
        }
    }
    if ((estado_actual != estado_anterior) || (estado_actual==ESTADO_REPOSO && estado_anterior==ESTADO_REPOSO && tiempo==1)) {
        switch (estado_actual) {
            case ESTADO_REPOSO:
                tiempo=0;
                LCD_Clear();
                ftoa(ph_actual,Phlocura,2);
                sprintf(buffer, "Perfil: %c / %s", plantaActual.nombre, Phlocura);
                LCD_Print(buffer);
                LCD_SetCursor(0,1);
                ftoa(Temp_actual,Templocura,2);
                sprintf(buffer,"5.men / %sc*",Templocura);
                LCD_Print(buffer);
                break;

            case MENU_PRINCIPAL:
                LCD_Clear();
                LCD_SetCursor(1,0);
                LCD_Print("1. Est Sensor");
                LCD_SetCursor(0,1);
                LCD_Print("4.Prfl A.Back");
                LCD_SetCursor(1,0);
                break;

            case MENU_ESTADO_SENSORES:
                LCD_Clear();
                ftoa(ph_actual,Phlocura,3);
                sprintf(buffer, "pH actual: %s", Phlocura);
                LCD_Print("0.Back");
                LCD_SetCursor(0,1);
                LCD_Print(buffer);
                LCD_SetCursor(1,0);
                break;

            case MENU_PERFILES:
                LCD_Clear();
                LCD_Print("1.Sel 2.Ld");
                LCD_SetCursor(0,1);
                LCD_Print("3.Del 0.Back");
                LCD_SetCursor(1,0);
                break;

            case MENU_PERFIL_SELECCIONAR:
                LCD_Clear();
                LCD_Print("0. Volver");
                LCD_SetCursor(0,1);
                sprintf(buffer,"%c,%c,%c",plntlist[0].nombre,plntlist[1].nombre,plntlist[2].nombre);
                LCD_Print(buffer);
                break;
            case MENU_CAMBIAR_PH:
                LCD_Clear();
                LCD_SetCursor(0,1);
                LCD_Print("A. + / B. -");
                LCD_SetCursor(1,0);
                LCD_Print("CMB PH /0. Volver");
                break;
                
            case MENU_CAMBIAR_MENOR:
                LCD_Clear();
                EdicionPH(1);
                break;
                
            case MENU_CAMBIAR_MAYOR:
                LCD_Clear();
                EdicionPH(0);
                break;
                
            case MOSTRAR_PH:
                LCD_Clear();
                LCD_SetCursor(1,0);
                ftoa(plantaActual.phmaxx,Phlocura,2);
                sprintf(buffer, "max:%s/ 5.B", Phlocura);
                LCD_Print(buffer);
                LCD_SetCursor(0,1);
                ftoa(plantaActual.phminn,Phlocura,2);
                sprintf(buffer, "min:%s", Phlocura);
                LCD_Print(buffer);
                break;

            case MENU_PERFIL_CARGAR:
                LCD_Clear();
                LCD_SetCursor(1,0);
                LCD_Print("0. Volver");
                LCD_SetCursor(0,1);
                LCD_Print("5. Cambiar PH");
                break;

            case MENU_PERFIL_BORRAR:
                LCD_Clear();
                LCD_SetCursor(1,0);
                LCD_Print("0. Volver");
                LCD_SetCursor(0,1);
                LCD_Print("5. Del prf act");
                break;
            
        }
        estado_anterior = estado_actual;
    }
}




/* [] END OF FILE */