/*
 * Author: Robert Neagu 
 * ID: 2318538N
 * Systems Programming - Coursework 1a
 *
 * This is my own work as defined in the Academic Ethics agreement I have signed
 */

#include "date.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct date {
  int year;
  int month;
  int day;
};

/**
 * Checks whether a number is within a range
 * 
 * @returns Whether the number is in range or not
 */
int isInRange(int number, int min, int max);
int isInRange(int number, int min, int max) {
  return (number >= min && number <= max);
}

Date *date_create(char *datestr) {
  Date *datePointer = (Date *) malloc(sizeof(Date));
  if (datePointer != NULL) {
    sscanf(datestr, "%d/%d/%d", &datePointer->day, &datePointer->month, &datePointer->year);
    // Make sure the date is within calendar range
    if (!isInRange(datePointer->day, 1, 31) || !isInRange(datePointer->month, 1, 12) || datePointer->year <= 0) {
      return NULL;
    }
  }
  return datePointer;
}

Date *date_duplicate(Date *d) {
  Date *dDatePointer = (Date *) malloc(sizeof(Date));
  if (dDatePointer != NULL) {
    *dDatePointer = *d;
  }
  return dDatePointer;
}
int date_compare(Date *date1, Date *date2) {
  int result = 0;
  if (date1 != NULL && date2 == NULL)
    return 1;
  if (date1 == NULL && date2 != NULL)
    return -1;
  if (date1 != NULL && date2 != NULL) {
    result = date1->year - date2->year;
    if (result == 0) {
      result = date1->month - date2->month;
      if (result == 0) {
        result = date1->day - date2->day;
      }
    }
  }
  return result;
}

void date_destroy(Date *d) {
  free(d);
}