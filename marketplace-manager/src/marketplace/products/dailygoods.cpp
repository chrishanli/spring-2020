//  dailygoods.cpp
//  implements functions for DailyGoods

#include <iostream>
#include <string>
#include "marketplace.h"

bool DailyGoods::renderIntoTable(int number, Table& table) {
    char* dgStr = new char[80];
    sprintf(dgStr, "%d\n%s\n%s\n%d\n$%.2f\n%s\n-\n-", number, "DG", this->name.c_str(), this->remaining, this->price, this->productionDate.format("dd/MM/yyyy").c_str());
    table.appendRow(dgStr);
    
    return true;
}

Goods::Category DailyGoods::getCategory() {
    return DAILYGOODS;
}

void DailyGoods::exportGoods(std::ofstream& of) {
    of << "category: DailyGoods" << std::endl;
    of << "name: " << this->name << std::endl;
    of << "quantity: " << this->remaining << std::endl;
    of << "production_date: " << this->productionDate.format("yyyy/MM/dd") << std::endl;
    of << "price: " << this->price << std::endl << std::endl;
}
