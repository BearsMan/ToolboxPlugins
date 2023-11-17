#pragma once

#include <cstdint>

constexpr uint64_t WEEKLY_BONUS_EPOCH = 130129308000000000ULL;

uint64_t getCurrentTime();
int getResetDay(uint64_t);
int getResetWeek(uint64_t);
