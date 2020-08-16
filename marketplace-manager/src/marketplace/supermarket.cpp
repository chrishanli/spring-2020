//  supermarket.cpp
//  defines supermarket methods including importing & exporting data

#include <iostream>
#include <fstream>
#include <string>
#include <kij.h>
#include "marketplace.h"

// reads supermarket's name from a file stream, returns the name if it found one
std::string importSupermarketName(std::ifstream& inFile) {
    std::string marketName = "default";
    while (!inFile.eof()) {
        inFile >> marketName;
        if (0 == marketName.compare("Name:")) {
            if (!inFile.eof()) inFile >> marketName;
            break;
        } 
    }
    return marketName;
}

// reads supermarket's initial cash from a file stream, returns its amount if it found one
float importInitialCash(std::ifstream& inFile) {
    std::string marketCash = "default";
    while (!inFile.eof()) {
        inFile >> marketCash;
        if (0 == marketCash.compare("Cash:")) {
            if (!inFile.eof()) inFile >> marketCash;
            break;
        }
    }
    return (float)atof(marketCash.c_str());
}

// reads info of one goods from a file stream, returns a Goods object if found one
Goods* importGoods(std::ifstream& inFile, int goodsNumber = 0) {
	// a temp string to store info read from every line
    std::string tempStr;
    // try to read out a record from the stream's current seek before reaching eof
    while (!inFile.eof()) {
        inFile >> tempStr;
        // if a 'category:' token met, which means the record's start
        if (0 == tempStr.compare("category:")) {
            // try to read out its category
            Goods::Category category;
            if (!inFile.eof()) inFile >> tempStr;
            else break;
            if (0 == tempStr.compare("Food")) {
                category = Goods::FOODS;
            } else if (0 == tempStr.compare("DailyGoods")) {
                category = Goods::DAILYGOODS;
            } else if (0 == tempStr.compare("ElectricalAppliance")) {
                category = Goods::ELECTRICAL;
            } else break;   // no such category, break
            
            // reads out other info
            char name[80], quantity[10], productionDate[20], price[10], period[10], colour[10];
            while (!inFile.eof()) {
                inFile >> tempStr;
                if (0 == tempStr.compare("name:")) {
                    // reads out product name
                    if (!inFile.eof()) inFile >> name;
                    else break;
                } else if (0 == tempStr.compare("quantity:")) {
                    // reads out quantity
                    if (!inFile.eof()) inFile >> quantity;
                    else break;
                } else if (0 == tempStr.compare("production_date:")) {
                    // reads out production_date
                    if (!inFile.eof()) inFile >> productionDate;
                    else break;
                } else if (0 == tempStr.compare("guarantee_period:")) {
                    // reads out production_date
                    if (!inFile.eof()) inFile >> period;
                    else break;
                } else if (0 == tempStr.compare("maintenance_period:")) {
                    // reads out maintenance_period
                    if (!inFile.eof()) inFile >> period;
                    else break;
                } else if (0 == tempStr.compare("color:")) {
                    // reads out colour
                    if (!inFile.eof()) inFile >> colour;
                    else break;
                } else if (0 == tempStr.compare("price:")){
                    // reads out price, and then break
                    if (!inFile.eof()) inFile >> price;
                    break;
                }
            }
            // returns a new object containing info received
            Time time(productionDate);
            float price_float = (float)atof(price);
            int quantity_int = atoi(quantity);
            
            if (category == Goods::DAILYGOODS) {
                DailyGoods* dg = new DailyGoods(name, time, price_float, quantity_int);
                return dg;
            } else if (category == Goods::FOODS) {
                int guarantee_period = atoi(period);
                Food* fd = new Food(name, time, price_float, quantity_int, guarantee_period);
                return fd;
            } else if (category == Goods::ELECTRICAL) {
                int warranty_period = atoi(period);
                ElecticalAppliance* ea = new ElecticalAppliance(name, time, price_float, quantity_int, warranty_period, colour);
                return ea;
            } else {
                break;
            }
        } // if "category"
    } // while not eof
    return NULL;
}


Supermarket::Supermarket(float initCash, std::string name) {
    this->cash = initCash;
    this->name = name;
}

// the table's static properties
static const char* colNames[] = {
	"No.", "Cat.", "Product Name", "Rem.", "Price", "Production", "B.B./Warr.", "Colour"
};
static int widths[] = {
	1, 1, 3, 1, 3, 3, 3, 2
};
static const char* buttons[] = {
	"Close"
};

void Supermarket::displayAllGoods() {
	std::string tableTitle = "In-Stock Info - Market Cash: $";
	tableTitle.append(Utils::itoa(this->cash, 2));

	// create a table to contain goods info
    Table table(tableTitle.c_str(), colNames, 8, widths, buttons, 1);
    std::vector<Goods*> expired;
    // pour all goods into table
    for (int i = 1; i <= this->goods.size(); i++) {
		// if an expired food was found, record it
        if (!this->goods[i-1]->renderIntoTable(i, table)) {
            expired.push_back(this->goods[i-1]);
        } 
    }
    // if there exists expired food, display an error
    if (expired.size() > 0) {
        std::string expDesc = "The following foods' guarantee period has expired:\n";
        char strBuff[80];
        for (int i = 1; i <= expired.size(); i++) {
            Food* expProduct = (Food *)expired[i-1];
            Time expiredDate = expProduct->productionDate;
            expiredDate += (expProduct->guaranteePeriod);
            sprintf(strBuff, "%d) %s * %d, expired on %s\n", i, expProduct->name.c_str(), expProduct->remaining, expiredDate.format("dd/MM/yyyy").c_str());
            expDesc.append(strBuff);
        }
        
        Kij_ErrorWin* win = new Kij_ErrorWin;
        win->mainTitle = XMU_MARKET_TITLE;
        win->winTitle = " Expired Foods Found! ";
        win->errInfo = expDesc.c_str();
        win->buttonsCount = 1;
        win->buttons = buttons;
        
        Kij_NewErrorWin(win);
        delete win;
    }
    
    // print table
    table.show();
}

Supermarket& Supermarket::operator << (Goods* newGoods) {
    this->goods.push_back(newGoods);
    return *this;
}

// import
void operator >> (std::ifstream& infile, Supermarket& sm) {
    // clear supermarket's original info (if any)
    sm.goods.clear();
    
    // set supermarket's name and initial cash
    std::string name = importSupermarketName(infile);
    float cash = importInitialCash(infile);
    sm.name = name;
    sm.cash = cash;
    
    // pour product recort into Supermarket;
    Goods* goods = importGoods(infile);
	for (int i = 2; goods; i++) {
        sm << goods;
        goods = importGoods(infile, i);
    }
}

// export
void operator << (std::ofstream& ofile, Supermarket& market) {
    // output name and cash
    ofile << "Name: " << market.name << std::endl;
    ofile << "Cash: $" << Utils::itoa(market.cash, 2) << std::endl << std::endl;
    
    std::vector<Goods*>::iterator i = market.goods.begin();
    while (i != market.goods.end()) {
        (*(i++))->exportGoods(ofile);
    }
}

Supermarket::~Supermarket() {
    std::vector<Goods *>::iterator i;
    for (i = this->goods.begin(); i != this->goods.end(); i++) {
        // releases resources
        switch ((*i)->getCategory()) {
            case Goods::DAILYGOODS:
                delete (DailyGoods *)(*i);
                break;
            case Goods::FOODS:
                delete (Food *)(*i);
                break;
            default:
                delete (ElecticalAppliance *)(*i);
                break;
        }
    }
}


std::string Supermarket::getName() {
    return this->name;
}

float Supermarket::getCash() {
    return this->cash;
}
