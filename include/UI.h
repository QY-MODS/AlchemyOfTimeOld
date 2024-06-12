#pragma once
#include "SKSEMenuFramework.h"
#include "Manager.h"


static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

namespace UI {
    struct Instance {
        std::pair<StageNo,StageNo> stage_number;
        std::string stage_name;
        Count count;
        float start_time;
        Duration duration;
        float delay_magnitude;
        std::string delayer;
        bool is_fake;
        bool is_transforming;
        bool is_decayed;
    };
    using InstanceMap = std::map<std::string, std::vector<Instance>>;
    using LocationMap = std::map<std::string, InstanceMap>;

    LocationMap locations;

    std::string last_generated = "";
    std::string item_current = "##current";
    std::string sub_item_current = "##item"; 
    bool is_list_box_focused = false;
    ImGuiTextFilter filter;
    ImGuiTextFilter filter2;
    ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

    Manager* M;
    void __stdcall RenderSettings();
    void __stdcall RenderStatus();
    void __stdcall RenderInspect();
    void Register(Manager* manager) {
        SKSEMenuFramework::SetSection(Utilities::mod_name);
        SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
        SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
        SKSEMenuFramework::AddSectionItem("Inspect", RenderInspect);
        M = manager;
    }

    void __stdcall RenderSettings(){

        for (const auto& [section_name, section_settings] : Settings::INI_settings) {
            if (ImGui::CollapsingHeader(section_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::BeginTable("table_settings", 2, table_flags)) {
                    for (const auto& [setting_name, setting] : section_settings) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        if (setting_name == "DisableWarnings") {
                            const auto previous_state = Settings::disable_warnings;
                            ImGui::Checkbox(setting_name.c_str(), &Settings::disable_warnings);
                            if (Settings::disable_warnings != previous_state) {
                                // save to INI
                                Settings::INI_settings[section_name][setting_name] = Settings::disable_warnings;
                                CSimpleIniA ini;
                                ini.SetUnicode();
                                ini.LoadFile(Settings::INI_path);
                                ini.SetBoolValue(section_name.c_str(), setting_name.c_str(), Settings::disable_warnings);
                                ini.SaveFile(Settings::INI_path);
                            }
                            ImGui::SameLine();
                            HelpMarker("Disables in-game warning pop-ups.");
                        } else {
                            // we just want to display the settings in read only mode
                            ImGui::Text(setting_name.c_str());
                        }
                        ImGui::TableNextColumn();
                        const auto temp_setting_val = setting_name == "DisableWarnings" ? Settings::disable_warnings  : setting;
                        const char* value = temp_setting_val ? "Enabled" : "Disabled";
                        const auto color = setting ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
                        ImGui::TextColored(color, value);
                    }
                    ImGui::EndTable();
                }
            }
        }
    };

    void __stdcall RenderStatus() {
        const auto color_operational = ImVec4(0, 1, 0, 1);
        const auto color_not_operational = ImVec4(1, 0, 0, 1);

        if (ImGui::BeginTable("table_status", 3, table_flags)) {
            ImGui::TableSetupColumn("Module");
            ImGui::TableSetupColumn("Default Preset");
            ImGui::TableSetupColumn("Custom Presets");
            ImGui::TableHeadersRow();
            for (const auto& [module_name,module_enabled] : Settings::INI_settings["Modules"]) {
                if (!module_enabled) continue;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(module_name.c_str());
                ImGui::TableNextColumn();
                const auto loaded_default = Settings::defaultsettings[module_name].IsHealthy();
                const auto loaded_default_str = loaded_default ? "Loaded" : "Not Loaded";
                const auto color = loaded_default ? color_operational : color_not_operational;
                ImGui::TextColored(color, loaded_default_str);
                ImGui::TableNextColumn();
                const auto& custom_presets = Settings::custom_settings[module_name];
                
				std::string loaded_custom_str = "Not Loaded";
				auto color_custom = color_not_operational;
                if (custom_presets.size() > 0) {
                    loaded_custom_str = std::format("Loaded ({})", custom_presets.size());
                    color_custom = color_operational;
				}
                ImGui::TextColored(color_custom, loaded_custom_str.c_str());
            }
            ImGui::EndTable();
        }
    };

    void __stdcall RenderInspect() {

        if (!M) {
            ImGui::Text("Not available");
            return;
        }

        FontAwesome::PushSolid();
        if (ImGui::Button((FontAwesome::UnicodeToUtf8(0xf021) + " Refresh").c_str())) {
            last_generated = std::format("{} (in-game hours)", RE::Calendar::GetSingleton()->GetHoursPassed());
            const auto& sources = M->GetSources();

            locations.clear();

            for (const auto& source : sources) {
                for (const auto& [location, instances] : source.data) {
                    const auto* locationReference = RE::TESForm::LookupByID<RE::TESObjectREFR>(location);
                    const char* locationName = locationReference ? locationReference->GetName() : (std::format("{:x}", location).c_str());

                    for (auto& stageInstance : instances) {
                        const auto* delayerForm = RE::TESForm::LookupByID(stageInstance.GetDelayerFormID());
                        auto delayer_name = delayerForm ? delayerForm->GetName() : std::format("{:x}", stageInstance.GetDelayerFormID());
                        if (delayer_name == "0") delayer_name = "None";
                        int max_stage_no = 0;
                        while (source.IsStageNo(max_stage_no+1)) max_stage_no++;
                        
                        const auto temp_stage_no = std::make_pair(stageInstance.no, max_stage_no);
                        auto temp_stagename = source.GetStageName(stageInstance.no);
                        temp_stagename = temp_stagename.empty() ? "" : std::format("({})", temp_stagename);
                        Instance instance(
                            temp_stage_no, 
                            temp_stagename,
                            stageInstance.count,
                            stageInstance.start_time,
                            source.GetStageDuration(stageInstance.no),
                            stageInstance.GetDelayMagnitude(),
                            delayer_name,
                            stageInstance.xtra.is_fake,
                            stageInstance.xtra.is_transforming,
                            stageInstance.xtra.is_decayed
                        );

                        const auto item = RE::TESForm::LookupByID(source.formid);
                        locations[std::string(locationName) + "##location"][(item ? item->GetName() : source.editorid)+ "##item"]
                            .push_back(
                            instance);
                    }
                }
            }

            const auto current = locations.find(item_current);
            if (current == locations.end()){
                item_current = "##current";
                sub_item_current = "##item"; 
            } else if (const auto item = current->second; item.find(sub_item_current) == item.end()) {
                sub_item_current = "##item"; 
            }
        }
        FontAwesome::Pop();
        
        ImGui::SameLine();
        ImGui::Text(("Last Generated: " + last_generated).c_str());

        ImGui::Text("Location");
        if (ImGui::BeginCombo("##combo 1", item_current.c_str())) {
            for (const auto & [key, value] : locations) {
                if (filter.PassFilter(key.c_str())) {
                    const bool is_selected = item_current == key;
                    if (ImGui::Selectable(key.c_str(), is_selected)) {
                        item_current = key;
                    }
                }
            }
            ImGui::EndCombo();
        }
        is_list_box_focused = ImGui::IsItemHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_NoNavOverride);
        ImGui::SameLine();
        filter.Draw("#filter1");

        is_list_box_focused = is_list_box_focused || ImGui::IsItemActive();

        if (locations.find(item_current) != locations.end()) 
        {
            InstanceMap& selectedItem = locations[item_current];
            ImGui::Text("Item");
            if (ImGui::BeginCombo("##combo 2", sub_item_current.c_str())) {
                for (const auto& [key, value] : selectedItem) {
                    if (filter2.PassFilter(key.c_str())) {
                        const bool is_selected = (sub_item_current == key);
                        if (ImGui::Selectable(key.c_str(), is_selected)) {
                            sub_item_current = key;
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            filter2.Draw("#filter2");

            if (selectedItem.find(sub_item_current) != selectedItem.end()) {

                auto & selectedInstances = selectedItem[sub_item_current];

                ImGui::Text("Instances");
                if (ImGui::BeginTable("table_inspect", 8, table_flags)) {
                    ImGui::TableSetupColumn("Stage");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableSetupColumn("Start Time");
                    ImGui::TableSetupColumn("Duration");
                    ImGui::TableSetupColumn("Time Modulation");
                    ImGui::TableSetupColumn("Dynamic Form");
                    ImGui::TableSetupColumn("Transforming");
                    ImGui::TableSetupColumn("Decayed");
                    ImGui::TableHeadersRow();
                    for (const auto& item : selectedInstances) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}/{} {}", item.stage_number.first, item.stage_number.second,item.stage_name).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.count).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.start_time).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.duration).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{} ({})", item.delay_magnitude, item.delayer).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(item.is_fake? "Yes" : "No");
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(item.is_transforming ? "Yes" : "No");
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(item.is_decayed ? "Yes" : "No");
                    }
                    ImGui::EndTable();
                }
            }
        }

    }

}