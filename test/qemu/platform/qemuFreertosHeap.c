#include <stdint.h>

#include "FreeRTOS.h"

/* Keep heap_4 outside the main SRAM window to reduce pressure from large localbase test data. */
__attribute__((section(".freertosHeap"), aligned(portBYTE_ALIGNMENT))) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
