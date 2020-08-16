//  electron.cpp
//  implements functions for Electrical Appliances

#include <iostream>
#include <string>
#include "marketplace.h"

bool ElecticalAppliance::renderIntoTable(int number, Table& table) {
    char* dgStr = new char[80];
    Time t(this->productionDate);
    t += this->warrantyPeriod;
    sprintf(dgStr, "%d\n%s\n%s\n%d\n$%.2f\n%s\n%s\n%s", number, "EA", this->name.c_str(), this->remaining, this->price, this->productionDate.format("dd/MM/yyyy").c_str(), t.format("dd/MM/yyyy").c_str(), this->colour.c_str());
    table.appendRow(dgStr);
    
    return true;
}

// gets the type of a product
Goods::Category ElecticalAppliance::getCategory() {
    return ELECTRICAL;
}

void ElecticalAppliance::exportGoods(std::ofstream& of) {
    of << "category: ElectricalAppliance" << std::endl;
    of << "name: " << this->name << std::endl;
    of << "quantity: " << this->remaining << std::endl;
    of << "production_date: " << this->productionDate.format("yyyy/MM/dd") << std::endl;
    of << "color: " << this->colour << std::endl;
    of << "maintenance_period: " << this->warrantyPeriod << std::endl;
    of << "price: " << this->price << std::endl << std::endl;
}
