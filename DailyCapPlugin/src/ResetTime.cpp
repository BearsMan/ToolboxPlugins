#include "ResetTime.h"
#include <Windows.h>

// Returns the current system time as a value equivalent to
// windows FILETIME: 10000000ths of a second since January
// 1st 1601 at 00:00UTC.
uint64_t getCurrentTime() {
    SYSTEMTIME time_utc;
    GetSystemTime(&time_utc);

    uint64_t time_raw = 0;
    SystemTimeToFileTime(&time_utc, reinterpret_cast<FILETIME*>(&time_raw));
    return time_raw;
}

// Returns the number of days since Monday 13 May 2013,
// in the fictional timezone UTC-15, which places the weekly
// bonus reset at midnight
//
// time_raw parameter is equivalent to windows FILETIME:
// 10000000ths of a second since January 1st 1601 at 00:00UTC.
int getResetDay(uint64_t time_raw)
{
    int64_t constexpr CONVERT_DAYS = 10000000ULL * 60 * 60 * 24;

    auto result = static_cast<int64_t>(time_raw - WEEKLY_BONUS_EPOCH);
    result /= CONVERT_DAYS;

    return static_cast<int>(result);
}

// Returns the number of weeks since Monday 13 May 2013,
// in the fictional timezone UTC-15, which places the weekly
// bonus reset at midnight
//
// time_raw parameter is equivalent to windows FILETIME:
// 10000000ths of a second since January 1st 1601 at 00:00UTC.
int getResetWeek(uint64_t time_raw)
{
    int64_t constexpr CONVERT_WEEKS = 10000000ULL * 60 * 60 * 24 * 7;

    auto result = static_cast<int64_t>(time_raw - WEEKLY_BONUS_EPOCH);
    result /= CONVERT_WEEKS;

    return static_cast<int>(result);
}