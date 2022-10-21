#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

#define onboardLED 2

#define GPIO_BUTTON_0 5
#define GPIO_BUTTON_1 18
#define GPIO_BUTTON_2 19

double AverageSecond = 0.0;

#define GPIO_OUTPUT_SEL  ((1ULL<<GPIO_BUTTON_0) | (1ULL<<GPIO_BUTTON_1) |  (1ULL<<GPIO_BUTTON_2))
#define GPIO_INPUT_IO_0     CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_txt_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\n";
const static char http_index_hml[] = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <meta
            charset="UTF-8"
            name="viewport"
            content="width = device-width initial-scale = 1.0"
        />
        <title>Communication </title>
    <body style="background-color: #6495ED ;">
        <div class="header"><h1>EP Electric EV2G</h1></div>
        <input
            class="button"
            id="btn0"
            type="button"
            value="Grid/Charge"
            onclick="sendRequestLed()"
        />
        <input
            class="button"
            id="btn1"
            type="button"
            value="Battery %"
            onclick="sendRequestData()"
        />
         <input
            class="button"
            id="btn2"
            type="button"
            value="First HouseGrid "
            onclick="TurnOnGridFirstHouse()"
        />

         <input
            class="button"
            id="btn3"
            type="button"
            value="Second HouseGrid "
            onclick="TurnOnGridSecondHouse()"
        />
        <div class="sensorVal">
            <p>Sensor Value:</p>
            <p id="sen"></p>
        </div>
        <style>
            * {
                margin: 0;
                padding: 1;
            }

            body {
                background-color: #d4dce2;
            }
            .button {
                border: none;
  color: white;
  padding: 20px 30px;
  width: 38%;
  height: 20%;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 26px;
  margin: 30px 20px;
  transition-duration: 0.4s;
  cursor: pointer;
  background-color: #e27217;
}
            .header {
                width: 100%;
                height: 150px;
                color: white;
                background-color: #6495ED;
                padding: 1;
                text-align: center;
            }


            .header h1 {
                color: white;
                vertical-align: center;
                font-size: 52px;
            }

            .btn {
                margin: 0;
                margin-top: 0.5%;
                background-color: #fb9541;
                width: 100%;
                border: none;
                color: white;
                text-align: center;
                text-decoration: none;
                font-size: 16px;
    
            }

            .btn:hover {
                cursor: pointer;
                background-color: #e27217;
            }

            .sensorVal {
                margin: 0;
                margin-top: 0.5%;
                width: 100%;
                height: 700px;
                color: white;
                background-color: #6495ED;
                padding: 0;
                text-align: center;
            }

            .sensorVal p {
                color: white;
                vertical-align: center;
                font-size: 38px;
            }

        </style>
        <script>
            function changeButton(value) { 
                var btn = document.getElementById("btn0"); 
                if(value === "0"){ 
                    btn.value = "Powering Grid"; 
                } else { 
                    btn.value = "Charging Battery"; 
                } 
            } 
              function SecondchangeButton(value) { 
                var btn = document.getElementById("btn2"); 
                if(value === "2"){ 
                    btn.value = "House 1 Off"; 
                } else { 
                    btn.value = "House 1 On"; 
                } 
            } 

                function ThirdchangeButton(value) { 
                            var btn = document.getElementById("btn3"); 
                            if(value === "3"){ 
                                btn.value = "House 2 Off"; 
                            } else { 
                                btn.value = "House 2 On"; 
                            } 
                        } 
                        
            
            function sendRequestLed(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            changeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "0", true); 
               http.send(); 
            } 
            
            function sendRequestData(){ 
                var http = new XMLHttpRequest();
                http.onreadystatechange = (()=>{ 
                    if(http.readyState === 4){ 
                        if(http.status === 200){
                            document.getElementById("sen").innerHTML = http.responseText; 
                        } 
                    } 
                }); 
                http.open("GET", "1", true);
                http.send(); } 
                
                 function TurnOnGridFirstHouse(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            SecondchangeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "2", true); 
               http.send(); 
            } 

            function TurnOnGridSecondHouse(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            ThirdchangeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "3", true); 
               http.send(); 
            } 

        </script>
    </body>
</html>
)=====";

#define EXAMPLE_ESP_WIFI_SSID "MONTOYA"
#define EXAMPLE_ESP_WIFI_PASS "123456789"
#define EXAMPLE_MAX_STA_CONN 3

static xQueueHandle duty_queue = NULL;

static EventGroupHandle_t s_wifi_event_group;

int i = 0;
int k = 1;

void wifi_init_softap()
{
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(NULL, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void onURL(struct netconn *conn, char command)
{

    if (command == '0') //If BTN0 is pressed
    {
        GPIO.out ^= BIT2;
        	
        if((GPIO.out & BIT2) == BIT2) //If the ONBOARD LED is On, send a 1 to the webpage
        {
            gpio_set_level(GPIO_BUTTON_0, 1);
            printf("LED Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "1", 1, NETCONN_NOCOPY);
        }
        else //If the ONBOARD LED is Off, send a 0 to the webpage
        {
            gpio_set_level(GPIO_BUTTON_0, 0);
            printf("LED Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);
        }
        
    }
    else if(command == '3'){
           GPIO.out ^= BIT18;
        	
        if((GPIO.out & BIT18) == BIT18){
            gpio_set_level(GPIO_BUTTON_1, 1);
            printf("LED2 Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "2", 1, NETCONN_NOCOPY); 
            i =0 ;
            }
            else{
            gpio_set_level(GPIO_BUTTON_1, 0);
            printf("LED2 Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);}

        }

    else if(command == '2'){
        GPIO.out ^= BIT19;

        if((GPIO.out & BIT19) == BIT19){
            gpio_set_level(GPIO_BUTTON_2, 1);
            printf("LED3 Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "3", 1, NETCONN_NOCOPY); 
            i =0 ;
            }
            else{
            gpio_set_level(GPIO_BUTTON_2, 0);
            printf("LED3 Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);}

    }

    else
    {   
        netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);
        netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
    }
}

static void http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;
    /* Read the data from the port, blocking if nothing yet there.
 We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);
    if (err == ERR_OK)
    {
        netbuf_data(inbuf, (void **)&buf, &buflen);
        /* Is this an HTTP GET command? (only check the first 5 chars, since
 there are other formats for GET, and we're keeping it very simple )*/
        if (buflen >= 5 &&
            buf[0] == 'G' &&
            buf[1] == 'E' &&
            buf[2] == 'T' &&
            buf[3] == ' ' &&
            buf[4] == '/')
        {
            /* Send the HTML header
 * subtract 1 from the size, since we dont send the \0 in the string
 * NETCONN_NOCOPY: our data is const static, so no need to copy it
 */
            onURL(conn, buf[5]);
        }
    }
    netconn_close(conn);
    netbuf_delete(inbuf);
}

static void http_server(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    do
    {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while (err == ERR_OK);
    netconn_close(conn);
    netconn_delete(conn);
}

void setADC()
{
    adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages
}

void setGPIO()
{
    gpio_pad_select_gpio(onboardLED);
    gpio_set_direction(onboardLED, GPIO_MODE_OUTPUT);

    gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_SEL;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);


}

void ADC(void *pvParameter){
    while(1){
        vTaskDelay(100/portTICK_PERIOD_MS);
        /*Read adc value @ CHANNEL 6*/
        float Firsthouse = adc1_get_raw(ADC1_CHANNEL_4);   
        /*Send to queue*/

			xQueueSendToBack(duty_queue,&Firsthouse,(TickType_t)10);
        

    }
}


void ADCtask(void *pvParameter){
	float Firsthouse;
    int i = 0;
    while(1){
        if(xQueueReceive(duty_queue,&Firsthouse,(TickType_t)10) == pdPASS){ //wait for queueu to recieve 
            printf("First house voltage : %3.f \n",Firsthouse);
            i++;
            if(i == 100){
                    gpio_set_level(GPIO_BUTTON_0, 1);
            vTaskDelay(900/portTICK_PERIOD_MS);
                    gpio_set_level(GPIO_BUTTON_0, 0);
                    i =0;
            }
        }
    }
}


void app_main()
{
    nvs_flash_init();
    setADC();
    setGPIO();
    wifi_init_softap();
  //  setupGPIO();
   
 //  xTaskCreate(&ADC,"CurrentSensor",2048,NULL,5,NULL);
  //  xTaskCreate(&ADCtask,"ADCtask",2048,NULL,5,NULL);
     xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
  
}