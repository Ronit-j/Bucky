/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

void app_driver_init(void);
int app_driver_toggle_state(void);
bool app_driver_get_state(void);

void turn_left();
void turn_right();
void move_forward();
void move_backward();
void start_motion();
void initialize_motors();
void stop();
void stop_if_obstacle();