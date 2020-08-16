//  marketplace.h
//  marketplace main classes (internal)

#ifndef marketplace_h
#define marketplace_h

#include <iostream>
#include <fstream>
#include <vector>
#include "utils/utils.h"
#include "display/table.h"

#ifndef XMU_MARKET_LOGO
#define XMU_MARKET_LOGO "   _  __ __  _____  ____  ___           __        __ \n  | |/ //  |/  / / / /  |/  /___ ______/ /_____  / /_\n  |   // /|_/ / / / / /|_/ / __ `/ ___/ //_/ _ \\/ __/\n /   |/ /  / / /_/ / /  / / /_/ / /  / ,< /  __/ /_  \n/_/|_/_/  /_/\\____/_/  /_/\\__,_/_/  /_/|_|\\___/\\__/"
#define XMU_MARKET_TITLE "-- XMU Market --"
#endif

class Order;

// class Goods
class Goods {
	// enables an OrderItem to access the name of a goods while rendering
	friend class OrderItem;
	// enables an Order to access the name of a goods while outputing an Order object
	friend class BuyOrder;
	friend class ReturnOrder;
	// enables calculating subtotals via times sign, giving it the right to fetch the Goods' price
    friend float operator * (int amount, Goods& goods);
    friend float operator * (Goods& goods, int amount);
	// enables the UI to calculate and display the grand subtotals
    friend void interface_add_product(Order& o);
protected:
    std::string name;       // name of a product
    Time productionDate;    // date of manufacture
    float price;            // product price
    int remaining;          // remaining amount of this product
	int productNumber;		// product number
public:
    enum Category {
        DAILYGOODS, FOODS, ELECTRICAL
    };
    
    Goods(std::string name, Time& productionDate, float price, int remaining, int goodsNo):
    productionDate(productionDate) {
        this->name = name;
        this->price = Utils::stdPrice(price);
        this->remaining = remaining;
		this->productNumber = goodsNo;
    }
    
    // add info of a product into a table
    virtual bool renderIntoTable(int number, Table& table) = 0;
    // gets the type of a product
    virtual Category getCategory() = 0;
    // exports a product into a certain file
    virtual void exportGoods(std::ofstream& of) = 0;
}; // class Goods

// calculates subtotals
float operator * (int amount, Goods& goods);
float operator * (Goods& goods, int amount);

// class dailygoods
class DailyGoods: public Goods {
public:
	DailyGoods(std::string name, Time& productionDate, float price, int remaining, int goodsNo = 1) :
		Goods(name, productionDate, price, remaining, goodsNo) {}
    
    // add info of a dailygoods product into a table
    bool renderIntoTable(int number, Table& table);
    // gets the type of a product
    Category getCategory();
    // exports a product into a certain file
    void exportGoods(std::ofstream& of);
};

class Supermarket;

// class food
class Food: public Goods {
	friend class Supermarket;	// enables Supermarket to check if a food object is expired
    int guaranteePeriod;        // guarantee period of food
public:
    Food(std::string name, Time& productionDate, float price, int remaining, int guaranteePeriod, int goodsNo = 1): Goods(name, productionDate, price, remaining, goodsNo), guaranteePeriod(guaranteePeriod) {}
    
    // add info of a food product into a table
    bool renderIntoTable(int number, Table& table);
    // gets the type of a product
    Category getCategory();
    // exports a product into a certain file
    void exportGoods(std::ofstream& of);
};


// class ElecticalAppliance
class ElecticalAppliance: public Goods {
    std::string colour;     // colour of electrical app.
    int warrantyPeriod;     // warranty expires date of a electrical app.
public:
    ElecticalAppliance(std::string name, Time& productionDate, float price, int remaining, int warrantyPeriod, std::string colour, int goodsNo = 1): Goods(name, productionDate, price, remaining, goodsNo), warrantyPeriod(warrantyPeriod), colour(colour) {}
    
    // add info of a ElecticalAppliance product into a table
    bool renderIntoTable(int number, Table& table);
    // gets the type of a product
    Category getCategory();
    // exports a product into a certain file
    void exportGoods(std::ofstream& of);
};

// class Supermarket
class Supermarket {
	// enables import of the supermarket's information from a file via >> operator
	friend void operator >> (std::ifstream& infile, Supermarket& sm);
	// enables export of the supermarket's information to a file via >> operator
    friend void operator << (std::ofstream& ofile, Supermarket& market);
	// enables interface to check the market's turnover
    friend void interface_show_turnover(Supermarket& su);
	// enables orders to change their supermarket's cash after they were confirmed
    friend class BuyOrder;
    friend class ReturnOrder;
    
private:
    // goods (must be pointers instead of references to enable dynamic binding and storage during runtime!)
    std::vector<Goods*> goods;
    // remaining cash
    float cash;
    // today's turnover
    float turnover = 0;
    // today's total sales count of certain categories
    int salesCount[3] = {0};
    // name of the supermarket
    std::string name;
    
public:
    // creates a market (initial cash is 0, by default)
    Supermarket(float initCash = 0, std::string name = "default market");
    // destroys all goods records
    ~Supermarket();
    
    // prints all goods as well as market's remaining cash
    void displayAllGoods();
    // gets name
    std::string getName();
    // gets current cash
    float getCash();
    // gets goods list
    std::vector<Goods*>& getAllGoods() { return this->goods; }
    
    // adds new goods object through its pointer
    Supermarket& operator << (Goods* newGoods);
};

// imports an init-list into a Supermarket instance
void operator >> (std::ifstream& infile, Supermarket& market);

// exports the Supermarket instance into an init-list
void operator << (std::ofstream& ofile, Supermarket& market);

// class OrderItem records goods' info and amount of a certain item in an order
class OrderItem {
    friend class BuyOrder;
    friend class ReturnOrder;
private:
    Goods* orderItem;       // must be a pointer which points at an existing object
                            // in the supermarket's database
    int amount;             // amount of the item purchased
    
    OrderItem() {
        this->orderItem = NULL;
        this->amount = 0;
    }
public:
    // initialise an order item pointing to the Goods pointer g
    OrderItem(Goods* g, int amount);

    // gets sub-total price of this item, with 2 decimal rounding-offs
    float getSubTotal() const {
        return Utils::stdPrice(amount * (*this->orderItem));
    }
    
    // adds info of a ordet item into a table
    void renderIntoTable(int number, Table& table);
};


// class Order
class Order {
	// enables interface to read and write the order
    friend void interface_buy(Supermarket& su, const char* saveTo);
    friend void interface_return(Supermarket& su, const char* saveTo);
	friend void interface_add_product(Order& o);
protected:
    Time createdTime;               // moment at which an order is created
    std::vector<OrderItem> items;   // things that the consumer purchased
    float totalPrice;               // total price
    Supermarket* su;                // the supermarket at which this order was made
    
    // adds goods item into the order
    void addGoods(OrderItem& orderItem);
    // new an order (default) without indicating supermarket it refers,
    Order() {
        this->totalPrice = 0;
        this->su = NULL;
    }
    
public:
    // new an order
    Order(Supermarket* su) {
        this->totalPrice = 0;
        this->su = su;
    }
    
    // adds an order item into an order
    virtual bool operator << (OrderItem& oi) = 0;
    
    // prints this order to an out stream, perhaps a file or screen
    virtual void outputOrder(std::ostream& os) = 0;
    
    // tell if it is a buying or returning order
    virtual bool isBuyOrder() = 0;
};

//class BuyOrder
class BuyOrder: public Order {
    friend void interface_buy(Supermarket& su);
public:
    BuyOrder(Supermarket* su) { this->su = su; }
    
    // sales a new product, returns whether the operation is done (T) or error occurred (F)
    bool operator << (OrderItem& oi);
    // judge if it is a buy order
    bool isBuyOrder() { return true; }
    // print order
    void outputOrder(std::ostream& os);
};


//class ReturnOrder
class ReturnOrder: public Order {
    std::string returnReason;
    std::string returnNumber;
public:
    ReturnOrder(Supermarket* su) {
        this->su = su;
        this->returnReason = "Not provided.";
        this->returnNumber = "000000000000";
    }
    
    ReturnOrder(Supermarket* su, std::string returnNumber) {
        this->su = su;
        this->returnReason = "Not provided.";
        this->returnNumber = returnNumber;
    }
    
    ReturnOrder(Supermarket* su, std::string reason, std::string returnNumber) {
        this->su = su;
        this->returnReason = reason;
        this->returnNumber = returnNumber;
    }
    
    void setReason(std::string reason) {
        this->returnReason = reason;
    }
    
    // returns a new product, returns whether the operation is done (T) or error occurred (F)
    bool operator << (OrderItem& oi);
    // judge if it is a buy order
    bool isBuyOrder() { return false; }
    // print order
    void outputOrder(std::ostream& os);
};

// outputs an order to a specified stream
void operator << (std::ostream& os, Order& order);

#endif /* marketplace_h */
