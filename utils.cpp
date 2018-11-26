#include <Arduino.h>
#include "utils.h"

void print_uint64_t (uint64_t num) {
    char rev[22];
    char *p = rev+1;
    
    if (num == 0) {
        Serial.print("0");
    }
    else {
        while (num > 0) {
            *p++ = '0' + (num % 10);
            num /= 10;
        }

        /* Print the number which is now in reverse */
        while (--p > rev) {
            Serial.print(*p);
        }
    }
}