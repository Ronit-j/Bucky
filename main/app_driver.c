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

//won't work. Trying through command line 

#define BUTTON_GPIO          0
#define BUTTON_ACTIVE_LEVEL  0

/* This are the GPIOs on which the output will be set */

#define INPUT1_MOTOR1    15
#define INPUT2_MOTOR1    13
#define INPUT1_MOTOR2    12
#define INPUT2_MOTOR2    14

//BOOT button can be pressed after five seconds have passed from the previous button press
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
    }
}

// static void configure_push_button(int gpio_num, void (*btn_cb)(void *))
// {
//     button_handle_t btn_handle = iot_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL);
//     if (btn_handle) {
//      printf("CONFIGURING?");
//         iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
//     }
// }

static void set_output_state(bool target)
{
    gpio_set_level(INPUT1_MOTOR1, target);
}

static void set_output_state_motor1(bool level1, bool level2)
{
    gpio_set_level(INPUT1_MOTOR1, level1);
    gpio_set_level(INPUT2_MOTOR1, level2);
}

static void set_output_state_motor2(bool level1, bool level2)
{
    gpio_set_level(INPUT1_MOTOR2, level1);
    gpio_set_level(INPUT2_MOTOR2, level2);
}

void app_driver_init()
{
    // configure_push_button(BUTTON_GPIO, push_btn_cb);

   /* Configure output */

    //FOR MOTOR 1
    gpio_config_t io_conf_input1_motor1 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_input1_motor1.pin_bit_mask = ((uint64_t)1 << INPUT1_MOTOR1);

    gpio_config_t io_conf_input2_motor1 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_input2_motor1.pin_bit_mask = ((uint64_t)1 << INPUT2_MOTOR1);

    //FOR MOTOR 2
    gpio_config_t io_conf_input1_motor2 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_input1_motor2.pin_bit_mask = ((uint64_t)1 << INPUT1_MOTOR2);

    gpio_config_t io_conf_input2_motor2 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_input2_motor2.pin_bit_mask = ((uint64_t)1 << INPUT2_MOTOR2);

    /* Configure the GPIO */
    gpio_config(&io_conf_input1_motor1);
    gpio_config(&io_conf_input2_motor1);

    gpio_config(&io_conf_input1_motor2);
    gpio_config(&io_conf_input2_motor2);

    initialize_motors();
    // start_motion();
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

void start_motion()
{
  move_forward(NULL);
  vTaskDelay(5000/ portTICK_PERIOD_MS);
  turn_right(NULL);
  vTaskDelay(5000/ portTICK_PERIOD_MS);
  move_forward(NULL);
  vTaskDelay(5000/ portTICK_PERIOD_MS);
  turn_left(NULL);
  vTaskDelay(5000/ portTICK_PERIOD_MS);
  move_backward(NULL);
}

void* turn_left(void *arg)
{
    printf("\n\n\nMOVING LEFT\n");
    // stop();
    set_output_state_motor1(0,1);
    set_output_state_motor2(1,0);
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    stop();
    printf("\nDONE MOVING LEFT");
    return NULL;
}

void* turn_right(void *arg)
{
    printf("\n\nMOVING RIGHT");
    // stop();
    set_output_state_motor1(1,0);
    set_output_state_motor2(0,1);
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    stop();
    printf("\n\nDONE MOVING RIGHT");
    return NULL;
}

void* move_forward(void *arg)
{
    printf("\n\nMOVING FORWARD");
    // stop();
    set_output_state_motor1(1,0);
    set_output_state_motor2(1,0);

    vTaskDelay(10000/ portTICK_PERIOD_MS);
    stop();
    printf("\nDONE MOVING FORWARD");
    return NULL;
}

void* move_backward(void *arg)
{
    printf("\n\nMOVING BACK");
    // stop();
    set_output_state_motor1(0,1);
    set_output_state_motor2(0,1);

    vTaskDelay(10000/ portTICK_PERIOD_MS);
    stop();
    printf("\nDONE MOVING BACK");
    return NULL;
}

void stop()
{
    printf("\nSTOPPING");
    set_output_state_motor1(0,0);
    set_output_state_motor2(0,0);
}

void initialize_motors()
{
    set_output_state_motor1(0,0);
    set_output_state_motor2(0,0);
    stop();
}