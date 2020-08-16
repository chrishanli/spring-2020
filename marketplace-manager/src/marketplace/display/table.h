//  table.h
//  to arrange for a table that contains series of data

#ifndef table_h
#define table_h

#include <vector>

// a table class, which could be rendered through the KIJ kit
class Table {
	// the table's properties
    const char* tableName;
    const char** columnNames;
    int columnCount;
    int* columnWidths;
    const char** buttons;
    int buttonsCount;
    
	// the table's data
    std::vector<char*> rows;
public:
	// appends a row into the table
    void appendRow(char *);
	// shows the table
    int show();
	// clears the data stored in the table
    void clearRow();
    
    Table(const char tableName[], const char* columnNames[], int columnCount, int columnWidths[], const char* buttons[], int buttonsCount) {
        this->tableName = tableName;
        this->columnNames = columnNames;
        this->columnCount = columnCount;
        this->columnWidths = columnWidths;
        this->buttons = buttons;
        this->buttonsCount = buttonsCount;
    }
    
    ~Table() {
        // releases all rows
        std::vector<char*>::iterator i;
        for (i = rows.begin(); i != rows.end(); i++) {
            delete [](*i);
        }
    }
};



#endif /* table_h */
