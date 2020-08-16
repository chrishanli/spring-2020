//  table.cpp
//  to arrange for a table that contains series of data

#include <iostream>
#include <kij.h>
#include "../marketplace.h"
#include "table.h"

void Table::appendRow(char *r) {
    this->rows.push_back(r);
}

int Table::show() {
	// arrange for a new table window object
    Kij_TableWin* win = new Kij_TableWin;
    win->mainTitle = XMU_MARKET_TITLE;
    win->winTitle = this->tableName;
    win->columnCount = this->columnCount;
    win->columnNames = this->columnNames;
    win->columnWidths = this->columnWidths;
    win->rows = this->rows.data();
    win->buttons = buttons;
	win->buttonsCount = buttonsCount;
    win->rowsCount = (int)this->rows.size();
    
	// show the table window, and then delete the window project immediately
    int ret = Kij_NewTableWin(win);
    delete win;
    
    return ret;
}

void Table::clearRow() {
    this->rows.clear();
}
