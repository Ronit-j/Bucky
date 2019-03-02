#pragma once

void HCSR04_tx_init();
void HCSR04_rx_init();
double get_obstacle_distance();
void uninstall_driver();
void start_measuring_distance(void *);