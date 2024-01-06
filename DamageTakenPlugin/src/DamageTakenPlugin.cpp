#include "DamageTakenPlugin.h"
#include "GWCA/Managers/MapMgr.h"
#include "GWCA/GameEntities/Party.h"

#include <Windows.h>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Packets/StoC.h>

DLLAPI ToolboxPlugin* ToolboxPluginInstance()
{
    static DamageTakenPlugin instance;
    return &instance;
}

void DamageTakenPlugin::Initialize(ImGuiContext *ctx, ImGuiAllocFns allocator_fns, HMODULE toolbox_dll) {
    ToolboxUIPlugin::Initialize(ctx, allocator_fns, toolbox_dll);
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericModifier>(
            &GenericModifier_Entry,
            [this](GW::HookStatus* status, const GW::Packet::StoC::GenericModifier* packet) -> void {
                return DamagePacketCallback(status, packet);
            });
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::MapLoaded>(
            &MapLoaded_Entry,
            [this](GW::HookStatus* status, const GW::Packet::StoC::MapLoaded* packet) -> void {
                return MapLoadedCallback(status, packet);
            });
}

void DamageTakenPlugin::Terminate() {
    ToolboxUIPlugin::Terminate();
}

void DamageTakenPlugin::Draw(IDirect3DDevice9*) {
    const auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
        const int party_size = static_cast<int>(GW::PartyMgr::GetPartySize());
        for (int i = 0; i < party_size; i++) {
            ImGui::Text("%S: %u", damage_taken[i].name.c_str(), damage_taken[i].damage);
        }
    }
    ImGui::End();
}

void DamageTakenPlugin::DrawSettings() {
    ToolboxUIPlugin::DrawSettings();
}

void DamageTakenPlugin::DamagePacketCallback(GW::HookStatus *, const GW::Packet::StoC::GenericModifier *packet) {
    switch (packet->type) {
        case GW::Packet::StoC::P156_Type::damage:
        case GW::Packet::StoC::P156_Type::critical:
        case GW::Packet::StoC::P156_Type::armorignoring:
            break;
        default:
            return;
    }

    // ignore heals
    if (packet->value >= 0) {
        return;
    }

    const GW::AgentArray* agents_ptr = GW::Agents::GetAgentArray();
    if (!agents_ptr) {
        return;
    }
    auto& agents = *agents_ptr;
    if (packet->target_id >= agents.size() || !agents[packet->target_id]) {
        return;
    }
    const GW::AgentLiving* const target = agents[packet->target_id]->GetAsAgentLiving();
    if (target->allegiance != GW::Constants::Allegiance::Ally_NonAttackable) {
        return;
    }

    const GW::AgentLiving* const cause = agents[packet->cause_id]->GetAsAgentLiving();

    const auto target_it = party_index.find(target->agent_id);
    if (target_it == party_index.end()) {
        return; // Target not in party
    }
    const size_t index = target_it->second;

    if(damage_taken[index].damage == 0) {
        GW::Agents::AsyncGetAgentName(target, damage_taken[index].name);
    }

    if (packet->cause_id >= agents.size() || !agents[packet->cause_id]) {
        return;
    }
    if(cause->allegiance == GW::Constants::Allegiance::Ally_NonAttackable) {
        // Don't count damage inflicted by self or allies, only enemies
        return;
    }

    long ldmg = 0;
    if (target->max_hp > 0 && target->max_hp < 100000) {
        ldmg = std::lround(-packet->value * target->max_hp);
        hp_map[target->player_number] = target->max_hp;
    }
    else {
        const auto it = hp_map.find(target->player_number);
        if (it == hp_map.end()) {
            // max hp not found, approximate with hp/lvl formula
            ldmg = std::lround(-packet->value * (target->level * 20 + 100));
        }
        else {
            // size_t maxhp = it->second;
            ldmg = std::lround(-packet->value * it->second);
        }
    }

    damage_taken[index].damage += static_cast<uint32_t>(ldmg);
}

void DamageTakenPlugin::MapLoadedCallback(GW::HookStatus *, const GW::Packet::StoC::MapLoaded *) {
    switch (GW::Map::GetInstanceType()) {
        case GW::Constants::InstanceType::Outpost:
            in_explorable = false;
            break;
        case GW::Constants::InstanceType::Explorable:
            party_index.clear();
            hp_map.clear();
            if (!in_explorable) {
                in_explorable = true;
                for (auto & i : damage_taken) {
                    i.Reset();
                }
            }
            break;
        case GW::Constants::InstanceType::Loading:
        default:
            break;
    }
}

void DamageTakenPlugin::Update(float d) {
    ToolboxPlugin::Update(d);

    if (party_index.empty()) {
        CreatePartyIndexMap();
    }
}

void DamageTakenPlugin::CreatePartyIndexMap() {
    if (!GW::PartyMgr::GetIsPartyLoaded()) {
        return;
    }
    const GW::PartyInfo* const info = GW::PartyMgr::GetPartyInfo();
    size_t index = 0;
    for (const GW::PlayerPartyMember& player : info->players) {
        const uint32_t id = GW::Agents::GetAgentIdByLoginNumber(player.login_number);
        party_index[id] = index++;

        for (const GW::HeroPartyMember& hero : info->heroes) {
            if (hero.owner_player_id == player.login_number) {
                party_index[hero.agent_id] = index++;
            }
        }
    }
    for (const GW::HenchmanPartyMember& hench : info->henchmen) {
        party_index[hench.agent_id] = index++;
    }
}
