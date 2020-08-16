//  strings.cpp

#include <iostream>
#include "utils.h"

std::string Utils::itoa(float number, int fraction) {
	// buffer must be big enough to contain a very, very big number
	char numberBuffer[20];
	if (fraction == 0) {
		sprintf(numberBuffer, "%d", (int)number);
	}
	else {
		char placeholderBuffer[10];
		sprintf(placeholderBuffer, "%%.%dlf", fraction);
		sprintf(numberBuffer, placeholderBuffer, (double)number);
	}
	return numberBuffer;
}