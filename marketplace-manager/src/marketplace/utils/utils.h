//  utils.h
//  utilities of libmarketplace

#ifndef utils_h
#define utils_h

#include <iostream>


// class Time
class Time {
private:
    int sec = 0;
    int min = 0;
    int hour = 0;
    int day = 0;
    int month = 0;
    int year = 0;
    
    // tells if the time is former than the target specified
    bool isBefore(Time& compareTo);
public:
    // give a time pointing to now
    Time() { set(); }
    // give a time pointing to a specified second, or day
    Time(int yyyy, int MM, int dd, int hh=0, int mm=0, int ss=0) {
        set(yyyy, MM, dd, hh, mm, ss);
    }
    // give a time pointing to a specified date string to be parsed
    Time(std::string dateStr) {
        parseDate(dateStr);
    }
    // sets time to a specified moment
    void set(int yyyy, int MM, int dd, int hh=0, int mm=0, int ss=0);
    // sets time to fit a UNIX timestamp
    void set(long timestamp);
    // sets time to current time, by default
    void set(std::string timepoint = "now");
    // renders time to a specified format
    std::string format(std::string formatStr);
    
    // tells a day is which day in a year
    int dayInYear(Time &_d);
    
    bool operator < (Time& compareTo);
    bool operator > (Time& compareTo);
    bool operator == (Time& compareTo);
    // calculates span of two DAYS
    int operator - (Time& compareTo);
    // add day period to time
    Time& operator += (int period);
    
    // parse this date from a string formatted like 1970/1/1
    void parseDate(std::string dateStr);
};


// class Utils: static functions which implements useful methods of this project
class Utils {
public:
    // get standardised price, with 2 decimal rounding-offs
    static float stdPrice(float price);

	// get string format of a number
	static std::string itoa(float number, int fraction = 0);
};



#endif /* utils_h */
