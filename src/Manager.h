#pragma once

#include "Settings.h"
#include "Utils.h"

class Manager {
    RE::TESObjectREFR* player_ref = RE::PlayerCharacter::GetSingleton()->As<RE::TESObjectREFR>();
    RefID player_refid = 20;

#define ENABLE_IF_NOT_UNINSTALLED if (isUninstalled) return;

    // void HandleFakePlacement(RE::TESObjectREFR* external_cont) {
    //     ENABLE_IF_NOT_UNINSTALLED
    //     // if the external container is already handled (handled_external_conts) return
    //     if (!external_cont) return RaiseMngrErr("external_cont is null");
    //     if (!external_cont->HasContainer()) return;
    //     if (!ExternalContainerIsRegistered(external_cont->GetFormID())) return;

    //     listen_container_change = false;
	// 	if (!external_cont) return RaiseMngrErr("external_cont is null");
    //     for (auto& src : sources) {
    //         if (!Utilities::Functions::containsValue(src.data, external_cont->GetFormID())) continue;
    //         for (const auto& [chest_ref, cont_ref] : src.data) {
    //             if (external_cont->GetFormID() != cont_ref) continue;
    //             Something1(src, chest_ref, external_cont);
    //             // break yok cunku baska fakeler de external_cont un icinde olabilir
    //         }
    //     }
    //     listen_container_change = true;
    // }

#undef ENABLE_IF_NOT_UNINSTALLED

    [[nodiscard]] Source* GetSource(const FormID formid) {
        for (auto& src : sources) {
            if (src.formid == formid) {
                return &src;
            }
        }
        logger::error("Container source not found");
        return nullptr;
    };

    [[nodiscard]] const bool IsRegistered(const SourceDataVal registry) {
        if (!registry) return false;
        for (auto& src : sources) {
            for (auto& [st_inst, registered_refid] : src.data) {
                if (registered_refid == registry) return true;
            }
        }
        return false;
	}
    
    void Register(const FormID formid, Count count, RefID location_refid, std::map<unsigned int, Duration> durations=Settings::default_durations){
        // create stages for this instance
        logger::trace("Registering new instance.");
        Stages stages;
        for (const auto& [stage, duration] : durations) {
            stages.emplace_back(formid, duration, stage, Settings::default_stage_names[stage]);
        }
        const float curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        // make new registry
        SomethingWithStages food_registry(stages, curr_time, count);
        auto src = GetSource(formid);
        if (!src) {
            Source new_source(formid,"");
            new_source.data.emplace_back(food_registry, location_refid);
            sources.push_back(new_source);
            logger::trace("New source created.");
            return;
        }
        // add it to the register if src exists
        const SourceDataKeyVal new_registry(food_registry,location_refid);
        src->data.push_back(new_registry);
        return;
    }
    
    [[nodiscard]] const bool ExternalContainerIsRegistered(const RefID external_container_id) {
        if (!external_container_id) return false;
        if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(external_container_id)->HasContainer()) {
            RaiseMngrErr("External container does not have a container.");
            return false;
        }
        return IsRegistered(external_container_id);
    }

    // external container can be found in the values of src.data
    [[nodiscard]] const bool ExternalContainerIsRegistered(const FormID formid,
                                                           const RefID external_container_id) {
        if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(external_container_id)->HasContainer()) {
            RaiseMngrErr("External container does not have a container.");
            return false;
        }
        const auto src = GetSource(formid);
        if (!src) return false;
        for (const auto& [st_inst, registered_refid] : src->data) {
            if (registered_refid == external_container_id) return true;
        }
        return false;
    }
    // [[nodiscard]] const bool IsRegistered(const FormID formid) {
    //     for (const auto& src : sources) {
    //         if (src.formid == formid) return true;
    //     }
    //     return false;
    // }

    // [[nodiscard]] const bool IsRegistered(const RE::TESObjectREFR* ref) {
    //     if (!ref) return false;
    //     if (ref->IsDisabled()) return false;
    //     if (ref->IsDeleted()) return false;
    //     const auto base = ref->GetBaseObject();
    //     if (!base) return false;
    //     return IsRegistered(base->GetFormID());
    // }


    [[nodiscard]] const bool SpoilageLVLMatches(const RE::InventoryEntryData* entry, const StageNo sp_lvl){
        auto extraLists = entry->extraLists;
        for (auto& xList : *extraLists) {
            if (xList && xList->HasType(RE::ExtraDataType::kTextDisplayData)) {
                logger::trace("SpoilageLVLMatches:ExtraDataList has kTextDisplayData");
                auto text = xList->GetByType<RE::ExtraTextDisplayData>()->displayName;
                const auto stage_names = Settings::default_stage_names; // could be made customizable in the future
                for (const auto& [lvl, name] : stage_names) {
                    if (text.contains(name)) return lvl == sp_lvl;
				}
            }
        }
        return false;
    }

    void RaiseMngrErr(const std::string err_msg_ = "Error") {
        logger::error("{}", err_msg_);
        Utilities::MsgBoxesNotifs::InGame::CustomErrMsg(err_msg_);
        Utilities::MsgBoxesNotifs::InGame::GeneralErr();
        Uninstall();
    }

    void InitFailed() {
        logger::critical("Failed to initialize Manager.");
        Utilities::MsgBoxesNotifs::InGame::InitErr();
        Uninstall();
        return;
    }

    void Init() {

        bool init_failed = false;

        if (init_failed) InitFailed();

        // Load also other settings...
        // _other_settings = Settings::LoadOtherSettings();
        logger::info("Manager initialized.");
    }

    void Uninstall() {
        isUninstalled = true;
        // Uninstall other settings...
        // Settings::UninstallOtherSettings();
    }

public:
    Manager(std::vector<Source>& data) : sources(data) { Init(); };

    static Manager* GetSingleton(std::vector<Source>& data) {
        static Manager singleton(data);
        return &singleton;
    }

    // const char* GetType() override { return "Manager"; }

    std::vector<Source> sources;
    bool listen_menuclose = false;
    bool listen_activate = true;
    bool listen_container_change = true;

    std::unordered_map<std::string, bool> _other_settings;
    bool isUninstalled = false;

    [[nodiscard]] const bool IsItem(const FormID formid) {
        RE::TESForm* form = Utilities::FunctionsSkyrim::GetFormByID(formid,"");
        return Utilities::FunctionsSkyrim::IsFoodItem(form);
    }

    // is it an item that fits to our criteria
    [[nodiscard]] const bool IsItem(const RE::TESObjectREFR* ref) {
        if (!ref) return false;
        if (ref->IsDisabled()) return false;
        if (ref->IsDeleted()) return false;
        const auto base = ref->GetBaseObject();
        if (!base) return false;
        return IsItem(base->GetFormID());
    }

    void HandleDrop(const FormID formid){
        if (!IsItem(formid)) return;
        // if (IsRegistered(formid)) UnRegister();
    }

    void HandlePickUp(const FormID formid, const Count count, const RefID refid){
        if (!IsItem(formid)) return;
        if (!IsRegistered(refid)) Register(formid, count, player_refid);
        // else {
        //     // we need to find all instances with the refid
        //     // and then find all instances with the matching formid among those
        //     // and then transfer the count from the matching instance to the player_refid as new registry
        //     std::vector<SourceDataKey*> instances_candidates;
        //     for (auto& src : sources) {
        //         for (auto& [st_inst, registered_refid] : src.data) {
        //             if (registered_refid == refid) {
        //                 if (st_inst.GetCurrentStage().formid == formid && count <= st_inst.count) {
        //                     instances_candidates.push_back(&st_inst);
        //                 }
        //             }
        //         }
        //     }
        // }
        
    }

    void HandleSell(){}

    [[nodiscard]] const bool IsExternalContainer(RefID refid){
        if (!refid) return false;
        if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(refid)->HasContainer()) return false;
        return ExternalContainerIsRegistered(refid);
    }

    [[nodiscard]] const bool IsExternalContainer(const FormID formid, const RefID refid){
        if (!refid) return false;
        if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(refid)->HasContainer()) return false;
        return ExternalContainerIsRegistered(formid, refid);
    }

    void LinkExternalContainer(const FormID formid, const Count item_count, const RefID externalcontainer) {
        listen_container_change = false;

        // if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer)->HasContainer()) {
        //     return RaiseMngrErr("External container does not have a container.");
        // }

        // logger::trace("Linking external container.");
        // const auto external_ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer);
        // const auto src = GetSource(formid);
        // if (!src) return RaiseMngrErr("Source not found.");
        // Stage new_stage(formid, item_count, externalcontainer);
        // SourceDataKey new_sourcedatakey()
        // src->data[chest_refid] = externalcontainer;

        // // if external container is one of ours (bcs of weight limit):
        // if (IsChest(externalcontainer) && src->capacity > 0) {
        //     logger::info("External container is one of our unowneds.");
        //     const auto weight_limit = src->capacity;
        //     if (external_ref->GetWeightInContainer() > weight_limit) {
        //         RemoveItemReverse(external_ref, player_ref, fakecontainer,
        //                    RE::ITEM_REMOVE_REASON::kStoreInContainer);
        //         src->data[chest_refid] = chest_refid;
        //     }
        // }

        // // add it to handled_external_conts
        // //handled_external_conts.insert(externalcontainer);

        // // if successfully transferred to the external container, check if the fake container is faved
        // if (src->data[chest_refid] != chest_refid &&
        //     IsFavorited(RE::TESForm::LookupByID<RE::TESBoundObject>(fakecontainer), external_ref)) {
        //     logger::trace("Faved item successfully transferred to external container.");
        //     external_favs.push_back(fakecontainer);
        // }


        listen_container_change = true;

    }

    void UnLinkExternalContainer(const FormID formid,const Count count,const RefID externalcontainer, const RefID new_container) { 
        if (!externalcontainer) return RaiseMngrErr("External container is null.");
        if (!new_container) return RaiseMngrErr("New container is null.");
        if (!IsExternalContainer(externalcontainer)) return RaiseMngrErr("External container is not registered.");

        // I need to be able to determine the stage of the stored item

        // I need to get all instances with the externalcontainer and then find the instance with the matching formid among those
        // std::vector<SourceDataKey*> instances_candidates;
        // for (auto& src : sources) {
        //     for (auto& [st_inst, registered_refid] : src.data) {
        //         if (registered_refid == externalcontainer) {
        //             if (st_inst.GetCurrentStage().formid == formid && count <= st_inst.count) {
        //                 instances_candidates.push_back(&st_inst);
        //             }
        //         }
        //     }
        // }
        // if (instances_candidates.empty()) return RaiseMngrErr("UnLinkExternalContainer: No matching instance found.");

        // auto src = GetSource(formid);
        // if (!src) return RaiseMngrErr("Source not found.");
        // for (auto& [st_inst, registered_refid] : src->data) {
        //     if (registered_refid == externalcontainer) {
        //         registered_refid = new_container;
        //         return;
        //     }
        // }
        // // remove it from handled_external_conts
        // //handled_external_conts.erase(externalcontainer);

        // // remove it from external_favs
        // const auto it = std::find(external_favs.begin(), external_favs.end(), fake_container_formid);
        // if (it != external_favs.end()) external_favs.erase(it);

        logger::trace("Unlinked external container.");
    }

    void ReplaceSpoilageText(RE::ExtraTextDisplayData* old_text, const StageName old_name, const StageName new_name) {
        // Find the position of "Fresh"
        auto old_display_name = std::string(old_text->displayName.c_str());
        size_t pos = old_display_name.find(old_name);

        if (pos != std::string::npos) old_display_name.replace(pos, new_name.size(), new_name);
        old_text->SetName(old_display_name.c_str());
    };

    bool UpdateSpoilage(RefID loc_refid) {
        logger::trace("Manager: Updating spoilage for loc_refid {}.",loc_refid);
        bool update_took_place = false;
        const auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(loc_refid);
        if (!ref) {
            RaiseMngrErr("UpdateSpoilage: ref is null.");
            return false;
        } else if (!ref->HasContainer() && loc_refid != 20) {
			RaiseMngrErr("UpdateSpoilage: ref does not have inventory.");
			return false;
        }
       
        for (auto& src : sources) {
            auto updated_stages = src.UpdateAllStages({loc_refid});
            if (updated_stages.empty()) {
                logger::trace("No updated stages for source {}", src.GetName());
				continue;
            } 
            else update_took_place = true;
            for (auto& updated_inst : updated_stages) {
                // need to reflect the update in the game
                auto inventory = ref->GetInventory();
                const auto old_stage_no = updated_inst.current_stage - 1;
                const auto new_text = Settings::default_stage_names[updated_inst.current_stage];
                const auto old_text = Settings::default_stage_names[old_stage_no];
                logger::trace("old_stage_no: {}", old_stage_no);
                for (auto& entry : inventory) {
                    if (!entry.second.second) continue;
                    logger::trace("entry.second.second is not null");
                    if (!entry.second.second->extraLists) continue;
                    logger::trace("entry.second.second->extraLists is not null");
                    if (entry.second.second->extraLists->empty()) continue;
                    logger::trace("entry.second.second->extraLists is not empty");
                    if (!(entry.first->GetFormID() == updated_inst.stages[old_stage_no].formid &&
                          SpoilageLVLMatches(entry.second.second.get(), old_stage_no) &&
                            entry.second.first == updated_inst.count)) continue;
                    logger::trace("SpoilageLVLMatches: true");
                    //get mgeff of the food
                    logger::trace("ISENCHANTED {}", entry.second.second->IsEnchanted());
                    if (auto item_ = entry.second.second->object->As<RE::AlchemyItem>()) {
                        logger::trace("{}", item_->GetName());
                        if (auto mgeff = item_->avEffectSetting) logger::trace("{}", mgeff->GetName());
                        auto alch_item = item_->As<RE::AlchemyItem>();
                        logger::trace("Has effect:{}", alch_item->HasEffect(RE::EffectArchetype::kValueModifier));
                        logger::trace("avEffectSetting: {}", alch_item->GetAVEffect()->GetName());
                        auto spoilage_effect = RE::IFormFactory::GetConcreteFormFactoryByType<RE::EffectSetting>()->Create();
                        spoilage_effect->magicItemDescription = "Spoiled.";
                        //spoilage_effect;
                        for (auto& effect : alch_item->effects) {
							logger::trace("{}", effect->baseEffect->GetName());
                            logger::trace("description: {}", effect->baseEffect->magicItemDescription);
                            //RE::EffectSetting* effectSetting_ = RE::TESForm::LookupByID<RE::EffectSetting>(0x0003eb42);
                            RE::EffectSetting* effectSetting_ = spoilage_effect;
                            logger::trace("{}", effectSetting_->GetName());
                            effect->baseEffect = effectSetting_;
                            logger::trace("{}", effect->baseEffect->GetName());
						}
                    }
                    // update the spoilage level of the entry
                    for (auto& xList : *entry.second.second->extraLists) {
                        if (xList->HasType(RE::ExtraDataType::kTextDisplayData)) {
                            logger::trace("ExtraDataList has kTextDisplayData");
                            ReplaceSpoilageText(xList->GetByType<RE::ExtraTextDisplayData>(), old_text, new_text);
                        } 
                        else {
                            logger::trace("ExtraDataList does not have kTextDisplayData");
                            auto textDisplayData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
                            logger::trace("Setting name to Spoiled.");
                            textDisplayData->SetName((std::string(entry.first->GetName()) + " (" + new_text + ")").c_str());
                            logger::trace("Adding to ExtraDataList.");
                            xList->Add(textDisplayData);
                            logger::trace("Added to ExtraDataList.");
                        }
                    }
                }
            }
        }
        return update_took_place;
    }

};