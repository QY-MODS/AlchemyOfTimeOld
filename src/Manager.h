#pragma once

#include "Settings.h"
#include "Utils.h"

class Manager {
    
    RE::TESObjectREFR* player_ref = RE::PlayerCharacter::GetSingleton()->As<RE::TESObjectREFR>();
    const RefID player_refid = 20;
    RE::EffectSetting* empty_mgeff;
    
    
    bool worldobjectsspoil;

    // Use Or Take Compatibility
    bool po3_use_or_take = false;

    // 0x0003eb42 damage health

    bool listen_activate = true;
    bool listen_crosshair = true;
    bool listen_container_change = true;
    bool listen_menuopenclose = true;

    bool isUninstalled = false;

    std::mutex mutex;
    
    std::vector<Source> sources;

    std::unordered_map<std::string, bool> _other_settings;


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
            if (src.formid == real_formid && !src.init_failed) {
                return &src;
            }
        }
        logger::info("Source not found");
        return nullptr;
    };

    [[nodiscard]] Source* GetStageSource(const FormID stage_formid) {
        if (!stage_formid) return nullptr;
        for (auto& src : sources) {
            for (auto& stage : src.stages) {
                if (stage.second.formid == stage_formid && !src.init_failed) {
                    return &src;
                }
            }
        }
        logger::warn("Stage source not found");
        return nullptr;
    };

    [[nodiscard]] const StageNo GetStageNoFromSource(Source* src,const FormID stage_id) {
        StageNo stage_no=0; // doesnt matter
        if (!src) {
            RaiseMngrErr("Source is null.");
            return stage_no;
        }
        if (auto p_stage_no = src->GetStageNo(stage_id)) stage_no = *p_stage_no;
        else RaiseMngrErr("Stage not found.");
        return stage_no;
	}

    [[nodiscard]] const StageInstance GetWOStageInstance(RefID wo_refid) {
        if (!wo_refid) {
            RaiseMngrErr("Ref is null.");
			return StageInstance();
        }
        for (const auto& src : sources) {
            for (const auto& st_inst : src.data) {
                if (st_inst.location == wo_refid) return st_inst;
            }
        }
        RaiseMngrErr("Stage instance not found.");
        return StageInstance();
    }

    [[nodiscard]] const StageInstance GetWOStageInstance(RE::TESObjectREFR* wo_ref) {
        if (!wo_ref) {
            RaiseMngrErr("Ref is null.");
            return StageInstance();
        }
        return GetWOStageInstance(wo_ref->GetFormID());
	}

    [[nodiscard]] Source* GetWOSource(RefID wo_refid) {
        if (!wo_refid) return nullptr;
        for (auto& src : sources) {
            for (const auto& st_inst : src.data) {
                if (st_inst.location == wo_refid) return &src;
            }
        }
        return nullptr;
    }

    [[nodiscard]] Source* GetWOSource(RE::TESObjectREFR* wo_ref) {
        if (!wo_ref) {
            RaiseMngrErr("Ref is null.");
            return nullptr;
        }
        return GetWOSource(wo_ref->GetFormID());
    }

    [[nodiscard]] const bool ExternalContainerIsRegistered(const FormID fake_formid,
                                                           const RefID external_container_id) {
        const auto src = GetStageSource(fake_formid);
        if (!src) return false;
        for (const auto& st_inst: src->data) {
            if (st_inst.location == external_container_id) return true;
        }
        return false;
    }

    void UpdateSpoilageInInventory(RE::TESObjectREFR* inventory_owner, Count update_count, FormID old_item, FormID new_item){
        if (new_item == old_item) return;
        if (!inventory_owner) return RaiseMngrErr("Inventory owner is null.");
        logger::trace("Updating spoilage in inventory of {} Count {} , Old item {} , New item {}",
                      inventory_owner->GetName(),
                      update_count, old_item, new_item);

        auto inventory = inventory_owner->GetInventory();
        auto entry = inventory.find(RE::TESForm::LookupByID<RE::TESBoundObject>(old_item));
        /*if (entry != inventory.end() && entry->second.second->extraLists && entry->second.second->extraLists->front()) {
			AddItem(inventory_owner, nullptr, new_item, update_count, entry->second.second->extraLists->front());
		} 
        else AddItem(inventory_owner, nullptr, new_item, update_count);*/
        if (entry == inventory.end()) logger::error("Item not found in inventory.");
        else {
            RemoveItemReverse(inventory_owner, nullptr, old_item, std::min(update_count, entry->second.first),
                              RE::ITEM_REMOVE_REASON::kRemove);
        }
        AddItem(inventory_owner, nullptr, new_item, update_count);
        logger::trace("Spoilage updated in inventory.");
    }

    void _UpdateSpoilageInWorld_Fake(RE::TESObjectREFR* wo_ref,RE::TESBoundObject* stage_bound) {
        logger::trace("Setting text display data.");
        wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
        auto textDisplayData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
        textDisplayData->SetName(stage_bound->GetName());
        wo_ref->extraList.Add(textDisplayData);
    }

    void _UpdateSpoilageInWorld_Custom(RE::TESObjectREFR* wo_ref, RE::TESBoundObject* stage_bound) {
        wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
        logger::trace("Setting ObjectReference to custom stage form.");
        Utilities::FunctionsSkyrim::SwapObjects(wo_ref, stage_bound);
    }

    void _UpdateSpoilageInWorld(RE::TESObjectREFR* wo_ref, RE::TESBoundObject* stage_bound, bool is_fake){
        if (is_fake) _UpdateSpoilageInWorld_Fake(wo_ref, stage_bound);
		else _UpdateSpoilageInWorld_Custom(wo_ref, stage_bound);
    };

    // updates and makes them take effect in the world
    void UpdateSpoilageInWorld(RE::TESObjectREFR* wo_ref) {
        logger::trace("Updating spoilage in world.");
        if (!wo_ref) return RaiseMngrErr("Ref is null.");
        const auto wo_refid = wo_ref->GetFormID();
        auto wo_ref_base = wo_ref->GetBaseObject();
        if (!wo_ref_base) return RaiseMngrErr("Ref base is null.");
        
        // registered olmak zorunda
        if (!RefIsRegistered(wo_ref->GetFormID())) return RaiseMngrErr("UpdateSpoilageInWorld:Ref is not registered.");
        // get the registered stage instance
        Source* source = GetWOSource(wo_ref);
        if (!source) return RaiseMngrErr("UpdateSpoilageInWorld 2: Source not found.");
        const auto st_inst = GetWOStageInstance(wo_ref);
        
        StageNo new_st_no;

        auto updates = source->UpdateAllStages({wo_refid});
        if (updates.empty()) new_st_no = st_inst.no;
        else if (updates.size() > 1) RaiseMngrErr("More than one update.");
        else new_st_no = updates.front().new_no;

        const auto bound = source->stages[new_st_no].GetBound();
        const bool new_is_fake_stage = source->IsFakeStage(new_st_no);

        if (IsStage(wo_ref) && new_is_fake_stage) Utilities::FunctionsSkyrim::SwapObjects(wo_ref, source->GetBoundObject());
        _UpdateSpoilageInWorld(wo_ref, bound, new_is_fake_stage);
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
                    logger::trace("Removing item reverse without extra data.");
                    ref_handle = moveFrom->RemoveItem(item_obj, count, reason, nullptr, moveTo);
                } else {
                    /*logger::trace("Removing item reverse with extra data.");
                    for (auto& extra : *asd) {
						if (extra->HasType(RE::ExtraDataType::kOwnership)) {
                            logger::trace("Removing item reverse with ownership.");
						} else {
							logger::trace("Removing item reverse without ownership.");
						}
					}*/
                    ref_handle = moveFrom->RemoveItem(item_obj, count, reason, asd->front(), moveTo);
                }
                setListenContainerChange(true);
                return ref_handle;
            }
        }
        setListenContainerChange(true);
        return ref_handle;
    }

    void AddItem(RE::TESObjectREFR* addTo, RE::TESObjectREFR* addFrom, FormID item_id,
                                                Count count, RE::ExtraDataList* xList=nullptr) {
        logger::trace("AddItem");
        //xList = nullptr;
        if (!addTo && !addFrom) return RaiseMngrErr("moveFrom and moveTo are both null!");
        
        logger::trace("Adding item.");

        setListenContainerChange(false);
        auto bound = RE::TESForm::LookupByID<RE::TESBoundObject>(item_id);
        addTo->AddObjectToContainer(bound, xList, count, addFrom);
        setListenContainerChange(true);
    }

    [[nodiscard]] const bool PickUpItem(RE::TESObjectREFR* item, Count count,const unsigned int max_try = 3) {
        //std::lock_guard<std::mutex> lock(mutex);
        
        logger::trace("PickUpItem");

        if (!item) {
            logger::warn("Item is null");
            return false;
        }
        if (!player_ref) {
            logger::warn("Actor is null");
            return false;
        }

        logger::trace("PickUpItem: Setting listen container change to false.");
        setListenContainerChange(false);
        //listen_container_change = false;

        logger::trace("PickUpItem: Getting item bound.");
        auto item_bound = item->GetBaseObject();
        const auto item_count = Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref);
        logger::trace("Item count: {}", item_count);
        unsigned int i = 0;
        if (!item_bound) {
            logger::warn("Item bound is null");
            return false;
        }

        item->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
        logger::trace("PickUpItem: Picking up item.");
        while (i < max_try) {
            logger::trace("Critical: PickUpItem");
            RE::PlayerCharacter::GetSingleton()->PickUpObject(item, count, true, false);
            logger::trace("Item picked up. Checking if it is in inventory...");
            auto new_item_count = Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref);
            if (new_item_count > item_count) {
                if (i) logger::warn("Item picked up with new item count: {}. Took {} extra tries.", new_item_count, i);
                setListenContainerChange(true);
                return true;
            } else
                logger::trace("item count: {}", Utilities::FunctionsSkyrim::GetItemCount(item_bound, player_ref));
            i++;
        }

        logger::warn("Item not picked up.");
        setListenContainerChange(true);
        //listen_container_change = true;
        return false;
    }

    /*void AddExtraText(RE::TESObjectREFR* ref, const std::string& text) {
		if (!ref) return RaiseMngrErr("Ref is null.");
		auto textDisplayData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
        textDisplayData->SetName((std::string(ref->GetDisplayFullName()) + text).c_str());
		ref->extraList.Add(textDisplayData);
	}*/

    // registers a new instance to an existing source or creates a new source and registers the instance
    void Register(const FormID source_formid, const Count count, const RefID location_refid) {
        // create stages for this instance
        logger::trace("Registering new instance.");
        if (!location_refid) return RaiseMngrErr("Location refid is null.");
        const float curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        // make new registry
        auto src = GetSource(source_formid);
        if (!src) {
            Source new_source(source_formid, "", empty_mgeff);
            if (new_source.init_failed) return RaiseMngrErr("Register: New source init failed.");
            new_source.data.emplace_back(curr_time, 0, count, location_refid);
            sources.push_back(new_source);
            logger::trace("New source created.");
            src = &sources.back();
        } else src->data.emplace_back(curr_time, 0, count, location_refid);
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

        po3_use_or_take = Utilities::IsPo3_UoTInstalled();

        // Load also other settings...
        // _other_settings = Settings::LoadOtherSettings();
        logger::info("Manager initialized.");

        // add safety check for the sources size say 5 million
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

    void setListenContainerChange(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        listen_container_change = value;
    }


    void setListenMenuOpenClose(const bool value) {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        listen_menuopenclose = value;
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

    void setListenCrosshair(const bool value) {
		std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
		listen_crosshair = value;
	}

    [[nodiscard]] bool getListenCrosshair() {
		std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
		return listen_crosshair;
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

    [[nodiscard]] bool getPO3UoTInstalled() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return po3_use_or_take;
    }

    [[nodiscard]] bool getUninstalled() {
        std::lock_guard<std::mutex> lock(mutex);  // Lock the mutex
        return isUninstalled;
    }

    [[nodiscard]] const bool IsStage(const RE::TESObjectREFR* ref) {
        if (!ref) return false;
        const auto base = ref->GetBaseObject();
        if (!base) return false;
        const auto formid = base->GetFormID();
        if (!Settings::IsItem(formid)) return false;
        return IsStage(formid);
    }

	[[nodiscard]] const bool IsStage(const FormID formid) {
        if (!Settings::IsItem(formid)) {
            logger::trace("Not an item.");
            return false;
        }
		for (const auto& src : sources) {
            if (src.formid == formid) return false;
            for (const auto& stage : src.stages) {
                if (stage.second.formid == formid) return true;
			}
        }
        return false;
    }

    // use it only for world objects! checks if there is a stage instance for the given refid
    [[nodiscard]] const bool RefIsRegistered(const RefID refid) {
        if (!refid) {
            logger::warn("Refid is null.");
            return false;
        }
        for (auto& src : sources) {
            for (auto& st_inst : src.data) {
                //logger::trace("RefIsRegistered: Checking refid {} , st_inst.location {}", refid, st_inst.location);
                if (st_inst.location == refid) return true;
            }
        }
        return false;
    }

    void RegisterAndUpdate(const FormID source_formid, const Count count, const RefID location_refid) {
        Register(source_formid, count, location_refid);
        // if the source is in an inventory
        auto src = GetSource(source_formid);
        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(location_refid);
        if (!ref) return RaiseMngrErr("Ref is null.");
        if (ref->HasContainer() || location_refid == player_refid) {
            logger::trace("Registering in inventory.");
            const auto stage_formid = src->stages[0].formid;
            // to change from the source form to the stage form
            UpdateSpoilageInInventory(ref, count, source_formid, stage_formid);
        }
    }

    // For stuff outside
    void RegisterAndUpdate(RE::TESObjectREFR* wo_ref) {
        if (!wo_ref) return RaiseMngrErr("Ref is null.");
        // create stages for this instance
        const auto base = wo_ref->GetBaseObject();
        if (!base) return RaiseMngrErr("Base is null.");
        const FormID formid = base->GetFormID();
        const auto refid = wo_ref->GetFormID();
        const Count count = wo_ref->extraList.GetCount();
        if (!count) {
            logger::warn("Count is 0.");
            return;
        }
        logger::trace("Registering world object. Formid {} , Count {} , Refid {}", formid, count, refid);

        Register(formid, count, wo_ref->GetFormID());
        UpdateSpoilageInWorld(wo_ref);
    }

    void HandleDrop(const FormID dropped_formid, Count count, RE::TESObjectREFR* dropped_stage_ref){
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleDrop: dropped_formid {} , Count {}", dropped_formid, count);
        if (!dropped_stage_ref) return RaiseMngrErr("Ref is null.");
        if (!IsStage(dropped_formid)) {
            logger::error("HandleDrop: Not a stage item.");
            return;
        }
        auto source = GetStageSource(dropped_formid);
        if (!source) {
            logger::critical("HandleDrop: Source not found! Fakeform not registered!");
            return;
        }
        const auto stage_no = GetStageNoFromSource(source, dropped_formid);
        // at the same stage but different start times
        std::vector<StageInstance*> instances_candidates;
        for (auto& st_inst : source->data) {
            if (st_inst.location == player_refid && st_inst.no == stage_no)
                instances_candidates.push_back(&st_inst);
		}
        // need to now order the instances_candidates by their start_time
        std::sort(instances_candidates.begin(), instances_candidates.end(), [](StageInstance* a, StageInstance* b) {
			return a->start_time > b->start_time; // keep the good stuff
		});

        auto real_bound = source->GetBoundObject();

        logger::trace("HandleDrop: setting count");
        bool handled_first_stack = false;
        for (const auto& instance : instances_candidates) {
            if (count <= instance->count) {
                logger::trace("SADSJFH�ADF 1");
                instance->count -= count;
                if (!handled_first_stack) {
                    dropped_stage_ref->extraList.SetCount(static_cast<uint16_t>(count));
                    dropped_stage_ref->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
                    source->data.emplace_back(instance->start_time, instance->no, count, dropped_stage_ref->GetFormID());
                    UpdateSpoilageInWorld(dropped_stage_ref);
                    handled_first_stack = true;
                } else {
                    logger::trace("SADSJFH�ADF 2");
                    auto new_ref = Utilities::FunctionsSkyrim::DropObjectIntoTheWorld(real_bound, count, nullptr);
					source->data.emplace_back(instance->start_time, instance->no, count, new_ref->GetFormID());
                    UpdateSpoilageInWorld(new_ref);
                }
                break;
            } else {
                count -= instance->count;
                if (!handled_first_stack) {
                    logger::trace("SADSJFH�ADF 3");
                    dropped_stage_ref->extraList.SetCount(static_cast<uint16_t>(instance->count));
                    dropped_stage_ref->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
                    instance->location = dropped_stage_ref->GetFormID();
                    UpdateSpoilageInWorld(dropped_stage_ref);
                    handled_first_stack = true;
                } else {
                    logger::trace("SADSJFH�ADF 4");
                    auto new_ref = Utilities::FunctionsSkyrim::DropObjectIntoTheWorld(real_bound, instance->count, nullptr);
                    instance->location = new_ref->GetFormID();
                    UpdateSpoilageInWorld(new_ref);
                }
            }
        }

        source->CleanUpData();
    }

    void HandlePickUp(const FormID pickedup_formid, const Count count, const RefID wo_refid, const bool eat,
                      RE::TESObjectREFR* npc_ref = nullptr) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::info("HandlePickUp: Formid {} , Count {} , Refid {}", pickedup_formid, count, wo_refid);
        const RefID npc_refid = npc_ref ? npc_ref->GetFormID() : player_refid;
        npc_ref = npc_ref ? npc_ref : player_ref; // naming...for the sake of functionality
        if (!RefIsRegistered(wo_refid)) {
            if (worldobjectsspoil){
                // bcs it shoulda been registered already before picking up
                logger::warn("HandlePickUp: Not registered world object refid: {}, pickedup_formid: {}", wo_refid,
                             pickedup_formid);
            } /*else if (npc_ref != nullptr) {
                logger::warn("If npc_ref is given, the world object refid should be registered!");
                return;
            }*/
            return RegisterAndUpdate(pickedup_formid, count, npc_refid);
        }
        // so it was registered before
        auto source = GetStageSource(pickedup_formid); // registeredsa stage olmak zorunda
        if (!source) return RaiseMngrErr("HandlePickUp: Source not found.");
        for (auto& st_inst: source->data) {
            if (st_inst.location == wo_refid) {
                st_inst.location = npc_refid;
                // if it needs to be replaced by a created form
                if (source->IsFakeStage(st_inst.no)) {
                    // try to replace it with fake form
                    auto new_f = source->stages[st_inst.no].formid;
                    UpdateSpoilageInInventory(npc_ref, count, pickedup_formid, new_f);
                    if (eat && npc_refid == player_refid) RE::ActorEquipManager::GetSingleton()->EquipObject(RE::PlayerCharacter::GetSingleton(), 
                        source->stages[st_inst.no].GetBound(), nullptr, count);
        //            const int count_diff = st_inst.count - count;
        //            if (count_diff > 0 && npc_ref && npc_ref->HasContainer()) {
        //                logger::trace("HandlePickUp: Adding the rest {} to the npc container.", count_diff);
        //                st_inst.count = count; // bcs of NPCs..
        //                RemoveItemReverse(npc_ref, nullptr, formid, count_diff, RE::ITEM_REMOVE_REASON::kRemove);
        //                AddItem(npc_ref, nullptr, source->stages[st_inst.no].formid, count_diff);
        //                source->data.emplace_back(st_inst.start_time, st_inst.no, count_diff, npc_ref->GetFormID());
				    //}
                }
                break;
			}
		}
		source->CleanUpData();
    }

    void HandleConsume(const FormID fake_formid, Count count) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleConsume");
        if (!IsStage(fake_formid)) {
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

    [[nodiscard]] const bool IsExternalContainer(FormID fake_id,RefID refid){
        if (!refid) return false;
        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(refid);
        if (!ref) return false;
        if (!ref->HasContainer()) return false;
        return ExternalContainerIsRegistered(fake_id,refid);
    }

    [[nodiscard]] const bool IsExternalContainer(RE::TESObjectREFR* external_ref) {
        if (!external_ref) return false;
        if (!external_ref->HasContainer()) return false;
        for (const auto& src : sources) {
			for (const auto& st_inst : src.data) {
				if (st_inst.location == external_ref->GetFormID()) return true;
			}
		}
        return false;
    }

    void LinkExternalContainer(const FormID formid, Count item_count, const RefID externalcontainer) {
        ENABLE_IF_NOT_UNINSTALLED
        const auto external_ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer);
        if (!external_ref) return RaiseMngrErr("External ref is null.");
        if (!external_ref->HasContainer()) {
            logger::error("External container does not have a container.");
            return;
        }

        logger::trace("Linking external container.");
        const auto source = GetStageSource(formid);
        if (!source) return RaiseMngrErr("LinkExternalContainer: Source not found.");
        int stage_no = -1;
        for (const auto& [st_no, stage] : source->stages) {
            if (stage.formid == formid) {
                stage_no = st_no;
                break;
            }
        }
        if (stage_no == -1) return RaiseMngrErr("LinkExternalContainer: Stage not found.");
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
                logger::trace("Linked external container.");
                break;
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
        if (!source) return RaiseMngrErr("UnLinkExternalContainer: Source not found.");
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
                if (ref->HasContainer() || ref->IsPlayerRef()) {
                    UpdateSpoilageInInventory(ref, updated_inst.count, old_formid, new_formid);
                } 
                else if (Settings::IsItem(ref) && worldobjectsspoil) {
                    logger::trace("UpdateSpoilage: ref out in the world.");
                    bool new_is_fake = src.IsFakeStage(updated_inst.new_no);
                    if (IsStage(ref) && new_is_fake) Utilities::FunctionsSkyrim::SwapObjects(ref, src.GetBoundObject());
                    _UpdateSpoilageInWorld(ref, src.stages[updated_inst.new_no].GetBound(), new_is_fake);
                } 
                else {
					RaiseMngrErr("UpdateSpoilage: Unknown ref type.");
					return false;
				}
            }
        }

        if (!update_took_place) {
            logger::trace("No update");
            return false;
        }

        return true;
    }

    // only necessary for the world objects who has fake counterparts
    void SwapWithStage(RE::TESObjectREFR* wo_ref) {
        if (!wo_ref) return RaiseMngrErr("Ref is null.");
        if (!Settings::IsItem(wo_ref)) return;
        // 1. registered olmadii icin stage olmayabilir
        // 2. counterparti fake olduu icin disarda base basele represente olabilir
        if (IsStage(wo_ref)) return;
        
        logger::trace("SwapWithStage");
        
        const auto wo_refid = wo_ref->GetFormID();
        const auto formid = wo_ref->GetBaseObject()->GetFormID();
        if (!RefIsRegistered(wo_ref->GetFormID())) {
            if (worldobjectsspoil) {
                // bcs it shoulda been registered already before picking up
                logger::warn("SwapWithStage: Not registered world object refid: {}, formid: {}", wo_refid, formid);
            }
            Register(formid, wo_ref->extraList.GetCount(), wo_ref->GetFormID());
            auto source = GetSource(formid);
            wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
            Utilities::FunctionsSkyrim::SwapObjects(wo_ref, source->stages[0].GetBound());
            return;
		}
        auto source = GetSource(formid);
        if (!source) return RaiseMngrErr("Source not found.");
        for (auto& st_inst : source->data) {
            if (st_inst.location == wo_ref->GetFormID()) {
                if (!source->IsFakeStage(st_inst.no)) {
                    logger::warn("SwapWithStage: Not a fake stage.");
                    return;
                }
                wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
                Utilities::FunctionsSkyrim::SwapObjects(wo_ref, source->stages[st_inst.no].GetBound(),false);
				return;
			}
		}
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