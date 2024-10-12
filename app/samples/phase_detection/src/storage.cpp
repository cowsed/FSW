#include "flight.h"

#include <zephyr/kernel.h>

void do_storage() { printk("stage %d", (int) Events::Noseover); }