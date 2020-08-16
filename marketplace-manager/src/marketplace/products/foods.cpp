//  foods.cpp
//  implements functions for Foods

#include <iostream>
#include <string>
#include "marketplace.h"

bool Food::renderIntoTable(int number, Table& table) {
    Time now;
    Time t = this->productionDate;
    t += this->guaranteePeriod;
    // if guarantee day has expired, not add into table and gives a false
    if (t < now) {
        return false;
    }
    
    char* dgStr = new char[80];
    sprintf(dgStr, "%d\n%s\n%s\n%d\n$%.2f\n%s\n%s\n-", number, "FO", this->name.c_str(), this->remaining, this->price, this->productionDate.format("dd/MM/yyyy").c_str(), t.format("dd/MM/yyyy").c_str());
    table.appendRow(dgStr);
    
    return true;
}

// gets the type of a product
Goods::Category Food::getCategory() {
    return FOODS;
}

void Food::exportGoods(std::ofstream& of) {
    of << "category: Food" << std::endl;
    of << "name: " << this->name << std::endl;
    of << "quantity: " << this->remaining << std::endl;
    of << "production_date: " << this->productionDate.format("yyyy/MM/dd") << std::endl;
    of << "guarantee_period: " << this->guaranteePeriod << std::endl;
    of << "price: " << this->price << std::endl << std::endl;
}
