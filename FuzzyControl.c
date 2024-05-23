/*                              Control difuso tipo Mamdami
 *           _____                     ____________       ___________
 *          /     \      e            |            |     |           |
 *    r --->|+ _  |------------------>| Control    |---->|   Planta  |--------------> y
 *          \____/   |    _______     |            |     |___________|     |
 *            ^      |-->| de/dt |--->| difuso     |                       |
 *            |           -------     |____________|                       |
 *            |                                                            |
 *            |_________________Retroalimentacion__________________________|
 *
 *
 *							           Control Difuso
 *  __________       ____________       ____________       _______________       ________
 * | Entrada  |     | Fuzicador  |     |   Sistema  |     |               |     | Salida |
 * | Nitida   |---->|    de      |---->|     de     |---->| Defuzificador |---->| Nitida |
 * |__________|     | Inferencia |     | Inferencia |     |_______________|     |________|
 *
 * */


/* system headers */
//#include "RedefineTypes.h"
#include "Platform_Types.h"
#include "ConstHeader.h"

/* own headers */
#include "FuzzyControl.h"
#include "FuzzyControl_def.h"
#include "FuzzyControl_cfg.h"
#define division(num, denum)   (num/denum)
/* private__variables */

typedef struct {
	float32 Actual = 0;
	float32 Siguiente = 0;
	uint8 Negativo = 0;
}stValoresMebrecia;

typedef struct {
	float32 SumarioaXFx = 0;
	float32 SumarioaFx = 0;
	float32 EjeX = 0;
}stSumatoria;

static float32 FuzzyControl_s_as32Minimo[FuzzyControl_enReglaTotal] = {0};
static float32 FuzzyControl_s_as32Maximo[FuzzyControl_enReglaTotal] = {0};
static float32 FuzzyControl_s_as32MError[FuzzyControl_enTotalError];
static float32 FuzzyControl_s_as32MDError[FuzzyControl_enTotalDerivadaError];
static float32 FuzzyControl_s_as32Salida[Fuzzycontrol_enTotalVoltaje] = {0};

/* private__functions */


/**/
static float32 FuzzyControl_s_f32TrapDerecho(sint32 s32Input, sint32 s32LadoIzq, sint32 s32LadoDer)
  {
    float32 f32Inputfuzificado = 0.5f;
    sint32 numerador =0;
    sint32 denominador =1;

    if(s32Input <= s32LadoIzq)
    {
      f32Inputfuzificado = Const_Cero;
    }
    else if(s32Input > s32LadoIzq && s32Input < s32LadoDer)
    {
      numerador = s32Input - s32LadoIzq;
      denominador = s32LadoDer - s32LadoIzq;
      
      f32Inputfuzificado = division((float32)numerador,denominador);
    }
    else
    {
      f32Inputfuzificado = Const_arribaAtope;
    }

    return f32Inputfuzificado;
  }
  
  
static float32 FuzzyControl_s_f32TrapIzq(sint32 s32Input, sint32 s32LadoIzq, sint32 s32LadoDer)
  {
      float32 f32Inputfuzificado = 0;
      sint32 numerador =0;
      sint32 denominador =1;
	  
      if(s32Input <= s32LadoIzq)
      {
        f32Inputfuzificado = Const_arribaAtope;
      }
      else if(s32Input > s32LadoIzq && s32Input < s32LadoDer)
      {        
	    numerador = s32Input - s32LadoIzq;
        denominador = s32LadoDer - s32LadoIzq;
      
        f32Inputfuzificado = division((float32)numerador,denominador);
	  
      }
      else
      {
        f32Inputfuzificado = Const_Cero;
      }
      return f32Inputfuzificado;
  }
  
static float32 FuzzyControl_s_f32Triangular(sint32 s32Input, sint32 s32LadoIzq, sint32 s32Centro, sint32 s32LadoDer)
  {
    float32 f32Inputfuzificado = 0;
    sint32 numerador =0;
    sint32 denominador =1;
	
    if(s32Input < s32LadoIzq)
    {
      f32Inputfuzificado = Const_Cero;
    }
    else if(s32Input >= s32LadoIzq && s32Input <= s32Centro)
    {      
	  numerador = s32LadoIzq - s32Input;
      denominador = s32LadoIzq;
      
      f32Inputfuzificado = division((float32)numerador,denominador);
    }
    else if(s32Input > s32Centro && s32Input <= s32LadoDer)
    {      
	  numerador = s32LadoDer - s32Input;
      denominador = s32LadoDer;
      
      f32Inputfuzificado = division((float32)numerador,denominador);
    }
    else
    {
      f32Inputfuzificado = Const_Cero;
    }
    return f32Inputfuzificado;
  }
  
static float32 FuzzyControl_s_f32Min(float32 x, float32 y)
  {
    float32 minimo = 0;
  
    if(x <= y)
    {
      minimo = x;
    }
    else
    {
      minimo = y;
    }
    return minimo;
  }
  
static float32 FuzzyControl_s_f32Max(float32 x, float32 y)
  {
    float32 maximo = 0;
    
    if(x >= y)
    {
      maximo = x;
    }
    else
    {
      maximo = y;
    }
    return maximo;
  }

static float32 FuzzyControl_s_f32SumatoriaTrap(stValoresMebrecia ValorMembrecia,stSumatoria *Sumatoria)
  {
  //pendiente y = mx + b, donde m es la pendiente y b la intereseccion x con y en este caso es cero, x = y/m 
  float32 EjeXinitPendeinte = 0;
  float32 f32SumatoriaEjeY = 0;
  float32 f32SumatoriaEjeX = 0;
  float32 EjeXLimit = 0;
  float32 EjeXLimit1 = 0;
  uint8 u8EjeX = 0;
  
  EjeXinitPendeinte = division(ValorMembrecia.Actual, (float32)FuzzyControl_nVoltajePendiente_cfg);
  EjeXLimit = division(ValorMembrecia.Siguiente, (float32)FuzzyControl_nVoltajePendiente_cfg);
  EjeXLimit1 = 100 - division(ValorMembrecia.Siguiente, (float32)FuzzyControl_nVoltajePendienteCentro_cfg);
  EjeXLimit = (EjeXLimit<= EjeXLimit1)?EjeXLimit1:EjeXLimit;

  if(ValorMembrecia.Actual > Const_Cero)
  {
        
    while(u8EjeX <=  EjeXinitPendeinte && u8EjeX < EjeXLimit)
    {
    	Sumatoria->SumarioaXFx += (FuzzyControl_nVoltajePendiente_cfg * u8EjeX) * (u8EjeX);
    	Sumatoria->SumarioaFx += FuzzyControl_nVoltajePendiente_cfg * u8EjeX;
      u8EjeX++; 
    }
    
    while(u8EjeX <= FuzzyControl_nVoltajeMax_cfg){
    	Sumatoria->SumarioaXFx += ValorMembrecia.Actual *  u8EjeX;
    	Sumatoria->SumarioaFx += ValorMembrecia.Actual;
      u8EjeX++;
      //u8EjeX++;
    }
    if(ValorMembrecia.Negativo == 1){
    	Sumatoria->SumarioaXFx = -Sumatoria->SumarioaXFx;
		Sumatoria->SumarioaFx = -Sumatoria->SumarioaFx;
    }
    else {
    	/* Nothing to do*/
    }

  }
  return f32SumatoriaEjeX;
  }
  
static float32 FuzzyControl_s_f32SumatoriaTrian(float32 ValorMembrecia, float32 Vsiguiente)
  {
  //pendiente y = mx + b, donde m es la pendiente y b la intereseccion x con y en este caso es cero, x = y/m 
  float32 EjeXinitPendeinte = 0;
  float32 f32SumatoriaEjeY = 0;
  float32 f32SumatoriaEjeX = 1;
  float32 EjeXLimit = 0;
  float32 EjeXLimit1 = 0;
  uint8 u8EjeX = 0;
  
  EjeXinitPendeinte = ValorMembrecia / FuzzyControl_nVoltajePendienteCentro_cfg;
  EjeXLimit = division(Vsiguiente, (float32)FuzzyControl_nVoltajePendiente_cfg);
  EjeXLimit1 = division(Vsiguiente, (float32)FuzzyControl_nVoltajePendienteCentro_cfg);
  EjeXLimit = (EjeXLimit<= EjeXLimit1)?EjeXLimit1:EjeXLimit;

  if(ValorMembrecia > Const_Cero)
  {
    while(u8EjeX <= EjeXinitPendeinte && u8EjeX < EjeXLimit){
      f32SumatoriaEjeY += ValorMembrecia *  u8EjeX;
      f32SumatoriaEjeX += ValorMembrecia;
      u8EjeX++;
    }   
    while(u8EjeX <= 20 && u8EjeX < EjeXLimit)
    {
      f32SumatoriaEjeY += (FuzzyControl_nVoltajePendienteCentro_cfg * u8EjeX) * (u8EjeX);
      f32SumatoriaEjeX += FuzzyControl_nVoltajePendienteCentro_cfg * u8EjeX;
	  u8EjeX++;
    }
    
    f32SumatoriaEjeY = f32SumatoriaEjeY / f32SumatoriaEjeX;
  }
  
  return f32SumatoriaEjeY;
  }
  
static float32 FuzzyControl_s_f32SumatoriaCentrotrian(float32 ValorMembrecia, float32 Vsiguiente)
  {
  //pendiente y = mx + b, donde m es la pendiente y b la intereseccion x con y en este caso es cero, x = y/m 
    float32 suma = 0;
  suma = FuzzyControl_s_f32SumatoriaTrian(ValorMembrecia,0) - FuzzyControl_s_f32SumatoriaTrian(ValorMembrecia,0);
  return suma;
  }
  
static float32 FuzzyControl_s_u32DesfuzificacionCentroide(float32 ValorNegativo, float32 ValorCero, float32 ValorPositivo)
  {
  /*negativo = 0.5, cero = 0.6, positivo= 0.7, semultiplica todo por 100 para efectos de calculo*/ 
	stValoresMebrecia ValorMembrecia = {0};
	stSumatoria Sumatoria = {0};
	float32 f32SumatoriaTotalXFy = 0;
	float32 u32Salida = 0;

	ValorMembrecia.Actual = ValorNegativo;
	ValorMembrecia.Siguiente = ValorCero;
	ValorMembrecia.Actual =  1;

   f32SumatoriaTotalXFy = -FuzzyControl_s_f32SumatoriaTrap(ValorMembrecia,Sumatoria);


   f32SumatoriaTotalXFy += FuzzyControl_s_f32SumatoriaCentrotrian(ValorCero,Sumatoria);

   ValorMembrecia.Actual = ValorPositivo;
   	ValorMembrecia.Siguiente = ValorCero;
   	ValorMembrecia.Actual =  0;
  f32SumatoriaTotalXFy +=  FuzzyControl_s_f32SumatoriaTrap(ValorMembrecia,Sumatoria);
  
  u32Salida = f32SumatoriaTotalXFy;//f32SumatoriaTotalXFy / f32SumatoriaTotalX;
    
    return u32Salida;
  }

/* public_functions */

float32 FuzzyControl_u32Mamdani(sint32 s32Error, sint32 s32DError)
{
  sint32 s32i;
  sint32 s32j;
  float32 u32U = 0;

  sint32 s32LadoIzq = 0;
  sint32 s32LadoDer = 100;
  sint32 s32Centro = 0;
  /*Fuzifiacion*/
  //error
  //u32U = FuzzyControl_s_f32TrapDerecho(s32Error, s32LadoIzq, s32LadoDer);
  FuzzyControl_s_as32MError[FuzzyControl_enErrorP] = FuzzyControl_s_f32TrapDerecho(s32Error, s32LadoIzq, s32LadoDer);
  s32LadoIzq = -15;
    s32LadoDer = 15;
  FuzzyControl_s_as32MError[FuzzyControl_enErrorZ] = FuzzyControl_s_f32Triangular(s32Error, s32LadoIzq, s32Centro, s32LadoDer);
  s32LadoIzq = -100;
    s32LadoDer = 0;
  FuzzyControl_s_as32MError[FuzzyControl_enErrorN] = FuzzyControl_s_f32TrapIzq(s32Error, s32LadoIzq, s32LadoDer);
   
   //cambio
    s32LadoIzq = 0;
    s32LadoDer = 5;
    FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorP] = FuzzyControl_s_f32TrapDerecho(s32DError, s32LadoIzq, s32LadoDer);
  s32LadoIzq = -1;
    s32LadoDer = 1;
  FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorZ] = FuzzyControl_s_f32Triangular(s32DError, s32LadoIzq, s32Centro, s32LadoDer);
  s32LadoIzq = -5;
    s32LadoDer = 0;
  FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorN] = FuzzyControl_s_f32TrapIzq(s32DError, s32LadoIzq, s32LadoDer);
   
    //u32U = FuzzyControl_s_as32MError[FuzzyControl_enErrorN];
   /*Controlador "sistema de inferencia"*/
   
   //minimos
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla1] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorN],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorN]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla2] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorN],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorZ]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla3] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorN],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorP]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla4] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorZ],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorN]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla5] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorZ],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorZ]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla6] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorZ],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorP]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla7] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorP],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorN]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla8] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorP],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorZ]);
  FuzzyControl_s_as32Minimo[FuzzyControl_enRegla9] = FuzzyControl_s_f32Min(FuzzyControl_s_as32MError[FuzzyControl_enErrorP],FuzzyControl_s_as32MDError[FuzzyControl_enDerivadaErrorP]);
  
  //Maximos
  //Parte negativa de salida
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla1] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Minimo[FuzzyControl_enRegla1], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla2]);
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla2] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Maximo[FuzzyControl_enRegla1], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla3]);
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla3] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Maximo[FuzzyControl_enRegla2], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla4]);
  FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeN] = FuzzyControl_s_as32Maximo[FuzzyControl_enRegla3];
  //s32U = FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeN];
  
  //Parte negativa de neutra
  FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeZ] = FuzzyControl_s_as32Minimo[FuzzyControl_enRegla5];
  //s32U = FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeZ];
  
  //Parte negativa de positiva
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla1] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Minimo[FuzzyControl_enRegla6], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla7]);
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla2] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Maximo[FuzzyControl_enRegla1], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla8]);
  FuzzyControl_s_as32Maximo[FuzzyControl_enRegla3] = FuzzyControl_s_f32Max(FuzzyControl_s_as32Maximo[FuzzyControl_enRegla2], FuzzyControl_s_as32Minimo[FuzzyControl_enRegla9]);
  FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeP] = FuzzyControl_s_as32Maximo[FuzzyControl_enRegla3];
  //s32U = FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeP];
  
  //desfusificacion 
  u32U =FuzzyControl_s_u32DesfuzificacionCentroide(FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeN], FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeZ],FuzzyControl_s_as32Salida[FuzzyControl_enVoltajeP]);

  return u32U;
}

*/
