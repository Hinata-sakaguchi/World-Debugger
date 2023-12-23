#ifndef DS_TIME_H
#define DS_TIME_H

#include <3ds.h>
#include <stdio.h>
#include <time.h>

const char* const months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

const char* const weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

const u16 daysAtStartOfMonthLUT[12] =
{
	0 % 7, //january    31
	31 % 7, //february   28+1(leap year)
	59 % 7, //march      31
	90 % 7, //april      30
	120 % 7, //may        31
	151 % 7, //june       30
	181 % 7, //july       31
	212 % 7, //august     31
	243 % 7, //september  30
	273 % 7, //october    31
	304 % 7, //november   30
	334 % 7  //december   31
};

static inline bool isLeapYear(int year)
{
	return (year % 4) == 0 && !((year % 100) == 0 && (year % 400) != 0);
}

static inline int getDayOfWeek(int day, int month, int year)
{
	//http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
	day += 2 * (3 - ((year / 100) % 4));
	year %= 100;
	day += year + (year / 4);
	day += daysAtStartOfMonthLUT[month] - (isLeapYear(year) && (month <= 1));
	return day % 7;
}
#endif // !DS_TIME_H
