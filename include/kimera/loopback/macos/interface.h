#ifndef LOOPBACK_INTERFACE_MAC_H
#define LOOPBACK_INTERFACE_MAC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void* init_capture();
void stop_capture(void*);

bool start_capture(void*, bool, char*, int, int, int);
bool pull_frame(void*, char*[], int[]);

int get_frame_width(void*);
int get_frame_height(void*);

#endif