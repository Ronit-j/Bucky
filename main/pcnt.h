#pragma once

int16_t get_count_left();
int16_t get_count_right();
void *pcnt_initialize(void *);
void pcnt_configure(void);
void IRAM_ATTR pcnt_intr_handler_right(void *);
void IRAM_ATTR pcnt_intr_handler_left(void *);
void reset_counters();
