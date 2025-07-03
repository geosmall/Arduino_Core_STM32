#ifndef Time_h
#define Time_h

// DateTimeFields represents calendar date & time with 7 fields, hour (0-23), min
// (0-59), sec (0-59), wday (0-6, 0=Sunday), mday (1-31), mon (0-11), year
// (70-206, 70=1970, 206=2106).  These 7 fields follow C standard "struct tm"
// convention, but are stored with only 8 bits to conserve memory.
typedef struct  {
  uint8_t sec;   // 0-59
  uint8_t min;   // 0-59
  uint8_t hour;  // 0-23
  uint8_t wday;  // 0-6, 0=sunday
  uint8_t mday;  // 1-31
  uint8_t mon;   // 0-11
  uint8_t year;  // 70-206, 70=1970, 206=2106
} DateTimeFields;

#ifdef __cplusplus

// Convert a "unixtime" number into 7-field DateTimeFields
void breakTime(uint32_t time, DateTimeFields &tm);  // break 32 bit time into DateTimeFields
// Convert 7-field DateTimeFields to a "unixtime" number.  The wday field is not used.
uint32_t makeTime(const DateTimeFields &tm); // convert DateTimeFields to 32 bit time

#endif /* __cplusplus */

#endif /* Time_h */