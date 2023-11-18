#include "DailyCap.h"
#include "ResetTime.h"

#include <imgui.h>

#include <iostream>
#include <regex>

namespace {
    std::string removeSpaces(std::string&& original) {
        const std::regex expr("\\s+");
        return std::regex_replace(original, expr, "");
    }
}

DailyCap::DailyCap(const char *name, int max, bool reset)
: name(name),
  ini_current_key(std::string(name).append("_current")),
  ini_day_start_key(std::string(name).append("_day_start")),
  ini_updated_key(std::string(name).append("_updated")),
  ini_display_key(removeSpaces(std::string(name)).append("_display")),
  imgui_display_toggle_key(std::string(name).append("##dailycap_displaytoggle")),
  default_cap(max), current_value(0), day_start_value(0),
  last_updated(WEEKLY_BONUS_EPOCH), resets_on_day_rollover(reset)
{

}

void DailyCap::HandleDailyReset(uint64_t timestamp) {
    if (getResetDay(timestamp) <= getResetDay(this->last_updated)) {
        return;
    }

    if (this->resets_on_day_rollover) {
        this->day_start_value = 0;
        this->current_value = 0;
    }
    else {
        this->day_start_value = this->current_value;
    }

    this->last_updated = timestamp;
}

int DailyCap::GetProgress() {
    uint64_t timestamp = getCurrentTime();
    HandleDailyReset(timestamp);

    return this->current_value - this->day_start_value;
}

int DailyCap::GetCap() {
    return this->default_cap;
}

void DailyCap::AddValue(int modifier) {
    uint64_t timestamp = getCurrentTime();
    HandleDailyReset(timestamp);

    this->current_value += modifier;
    this->last_updated = timestamp;
}

void DailyCap::SetValue(int newValue) {
    uint64_t timestamp = getCurrentTime();
    HandleDailyReset(timestamp);

    this->current_value = newValue;
    this->last_updated = timestamp;
}

void DailyCap::LoadProgress(const CSimpleIniA& ini, const char *account, int defaultStartValue) {
    this->day_start_value = ini.GetLongValue(account, this->ini_day_start_key.c_str(), -1);
    this->current_value = ini.GetLongValue(account, this->ini_current_key.c_str(),  -1);

    const char* _last_updated = ini.GetValue(account, this->ini_updated_key.c_str(), "0");
    this->last_updated = _strtoui64(_last_updated, nullptr, 10);

    if (this->current_value == -1 && this->day_start_value == -1) {
        this->current_value = defaultStartValue;
        this->day_start_value = defaultStartValue;
    }
}

void DailyCap::SaveProgress(CSimpleIniA& ini, const char *account) {
    constexpr int BUF_SIZE = 21;
    char last_updated_buf[BUF_SIZE];

    if(0 != _ui64toa_s(this->last_updated, last_updated_buf, BUF_SIZE, 10)) {
        std::cerr <<"Plugin Error (DailyCapPlugin): failed to serialize last_updated to string" << std::endl;
    }

    ini.SetLongValue(account, this->ini_day_start_key.c_str(), this->day_start_value);
    ini.SetLongValue(account, this->ini_current_key.c_str(), this->current_value);
    ini.SetValue(account, this->ini_updated_key.c_str(), last_updated_buf);
}

void DailyCap::DrawInternal() {
    if(this->display) {
        ImGui::Text("%s: %d/%d", this->name, this->GetProgress(), this->GetCap());
    }
}

void DailyCap::DrawSettingsInternal() {
    ImGui::Checkbox(imgui_display_toggle_key.c_str(), &this->display);
}

void DailyCap::LoadSettings(const CSimpleIniA& ini, const char* section) {
    this->display = ini.GetBoolValue(section, this->ini_display_key.c_str(), true);
}

void DailyCap::SaveSettings(CSimpleIniA& ini, const char* section) {
    ini.SetBoolValue(section, this->ini_display_key.c_str(), this->display);
}