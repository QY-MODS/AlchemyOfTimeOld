#pragma once
#include "SKSEMenuFramework.h"
#include "Manager.h"

namespace UI {
    struct Instance {
        StageNo stage_number;
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

    inline static std::string item_current = "##current";
    inline static std::string sub_item_current = "##item"; 
    inline static bool is_list_box_focused = false;
    static ImGuiTextFilter filter;
    static ImGuiTextFilter filter2;

    Manager* M;
    void __stdcall Render();
    void Register(Manager* manager) {
        SKSEMenuFramework::SetSection("Alchemy Of Time");
        SKSEMenuFramework::AddSectionItem("Print", Render);
        M = manager;
    }
    void __stdcall Render() {

        if (!M) {
            ImGui::Text("Not available");
            return;
        }

        if (ImGui::Button("Generate")) {
            auto& sources = M->GetSources();

            locations.clear();

            for (const auto& source : sources) {
                for (const auto& [location, instances] : source.data) {
                    const auto* locationReference = RE::TESForm::LookupByID<RE::TESObjectREFR>(location);
                    const char* locationName = locationReference ? locationReference->GetName() : (std::format("{:x}", location).c_str());

                    for (auto& stageInstance : instances) {
                        const auto* delayerForm = RE::TESForm::LookupByID(stageInstance.GetDelayerFormID());
                        Instance instance(
                            stageInstance.no,
                            stageInstance.count,
                            stageInstance.start_time,
                            source.GetStageDuration(stageInstance.no),
                            stageInstance.GetDelayMagnitude(),
                            delayerForm ? delayerForm->GetName()
                                        : std::format("{:x}", stageInstance.GetDelayerFormID()),
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

                static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
                if (ImGui::BeginTable("table1", 9, flags)) {
                    ImGui::TableSetupColumn("Stage Number");
                    ImGui::TableSetupColumn("Count");
                    ImGui::TableSetupColumn("Start Time");
                    ImGui::TableSetupColumn("Duration");
                    ImGui::TableSetupColumn("Delay Magnitude");
                    ImGui::TableSetupColumn("Delayer");
                    ImGui::TableSetupColumn("Fake");
                    ImGui::TableSetupColumn("Transforming");
                    ImGui::TableSetupColumn("Decayed");
                    ImGui::TableHeadersRow();
                    for (const auto& item : selectedInstances) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.stage_number).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.count).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.start_time).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.duration).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(std::format("{}", item.delay_magnitude).c_str());
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(item.delayer.c_str());
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