#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
 
#define LOW  0  /* LOGIC LOW*/
#define HIGH 1  /* LOGIC HIGH*/
 

void turnOFF( uint8_t *led, int size )
{
    for( int i = 0; i < size; i++ )
        gpio_set_level( led[i], LOW );
}

void setOutputs( uint8_t *out, int size )
{
    /* iterate over the size of the array */
    for ( int i = 0; i < size; i++ ) 
    {
        gpio_pad_select_gpio(out[i]); /* select the GPIO pins */
        gpio_set_direction( out[i], GPIO_MODE_OUTPUT ); /* set direction as outputs */
    }
    return;
}
 
void sweep( uint8_t *led, int size )
{
    turnOFF( led, size );
    /* iterate over the size of the array */
    for ( int i = 0; i < size; i++ )
    {
        gpio_set_level( led[i], HIGH );
        vTaskDelay( 200 / portTICK_PERIOD_MS ); /* .2 second delay */
    }

    turnOFF( led, size );

    /* iterate over the size of the array */
    for ( int i = size - 1; i >= 0; i-- )
    {
      gpio_set_level( led[i], HIGH );
      vTaskDelay( 200 / portTICK_PERIOD_MS ); /* .2 second delay */
    }

    turnOFF( led, size );
}
 
void led_chaser( uint8_t *led, int size )
{
    turnOFF( led, size );
    /* iterate over the size of the array */
    for ( int i = 0; i < size; i++ )
    {
       gpio_set_level( led[i], HIGH );
       vTaskDelay( 200 / portTICK_PERIOD_MS ); /* .2 second delay */

       if( i >= 1 && i < 6 )
          gpio_set_level( led[i-1], LOW );
    }
    turnOFF( led, size );
}
 
void lightShow( uint8_t *led, int size )
{
    int k = 0;
    turnOFF( led, size );
    for( int i = 0; i < size; i++ )
    {
        k = rand() % size;
        gpio_set_level( led[k] , HIGH );
        vTaskDelay( (rand() % 750) / portTICK_PERIOD_MS ); /* ?? second delay */
        gpio_set_level( led[k] , LOW );
    }
    turnOFF( led, size );
}
 
void app_main(void)
{
    // int i = 0;
    /*GPIOs pins*/
    uint8_t led[] = {4, 17, 5, 18, 19, 21}; /* create an array of 6 leds */
    int size = sizeof(led)/sizeof(uint8_t); /* get size of the array, use sizeof() to allow scalabilty */
    setOutputs( led,size ); /* intialize GPIOs pins */
    while (1)
    {
      sweep( led, size );
      sweep( led, size ) ;
      sweep( led, size );

      vTaskDelay( 2000 / portTICK_PERIOD_MS ); // Delay for 2 seconds between modes

      led_chaser( led, size );
      led_chaser( led, size );
      led_chaser( led, size );
      vTaskDelay( 2000 / portTICK_PERIOD_MS ); // Delay for 2 seconds between modes

      lightShow( led, size );
      lightShow( led, size );
      lightShow( led, size );
      vTaskDelay( 2000 / portTICK_PERIOD_MS ); // Delay for 2 seconds between modes
    }
}

// gpio_set_level(led[i], LOW);
// vTaskDelay(1000 / portTICK_PERIOD_MS); /* 1 second delay */