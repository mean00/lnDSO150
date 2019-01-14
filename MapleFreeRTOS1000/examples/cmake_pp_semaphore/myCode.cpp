/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/
#include "MapleFreeRTOS1000_pp.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include <Wire.h>


xBinarySemaphore *sem;
void myLoop(void) ;

void Task0( void * )
{
    
    while(1)
    {
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        sem->give();
        xDelay( 10 );
        sem->take();
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        xDelay( 600 );
    }
}
void Task1( void * )
{
    
    while(1)
    {
        sem->take();
        xDelay( 1500 );
        sem->give();
        xDelay( 10 );
    }
}
 
void mySetup() 
{
    Wire.begin();
    pinMode(LED_BUILTIN,OUTPUT);
    //
    sem=new xBinarySemaphore();
     xTaskCreate( Task0, "Task0", 200, NULL, 1, NULL );
     xTaskCreate( Task1, "Task1", 200, NULL, 1, NULL );
     vTaskStartScheduler(); 
     
}
/**
 */



void myLoop(void) 
{
   
    
  
    
}
//-
