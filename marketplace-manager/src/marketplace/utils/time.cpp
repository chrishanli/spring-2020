//  time.cpp
//  implements manipulations of class Time

#include <string>
#include <iostream>
#include <time.h>

#include "utils.h"

static const int MDAYS[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const int MDAYS_LEAP[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

inline bool isLeapYear(int yyyy) {
    return (yyyy % 4 && !(yyyy % 100)) || yyyy % 400;
}

void Time::set(int yyyy, int MM, int dd, int hh, int mm, int ss) {
    time_t timeStamp = time(NULL);
    tm* time = localtime(&timeStamp);
    // set year
    year = yyyy >= 1970 ? yyyy : time->tm_year + 1900;
    // set month
    month = MM < 13 && MM > 0 ? MM : time->tm_mon + 1;
    // set day
    const int* daysTable = isLeapYear(yyyy) ? MDAYS_LEAP : MDAYS;
    day = dd <= daysTable[month] && dd >= 1 ? dd : time->tm_mday;
    // set hour, minute and second
	hour = hh;
	min = mm;
	sec = ss;
}

void Time::set(long timeStamp) {
    tm* time = localtime((time_t *)&timeStamp);
    year = time->tm_year + 1900;
    month = time->tm_mon + 1;
    day = time->tm_mday;
    hour = time->tm_hour;
    min = time->tm_min;
    sec = time->tm_sec;
}

void Time::set(std::string timepoint) {
    // get timestamp
    time_t timeStamp = time(NULL);
    tm* time = localtime(&timeStamp);
    if (0 == timepoint.compare("today")) {
        year = time->tm_year + 1900;
        month = time->tm_mon + 1;
        day = time->tm_mday;
        hour = 0;
        min = 0;
        sec = 0;
    } else {
        year = time->tm_year + 1900;
        month = time->tm_mon + 1;
        day = time->tm_mday;
        hour = time->tm_hour;
        min = time->tm_min;
        sec = time->tm_sec;
    }
}

bool Time::isBefore(Time &_d2) {
    if (year < _d2.year)
        return true;
    if (year <= _d2.year && month < _d2.month)
        return true;
    if (year <= _d2.year && month <= _d2.month && day < _d2.day)
        return true;
    if (year <= _d2.year && month <= _d2.month && day <= _d2.day &&
        hour < _d2.hour)
        return true;
    if (year <= _d2.year && month <= _d2.month && day <= _d2.day &&
        hour <= _d2.hour && min < _d2.min)
        return true;
    if (year <= _d2.year && month <= _d2.month && day <= _d2.day &&
        hour <= _d2.hour && min <= _d2.min && sec < _d2.sec)
        return true;
    return false;
}


int Time::dayInYear(Time &_d) {
    int day = 0;
    const int* daysTable = isLeapYear(_d.year) ? MDAYS_LEAP : MDAYS;
    for(int i = 1; i < _d.month; ++i) {
        day += daysTable[i];
    }
    return day + _d.day;
}

bool Time::operator < (Time& compareTo) {
    return isBefore(compareTo);
}

bool Time::operator > (Time& compareTo) {
    return !isBefore(compareTo);
}

bool Time::operator == (Time& compareTo) {
    return
    this->year == compareTo.year &&
    this->month == compareTo.month &&
    this->day == compareTo.day &&
    this->hour == compareTo.hour &&
    this->min == compareTo.min &&
    this->sec == compareTo.sec;
}


int Time::operator - (Time& compareTo) {
    int interval = 0;
    if (this->isBefore(compareTo)) {
        // not allowed
        return -1;
    }
    if(this->year == compareTo.year &&
       this->month == compareTo.month) {
        return this->day - compareTo.day;
    } else if (this->year == compareTo.year) {
		// if same year but not same month, then calculate date difference
        int d1 = dayInYear(*this), d2 = dayInYear(compareTo);
        return d1 - d2;
    } else {
		// if neither year nor month are the same, first calculate years difference first
        int day_1, day_2, day_3;
        // second, get the count of the rest days in the year of d2
        if(isLeapYear(compareTo.year))
            day_1 = 366 - dayInYear(compareTo);
        else
            day_1 = 365 - dayInYear(compareTo);
		// third, get the count of the past days in the year of d1 (day_2)
        day_2 = dayInYear(*this);
        // then, fetch days of the years in between (day_3)
        day_3 = 0;
        for(int yr = compareTo.year + 1; yr < this->year; yr++) {
            if(isLeapYear(yr))
                day_3 += 366;
            else
                day_3 += 365;
        }
        return day_1 + day_2 + day_3;
    }
    
    return interval;
}

Time& Time::operator += (int period) {
    const int* dayTable = isLeapYear(this->year) ? MDAYS_LEAP : MDAYS;
    // adds period to date
    this->day += period;
    while (this->day > dayTable[this->month]) {
        this->month++;
        // if month out of bound
        if (this->month > 12) {
            this->year++;
            this->month = 1;
            dayTable = isLeapYear(this->year) ? MDAYS_LEAP : MDAYS;
        }
        this->day = this->day - dayTable[this->month];
    }
    return *this;
}


inline void strReplace(std::string& src, std::string toFind, std::string replaceTo) {
    size_t place = src.find(toFind);
    if (place != std::string::npos) {
        src.replace(place, toFind.size(), replaceTo);
    }
}

std::string Time::format(std::string formatString) {
    char tmp[5] = {0};
    // parse year
    sprintf(tmp, "%4d", this->year);
    strReplace(formatString, "yyyy", tmp);
    // parse month
    sprintf(tmp, "%02d", this->month);
    strReplace(formatString, "MM", tmp);
    // parse date
    sprintf(tmp, "%02d", this->day);
    strReplace(formatString, "dd", tmp);
    // parse hour
    sprintf(tmp, "%02d", this->hour);
    strReplace(formatString, "hh", tmp);
    // parse minute
    sprintf(tmp, "%02d", this->min);
    strReplace(formatString, "mm", tmp);
    // parse second
    sprintf(tmp, "%02d", this->sec);
    strReplace(formatString, "ss", tmp);
    return formatString;
}

void Time::parseDate(std::string dateStr) {
    int year, month, day;
    sscanf(dateStr.c_str(), "%d/%d/%d", &year, &month, &day);
    this->set(year, month, day);
}
