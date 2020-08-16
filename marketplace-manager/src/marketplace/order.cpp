//  order.cpp
//  defines ways in which the two types of orders manipulate their items

#include <iostream>
#include <string>
#include <iomanip>
#include "marketplace.h"

using std::endl;
using std::setw;

OrderItem::OrderItem(Goods* g, int amount) {
    this->orderItem = g;
    this->amount = amount;
}

void OrderItem::renderIntoTable(int number, Table& table) {
    //"No.", "Product Name", "Price", "Amount", "Subtotal"
    char* rdStr = new char[80];
    sprintf(rdStr, "%d\n%s\n$%.2f\n%d\n$%.2f",
            number, this->orderItem->name.c_str(), this->orderItem->price,
            this->amount, this->getSubTotal());
    table.appendRow(rdStr);
}

void Order::addGoods(OrderItem& orderItem) {
    this->items.push_back(orderItem);
    this->totalPrice += orderItem.getSubTotal();
}


void BuyOrder::outputOrder(std::ostream& outstream) {
    // create a table to contain order items info
    outstream << "Buy Order: " << this->createdTime.format("dd/MM/yyyy");
    outstream << " at " << this->su->getName() << endl;
    
    // first row
    outstream << setw(5) << "No." << setw(20) << "Name" << setw(5) << "Qtt.";
    outstream << setw(10) << "Price" << setw(10) << "SubT." << endl;
    
    // separators
    outstream << setw(5) << "====" << setw(20) << "===================" << setw(5) << "====";
    outstream << setw(10) << "=========" << setw(10) << "=========" << endl;
    
    for (int i = 1; i <= this->items.size(); i++) {
        OrderItem& tmp = this->items[i-1];
        outstream << setw(5) << i << setw(20) << tmp.orderItem->name ;
        outstream << setw(5) << tmp.amount;
        outstream << setw(10) << tmp.orderItem->price;
        outstream << setw(10) << tmp.getSubTotal() << endl;
    }
    
    outstream << "Total: $" << this->totalPrice << std::endl;
    outstream << "Market Cash: $" << this->su->getCash() << std::endl;
}


void ReturnOrder::outputOrder(std::ostream& outstream) {
    // create a table to contain order items info
    outstream << "Return Order: " << this->createdTime.format("dd/MM/yyyy");
    outstream << " at " << this->su->getName() << std::endl;
    outstream << "Reason: " << this->returnReason << std::endl;
    outstream << "Control Number: " << this->returnNumber << std::endl;
    
    // first row
    outstream << setw(5) << "No." << setw(20) << "Name" << setw(5) << "Qtt.";
    outstream << setw(10) << "Price" << setw(10) << "SubT." << endl;
    
    // separators
    outstream << setw(5) << "====" << setw(20) << "===================" << setw(5) << "====";
    outstream << setw(10) << "=========" << setw(10) << "=========" << endl;
    
    for (int i = 1; i <= this->items.size(); i++) {
        OrderItem& tmp = this->items[i-1];
        outstream << setw(5) << i << setw(20) << tmp.orderItem->name ;
        outstream << setw(5) << tmp.amount;
        outstream << setw(10) << tmp.orderItem->price;
        outstream << setw(10) << tmp.getSubTotal() << endl;
    }
    
    outstream << "Total: $" << this->totalPrice << std::endl;
    outstream << "Market Cash: $" << this->su->getCash() << std::endl;
}

void operator << (std::ostream& os, Order& order) {
    order.outputOrder(os);
}

float operator * (int amount, Goods& goods) {
    return amount * goods.price;
}

float operator * (Goods& goods, int amount) {
    return amount * goods.price;
}


bool BuyOrder::operator << (OrderItem& oi) {
    // check if still in-stock
    bool isOK = false;
	// iterate through the orcheck remaining
    std::vector<Goods*>::iterator i = this->su->getAllGoods().begin();
    for (; i != this->su->getAllGoods().end(); i++) {
        if (*i == oi.orderItem) {
			// check if the amount of such a product is sufficient
            isOK = ((*i)->remaining >= oi.amount);
            break;
        }
    }
    if (!isOK) return false;
    
    this->items.push_back(oi);
    float subTotal = oi.getSubTotal();
    this->totalPrice += subTotal;
    
    // diminish in-stock amount
    (*i)->remaining -= oi.amount;
    // add up supermarket cash
    this->su->cash += subTotal;
    // record to today's turnover
    this->su->turnover += subTotal;
    this->su->salesCount[oi.orderItem->getCategory()] += oi.amount;
    
    return true;
}


bool ReturnOrder::operator << (OrderItem& oi) {
    // check if still in-stock
    bool isOK = false;
    std::vector<Goods*>::iterator i = this->su->getAllGoods().begin();
    for (; i != this->su->getAllGoods().end(); i++) {
		// found the corresponding item
        if (*i == oi.orderItem) {   
            isOK = true;
            break;
        }
    }
    if (!isOK) return false;
    
    this->items.push_back(oi);
    
    float subTotal = oi.getSubTotal();
    this->totalPrice += subTotal;
    
    // adds up in-stock amount
    (*i)->remaining += oi.amount;
    // diminish supermarket cash
    this->su->cash -= subTotal;
    // record to today's turnover
    this->su->turnover -= subTotal;
    this->su->salesCount[oi.orderItem->getCategory()] -= oi.amount;
    
    return true;
}
