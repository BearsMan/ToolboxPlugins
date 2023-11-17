#pragma once

#include "DailyCap.h"

class DailyCapWithBonusWeek : public DailyCap {
private:
    int bonus_week;
public:
    DailyCapWithBonusWeek(const char* name, int max, int bonus_week);

    int GetCap() override;
};
