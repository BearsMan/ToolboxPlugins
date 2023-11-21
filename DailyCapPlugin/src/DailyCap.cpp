#include "DailyCap.h"
#include "ResetTime.h"

#include <imgui.h>

#include <iostream>
#include <regex>

namespace {
    std::string sanitize_ini_key(std::string&& original) {
        char* buffer = new char[original.length() * 2];
        int idx = 0;

        auto is_valid_char = [](char c) {
            return (c >= '0' && c <= '9')
              || (c >= 'A' && c <= 'Z')
              || (c >= 'a' && c <= 'z')
              || c == '_';
        };

        for(auto& c : original) {
            if (!is_valid_char(c)) continue;

            if (c >= 'A' && c <= 'Z' && idx > 0)
            {
                buffer[idx++] = '_';
            }

            buffer[idx++] = static_cast<char>(std::tolower(c));
        }
        buffer[idx] = '\0';

        std::string result(buffer);

        delete[] buffer;

        return result;
    }

    // Size argument for ImGui::ProgressBar, corresponding to:
    // * x stretches to end
    // * y auto sizes
    const ImVec2 progress_bar_size_arg = ImVec2(-1, 0);
}

DailyCap::DailyCap(const char *name, int max, bool reset)
: name(name),
  ini_current_key(sanitize_ini_key(std::string(name)).append("_current")),
  ini_day_start_key(sanitize_ini_key(std::string(name)).append("_day_start")),
  ini_updated_key(sanitize_ini_key(std::string(name)).append("_updated")),
  ini_display_key(sanitize_ini_key(std::string(name)).append("_display")),
  imgui_display_toggle_key(std::string(name).append("##dailycap_displaytoggle")),
  default_cap(max), current_value(0), day_start_value(0),
  last_updated(WEEKLY_BONUS_EPOCH), resets_on_day_rollover(reset)
{

}

// Compares the provided timestamp to the timestamp of the last update.  If the last update was on a previous day,
// resets the daily cap.
//
// Returns true iff a reset occurred.
bool DailyCap::HandleDailyReset(uint64_t timestamp) {
    if (getResetDay(timestamp) <= getResetDay(this->last_updated)) {
        return false;
    }

    if (this->resets_on_day_rollover) {
        this->day_start_value = 0;
        this->current_value = 0;
    }
    else {
        this->day_start_value = this->current_value;
    }

    this->last_updated = timestamp;
    return true;
}

// Performs a daily reset, if applicable, then gets the progress towards today's daily cap.
//
// Returns a tuple of the progress, and whether a daily reset occurred
std::tuple<int,bool> DailyCap::GetProgress() {
    uint64_t timestamp = getCurrentTime();
    bool resetHappened = HandleDailyReset(timestamp);

    return std::make_tuple(this->current_value - this->day_start_value, resetHappened);
}

// Gets today's cap on progress.  The cap may vary due to weekly bonuses.
int DailyCap::GetCap() {
    return this->default_cap;
}

// Performs a daily reset, if applicable, then adds `modifier` to the current progress.
//
// Returns true iff a daily reset occurred.
bool DailyCap::AddValue(int modifier) {
    uint64_t timestamp = getCurrentTime();
    bool resetHappened = HandleDailyReset(timestamp);

    this->current_value += modifier;
    this->last_updated = timestamp;

    return resetHappened;
}

// Performs a daily reset, if applicable, then sets the current progress to `newValue`.
//
// Returns true iff a daily reset occurred.
bool DailyCap::SetValue(int newValue) {
    uint64_t timestamp = getCurrentTime();
    bool resetHappened = HandleDailyReset(timestamp);

    this->current_value = newValue;
    this->last_updated = timestamp;

    return resetHappened;
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

    if (0 != _ui64toa_s(this->last_updated, last_updated_buf, BUF_SIZE, 10)) {
        std::cerr <<"Plugin Error (DailyCapPlugin): failed to serialize last_updated to string" << std::endl;
    }

    ini.SetLongValue(account, this->ini_day_start_key.c_str(), this->day_start_value);
    ini.SetLongValue(account, this->ini_current_key.c_str(), this->current_value);
    ini.SetValue(account, this->ini_updated_key.c_str(), last_updated_buf);
}

// Draws the progress for this cap to the current open ImGui context.
// NB. Expects ImGui styles to be pushed onto the stack, such that the default text color
// is the most recent, and pops one style from the stack.
//
// Returns true iff the operation caused a daily reset to occur.
bool DailyCap::DrawInternal() {
    bool resetHappened = false;

    if(this->display) {
        int progress;
        int cap = this->GetCap();
        std::tie(progress, resetHappened) = this->GetProgress();

        float percent = static_cast<float>(progress) / static_cast<float>(cap);

        char progress_text[32];
        sprintf_s(progress_text, "%d/%d", progress, cap);
        float window_width = ImGui::GetWindowWidth();
        float text_width = ImGui::CalcTextSize(progress_text).x;

        ImGui::Text("%s:", this->name);
        ImGui::ProgressBar(percent, progress_bar_size_arg, "");
        ImGui::SameLine(0.0f);
        ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
        ImGui::PopStyleColor();
        ImGui::Text("%d/%d", progress, cap);
    }
    else {
        ImGui::PopStyleColor();
    }

    return resetHappened;
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