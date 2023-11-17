#pragma once

#include <GWCA/Utilities/Hook.h>
#include <ToolboxUIPlugin.h>

#include "DailyCap.h"
#include "DailyCapWithBonusWeek.h"

namespace GW::Constants {
    enum class TitleID : uint32_t;
}

class DailyCapPlugin : public ToolboxUIPlugin {
public:
    [[nodiscard]] const char* Name() const override { return "Daily Cap Tracker"; }
    [[nodiscard]] const char* Icon() const override { return ICON_FA_DOLLAR_SIGN; }
    [[nodiscard]] bool HasSettings() const override { return true; }

    DailyCapPlugin();

    void Initialize(ImGuiContext*, ImGuiAllocFns, HMODULE) override;
    void SignalTerminate() override;
    void Draw(IDirect3DDevice9*) override;
    void DrawSettings() override;
    void LoadSettings(const wchar_t*) override;
    void SaveSettings(const wchar_t*) override;

    void OnTitleUpdated(GW::Constants::TitleID title_id, int new_value);
    void OnServerChatMessage(const wchar_t* message_enc);
    void OnDialogSent(uint32_t dialog_id);
private:
    DailyCapWithBonusWeek gladiator_cap;
    DailyCapWithBonusWeek champion_cap;
    DailyCapWithBonusWeek hero_cap;
    DailyCapWithBonusWeek codex_cap;

    DailyCap gladiator_box_cap;
    DailyCap champion_box_cap;
    DailyCap hero_box_cap;
    DailyCap codex_box_cap;
    DailyCap zkey_cap;

    bool expecting_zkey_message = false;

    const std::vector<DailyCap*> tracked_caps;
    GW::HookEntry hook_entry;

    std::vector<DailyCap*> make_tracked_caps();
};