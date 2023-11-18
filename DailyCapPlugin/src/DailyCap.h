#pragma once

// NOTE: filesystem include is required for ToolboxIni.h

#include <cstdint>
#include <filesystem>
#include <string>

#include <ToolboxIni.h>

class DailyCap {
private:
    const char* name;
    int current_value;
    int day_start_value;
    uint64_t last_updated;
    bool resets_on_day_rollover = false;

    bool display = true;

    const std::string ini_day_start_key;
    const std::string ini_current_key;
    const std::string ini_updated_key;

    const std::string ini_display_key;

    const std::string imgui_display_toggle_key;
protected:
    const int default_cap;
public:
    DailyCap(const char* name, int max, bool reset = false);
    virtual ~DailyCap() = default;

    void DrawInternal();
    void DrawSettingsInternal();
    void LoadSettings(const CSimpleIniA& ini, const char* section);
    void SaveSettings(CSimpleIniA& ini, const char* section);

    int GetProgress();
    virtual int GetCap();

    void LoadProgress(const CSimpleIniA& ini, const char* account, int defaultStartValue);
    void SaveProgress(CSimpleIniA& ini, const char* account);

    void AddValue(int modifier);
    void SetValue(int newValue);
    void HandleDailyReset(uint64_t timestamp);
};
