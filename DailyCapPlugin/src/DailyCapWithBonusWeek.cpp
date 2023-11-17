#include "DailyCapWithBonusWeek.h"

#include "ResetTime.h"

DailyCapWithBonusWeek::DailyCapWithBonusWeek(const char* name, int max, int bonus_week)
: DailyCap(name, max, false), bonus_week(bonus_week) {

}

int DailyCapWithBonusWeek::GetCap() {
    if (this->bonus_week == (getResetWeek(getCurrentTime()) % 6)) {
        return 2 * this->default_cap;
    }
    else {
        return this->default_cap;
    }
}