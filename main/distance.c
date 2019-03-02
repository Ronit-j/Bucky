#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"
#include <sys/time.h>
#include "driver/gpio.h"

#define RMT_TX_CHANNEL 1 /* RMT channel for transmitter */
#define RMT_TX_GPIO_NUM PIN_TRIGGER /* GPIO number for transmitter signal */
#define RMT_RX_CHANNEL 0 /* RMT channel for receiver */
#define RMT_RX_GPIO_NUM PIN_ECHO /* GPIO number for receiver */
#define RMT_CLK_DIV 100 /* RMT counter clock divider */
#define RMT_TX_CARRIER_EN 0 /* Disable carrier */
#define rmt_item32_tIMEOUT_US 9500 /*!< RMT receiver timeout value(us) */

#define RMT_TICK_10_US (80000000/RMT_CLK_DIV/100000) /* RMT counter value for 10 us.(Source clock is APB clock) */
#define ITEM_DURATION(d) ((d & 0x7fff)*10/RMT_TICK_10_US)

#define PIN_TRIGGER 22
#define PIN_ECHO 23

double distance = 0;
bool driver_installed = false;

/*
*  Initialize the Transmitter channel
*/
void HCSR04_tx_init()
{
	rmt_config_t rmt_tx;
	rmt_tx.channel = RMT_TX_CHANNEL;
	rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = RMT_CLK_DIV;
	rmt_tx.tx_config.loop_en = false;
	rmt_tx.tx_config.carrier_duty_percent = 50;
	rmt_tx.tx_config.carrier_freq_hz = 3000;
	rmt_tx.tx_config.carrier_level = 1;
	rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
	rmt_tx.tx_config.idle_level = 0;
	rmt_tx.tx_config.idle_output_en = true;
	rmt_tx.rmt_mode = 0;
	rmt_config(&rmt_tx);
	rmt_driver_install(rmt_tx.channel, 0, 0);
}

/*
*  Initialize the Receiver channel
*/
void HCSR04_rx_init()
{
	printf("\nINITIATING RX Driver\n");
	rmt_config_t rmt_rx;
	rmt_rx.channel = RMT_RX_CHANNEL;
	rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
	rmt_rx.clk_div = RMT_CLK_DIV;
	rmt_rx.mem_block_num = 1;
	rmt_rx.rmt_mode = RMT_MODE_RX;
	rmt_rx.rx_config.filter_en = true;
	rmt_rx.rx_config.filter_ticks_thresh = 100;
	rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
	rmt_config(&rmt_rx);
	rmt_driver_install(rmt_rx.channel, 2000, 0);
}

// don't know if works or not
void uninstall_driver()
{
	rmt_driver_uninstall(RMT_RX_CHANNEL);
	driver_installed = false;
}

/* Get the current distance as measured by the ultrasonic sensor */
double get_obstacle_distance()
{
	double temp_distance = distance;
	return temp_distance*100;				//as distance is in meters
}


void start_measuring_distance(void *arg)
{
	HCSR04_tx_init();
	HCSR04_rx_init();

	rmt_item32_t item;
	//this is for defining the wave
	item.level0 = 1;
	item.duration0 = RMT_TICK_10_US;
	item.level1 = 0;
	item.duration1 = RMT_TICK_10_US; // for one pulse this doesn't matter

	size_t rx_size = 0;
	RingbufHandle_t rb = NULL;
	rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);
	rmt_rx_start(RMT_RX_CHANNEL, 1);

	int buffer_reset_count = 0;

	while(1)
	{
		//write to TRIGGER pin so that it generates waves and sends it
		rmt_write_items(RMT_TX_CHANNEL, &item, 1, true);
		rmt_wait_tx_done(RMT_TX_CHANNEL, portMAX_DELAY);

		//read from ECHO pin, whose data is received in Ring buffer
		rmt_item32_t* item = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, portMAX_DELAY);
		distance = 340.29 * ITEM_DURATION(item->duration0) / (1000 * 1000 * 2); // distance in meters
		printf("###################################################Distance is %f cm\n", distance * 100); // distance in centimeters

		vRingbufferReturnItem(rb, (void*) item);
		vTaskDelay(500 / portTICK_PERIOD_MS);

		buffer_reset_count++;
		printf("\n\n#############INCREMENTING BUFFER%d",buffer_reset_count);
		printf("\n###########CURRENT FREE SIZE = \t%d\n", xRingbufferGetCurFreeSize(rb));

		// We tried Deleting the buffer but it FAILED
		// if(xRingbufferGetCurFreeSize(rb) < 20)
		// {
		// 	printf("\n$$$$$$$#######$$$$$$$$DELETING RING BUFFER\n");
		// 	rmt_rx_stop(RMT_RX_CHANNEL);
		// 	vRingbufferDelete(rb);
		// 	// uninstall_driver();

		// 	HCSR04_rx_init();
		// 	rmt_rx_start(RMT_RX_CHANNEL, 1);
		// }

		// printf("\n###########CURRENT MAX ITEM SIZE = \t%d\n", xRingbufferGetMaxItemSize(rb));
		if(buffer_reset_count > 100)
		{	
			//This was the code to reset the remote peripheral, but the thread stopped after that

			// periph_module_reset(PERIPH_RMT_MODULE);
			// // rmt_config(&rmt_rx);
			// // rmt_rx_start(rmt_rx.channel, 1);
			// HCSR04_rx_init();
			// rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &rb);
			// rmt_rx_start(RMT_RX_CHANNEL, 1);

			// printf("\n\n\n#######################################################RESETTING RX CHANNEL\n\n\n");
			// buffer_reset_count = 0;
		}
	}

}