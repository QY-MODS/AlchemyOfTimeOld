#pragma once
#include "SimpleIni.h"
#include "Utils.h"

using namespace Utilities::Types;


struct Source {
    // TODO: reconsider consts here
    FormID formid=0;
    std::string editorid="";
    std::vector<Stage> stages; // spoilage stages
    SourceData data;

    bool init_failed = false;

    Source(const FormID id, const std::string id_str)
        : formid(id), editorid(id_str) {
        if (!formid) {
            auto form = RE::TESForm::LookupByEditorID<RE::TESForm>(editorid);
            if (form) {
                logger::trace("Found formid for editorid {}", editorid);
                formid = form->GetFormID();
            } else logger::error("Could not find formid for editorid {}", editorid);
        }

        RE::TESForm* form = Utilities::FunctionsSkyrim::GetFormByID(formid, editorid);
        auto bound_ = GetBoundObject();
        if (!form || !bound_) {
            InitFailed();
            return;
        }
        
        if (!Utilities::FunctionsSkyrim::IsFoodItem(form)){
            InitFailed();
            return;
        }
    };
    
    const std::string_view GetName() {
        auto form = Utilities::FunctionsSkyrim::GetFormByID(formid, editorid);
        if (form) return form->GetName();
        else return "";
    };

    RE::TESBoundObject* GetBoundObject() {
        return Utilities::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid, editorid);
    };

    std::vector<SourceDataKey> UpdateAllStages(const std::vector<RefID>& filter = {}){
        logger::trace("Updating all stages");
        // save the updated instances
        std::vector<SourceDataKey> updated_instances;
        for (auto& instance_refid : data) {
            // if the refid is not in the filter, skip
            if (!filter.empty() && std::find(filter.begin(), filter.end(), instance_refid.refid) == filter.end()) continue;
            if (_UpdateStage(instance_refid)) updated_instances.push_back(instance_refid.instance);
        }
        return updated_instances;
    }

private:
    [[nodiscard]] const bool _UpdateStage(SourceDataKeyVal& st_inst_ref){
        bool updated = st_inst_ref.instance.Update();
        if (st_inst_ref.instance.IsAtLastStage()) {
            auto it = std::find(data.begin(), data.end(), st_inst_ref);
            if (it != data.end()) data.erase(it);
        }
        return updated;
    }
    void InitFailed(){
        logger::error("Initialisation failed.");
        formid = 0;
        editorid = "";
        stages.clear();
        data.clear();
        init_failed = true;
    }
};


namespace Settings {
    std::map<StageNo, Duration> default_durations = {{0,0.01f},{1,12.f},{2,6.f},{3,120.f},{4,0.f}}; // 0:Fresh, 1:Stale, 2:Spoiled, 3:Rotten, 4:Decayed
    std::map<StageNo, StageName> default_stage_names = {
        {0, "Fresh"}, {1, "Stale"}, {2, "Spoiled"}, {3, "Rotten"}, {4, "Decayed"}};
    const FormID rot_scale_formid = 0x0100306D; // 0x0100306d: "Rot Scale"
}