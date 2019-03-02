    
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
#include <pthread.h>

#include "app_priv.h"
#include "pcnt.h"
#include "distance.h"

/* This is the button that is used for toggling the output */
// both unused
#define BUTTON_GPIO          0
#define BUTTON_ACTIVE_LEVEL  0

/* This are the GPIOs for Motor Inputs */
#define INPUT1_MOTOR1    27
#define INPUT2_MOTOR1    14
#define INPUT1_MOTOR2    16
#define INPUT2_MOTOR2    17

/* Minimum distance that an object can be before the robot stops movement */
#define OBSTACLE_DISTANCE_LIMIT 30.00

/* GPIOs for Enable and PWM signal (Not used currently) */
#define ENABLE_MOTOR1    26
#define ENABLE_MOTOR2    5
// #define LEDC_MOTOR1    (26)
// #define LEDC_MOTOR2    (5)

//button can be pressed after five seconds have passed from the previous button press
//unused
#define DEBOUNCE_TIME  500

static bool g_output_state;
static void push_btn_cb(void* arg)
{
    static uint64_t previous;
    uint64_t current = xTaskGetTickCount();
    if ((current - previous) > DEBOUNCE_TIME) {
        printf("\nButton pressed");

        previous = current;
        initialize_motors();    //set both motors to 0,0 (Stop)

        vTaskDelay(5);

        start_motion();
        stop();
        // //change speed of motor
        // for(int i = 0; i < 5; i++)
        // {
        //     printf("\nINCREASING SPEED NOW");
        //     ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 6000+i*500);
        //     ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        //     vTaskDelay(2000 / portTICK_PERIOD_MS);
        // }
    }
}

static void configure_push_button(int gpio_num, void (*btn_cb)(void *))
{
    button_handle_t btn_handle = iot_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        printf("\nCONFIGURING?");
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
    }
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
    configure_push_button(BUTTON_GPIO, push_btn_cb);

    printf("\nAPP DRIVER INIT");

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

    gpio_config_t io_conf_enable_motor1 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_enable_motor1.pin_bit_mask = ((uint64_t)1 << ENABLE_MOTOR1);

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

    gpio_config_t io_conf_enable_motor2 = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf_enable_motor2.pin_bit_mask = ((uint64_t)1 << ENABLE_MOTOR2);

    /* Configure the GPIO */
    gpio_config(&io_conf_input1_motor1);
    gpio_config(&io_conf_input2_motor1);
    gpio_config(&io_conf_enable_motor1);

    gpio_config(&io_conf_input1_motor2);
    gpio_config(&io_conf_input2_motor2);
    gpio_config(&io_conf_enable_motor2);

    // gpio_set_level(ENABLE_MOTOR1, 1);
    printf("\nENABLE_MOTOR1 set");
    /*
    Configuring Timer and Channel for PWM signal
    */
    #if 0
    ledc_timer_config_t ledc_timer = {
        .bit_num = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_1 = {
        .channel = LEDC_CHANNEL_0,
        .duty = 100,
        .gpio_num = LEDC_MOTOR1,
        .intr_type = LEDC_INTR_DISABLE,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel_1);

    ledc_channel_config_t ledc_channel_2 = {
        .channel = LEDC_CHANNEL_0,
        .duty = 100,
        .gpio_num = LEDC_MOTOR2,
        .intr_type = LEDC_INTR_DISABLE,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel_2);
    #endif

    gpio_set_level(ENABLE_MOTOR1, 1);
    gpio_set_level(ENABLE_MOTOR2, 1);

    initialize_motors();
    // start_motion();
}

int IRAM_ATTR app_driver_toggle_state(void)
{
    printf("\n\nTOGGLE KIYA");
    g_output_state = ! g_output_state;
    set_output_state_motor1(g_output_state,!g_output_state);
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}

void turn_left()
{
    printf("\n\n\nMOVING LEFT\n");
    // stop();
    set_output_state_motor1(0,1);
    set_output_state_motor2(1,0);

    while(1)
    {
        if(get_count_left() == 20)
        {
            printf("Reached 20 LEFT\n\n");
            stop();
            break;
        }
    }

    printf("\nDONE MOVING LEFT");
}

void turn_right()
{
    printf("\n\nMOVING RIGHT");
    
    set_output_state_motor1(1,0);
    set_output_state_motor2(0,1);

    while(1)
    {
        if(get_count_right() == 20)
        {
            printf("Reached 20 RIGHT\n\n");
            stop();
            break;
        }
    }

    printf("\n\nDONE MOVING RIGHT");
}

void move_forward(int distance_in_cm)
{
    //20ticks = 20CM for the robot wheels i.e. 1 Pulse = 1 CM
    bool obstacle = false;
    printf("\n\nMOVING FORWARD");
    
    set_output_state_motor1(1,0);
    set_output_state_motor2(1,0);
    
    while(1)
    {
    	//for moving upto the specified distance
        if(get_count_right() >= distance_in_cm && get_count_left() >= distance_in_cm)
        {
            stop();
            break;
        }

        //for stopping if obstacle is closer than the limit
        if(get_obstacle_distance() <= OBSTACLE_DISTANCE_LIMIT && get_obstacle_distance() != 0)
        {
            stop_if_obstacle();
            printf("Obstacle in Front \n");
            obstacle = true;
        }

        //if obstacle goes away, then resume the movement
        if(obstacle == true && ( get_obstacle_distance() > OBSTACLE_DISTANCE_LIMIT || get_obstacle_distance() == 0) )
        {
            set_output_state_motor1(1,0);
            set_output_state_motor2(1,0);
            obstacle = false;
        }
    }

    printf("\nDONE MOVING FORWARD");
}

void move_backward(int distance_in_cm)
{
    printf("\n\nMOVING BACK");
    
    set_output_state_motor1(0,1);
    set_output_state_motor2(0,1);

    while(1)
    {
        if(get_count_right() >= distance_in_cm && get_count_left() >= distance_in_cm)
        {
            stop();
            break;
        }
    }

    printf("\nDONE MOVING BACK");
}

void stop()
{
    printf("\nSTOPPING");
    set_output_state_motor1(0,0);
    set_output_state_motor2(0,0);
    vTaskDelay(500/ portTICK_PERIOD_MS);
    reset_counters();
}


/* Initializes the motors */
void initialize_motors()
{
    set_output_state_motor1(0,0);
    set_output_state_motor2(0,0);
    stop();
}

void stop_if_obstacle()
{
    printf("\nSTOPPING");
    set_output_state_motor1(0,0);
    set_output_state_motor2(0,0);
    
}