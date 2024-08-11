#include <zephyr/kernel.h>

int main() {
    while (true) {
        printk("Hiii\n");
        k_msleep(1000);
    }
}