/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <iot_button.h>
#include "driver/ledc.h"

#include "app_priv.h"

/* This is the button that is used for toggling the output */
#define BUTTON_GPIO          0
#define BUTTON_ACTIVE_LEVEL  0
/* This is the GPIO on which the output will be set */
#define OUTPUT_GPIO    27
#define OUTPUT_GPIO2    14
#define ENABLE_GPIO    12
#define LEDC_IO_0    (12)

//button can be pressed after five seconds have passed from the previous button press
#define DEBOUNCE_TIME  500

static bool g_output_state;
static void push_btn_cb(void* arg)
{
    static uint64_t previous;
    uint64_t current = xTaskGetTickCount();
    if ((current - previous) > DEBOUNCE_TIME) {
        printf("BHAI BHAI");

        previous = current;
        app_driver_toggle_state();

        vTaskDelay(5);

        
        //change speed of motor
        for(int i = 0; i < 5; i++)
        {
            printf("INCREASING SPEED NOW");    
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 6000+i*500);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

static void configure_push_button(int gpio_num, void (*btn_cb)(void *))
{
    button_handle_t btn_handle = iot_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        printf("CONFIGURING?");
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
    }
}

static void set_output_state(bool target)
{
    gpio_set_level(OUTPUT_GPIO, target);
    gpio_set_level(OUTPUT_GPIO2, !target);
}

void app_driver_init()
{
    configure_push_button(BUTTON_GPIO, push_btn_cb);

    printf("APP DRIVER INIT");

    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << OUTPUT_GPIO);

    gpio_config_t io_conf2 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf2.pin_bit_mask = ((uint64_t)1 << OUTPUT_GPIO2);


    gpio_config_t io_conf3 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf3.pin_bit_mask = ((uint64_t)1 << ENABLE_GPIO); 


    /* Configure the GPIO */
    gpio_config(&io_conf);
    gpio_config(&io_conf2);
    gpio_config(&io_conf3);

    // gpio_set_level(ENABLE_GPIO, 1);
    printf("ENABLE_GPIO set");


    /*
    Configuring Timer and Channel for PWM signal
    */
    ledc_timer_config_t ledc_timer = {
        .bit_num = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 100,
        .gpio_num = LEDC_IO_0,
        .intr_type = LEDC_INTR_DISABLE,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);
}

int IRAM_ATTR app_driver_toggle_state(void)
{
    printf("\nTOGGLE KIYA");
    g_output_state = ! g_output_state;
    set_output_state(g_output_state);
    return ESP_OK;
}

bool app_driver_get_state(void)
{

    return g_output_state;
}
