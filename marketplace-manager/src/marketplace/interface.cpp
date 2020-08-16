//  interface.cpp
//  terminal user interface (TUI) functions

#include <iostream>
#include <fstream>
#include <kij.h>            // display extension
#include "display/table.h"
#include "marketplace.h"
#include "interface.h"

using std::cout;
using std::endl;

// show confirm dialogue
static const char* btns_only_ok[] = { "OK" };
static const char* btns_yes_no[] = { "Yes", "No" };

void interface_report_err(const char* errTitle, const char* errBody) {
    Kij_ErrorWin* win = new Kij_ErrorWin;
    win->mainTitle = "";
    win->winTitle = errTitle;
    win->errInfo = errBody;
    win->buttonsCount = 1;
    win->buttons = btns_only_ok;
    
    Kij_NewErrorWin(win);
    delete win;
}

// report information dialogue
void interface_report_info(const char* infoTitle, const char* infoBody) {
    Kij_InfoWin* win = new Kij_InfoWin;
    win->mainTitle = "";
    win->winTitle = infoTitle;
    win->info = infoBody;
    win->buttonsCount = 1;
    win->buttons = btns_only_ok;
    
    Kij_NewInfoWin(win);
    delete win;
}

// show confirm dialogue
bool interface_ask(const char* askTitle, const char* askBody) {
    Kij_InfoWin* win = new Kij_InfoWin;
    win->mainTitle = "";
    win->winTitle = askTitle;
    win->info = askBody;
    win->buttonsCount = 2;
    win->buttons = btns_yes_no;
    
    int sel = Kij_NewInfoWin(win);
    delete win;
    return (sel == 0);
}

// initialise interface
void interface_init() {
    // displays init drawings
    Kij_InitWin* win = new Kij_InitWin;
    win->logo = XMU_MARKET_LOGO;
    win->showPeriod = 1000;
    win->prompt = "Pleasw wait while system is initialising...";
    win->needProgBar = 'T';
    
    Kij_NewInitWin(win);
    delete win;
}


static const char* menu_choices[] = {
	"Check In-Stock Info",
	"Purchase",
	"Return Products",
	"Re-initialise",
	"Show Turnover of Today",
	"About"
};

// show menu
int interface_show_menu() {
    // displays init drawings
    Kij_SelectWin* win = new Kij_SelectWin;
    win->mainTitle = XMU_MARKET_TITLE;
    win->winTitle = "Menu";
    win->choiceDesc = "Choose an operation listed below using arrow keys";
    win->choicesCount = 6;
    win->choices = menu_choices;
    win->winFootNote = "Press Enter to Submit";
    win->statusBarText = "XMU Market 1.0";
    
    int res = Kij_NewSelectWin(win);
    delete win;
    
    return res;
}


// add product entry static labels
static const char* addProductLabels[] = {
	"Product Number:",
	"Amount:"
};
const char* addProductButtons[] = {
	"View In-Stock Info",
	"OK",
	"Cancel"
};

// show add product dialogue
void interface_add_product(Order& o) {
    Kij_InputWin* inputWin = new Kij_InputWin;
    inputWin->mainTitle = XMU_MARKET_TITLE;
    inputWin->winTitle = "Add Prodruct via Product Number";
    inputWin->inputBoxLabelsCount = 2;
    inputWin->inputBoxLabels = addProductLabels;
    inputWin->buttonsCount = 3;
    inputWin->buttons = addProductButtons;
    
    // new an input window
    Kij_InputWin_Results* inr;
    char buff[80];
    while (true) {
        inr = Kij_NewInputWin(inputWin);
        // check select product info
        int sel = inr->selectedButton;
        
        if (sel == 0) { // wants to check in-stock
            o.su->displayAllGoods();
        } else if (sel == 1) { // selected ok
            int prodNumber = atoi(inr->texts[0]);
            if (prodNumber < 1 || prodNumber > o.su->getAllGoods().size()) {
                interface_report_err("Bad Product Number", "Product number is invalid.\nPlease try again.");
                // clear caches
                Kij_Destroy_InputWin_Results(inr);
                continue;
            }
            
            int amount = atoi(inr->texts[1]);
            Kij_Destroy_InputWin_Results(inr);
            
            // check goods that is correspond to this number
            Goods* selectedGoods = o.su->getAllGoods()[prodNumber-1];
            sprintf(buff, "Name: %s\nAmount: %d\nSubtotal: $%.2f", selectedGoods->name.c_str(), amount, amount * (*selectedGoods));
            
            // last confirm
            if (interface_ask("  Confirm Your Choice  ", buff)) {
                OrderItem oi(selectedGoods, amount);
                if (!(o << oi)) {
                    // lack in stock (buy order) or no such product (return order)
                    if (o.isBuyOrder()) {
                        interface_report_err("Error", "Selected product is lack in stock. Please try another.");
                    } else {
                        interface_report_err("Error", "No product of this number found in db. Please try another.");
                    }
                } else {
                    // adds to order succeeded
                    break;
                }
            }
        } else { // selected cancel, close input window
            break;
        }
    }
    delete inputWin;
}

static const char* colNames[] = {
	"No.", "Product Name", "Price", "Quantity", "Subtotal"
};
static int widths[] = {
	1, 3, 1, 1, 1
};
static const char* buttons[] = {
	"Add Product", "Remove Product", "Done"
};

// show buy product
void interface_buy(Supermarket& su, const char* saveTo) {
    BuyOrder bo(&su);
    // display buy-table
    Time t;
    std::string title = "Buy - at ";
    title.append(t.format("dd/MM/yyyy hh:mm:ss"));
    
    Table table(title.c_str(), colNames, 5, widths, buttons, 3);
    // pour all bought into table
    int select = 0;
    while (select != 2) {
        table.clearRow();
        for (int i = 1; i <= bo.items.size(); i++) {
            bo.items[i-1].renderIntoTable(i, table);
        }
        
		// must be pointer, for it will be inserted as a row which will be deleted thereafter
        char* grandTotalStr = new char[80]; 
        sprintf(grandTotalStr, "\n\n\nTotal:\n$%.2f", bo.totalPrice);
        table.appendRow(grandTotalStr);
        
        // print table
        select = table.show();
        switch (select) {
            case 0:     // add buy product
                interface_add_product(bo);
                break;
            case 1:     // remove bought product
                interface_report_err("Not Emplemented", "This function was not yet implemented because\nthe related standard of such a function was \nnot pointed out explicitly in the project's demands");
                break;
            default:    // done
                break;
        }
    }
    
    // reaches here indicates user selected "done", so do sum-up
	char sumUpInfo[80];
	sprintf(sumUpInfo, "Bought %zu items\nTotal income: $%.2f\n", bo.items.size(), bo.totalPrice);
	interface_report_info("Order completed", sumUpInfo);

    // save to document
    std::ofstream of_bl;
    std::string saveFileName = saveTo;
    saveFileName.append("~");
    saveFileName.append(t.format("dd-MM-yyyy_hh.mm.ss"));
    of_bl.open(saveFileName);
    of_bl << bo;
    of_bl.close();
    
    interface_report_info(" Order completed ", " Order was saved to local disk. ");
    
}

// show return product interface
void interface_return(Supermarket& su, const char* saveTo) {
    std::string retNumber;
    // ask return number
    {
        Kij_InputWin* win = new Kij_InputWin;
        win->mainTitle = XMU_MARKET_TITLE;
        win->winTitle = " Enter control number of the order ";
        win->inputBoxLabelsCount = 1;
        const char* lbls[] = { "Control Number:" };
        win->inputBoxLabels = lbls;
        
        win->buttonsCount = 2;
        const char* btns[] = {
            "OK",
            "Cancel"
        };
        win->buttons = btns;
        Kij_InputWin_Results* inr = Kij_NewInputWin(win);
        delete win;
        
        if (inr->selectedButton == 1) { return; }
        
        if (0 == strcmp(inr->texts[0], "")) {
            interface_report_err(" Illegal Control Number ", " Control number was illegal. ");
            return;
        }
        retNumber = inr->texts[0];
        Kij_Destroy_InputWin_Results(inr);
    }
    
    ReturnOrder ro(&su, retNumber);
    // display return table
    Time t;
    std::string title = "Return - Control number: ";
    title.append(retNumber);
    
    Table table(title.c_str(), colNames, 5, widths, buttons, 3);
    // pour all bought into table
    int select = 0;
    while (select != 2) {
        table.clearRow();
        for (int i = 1; i <= ro.items.size(); i++) {
            ro.items[i-1].renderIntoTable(i, table);
        }
        
        char* grandTotalStr = new char[80];
        sprintf(grandTotalStr, "\n\n\nTotal:\n$%.2f", ro.totalPrice);
        table.appendRow(grandTotalStr);
        
        // print table
        select = table.show();
        switch (select) {
            case 0:     // add buy product
                interface_add_product(ro);
                break;
            case 1:     // remove buy product
				interface_report_err("Not Emplemented", "This function was not yet implemented because\nthe related standard of such a function was \nnot pointed out explicitly in the project's demands");
                break;
            case 2:    // done
                break;
        }
    }
    
    // reaches here indicates user selected "done", so do sum-up
    char sumUpInfo[80];
    sprintf(sumUpInfo, "Returned %zu items\nTotal money back: $%.2f\n", ro.items.size(), ro.totalPrice);
    interface_report_info(" Return order completed ", sumUpInfo);
    
    // save to document
    std::ofstream of_bl;
    std::string saveFileName = saveTo;
    saveFileName.append("~");
    saveFileName.append(t.format("dd-MM-yyyy_hh.mm.ss"));
    of_bl.open(saveFileName);
    of_bl << ro;
    of_bl.close();
    
    interface_report_info(" Return order completed ", "Order was saved to local disk.");
    
}

// show today's turnover
void interface_show_turnover(Supermarket& su) {
    std::string turnoverStr = "Today's income: $";
    turnoverStr.append(Utils::itoa(su.turnover, 2));
    
	char tempStr[80];
    sprintf(tempStr, "\n1) %d DailyGoods item(s) sold;", su.salesCount[Goods::DAILYGOODS]);
    turnoverStr.append(tempStr);
    sprintf(tempStr, "\n2) %d Foods item(s) sold;", su.salesCount[Goods::FOODS]);
    turnoverStr.append(tempStr);
    sprintf(tempStr, "\n3) %d Electrical item(s) sold", su.salesCount[Goods::ELECTRICAL]);
    turnoverStr.append(tempStr);
    
    Time t;
    std::string title = "Turnover - ";
    title.append(t.format("dd/MM/yyyy"));
    
    Kij_InfoWin* win = new Kij_InfoWin;
    win->mainTitle = XMU_MARKET_TITLE;
    win->winTitle = title.c_str();
    win->info = turnoverStr.c_str();
    win->buttonsCount = 1;
    const char* btns[] = { "OK" };
    win->buttons = btns;
    
    Kij_NewInfoWin(win);
    delete win;
}

void interface_show_about() {
    Kij_InfoWin* win = new Kij_InfoWin;
    win->mainTitle = XMU_MARKET_TITLE;
    win->winTitle = " About XMU Market ";
    win->info = {
        XMU_MARKET_LOGO
		"\n\nXMU Market 1.0\nCopyright (c) 2020 Han Li Studios.\nCourse Design of the C++ Programming Cource"
    };
    win->buttonsCount = 1;
    win->buttons = btns_only_ok;
    
    Kij_NewInfoWin(win);
    delete win;
}


// main entrance of marketplace
int DisplayMarketplace() {
    // reads market config which indicates where the goods lists were placed
    std::ifstream marketConf("market.conf");
    if (!marketConf) {
        interface_report_err("Error", "Market config file could not be opened at this time.");
        return 255;
    }
    char inFile[265];
    marketConf >> inFile;
    char buyOrderPrefix[256];
    marketConf >> buyOrderPrefix;
    char retOrderPrefix[256];
    marketConf >> retOrderPrefix;
    char ouFile[256];
    marketConf >> ouFile;
    
    marketConf.close();
    
    Time now;
    now.set("today");
    
    // read in goods config
    Supermarket su;
    std::ifstream inconfig(inFile);
    if (!inconfig) {
        interface_report_err("Error", "Initialise file could not be opened at this time");
        return 255;
    }
    
    // initialise supermarket and interface
    interface_init();
    inconfig >> su;
    inconfig.close();
    
    // show menu
    int select = interface_show_menu();
    while (select != 255) {
        switch (select) {
            case 0: // show in-stock
                su.displayAllGoods();
                break;
            case 1: // buy
                interface_buy(su, buyOrderPrefix);
                break;
            case 2: // return
                interface_return(su, retOrderPrefix);
                break;
            case 3: // re-initialise
                interface_init();
                inconfig.open(inFile);
                inconfig >> su;
                inconfig.close();
                interface_report_info("Info", "Re-initialise Complete");
                break;
            case 4: // show turnover
                interface_show_turnover(su);
                break;
            case 5: // show about
                interface_show_about();
                break;
            default:
                break;
        }
        
        // must let user re-select menu
        select = interface_show_menu();
    }
    
    // save init-list to file
    std::ofstream outconfig;
    outconfig.open(ouFile);
    outconfig << su;
    outconfig.close();
    
    interface_report_info("Info", "Market information file was saved to local disk.");
    
    return 0;
}
