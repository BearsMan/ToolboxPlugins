#pragma once

#include <ToolboxUIPlugin.h>

#include <GWCA/Packets/StoC.h>
#include <GWCA/Utilities/Hook.h>

struct DamageTaken {
    std::wstring name;
    uint32_t damage = 0;

    void Reset()
    {
        name = L"";
        damage = 0;
    }
};

class DamageTakenPlugin : public ToolboxUIPlugin {
public:
    [[nodiscard]] const char* Name() const override { return "Damage Taken Plugin"; }
    [[nodiscard]] const char* Icon() const override { return "\xef\x81\x83"	; } // U+f043 ()
    [[nodiscard]] bool HasSettings() const override { return true; }

    void Initialize(ImGuiContext* ctx, ImGuiAllocFns allocator_fns, HMODULE toolbox_dll) override;
    void Terminate() override;
    void Draw(IDirect3DDevice9*) override;
    void DrawSettings() override;

    void Update(float d) override;

private:
    bool in_explorable = false;

    void CreatePartyIndexMap();
    void DamagePacketCallback(GW::HookStatus*, const GW::Packet::StoC::GenericModifier* packet);
    void MapLoadedCallback(GW::HookStatus*, const GW::Packet::StoC::MapLoaded* packet);

    DamageTaken damage_taken[12];
    std::map<DWORD, uint32_t> hp_map{};
    std::map<DWORD, size_t> party_index{};

    GW::HookEntry GenericModifier_Entry;
    GW::HookEntry MapLoaded_Entry;
};