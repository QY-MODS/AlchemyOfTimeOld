#pragma once

#include "Settings.h"
#include "Utils.h"

class Manager {
    
    RE::TESObjectREFR* player_ref = RE::PlayerCharacter::GetSingleton()->As<RE::TESObjectREFR>();
    const RefID player_refid = 20;
    RE::EffectSetting* empty_mgeff;
    
    
    bool worldobjectsspoil;
    std::vector<std::string> exlude_list;

    // 0x0003eb42 damage health

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

    [[nodiscard]] Source* GetSource(const FormID real_formid) {
        for (auto& src : sources) {
            if (src.formid == real_formid) {
                return &src;
            }
        }
        logger::warn("Container source not found");
        return nullptr;
    };

    // USE IT ONLY FOR REFS IN THE WORLD
    [[nodiscard]] std::vector<StageInstance*> GetRefSourceData(const RefID refid) {
        std::vector<StageInstance*> instances_candidates;
        bool source_found = false;
        for (auto& src : sources) {
            for (auto& st_inst : src.data) {
				if (st_inst.location == refid) {
                    instances_candidates.push_back(&st_inst);
                    source_found = true;
				}
			}
            if (source_found) return instances_candidates;
        }
        logger::warn("Container source not found");
        return instances_candidates;
    };

    [[nodiscard]] Source* GetStageSource(const FormID fake_formid) {
        if (!fake_formid) return nullptr;
        for (auto& src : sources) {
            for (auto& stage : src.stages) {
                if (stage.second.formid == fake_formid) {
                    return &src;
                }
            }
        }
        return nullptr;
    };
    

    [[nodiscard]] const bool ExternalContainerIsRegistered(const FormID fake_formid,
                                                           const RefID external_container_id) {
        const auto src = GetSource(fake_formid);
        if (!src) return false;
        for (const auto& st_inst: src->data) {
            if (st_inst.location == external_container_id) return true;
        }
        return false;
    }

    void UpdateSpoilageInInventory(RE::TESObjectREFR* inventory_owner, Count update_count, FormID old_item, FormID new_item){
        if (!inventory_owner) return RaiseMngrErr("Inventory owner is null.");
        logger::trace("Updating spoilage in inventory. Count {} , Old item {} , New item {}", update_count, old_item, new_item);

        setListenContainerChange(false);
        auto inventory = inventory_owner->GetInventory();
        auto entry = inventory.find(RE::TESForm::LookupByID<RE::TESBoundObject>(old_item));
        if (entry == inventory.end()) logger::error("Item not found in inventory.");
        else inventory_owner->RemoveItem(entry->first, std::min(update_count,entry->second.first), RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
        inventory_owner->AddObjectToContainer(RE::TESForm::LookupByID<RE::TESBoundObject>(new_item), nullptr, update_count, nullptr);
        logger::trace("Spoilage updated in inventory.");
        setListenContainerChange(true);
    }

    void UpdateSpoilageInWorld(RE::TESObjectREFR* ref, FormID formid) {
        if (!ref) return RaiseMngrErr("Ref is null.");
        /*if (ref->extraList.HasType<RE::ExtraTextDisplayData>()) {
            ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
        }*/
        auto fake_form = RE::TESForm::LookupByID(formid);
        if (!fake_form) return RaiseMngrErr("Fake form not found.");
        //ref->SetObjectReference(static_cast<RE::TESBoundObject*>(fake_form)); //po3 papyrustweaks
        
        //logger::trace("Setting ObjectReference to fake form.");
        //Looked up here (wSkeever): https:  // www.nexusmods.com/skyrimspecialedition/mods/73607
        /*float afX = 100;
        float afY = 100;
        float afZ = 100;
        float afMagnitude = 100;*/
        /*auto args = RE::MakeFunctionArguments(std::move(afX), std::move(afY), std::move(afZ), std::move(afMagnitude));
        vm->DispatchMethodCall(object, "ApplyHavokImpulse", args, callback);*/
        SKSE::GetTaskInterface()->AddTask(
            [ref, fake_form]() { 
                ref->SetObjectReference(static_cast<RE::TESBoundObject*>(fake_form)); 

            //auto player_ch = RE::PlayerCharacter::GetSingleton();
            //player_ch->StartGrabObject();
            ref->Disable();
            ref->Enable(false);
            auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            auto policy = vm->GetObjectHandlePolicy();
            auto handle = policy->GetHandleForObject(ref->GetFormType(), ref);
            RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
            vm->CreateObject2("ObjectReference", object);
            vm->BindObject(object, handle, false);
            if (!object) logger::warn("Object is null");
            else logger::trace("FUSRODAH");
            auto args = RE::MakeFunctionArguments(std::move(0.f), std::move(0.f), std::move(0.f), std::move(0.f));
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
            vm->DispatchMethodCall(object, "ApplyHavokImpulse", args, callback);
		});
	}

    const RE::ObjectRefHandle RemoveItemReverse(RE::TESObjectREFR* moveFrom, RE::TESObjectREFR* moveTo, FormID item_id, Count count,
                                                RE::ITEM_REMOVE_REASON reason) {
        logger::trace("RemoveItemReverse");

        auto ref_handle = RE::ObjectRefHandle();

        if (!moveFrom && !moveTo) {
            RaiseMngrErr("moveFrom and moveTo are both null!");
            return ref_handle;
        }
        logger::trace("Removing item reverse");

        setListenContainerChange(false);

        auto inventory = moveFrom->GetInventory();
        for (auto item = inventory.rbegin(); item != inventory.rend(); ++item) {
            auto item_obj = item->first;
            if (!item_obj) RaiseMngrErr("Item object is null");
            if (item_obj->GetFormID() == item_id) {
                auto inv_data = item->second.second.get();
                if (!inv_data) RaiseMngrErr("Item data is null");
                auto asd = inv_data->extraLists;
                if (!asd || asd->empty()) {
                    ref_handle = moveFrom->RemoveItem(item_obj, count, reason, nullptr, moveTo);
                } else {
                    ref_handle = moveFrom->RemoveItem(item_obj, count, reason, asd->front(), moveTo);
                }
                setListenContainerChange(true);
                return ref_handle;
            }
        }
        setListenContainerChange(true);
        return ref_handle;
    }

    [[nodiscard]] const bool PickUpItem(RE::TESObjectREFR* item, Count count,const unsigned int max_try = 3) {
        logger::trace("PickUpItem");

        // std::lock_guard<std::mutex> lock(mutex);
        if (!item) {
            logger::warn("Item is null");
            return false;
        }
        if (!player_ref) {
            logger::warn("Actor is null");
            return false;
        }

        setListenContainerChange(false);

        auto item_bound = item->GetBaseObject();
        const auto item_count = Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref);
        logger::trace("Item count: {}", item_count);
        unsigned int i = 0;
        if (!item_bound) {
            logger::warn("Item bound is null");
            return false;
        }
        while (i < max_try) {
            logger::trace("Critical: PickUpItem");
            RE::PlayerCharacter::GetSingleton()->PickUpObject(item, count, false, false);
            logger::trace("Item picked up. Checking if it is in inventory...");
            if (Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref) > item_count) {
                if (i) logger::warn("Item picked up. Took {} extra tries.", i);
                setListenContainerChange(true);
                return true;
            } else
                logger::trace("item count: {}", Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref));
            i++;
        }

        setListenContainerChange(true);
        return false;
    }

    [[nodiscard]] const bool IsInExclude(const FormID formid) {
        auto form = RE::TESForm::LookupByID(formid);
        if (!form) {
            logger::warn("Form not found.");
            return false;
        }
        std::string form_string = std::string(form->GetName());
        if (Utilities::Functions::includesString(form_string, exlude_list)) return true;
		return false;
    }

    void RaiseMngrErr(const std::string err_msg_ = "Error") {
        logger::critical("{}", err_msg_);
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

        empty_mgeff = RE::IFormFactory::GetConcreteFormFactoryByType<RE::EffectSetting>()->Create();
        empty_mgeff->magicItemDescription = std::string(" ");

        if (init_failed) InitFailed();

        worldobjectsspoil = true;
        exlude_list = Settings::LoadExcludeList();

        // Load also other settings...
        // _other_settings = Settings::LoadOtherSettings();
        logger::info("Manager initialized.");
    }

    void Uninstall() {
        isUninstalled = true;
        // Uninstall other settings...
        // Settings::UninstallOtherSettings();
    }

    void setListenActivate(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        listen_activate = value;
    }

    void setUninstalled(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        isUninstalled = value;
    }

public:
    Manager(std::vector<Source>& data) : sources(data) { Init(); };

    static Manager* GetSingleton(std::vector<Source>& data) {
        static Manager singleton(data);
        return &singleton;
    }

    // const char* GetType() override { return "Manager"; }

    std::vector<Source> sources;
    bool listen_activate = true;
    bool listen_container_change = true;
    bool listen_menuopenclose = true;

    std::unordered_map<std::string, bool> _other_settings;
    bool isUninstalled = false;

    std::mutex mutex;

    void setListenMenuOpenClose(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        listen_menuopenclose = value;
    }

    [[nodiscard]] bool getListenMenuOpenClose() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return listen_menuopenclose;
    }


    [[nodiscard]] bool getListenActivate() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return listen_activate;
    }

    [[nodiscard]] bool getListenContainerChange() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return listen_container_change;
    }

    void setListenContainerChange(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        listen_container_change = value;
    }

    [[nodiscard]] bool getUninstalled() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return isUninstalled;
    }

    [[nodiscard]] const bool IsItem(const FormID formid) {
        if (Utilities::FunctionsSkyrim::IsFoodItem(formid) && !IsInExclude(formid)) return true;
        return false;
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

    [[nodiscard]] const bool IsFake(const RE::TESObjectREFR* ref) {
        if (!ref) return false;
        const auto base = ref->GetBaseObject();
        if (!base) return false;
        const auto formid = base->GetFormID();
        if (!IsItem(formid)) return false;
        return IsFake(formid);
    }

	[[nodiscard]] const bool IsFake(const FormID formid) {
        if (!IsItem(formid)) return false;
		for (const auto& src : sources) {   
            for (const auto& stage : src.stages) {
                if (stage.second.formid == formid) return true;
			}
        }
        return false;
    }

    [[nodiscard]] const bool IsRegistered(const FormID fakeid) {
        if (!fakeid) return false;
        for (auto& src : sources) {
            for (auto& stage : src.stages) {
				if (stage.second.formid == fakeid) return true;
			}
        }
        return false;
    }

    [[nodiscard]] const bool RefIsRegistered(const RefID refid) {
        if (!refid) return false;
        for (auto& src : sources) {
            for (auto& st_inst : src.data) {
                if (st_inst.location == refid) return true;
            }
        }
        return false;
    }

    void Register(const FormID formid, Count count, RefID location_refid) {
        // create stages for this instance
        logger::trace("Registering new instance.");
        if (!IsItem(formid)) return RaiseMngrErr("Not an item.");
        if (!location_refid) return RaiseMngrErr("Location refid is null.");
        const float curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        // make new registry
        auto src = GetSource(formid);
        if (!src) {
            Source new_source(formid, "", empty_mgeff);
            new_source.data.emplace_back(curr_time, 0, count, location_refid);
            sources.push_back(new_source);
            logger::trace("New source created.");
            src = &sources.back();
        } else src->data.emplace_back(curr_time, 0, count, location_refid);

        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(location_refid);
        if (!ref) return RaiseMngrErr("Ref is null.");
        if (ref->HasContainer() || location_refid == 20) {
            logger::trace("Registering in inventory.");
            auto new_formid = src->stages[0].formid;
            UpdateSpoilageInInventory(ref, count, formid, new_formid);
        }
    }

    // For stuff outside
    void Register(const FormID formid, Count count, RE::TESObjectREFR* ref) {
        if (!worldobjectsspoil) return;
        if (!ref) return RaiseMngrErr("Ref is null.");
        // create stages for this instance
        Register(formid, count, ref->GetFormID());

        auto src = GetSource(formid);
        UpdateSpoilageInWorld(ref, src->stages[0].formid);
        /*auto textDisplayData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
        textDisplayData->SetName((std::string(ref->GetDisplayFullName()) + " (Fresh)").c_str());
        ref->extraList.Add(textDisplayData);*/
    }


    void HandleDrop(const FormID formid, Count count, RE::TESObjectREFR* ref){
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleDrop: Formid {} , Count {}", formid, count);
        if (!ref) return RaiseMngrErr("Ref is null.");
        if (!IsFake(formid)) return;
        if (!IsRegistered(formid)) return;
        auto source = GetStageSource(formid);
        if (!source) {
            logger::critical("HandleDrop: Source not found.");
            return;
        }
        int stage_no = -1;
        for (const auto& [st_no, stage] : source->stages) {
			if (stage.formid == formid) {
                stage_no = st_no;
				break;
			}
		}
        if (stage_no == -1) return RaiseMngrErr("HandleDrop: Stage not found.");
        std::vector<StageInstance*> instances_candidates;
        for (auto& st_inst : source->data) {
            if (static_cast<int>(st_inst.no) != stage_no) continue;
            if (st_inst.location == 20) instances_candidates.push_back(&st_inst);
		}
        // need to now order the instances_candidates by their start_time
        std::sort(instances_candidates.begin(), instances_candidates.end(), [](StageInstance* a, StageInstance* b) {
			return a->start_time < b->start_time;
		});

        if (worldobjectsspoil) {
            auto player_ch = RE::PlayerCharacter::GetSingleton();
            if (!PickUpItem(ref, count,1)) return RaiseMngrErr("HandleDrop: Item not picked up.");
            auto fake_bound = RE::TESForm::LookupByID<RE::TESBoundObject>(formid);
            for (const auto& instance : instances_candidates) {
                if (count <= instance->count) {
				    instance->count -= count;
                    auto ref_ = RemoveItemReverse(player_ch, nullptr, formid, count, RE::ITEM_REMOVE_REASON::kDropping);
                    if (!ref_) return RaiseMngrErr("HandleDrop: Item not removed ref null.");
                    source->data.emplace_back(instance->start_time, instance->no, count, ref_.get()->GetFormID());
                    return;
			    }
			    else {
				    count -= instance->count;
                    auto ref_ = player_ch->RemoveItem(fake_bound,instance->count,RE::ITEM_REMOVE_REASON::kDropping,nullptr,nullptr);
                    if (!ref_) return RaiseMngrErr("HandleDrop: Item not removed ref null.");
                    instance->location = ref_.get()->GetFormID();
			    }
            }
        } 
        else {
            for (const auto& instance : instances_candidates) {
                if (count <= instance->count) {
                    instance->count -= count;
                    source->data.emplace_back(instance->start_time, instance->no, count, ref->GetFormID());
                    return;
                } else {
                    count -= instance->count;
                    instance->location = ref->GetFormID();
                }
            }
        }
        source->CleanUpData();
    }

    void HandlePickUp(const FormID formid, const Count count, const RefID refid) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::info("HandlePickUp: Formid {} , Count {} , Refid {}", formid, count, refid);
        if (!IsFake(formid)) {
            logger::warn("HandlePickUp: Not a fake.");
            return;
        }
        if (!RefIsRegistered(refid)) {
            if (worldobjectsspoil){
                logger::warn("HandlePickUp: Not registered.");
                return;
            }
            return Register(formid,count,20);
        }
        for (auto st_inst: GetRefSourceData(refid)) {
            st_inst->location = 20;
        }
    }

    void HandleConsume(const FormID fake_formid, Count count) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleConsume");
        if (!IsFake(fake_formid)) {
            logger::warn("HandleConsume: Not a fake item.");
            return;
        }

        auto source = GetStageSource(fake_formid);
        // check if player has the fake item
        // sometimes player does not have the fake item but it can still be there with count = 0.
        const auto fake_obj = RE::TESForm::LookupByID<RE::TESBoundObject>(fake_formid);
        if (!fake_obj) return RaiseMngrErr("Fake object not found.");

        int registered_count = 0;
        for (const auto& st_inst : source->data) {
            if (st_inst.location == 20) registered_count += st_inst.count;
		}
        int diff = registered_count - static_cast<int>(count);
        if (diff < 0) {
			logger::warn("HandleConsume: something could have gone wrong with registration");
			return;
		}
        bool adjust_registered_count = false;
        auto inventory = player_ref->GetInventory();
        auto entry = inventory.find(fake_obj);
        if (entry != inventory.end()) {
            if (entry->second.first == registered_count){
                logger::warn("HandleConsume: Seems like false alarm");
            } 
            else if (entry->second.first == diff) {
                logger::trace("HandleConsume: Inventory has expected count");
                adjust_registered_count = true;
            }
        } else {
			logger::trace("HandleConsume: Item entry not found in inventory");
			adjust_registered_count = true;
		}
        if (!adjust_registered_count) return;
        
        logger::trace("HandleConsume: Adjusting registered count");

        int stage_no = -1;
        for (const auto& [st_no, stage] : source->stages) {
            if (stage.formid == fake_formid) {
                stage_no = st_no;
                break;
            }
        }
        if (stage_no == -1) return RaiseMngrErr("HandleConsume: Stage not found.");

        std::vector<StageInstance*> instances_candidates;
        for (auto& st_inst : source->data) {
            if (static_cast<int>(st_inst.no) != stage_no) continue;
            if (st_inst.location == 20) instances_candidates.push_back(&st_inst);
        }
        std::sort(instances_candidates.begin(), instances_candidates.end(),
                  [](StageInstance* a, StageInstance* b) { return a->start_time < b->start_time; });

        for (const auto& instance : instances_candidates) {
			if (count <= instance->count) {
				instance->count -= count;
				return;
			} else {
				count -= instance->count;
				instance->location = 20;
			}
		}
        logger::trace("HandleConsume: updated.");
    }

    void HandleSell(){}

    [[nodiscard]] const bool IsExternalContainer(FormID fake_id,RefID refid){
        if (!refid) return false;
        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(refid);
        if (!ref) return false;
        if (!ref->HasContainer()) return false;
        return ExternalContainerIsRegistered(fake_id,refid);
    }

    void LinkExternalContainer(const FormID formid, Count item_count, const RefID externalcontainer) {

         if (!RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer)->HasContainer()) {
             logger::warn("External container does not have a container.");
             return;
         }

         logger::trace("Linking external container.");
         const auto external_ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer);
         if (!external_ref) return RaiseMngrErr("External ref is null.");
         const auto source = GetStageSource(formid);
         if (!source) return RaiseMngrErr("Source not found.");
         int stage_no = -1;
         for (const auto& [st_no, stage] : source->stages) {
             if (stage.formid == formid) {
                 stage_no = st_no;
                 break;
             }
         }
         if (stage_no == -1) return RaiseMngrErr("HandleDrop: Stage not found.");
         std::vector<StageInstance*> instances_candidates;
         for (auto& st_inst : source->data) {
             if (static_cast<int>(st_inst.no) != stage_no) continue;
             if (st_inst.location == 20) instances_candidates.push_back(&st_inst);
         }
         // need to now order the instances_candidates by their start_time
         std::sort(instances_candidates.begin(), instances_candidates.end(),
                   [](StageInstance* a, StageInstance* b) { return a->start_time > b->start_time; });

         for (const auto& instance : instances_candidates) {
             if (item_count <= instance->count) {
                 instance->count -= item_count;
                 source->data.emplace_back(instance->start_time, instance->no, item_count, externalcontainer);
                 return;
             } else {
                 item_count -= instance->count;
                 instance->location = externalcontainer;
             }
         }

        source->CleanUpData();
        // Stage new_stage(formid, item_count, externalcontainer);
        // SourceDataKey new_sourcedatakey()
        // src->data[chest_refid] = externalcontainer;

        // // add it to handled_external_conts
        // //handled_external_conts.insert(externalcontainer);

        // // if successfully transferred to the external container, check if the fake container is faved
        // if (src->data[chest_refid] != chest_refid &&
        //     IsFavorited(RE::TESForm::LookupByID<RE::TESBoundObject>(fakecontainer), external_ref)) {
        //     logger::trace("Faved item successfully transferred to external container.");
        //     external_favs.push_back(fakecontainer);
        // }


    }

    void UnLinkExternalContainer(const FormID fake_formid, Count count,const RefID externalcontainer) { 
        if (!externalcontainer) return RaiseMngrErr("External container is null.");
        if (!fake_formid) {
            logger::error("Fake formid is null.");
            return;
        }
        if (!IsExternalContainer(fake_formid, externalcontainer)) {
            logger::error("External container is not registered.");
            return;
        }

        auto source = GetStageSource(fake_formid);
        if (!source) return RaiseMngrErr("Source not found.");
        int stage_no = -1;
        for (const auto& [st_no, stage] : source->stages) {
			if (stage.formid == fake_formid) {
				stage_no = st_no;
				break;
			}
		}
        if (stage_no == -1) return RaiseMngrErr("Stage not found.");
        std::vector<StageInstance*> instances_candidates;
        for (auto& st_inst : source->data) {
			if (static_cast<int>(st_inst.no) != stage_no) continue;
			if (st_inst.location == externalcontainer) instances_candidates.push_back(&st_inst);
        }
        // need to now order the instances_candidates by their start_time
        std::sort(instances_candidates.begin(), instances_candidates.end(),
				  [](StageInstance* a, StageInstance* b) { return a->start_time < b->start_time; });

        for (const auto& instance : instances_candidates) {
            if (count <= instance->count) {
                instance->count -= count;
                source->data.emplace_back(instance->start_time, instance->no, count, 20);
				return;
			} else {
                count -= instance->count;
				instance->location = 20;
			}
        }
        
        source->CleanUpData();
        logger::trace("Unlinked external container.");

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

    bool UpdateSpoilage(RefID loc_refid) {
        logger::trace("Manager: Updating spoilage for loc_refid {}.",loc_refid);
        return UpdateSpoilage(RE::TESForm::LookupByID<RE::TESObjectREFR>(loc_refid));
    }

    bool UpdateSpoilage(RE::TESObjectREFR* ref) {
        logger::trace("Manager: Updating spoilage.");
        if (!ref) {
            RaiseMngrErr("UpdateSpoilage: ref is null.");
            return false;
        }

        bool update_took_place = false;
        const auto loc_refid = ref->GetFormID();
        for (auto& src : sources) {
            auto updated_stages = src.UpdateAllStages({loc_refid});  // list of somethingwithstages
            if (updated_stages.empty()) continue;
            update_took_place = true;
            for (auto& updated_inst : updated_stages) {
                auto old_formid = src.stages[updated_inst.old_no].formid;
                auto new_formid = src.stages[updated_inst.new_no].formid;
                if (IsFake(ref)) {
                    logger::trace("UpdateSpoilage: ref out in the world.");
                    logger::info("MAKE THIS OPTIONAL");
                    UpdateSpoilageInWorld(ref,new_formid);
                } 
                else if (ref->HasContainer() || ref->IsPlayerRef()) {
                    UpdateSpoilageInInventory(ref, updated_inst.count, old_formid, new_formid);
                } 
                else {
					RaiseMngrErr("UpdateSpoilage: Unknown ref type.");
					return false;
				}
            }
        }

        if (!update_took_place) {
            logger::info("No update");
            return false;
        }

        return true;
    }


    //void ReceiveData() {
    //    logger::info("--------Receiving data---------");

    //    // std::lock_guard<std::mutex> lock(mutex);

    //    setListenContainerChange(false);

    //    std::map<RefID, std::pair<bool, bool>> chest_equipped_fav;

    //    bool no_match;
    //    FormID realForm;
    //    RefID chestRef;
    //    FormIDX fakeForm;
    //    RefID contRef;
    //    std::map<RefID, FormFormID> unmathced_chests;
    //    for (const auto& [realForm, st_inst] : m_Data) {
    //        no_match = true;
    //        for (auto& src : sources) {
    //            if (realForm == src.formid) {
    //                auto stages = GetSpoilageStages(realForm);
    //                if (!src.data.push_back({fakecontForm, fakeForm_stage}).second) {
    //                    return RaiseMngrErr(
    //                        std::format("RefID {} or RefID {} at formid {} already exists in sources data.", chestRef,
    //                                    contRef, realForm));
    //                }
    //                if (!ChestToFakeContainer.insert({chestRef, {realForm, fakeForm.id}}).second) {
    //                    return RaiseMngrErr(
    //                        std::format("realForm {} with fakeForm {} at chestref {} already exists in "
    //                                    "ChestToFakeContainer.",
    //                                    chestRef, realForm, fakeForm.id));
    //                }
    //                if (!fakeForm.name.empty()) renames[fakeForm.id] = fakeForm.name;
    //                if (chestRef == contRef)
    //                    chest_equipped_fav[chestRef] = {fakeForm.equipped, fakeForm.favorited};
    //                else if (fakeForm.favorited)
    //                    external_favs.push_back(fakeForm.id);
    //                no_match = false;
    //                break;
    //            }
    //        }
    //        if (no_match) unmathced_chests[chestRef] = {realForm, fakeForm.id};
    //    }

    //    // handle the unmathced chests
    //    // user probably changed the INI. we try to retrieve the items.
    //    for (const auto& [chestRef_, RealFakeForm_] : unmathced_chests) {
    //        logger::warn("FormID {} not found in sources.", RealFakeForm_.outerKey);
    //        if (_other_settings[Settings::otherstuffKeys[0]]) {
    //            Utilities::MsgBoxesNotifs::InGame::ProblemWithContainer(std::to_string(RealFakeForm_.outerKey));
    //        }
    //        logger::info("Deregistering chest");

    //        auto chest = RE::TESForm::LookupByID<RE::TESObjectREFR>(chestRef_);
    //        if (!chest) return RaiseMngrErr("Chest not found");
    //        RemoveAllItemsFromChest(chest, player_ref);
    //        // also remove the associated fake item from player or unowned chest
    //        auto fake_id = RealFakeForm_.innerKey;
    //        if (fake_id) {
    //            RemoveItemReverse(player_ref, nullptr, fake_id, RE::ITEM_REMOVE_REASON::kRemove);
    //            RemoveItemReverse(unownedChestOG, nullptr, fake_id, RE::ITEM_REMOVE_REASON::kRemove);
    //        }
    //        // make sure no item is left in the chest
    //        if (chest->GetInventory().size()) {
    //            logger::critical("Chest still has items in it. Degistering failed");
    //            Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Items might not have been retrieved successfully.");
    //        }

    //        m_Data.erase({RealFakeForm_.outerKey, chestRef_});
    //    }


    //    // Now i need to iterate through the chests deal with some cases
    //    std::vector<RefID> handled_already;
    //    for (const auto& [chest_ref, _] : ChestToFakeContainer) {
    //        if (std::find(handled_already.begin(), handled_already.end(), chest_ref) != handled_already.end()) continue;
    //        Something2(chest_ref, handled_already);
    //    }

    //    // print handled_already
    //    logger::info("handled_already: ");
    //    for (const auto& ref : handled_already) {
    //        logger::info("{}", ref);
    //    }

    //    handled_already.clear();

    //    // I make the fake containers in player inventory equipped/favorited:
    //    logger::trace("Equipping and favoriting fake containers in player's inventory");
    //    auto inventory_changes = player_ref->GetInventoryChanges();
    //    auto entries = inventory_changes->entryList;
    //    for (auto it = entries->begin(); it != entries->end(); ++it) {
    //        auto fake_formid = (*it)->object->GetFormID();
    //        if (IsFakeContainer(fake_formid)) {
    //            bool is_equipped_x = chest_equipped_fav[GetFakeContainerChest(fake_formid)].first;
    //            bool is_faved_x = chest_equipped_fav[GetFakeContainerChest((*it)->object->GetFormID())].second;
    //            if (is_equipped_x) {
    //                logger::trace("Equipping fake container with formid {}", fake_formid);
    //                EquipItem((*it)->object);
    //                /*RE::ActorEquipManager::GetSingleton()->EquipObject(player_ref->As<RE::Actor>(), (*it)->object,
    //                    nullptr,1,(const RE::BGSEquipSlot*)nullptr,true,false,false,false);*/
    //            }
    //            if (is_faved_x) {
    //                logger::trace("Favoriting fake container with formid {}", fake_formid);
    //                FaveItem((*it)->object);
    //                // inventory_changes->SetFavorite((*it), (*it)->extraLists->front());
    //            }
    //        }
    //    }

    //    logger::info("--------Receiving data done---------");
    //    Print();

    //    // At the very end check for nonexisting fakeforms from ChestToFakeContainer
    //    /*RE::TESBoundObject* fake_cont_;
    //    for (auto it = ChestToFakeContainer.begin(); it != ChestToFakeContainer.end();++it) {
    //        fake_cont_ = RE::TESForm::LookupByID<RE::TESBoundObject>(it->second.innerKey);
    //        if (!fake_cont_) logger::error("Fake container not found with formid {}", it->second.innerKey);
    //        else if (!std::strlen(fake_cont_->GetName()))
    //            logger::error("Fake container with formid {} does not have name.", it->second.innerKey);
    //    }*/

    //    current_container = nullptr;
    //    setListenContainerChange(true);
    //};

#undef ENABLE_IF_NOT_UNINSTALLED
};