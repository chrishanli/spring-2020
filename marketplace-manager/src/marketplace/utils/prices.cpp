//  prices.cpp

#include <iostream>
#include "utils.h"

float Utils::stdPrice(float price) {
    int lastItem = (int)(price * 1000);
    // if the last one digit is larger than 4, then increase it by one
    if (lastItem - (lastItem / 10) * 10 > 4) {
        lastItem += 10;
    }
    // diminish fractions
    lastItem /= 10;
    return (float)lastItem / 100;
}
