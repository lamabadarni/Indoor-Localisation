#include "esp_random.h"
#include "platform_random.h"

int platform_random(int max) {
    return (int)(esp_random() % max);
}
