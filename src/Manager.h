#pragma once

#include "Settings.h"
#include "Utils.h"

class Manager : public Utilities::SaveLoadData {
    
    //const RefID chest_Refid = 0x0201824C;

    RE::TESObjectREFR* player_ref = RE::PlayerCharacter::GetSingleton()->As<RE::TESObjectREFR>();
    RE::EffectSetting* empty_mgeff = nullptr;

    std::map<RefID,std::set<FormID>> external_favs;
    std::map<FormFormID,std::pair<int,Count>> handle_crafting_instances; // real-stage:added-total before adding (both real)
    std::map<FormID, bool> faves_list;
    std::map<FormID, bool> equipped_list;
    std::vector<StageInstance*> instances_to_be_updated; // TODO
    
    std::map<RefID,std::vector<StageInstance*>> is_also_time_modulator;

    bool worldobjectsevolve = false;

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

    unsigned int _instance_limit = 200000;


#define ENABLE_IF_NOT_UNINSTALLED if (isUninstalled) return;

    [[nodiscard]] const bool IsStage(FormID some_formid, Source* source) {
        for (auto& [st_no, stage] : source->stages) {
            if (stage.formid == some_formid) {
                return true;
            }
        }
        return false;
    }

	[[nodiscard]] const unsigned int GetNInstances() {
        unsigned int n = 0;
        for (auto& src : sources) {
            for (const auto& [loc, _] : src.data) {
                n += static_cast<unsigned int>(src.data[loc].size());
            }
        }
		return n;
	}

    [[nodiscard]] Source* _MakeSource(const FormID source_formid, Settings::DefaultSettings* settings) {
        if (!source_formid) return nullptr;
        Source new_source(source_formid, "", empty_mgeff, settings);
        if (new_source.init_failed) return nullptr;
        sources.push_back(new_source);
        return &sources.back();
    }

    [[nodiscard]] Source* GetSource(const FormID some_formid) {
        if (!some_formid) return nullptr;
        const auto some_form = Utilities::FunctionsSkyrim::GetFormByID(some_formid);
        if (!some_form) return nullptr;
        // maybe it already exists
        for (auto& src : sources) {
            if (src.init_failed) continue;
            if (src.formid == some_formid) {
                return &src;
            }
            for (auto& [st_no, stage] : src.stages) {
                if (stage.formid == some_formid) {
                    return &src;
                }
            }
        }
        // doesnt exist so we make new
        // registered stageler arasinda deil. yani fake de deil
        // custom stage mi deil mi onu anlamam lazim
        auto _qformtype = Settings::GetQFormType(some_formid);
        if (!_qformtype.empty() && Settings::custom_settings.contains(_qformtype)) {
            auto& custom_settings = Settings::custom_settings[_qformtype];
            for (auto& [names,sttng]: custom_settings){
                if (sttng.init_failed) continue;
                for (auto& name : names) {
                    const auto temp_cstm_formid = Utilities::FunctionsSkyrim::GetFormEditorIDFromString(name);
                    if (temp_cstm_formid<=0) continue;
                    if (const auto temp_cstm_form = Utilities::FunctionsSkyrim::GetFormByID(temp_cstm_formid, name)) {
                        if (temp_cstm_form->GetFormID() == some_formid) return _MakeSource(some_formid, &sttng);
                    }
                }
                if (Utilities::Functions::String::includesWord(some_form->GetName(), names)) {
                    return _MakeSource(some_formid, &sttng);
                }

            }
        }

        logger::info("Source not found");
        return nullptr;
    };

    [[maybe_unused]] Source* GetStageSource(const FormID some_formid) {
        if (!some_formid) return nullptr;
        // maybe it already exists
        for (auto& src : sources) {
            if (src.init_failed) continue;
            if (IsStage(some_formid, &src)) return &src;
        }
        // registered stageler arasinda deil
        // custom stage mi deil mi onu anlamam lazim
        auto _qformtype = Settings::GetQFormType(some_formid);
        if (!_qformtype.empty() && Settings::custom_settings.contains(_qformtype)) {
            auto& custom_settings = Settings::custom_settings[_qformtype];
            for (auto& [names, sttng] : custom_settings) {
                if (const auto temp_cstm_form = Utilities::FunctionsSkyrim::GetFormByID(some_formid)) {
                    if (Utilities::Functions::String::includesWord(temp_cstm_form->GetName(), names)) {
                        auto _src = _MakeSource(some_formid, &sttng);
                        if (IsStage(some_formid, _src)) return _src;
                    }
                }
                for (auto& name : names) {
                    const auto temp_cstm_formid = Utilities::FunctionsSkyrim::GetFormEditorIDFromString(name);
                    if (temp_cstm_formid <= 0) continue;
                    if (const auto temp_cstm_form = Utilities::FunctionsSkyrim::GetFormByID(temp_cstm_formid, name)) {
                        if (temp_cstm_form->GetFormID() == some_formid) {
                            auto _src = _MakeSource(some_formid, &sttng);
                            if (IsStage(some_formid, _src)) return _src;
                        }
                    }
                }
            }
        }

        logger::info("Source not found");
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

    [[nodiscard]] StageInstance* GetWOStageInstance(RefID wo_refid) {
        if (!wo_refid) {
            RaiseMngrErr("Ref is null.");
			return nullptr;
        }
        if (sources.empty()) return nullptr;
        for (auto& src : sources) {
            if (!src.data.contains(wo_refid)) continue;
            auto& instances = src.data[wo_refid];
            if (instances.size() == 1) return &instances[0];
            else if (instances.empty()) {
                logger::error("Stage instance found but empty.");
            } 
            else if (instances.size() > 1) {
				logger::error("Multiple stage instances found.");
			}
        }
        logger::error("Stage instance not found.");
        return nullptr;
    }

    [[nodiscard]] StageInstance* GetWOStageInstance(RE::TESObjectREFR* wo_ref) {
        if (!wo_ref) {
            RaiseMngrErr("Ref is null.");
            return nullptr;
        }
        return GetWOStageInstance(wo_ref->GetFormID());
	}

    [[nodiscard]] Source* GetWOSource(RefID wo_refid) {
        if (!wo_refid) return nullptr;
        if (sources.empty()) return nullptr;
        for (auto& src : sources) {
            if (src.data.contains(wo_refid) && !src.data[wo_refid].empty()) {
                if (src.data[wo_refid].size() == 1) return &src;
                else if (src.data[wo_refid].size() > 1) {
                    RaiseMngrErr("Multiple stage instances found.");
                    return nullptr;
                }
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

    void _ApplyEvolutionInInventory(RE::TESObjectREFR* inventory_owner, Count update_count, FormID old_item, FormID new_item) {
        
        logger::trace("Updating stage in inventory of {} Count {} , Old item {} , New item {}",
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
        logger::trace("Stage updated in inventory.");

    }

    void _ApplyEvolutionInInventoryX(RE::TESObjectREFR* inventory_owner, Count update_count, FormID old_item,
                                   FormID new_item) {
        logger::trace("Updating stage in inventory of {} Count {} , Old item {} , New item {}",
                      inventory_owner->GetName(), update_count, old_item, new_item);

        auto inventory = inventory_owner->GetInventory();
        auto entry = inventory.find(RE::TESForm::LookupByID<RE::TESBoundObject>(old_item));
        bool has_xList = false;
        if (entry != inventory.end()){
            if (auto asd = entry->second.second.get()) {
                if (asd->extraLists && !asd->extraLists->empty()) {
                    logger::trace("Front 1");
                    if (asd->extraLists->front()) has_xList = true;
                    else logger::trace("ExtraDataList front is null.");
				} else logger::trace("ExtraDataList is null or empty.");
			} else logger::trace("Item data is null.");
        } else logger::trace("Item not found in inventory.");
        const auto __count = std::min(update_count, entry->second.first);
        //else AddItem(inventory_owner, nullptr, new_item, update_count);
        auto ref_handle = Utilities::FunctionsSkyrim::WorldObject::DropObjectIntoTheWorld(RE::TESForm::LookupByID<RE::TESBoundObject>(new_item),
                                                           __count);
        if (has_xList) {
            if (!Utilities::FunctionsSkyrim::xData::UpdateExtras(entry->second.second->extraLists->front(),
                                                                 &ref_handle->extraList)) {
                logger::info("ExtraDataList not updated.");
            }
        } else logger::info("original ExtraDataList is null.");

        setListenContainerChange(false);
        if (!Utilities::FunctionsSkyrim::WorldObject::PlayerPickUpObject(ref_handle, __count)) {
			logger::error("Item not picked up.");
			return;
		}
        setListenContainerChange(true);
        
        //AddItem(inventory_owner, nullptr, new_item, update_count, {});
        if (entry != inventory.end()) {
            RemoveItemReverse(inventory_owner, nullptr, old_item, __count,
                                RE::ITEM_REMOVE_REASON::kRemove);
        }
        else logger::error("Item not found in inventory.");
        logger::trace("Stage updated in inventory.");
    }

    void ApplyEvolutionInInventory(std::string _qformtype_, RE::TESObjectREFR* inventory_owner, Count update_count,
                                   FormID old_item, FormID new_item) {
        if (!inventory_owner) return RaiseMngrErr("Inventory owner is null.");
        if (!old_item || !new_item) return RaiseMngrErr("Item is null.");
        if (!update_count) {
            logger::warn("Update count is 0.");
            return;
        }
        if (old_item == new_item) {
			logger::info("ApplyEvolutionInInventory: New item is the same as the old item.");
			return;
		}
        
        bool is_faved = false;
        bool is_equipped = false;
        if (inventory_owner->IsPlayerRef()) {
			is_faved = Utilities::FunctionsSkyrim::Inventory::IsPlayerFavorited(
				RE::TESForm::LookupByID<RE::TESBoundObject>(old_item));
			is_equipped = Utilities::FunctionsSkyrim::Inventory::IsEquipped(
				RE::TESForm::LookupByID<RE::TESBoundObject>(old_item));
		}
        if (Utilities::Functions::Vector::HasElement<std::string>(Settings::xQFORMS, _qformtype_)) {
			_ApplyEvolutionInInventoryX(inventory_owner, update_count, old_item, new_item);
		} else _ApplyEvolutionInInventory(inventory_owner, update_count, old_item, new_item);

        if (is_faved) Utilities::FunctionsSkyrim::Inventory::FavoriteItem(
			RE::TESForm::LookupByID<RE::TESBoundObject>(new_item), inventory_owner);
		if (is_equipped) Utilities::FunctionsSkyrim::Inventory::EquipItem(
			RE::TESForm::LookupByID<RE::TESBoundObject>(new_item));
    }

    void _ApplyStageInWorld_Fake(RE::TESObjectREFR* wo_ref, const char* xname) {
        if (!xname) {
            logger::error("ExtraTextDisplayData is null.");
            return;
        }
        logger::trace("Setting text display data.");
        wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
        auto xText = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
        xText->SetName(xname);
        logger::trace("{}", xText->displayName);
        wo_ref->extraList.Add(xText);
    }

    void _ApplyStageInWorld_Custom(RE::TESObjectREFR* wo_ref, RE::TESBoundObject* stage_bound) {
        wo_ref->extraList.RemoveByType(RE::ExtraDataType::kTextDisplayData);
        logger::trace("Setting ObjectReference to custom stage form.");
        Utilities::FunctionsSkyrim::WorldObject::SwapObjects(wo_ref, stage_bound);
    }

    void ApplyStageInWorld(RE::TESObjectREFR* wo_ref, Stage& stage, RE::TESBoundObject* source_bound = nullptr) {
        if (!source_bound) _ApplyStageInWorld_Custom(wo_ref, stage.GetBound());
        else {
            Utilities::FunctionsSkyrim::WorldObject::SwapObjects(wo_ref, source_bound);
            _ApplyStageInWorld_Fake(wo_ref, stage.GetExtraText());
        }
    }

  //  [[maybe_unused]] void AlignRegistries(std::vector<RefID> locs) {
  //      ENABLE_IF_NOT_UNINSTALLED
  //      logger::trace("Aligning registries.");
		//for (auto& src : sources) {
		//	for (auto& st_inst : src.data) {
  //              if (!Utilities::Functions::Vector::HasElement(locs,st_inst.location)) continue;
  //              // POPULATE THIS
  //              if (st_inst.location == player_refid) HandleConsume(st_inst.xtra.form_id);
		//	}
		//}
  //  }

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

    [[maybe_unused]] const bool PickUpItem(RE::TESObjectREFR* item, Count count,const unsigned int max_try = 3) {
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
        const auto item_count = Utilities::FunctionsSkyrim::Inventory::GetItemCount(item_bound, player_ref);
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
            auto new_item_count = Utilities::FunctionsSkyrim::Inventory::GetItemCount(item_bound, player_ref);
            if (new_item_count > item_count) {
                if (i) logger::warn("Item picked up with new item count: {}. Took {} extra tries.", new_item_count,i); 
                setListenContainerChange(true); 
                return true;
            } 
            else
                logger::trace("item count: {}", Utilities::FunctionsSkyrim::Inventory::GetItemCount(item_bound, player_ref));
            i++;
        }

        logger::warn("Item not picked up.");
        setListenContainerChange(true);
        //listen_container_change = true;
        return false;
    }
    
    // TAMAM
    // _UpdateTimeModulators ve UpdateStages kullaniyo
    bool _UpdateStagesInSource(std::vector<RE::TESObjectREFR*> refs, Source* src, const float curr_time) {
        if (!src) {
            RaiseMngrErr("_UpdateStages: Source is null.");
            return false;
        }
        if (refs.empty()) {
            RaiseMngrErr("_UpdateStages: Refs is empty.");
            return false;
        }

        std::map<RefID, RE::TESObjectREFR*> ids_refs;
        std::vector<RefID> refids;
        for (auto& ref : refs) {
            if (!ref) {
                RaiseMngrErr("_UpdateStages: ref is null.");
                return false;
            }
            ids_refs[ref->GetFormID()] = ref;
            refids.push_back(ref->GetFormID());
        }
        auto updated_stages = src->UpdateAllStages(refids, curr_time);
        if (updated_stages.empty()) {
            logger::trace("No update2");
            return false;
        }

        for (auto& [loc,updates] : updated_stages) {
            for (auto& update : updates) {
                if (!update.oldstage || !update.newstage || !update.count) {
                    logger::error("_UpdateStages: Update is null.");
                    continue;
                }
                auto ref = ids_refs[loc];
                if (ref->HasContainer() || ref->IsPlayerRef()) {
                    ApplyEvolutionInInventory(src->qFormType,ref, update.count, update.oldstage->formid, update.newstage->formid);
                    if (src->IsDecayedItem(update.newstage->formid)) {
                        RegisterAndGo(update.newstage->formid, update.count, loc);
                    }
                }
                // WO
                else if (worldobjectsevolve) {
                    logger::trace("_UpdateStages: ref out in the world.");
                    auto bound = src->IsFakeStage(update.newstage->no) ? src->GetBoundObject() : nullptr;
                    ApplyStageInWorld(ref, *update.newstage, bound);
                    if (src->IsDecayedItem(update.newstage->formid)) {
                        RegisterAndGo(update.newstage->formid,update.count, loc);
                    }
                } else {
                    logger::critical("_UpdateStages: Unknown ref type.");
                    return false;
                }
            }
        }
        //src->CleanUpData();
        return true;
    }

    bool _UpdateStagesOfRef(RE::TESObjectREFR* ref, const float _time, bool is_inventory, bool cleanup = true) {
        if (!ref) {
			RaiseMngrErr("_UpdateStagesOfRef: Ref is null.");
			return false;
		}
        const RefID refid = ref->GetFormID();
        bool update_took_place = false;
        for (auto& src : sources) {
            if (!src.data.contains(refid)) continue;
            if (src.data[refid].empty()) continue;
            auto* src_ptr = &src;
            const bool temp_update_took_place = _UpdateStagesInSource({ref}, src_ptr, _time);
            if (temp_update_took_place && cleanup) src.CleanUpData();
            // consider merginf this with the regular stage update
            if (is_inventory) src_ptr->UpdateTimeModulationInInventory(ref, _time);
            if (!update_took_place) update_took_place = temp_update_took_place;
        }
        return update_took_place;
    }

    // only for time modulators which are also stages.
    bool _UpdateTimeModulators(RE::TESObjectREFR* inventory_owner, const float curr_time) {

        if (!inventory_owner) {
			RaiseMngrErr("_UpdateTimeModulators: Inventory owner is null.");
			return false;
		}

        const RefID inventory_owner_refid = inventory_owner->GetFormID();


        //std::vector<StageInstance*> all_instances_in_inventory;
        std::vector<std::pair<std::pair<size_t,size_t>, float>> queued_updates;  // pair: source owner index , hitting time
        // since these instances are in the same inventory, they are affected by the same time modulator

        size_t index_src = 0;
        for (auto& src : sources) {
			if (!src.data.contains(inventory_owner_refid)) continue;
            const auto temp_time_modulators = Utilities::Functions::getKeys<FormID,float>(src.defaultsettings->delayers);
            size_t index_inst = 0;
			for (auto& st_inst : src.data[inventory_owner_refid]) {
				if (st_inst.xtra.is_decayed || !src.stages.contains(st_inst.no)) continue;
                if (Utilities::Functions::Vector::HasElement<FormID>(temp_time_modulators,st_inst.xtra.form_id)) {
                    const auto delay_slope = st_inst.GetDelaySlope();
                    if (delay_slope == 0) continue;
                    const auto schranke = delay_slope > 0 ? src.stages[st_inst.no].duration : 0.f;
                    const auto hitting_time = st_inst.GetHittingTime(schranke);
                    if (hitting_time<=0) {
						logger::warn("Hitting time is 0 or less!");
						continue;
					}
                    else if (hitting_time <= curr_time) queued_updates.push_back({{index_src, index_inst}, hitting_time});
                }
                index_inst++;
			}
			index_src++;
		}
        
        if (queued_updates.empty()) {
            logger::trace("No queued updates.");
            return false;
        }

        size_t max_try = queued_updates.size()+100;
        bool update_happened = false;
        while (!queued_updates.empty() && max_try > 0) {
            max_try--;
            // order them by hitting time
            std::sort(queued_updates.begin(), queued_updates.end(),
                      [](std::pair<std::pair<size_t, size_t>, float> a, std::pair<std::pair<size_t, size_t>, float> b) {
                          return a.second < b.second;
                      });

            const auto& q_u = queued_updates[0];
            index_src = q_u.first.first;
            const auto index_inst = q_u.first.second;
            const auto hitting_time = q_u.second;
            queued_updates.erase(queued_updates.begin());

            auto& src = sources[index_src];
            auto& st_inst = src.data[inventory_owner_refid][index_inst];
            const auto _t_ = hitting_time + std::min(0.0001f, curr_time - hitting_time);
            const auto formid_before = st_inst.xtra.form_id;
            if (_UpdateStagesOfRef(inventory_owner, _t_, true, false)) {
				logger::trace("Time modulator updated.");
                update_happened = true;
                if (formid_before != st_inst.xtra.form_id && !st_inst.xtra.is_decayed) {
                    const auto temp_time_modulators =
                        Utilities::Functions::getKeys<FormID, float>(src.defaultsettings->delayers);
                    if (Utilities::Functions::Vector::HasElement<FormID>(temp_time_modulators, st_inst.xtra.form_id)) {
                        const auto delay_slope = st_inst.GetDelaySlope();
                        if (delay_slope != 0) {
                            const auto schranke = delay_slope > 0 ? src.stages[st_inst.no].duration : 0.f;
                            const auto new_hitting_time = st_inst.GetHittingTime(schranke);
                            if (new_hitting_time <= 0) {
                                logger::warn("Hitting time is 0 or less!");
                            } else if (new_hitting_time <= curr_time) {
                                queued_updates.push_back({{index_src, index_inst}, new_hitting_time});
                                max_try++; 
                            }
                        }
                    }
				}
			}

        }

        return update_happened;
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
        empty_mgeff->data.flags.set(RE::EffectSetting::EffectSettingData::Flag::kNoDuration);


        po3_use_or_take = Utilities::IsPo3_UoTInstalled();

        if (Settings::INI_settings.contains("Other Settings")){
            if (Settings::INI_settings["Other Settings"].contains("WorldObjectsEvolve")) {
				worldobjectsevolve = Settings::INI_settings["Other Settings"]["WorldObjectsEvolve"];
			} else logger::warn("WorldObjectsEvolve not found.");
        } 
        else logger::critical("Other Settings not found.");
        
        _instance_limit = Settings::nMaxInstances;

        if (init_failed) InitFailed();
        logger::info("Manager initialized.");

        // add safety check for the sources size say 5 million
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

    void Uninstall() {
        isUninstalled = true;
        // Uninstall other settings...
        // Settings::UninstallOtherSettings();
    }

    static Manager* GetSingleton(std::vector<Source>& data) {
        static Manager singleton(data);
        return &singleton;
    }

    const char* GetType() override { return "Manager"; }

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

    // use it only for world objects! checks if there is a stage instance for the given refid
    [[nodiscard]] const bool RefIsRegistered(const RefID refid) {
        if (!refid) {
            logger::warn("Refid is null.");
            return false;
        }
        if (sources.empty()) {
			logger::warn("Sources is empty.");
			return false;
		}
        for (auto& src : sources) {
            if (src.data.contains(refid)) return true;
        }
        return false;
    }

    // TAMAM
    // giris noktasi
    void RegisterAndGo(const FormID some_formid, const Count count, const RefID location_refid) {
        ENABLE_IF_NOT_UNINSTALLED
        if (!some_formid) {
            logger::warn("Formid is null.");
            return;
        }
        if (!count) {
            logger::warn("Count is 0.");
            return;
        }
        if (!location_refid) return RaiseMngrErr("Location refid is null.");

        if (GetNInstances() > _instance_limit) {
			logger::warn("Instance limit reached.");
            Utilities::MsgBoxesNotifs::InGame::CustomErrMsg(
                std::format("The mod is tracking over {} instances. Maybe it is not bad to check your memory usage and "
                            "skse co-save sizes.",
                            _instance_limit));
		}

        logger::trace("Registering new instance.Formid {} , Count {} , Location refid {}", some_formid, count, location_refid);
        // make new registry
        auto src = GetSource(some_formid);  // also saves it to sources if it was created new
        // can be null if it is not a custom stage and not registered before
        if (!src) {
            logger::trace("No existing source and no custom settings found for the formid {}", some_formid);
            const auto qform_type = Settings::GetQFormType(some_formid);
            if (!Settings::IsItem(some_formid, qform_type, true)) {
                logger::trace("Not an item.");
				return;
            }
            else if (!Settings::defaultsettings.contains(qform_type)){
                logger::trace("No default settings found for the qform_type {}", qform_type);
				return;
            } else if (Settings::defaultsettings[qform_type].init_failed) {
				logger::trace("Default settings not loaded for the qform_type {}", qform_type);
                return;
            } else {
                logger::trace("Creating new source for the formid {}", some_formid);
                src = _MakeSource(some_formid,nullptr);  // stage item olarak dusunulduyse, custom a baslangic itemi olarak koymali
            }
        }
        if (!src) return RaiseMngrErr("Register: Source is null.");

        const auto stage_no = src->formid == some_formid ? 0 : GetStageNoFromSource(src, some_formid);
        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(location_refid);
        if (!ref) return RaiseMngrErr("Register: Ref is null.");

        if (ref->HasContainer() || location_refid == player_refid) {
            logger::trace("Registering in inventory.");
            if (!src->InitInsertInstanceInventory(stage_no, count, ref)) return RaiseMngrErr("Register: InsertNewInstance failed 1.");
            const auto stage_formid = src->stages[stage_no].formid;
            // to change from the source form to the stage form
            ApplyEvolutionInInventory(src->qFormType,ref, count, some_formid, stage_formid);
        } 
        else {
            logger::trace("Registering in world.");
            if (!src->InitInsertInstanceWO(stage_no, count, location_refid)) return RaiseMngrErr("Register: InsertNewInstance failed 1.");
            auto bound =  src->IsFakeStage(stage_no) ? src->GetBoundObject() : nullptr;
            ApplyStageInWorld(ref, src->stages[stage_no], bound);
        }

        src->PrintData();
    }

    // TAMAM
    // giris noktasi RegisterAndGo uzerinden
    void RegisterAndGo(RE::TESObjectREFR* wo_ref) {
        if (!worldobjectsevolve) return;
        if (!wo_ref) {
            logger::warn("Ref is null.");
			return;
        }
        const auto wo_refid = wo_ref->GetFormID();
        if (!wo_refid) {
			logger::warn("Refid is null.");
			return;
		}
        const auto wo_bound = wo_ref->GetBaseObject();
        if (!wo_bound) {
            logger::warn("Bound is null.");
            return;
        }
        if (RefIsRegistered(wo_refid)) {
			logger::warn("Ref is already registered.");
			return;
		}
        RegisterAndGo(wo_bound->GetFormID(), wo_ref->extraList.GetCount(), wo_refid);
    }

    // pffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    [[maybe_unused]] void _HandleDrop2(Source*, RefID, Count) {
		/*ENABLE_IF_NOT_UNINSTALLED
        if (!refid) return RaiseMngrErr("Ref is null.");
        if (!count) {
            logger::warn("Count is 0.");
            return;
        }*/

        /*auto a_obj = wo_ref->GetBaseObject();
        setListenContainerChange(false);
        a_obj->Activate(wo_ref, player_ref, 0, a_obj, 0);
        setListenContainerChange(true);*/
        /*if (!PickUpItem(wo_ref, 1)) {
			logger::error("HandleDrop: Item could not be picked up.");
			return;
        }*/
        /* else {
            logger::trace("HandleDrop: Item picked up.");
            return;
        }*/
        //dropped_stage_ref->extraList.SetCount(static_cast<uint16_t>(count));
        
    
    }
    // TAMAM
    // update oncesinde ettiini varsayiyo
    void HandleDrop(const FormID dropped_formid, Count count, RE::TESObjectREFR* dropped_stage_ref){
        ENABLE_IF_NOT_UNINSTALLED
        
        if (!dropped_formid) return RaiseMngrErr("Formid is null.");
        if (!dropped_stage_ref) return RaiseMngrErr("Ref is null.");
        if (!count) {
			logger::warn("Count is 0.");
			return;
		}

        //mutex.lock();

        logger::trace("HandleDrop: dropped_formid {} , Count {}", dropped_formid, count);
        if (!dropped_stage_ref) return RaiseMngrErr("Ref is null.");

        auto source = GetSource(dropped_formid);
        if (!source) {
            logger::warn("HandleDrop: Source not found! Can be if it was never registered before.");
            return;
        } 
        else if (source->formid == dropped_formid && !IsStage(dropped_formid, source)) {
			logger::warn("HandleDrop: Source same formid as dropped one! Can be bcs the item's evolution did not start yet or the stage item is the same as source item.");
			return;
		}
        
        // count muhabbetleri... ay ay ay (bisey yapmiyorum bu ikisinde an itibariyle)
        if (RefIsRegistered(dropped_stage_ref->GetFormID())){
            // eventsink bazen bugliyodu ayni refe gosteriyodu countlar split olunca
            // ama extrayi attiimdan beri olmuyodu
            logger::warn("Ref is registered at HandleDrop!");
            /*const auto curr_count = dropped_stage_ref->extraList.GetCount();
            Utilities::FunctionsSkyrim::WorldObject::SetObjectCount(dropped_stage_ref, count + curr_count);*/
            return;
        }
        else if (dropped_stage_ref->extraList.GetCount() != count) {
			logger::warn("HandleDrop: Count mismatch: {} , {}", dropped_stage_ref->extraList.GetCount(), count);
            // try to assign stage instances to the dropped item
            const auto refid = dropped_stage_ref->GetFormID();
            source->MoveInstances(player_refid, refid, dropped_formid, count, true);
            // remove all instances except the last one
            auto& vec = source->data[refid];
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i == 0) vec[i].count = count;
				else vec[i].count = 0;
            }
            source->CleanUpData();
            /*return _HandleDrop2(source,dropped_stage_ref, count);*/
            return;
		}
        
        source->PrintData();
        // at the same stage but different start times
        std::vector<size_t> instances_candidates = {};
        if (source->data.contains(player_refid)){
            size_t index__ = 0;
            for (auto& st_inst : source->data[player_refid]) {
                if (st_inst.xtra.form_id == dropped_formid) {
                    instances_candidates.push_back(index__);
                }
                index__++;
		    }
        }
        if (instances_candidates.empty()) {
			logger::error("HandleDrop: No instances found. Can be when dropping not yet registered stage items.");
			return;
		}
        // need to now order the instances_candidates by their elapsed times
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        std::sort(instances_candidates.begin(), instances_candidates.end(), 
            [source,curr_time](size_t a, size_t b) {
            return source->data[player_refid][a].GetElapsed(curr_time) >
                   source->data[player_refid][b].GetElapsed(curr_time);  // use up the old stuff first
		});

        logger::trace("HandleDrop: setting count");

        for (const auto& i : Settings::xRemove) {
            dropped_stage_ref->extraList.RemoveByType(static_cast<RE::ExtraDataType>(i));
        }


        bool handled_first_stack = false;
        std::vector<size_t> removed_indices;
        for (size_t index : instances_candidates) {
            
            if (!count) break;
            StageInstance* instance = nullptr;
            if (removed_indices.empty()) {
                instance = &source->data[player_refid][index];
            } 
			else {
                int shift = 0;
				for (size_t removed_index: removed_indices) {
                    if (index == removed_index) return RaiseMngrErr("HandleDrop: Index already removed.");
                    if (index > removed_index) shift++;
				}
                instance = &source->data[player_refid][index - shift];
			}
            instance->RemoveTimeMod(curr_time);

            if (count <= instance->count) {
                
                logger::trace("instance count: {} vs count {}", instance->count,count);
                instance->count -= count;
                logger::trace("instance count: {} vs count {}", instance->count,count);

                StageInstance new_instance(*instance);
                new_instance.count = count;

                if (!handled_first_stack) {
                    logger::trace("SADSJFHOADF 1");
                    if (Utilities::FunctionsSkyrim::WorldObject::GetObjectCount(dropped_stage_ref) != static_cast<int16_t>(count)) {
                        Utilities::FunctionsSkyrim::WorldObject::SetObjectCount(dropped_stage_ref, count);
                    }
                    //dropped_stage_ref->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
                    if (!source->InsertNewInstance(new_instance, dropped_stage_ref->GetFormID())) {
                        return RaiseMngrErr("HandleDrop: InsertNewInstance failed.");
                    }
                    auto bound = new_instance.xtra.is_fake ? source->GetBoundObject() : nullptr;
                    ApplyStageInWorld(dropped_stage_ref, source->stages[new_instance.no], bound);
                    handled_first_stack = true;
                } 
                else {
                    logger::trace("SADSJFHOADF 2");
                    const auto bound_to_drop = instance->xtra.is_fake ? source->GetBoundObject() : instance->GetBound();
                    auto new_ref = Utilities::FunctionsSkyrim::WorldObject::DropObjectIntoTheWorld(bound_to_drop, count);
                    if (!new_ref) return RaiseMngrErr("HandleDrop: New ref is null.");
                    if (new_ref->extraList.GetCount() != count) {
						logger::warn("HandleDrop: NewRefCount mismatch: {} , {}", new_ref->extraList.GetCount(), count);
					}
                    if (!source->InsertNewInstance(new_instance, new_ref->GetFormID())) {
                        return RaiseMngrErr("HandleDrop: InsertNewInstance failed.");
                    }
                    if (new_instance.xtra.is_fake) _ApplyStageInWorld_Fake(new_ref, source->stages[new_instance.no].GetExtraText());
                }
                count = 0;
                /*break;*/
            } 
            else {

                logger::trace("instance count: {} vs count {}", instance->count,count);
                count -= instance->count;
                logger::trace("instance count: {} vs count {}", instance->count,count);
                
                removed_indices.push_back(index);

                if (!handled_first_stack) {
                    logger::trace("SADSJFHOADF 3");
                    if (Utilities::FunctionsSkyrim::WorldObject::GetObjectCount(dropped_stage_ref) != static_cast<int16_t>(count)) {
                        Utilities::FunctionsSkyrim::WorldObject::SetObjectCount(dropped_stage_ref, instance->count);
                    }
                    //dropped_stage_ref->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
                    auto bound = instance->xtra.is_fake ? source->GetBoundObject() : nullptr;
                    const auto temp_stage_no = instance->no;
                    // I NEED TO DO EVERYTHING THAT USES instance BEFORE MoveInstance!!!!!!!!
                    if (!source->MoveInstance(player_refid, dropped_stage_ref->GetFormID(), instance)) {
                        return RaiseMngrErr("HandleDrop: MoveInstance failed.");
                    }
                    ApplyStageInWorld(dropped_stage_ref, source->stages[temp_stage_no], bound);
                    handled_first_stack = true;
                } 
                else {
                    logger::trace("SADSJFHOADF 4");
                    const auto temp_is_fake = instance->xtra.is_fake;
                    const char* extra_text = temp_is_fake ? source->stages[instance->no].GetExtraText() : nullptr;
                    const auto bound_to_drop = temp_is_fake ? source->GetBoundObject() : instance->GetBound();
                    auto new_ref =
                        Utilities::FunctionsSkyrim::WorldObject::DropObjectIntoTheWorld(bound_to_drop, instance->count);
                    if (!new_ref) return RaiseMngrErr("HandleDrop: New ref is null.");
                    if (new_ref->extraList.GetCount() != instance->count) {
                        logger::warn("HandleDrop: NewRefCount mismatch: {} , {}", new_ref->extraList.GetCount(), instance->count);
                    }
                    // I NEED TO DO EVERYTHING THAT USES instance BEFORE MoveInstance!!!!!!!!
                    if (!source->MoveInstance(player_refid, new_ref->GetFormID(), instance)) {
						return RaiseMngrErr("HandleDrop: MoveInstance failed.");
					}
                    if (temp_is_fake) _ApplyStageInWorld_Fake(new_ref, extra_text);
                }
            }
        }

        if (count > 0) {
            logger::warn("HandleDrop: Count is still greater than 0. Dropping the rest to the world.");
            auto dropbound = Utilities::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(dropped_formid);
            auto new_ref = Utilities::FunctionsSkyrim::WorldObject::DropObjectIntoTheWorld(dropbound, count);
            if (!new_ref) {
                logger::error("HandleDrop: New ref is null.");
                return;
            }
            const auto __stage_no = GetStageNoFromSource(source, dropped_formid);
            if (!source->InitInsertInstanceWO(__stage_no, count, new_ref->GetFormID())) {
            	return RaiseMngrErr("HandleDrop: InsertNewInstance failed 2.");
            }
            //AddItem(player_ref, nullptr, dropped_formid, count);
            
        }

        //dropped_stage_ref->extraList.RemoveByType(RE::ExtraDataType::kOwnership);
        Utilities::FunctionsSkyrim::xData::PrintObjectExtraData(dropped_stage_ref);
        // TODO: add delayer stuff

        source->CleanUpData();
        source->PrintData();
        //mutex.unlock();
    }

    // TAMAM
    // giris noktasi RegisterAndGo uzerinden
    // registeredsa update ediyo inventoryi
    void HandlePickUp(const FormID pickedup_formid, const Count count, const RefID wo_refid, const bool eat,
                      RE::TESObjectREFR* npc_ref = nullptr) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::info("HandlePickUp: Formid {} , Count {} , Refid {}", pickedup_formid, count, wo_refid);
        const RefID npc_refid = npc_ref ? npc_ref->GetFormID() : player_refid;
        npc_ref = npc_ref ? npc_ref : player_ref; // naming...for the sake of functionality
        if (!RefIsRegistered(wo_refid)) {
            if (worldobjectsevolve){
                // bcs it shoulda been registered already before picking up (with some exceptions)
                logger::warn("HandlePickUp: Not registered world object refid: {}, pickedup_formid: {}", wo_refid,
                             pickedup_formid);
            }
            return RegisterAndGo(pickedup_formid, count, npc_refid);
        }
        // so it was registered before
        auto source = GetWOSource(wo_refid);
        if (!source) return RaiseMngrErr("HandlePickUp: Source not found.");
        auto st_inst = GetWOStageInstance(wo_refid);
        if (!st_inst) return RaiseMngrErr("HandlePickUp: Source not found.");
        const auto instance_is_fake = st_inst->xtra.is_fake;
        const auto instance_formid = st_inst->xtra.form_id;
        const auto instance_bound = st_inst->GetBound();
        // I NEED TO DO EVERYTHING THAT USES instance BEFORE MoveInstance!!!!!!!!
        if (!source->MoveInstance(wo_refid, npc_refid, st_inst)) return RaiseMngrErr("HandlePickUp: MoveInstance failed.");
        if (instance_is_fake && pickedup_formid != source->formid) logger::warn("HandlePickUp: Not the same formid as source formid. OK if NPC picked up.");
        ApplyEvolutionInInventory(source->qFormType, npc_ref, count, pickedup_formid, instance_formid);
        if (eat && npc_refid == player_refid) RE::ActorEquipManager::GetSingleton()->EquipObject(RE::PlayerCharacter::GetSingleton(), 
            instance_bound,nullptr, count);
        else source->CleanUpData();

        source->PrintData();
        
        // TODO: add delayer stuff here and prolly also updating
    }
    // TAMAM
    // UpdateStages
    void HandleConsume(const FormID stage_formid) {
        ENABLE_IF_NOT_UNINSTALLED
        if (!stage_formid) {
            logger::warn("HandleConsume:Formid is null.");
            return;
        }
        auto source = GetSource(stage_formid);
        if (!source) {
            logger::warn("HandleConsume:Source not found.");
            return;
        }
        if (stage_formid == source->formid && !IsStage(stage_formid, source)) {
			logger::warn("HandleConsume:Formid is the same as the source formid.");
			return;
		}

        logger::trace("HandleConsume");

        int total_registered_count = 0;
        std::vector<StageInstance*> instances_candidates = {};
        for (auto& st_inst : source->data[player_refid]) {
            if (st_inst.xtra.form_id == stage_formid) {
                total_registered_count += st_inst.count;
                instances_candidates.push_back(&st_inst);
            }
        }

        if (instances_candidates.empty()) {
            logger::warn("HandleConsume: No instances found.");
            return;
        }
        if (total_registered_count == 0) {
			logger::warn("HandleConsume: Nothing to consume.");
			return;
		}

        // check if player has the item
        // sometimes player does not have the item but it can still be there with count = 0.
        const auto player_inventory = player_ref->GetInventory();
        const auto entry = player_inventory.find(Utilities::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(stage_formid));
        const auto player_count = entry != player_inventory.end() ? entry->second.first : 0;
        int diff = total_registered_count - player_count;
        if (diff < 0) {
            logger::warn("HandleConsume: Something could have gone wrong with registration.");
            return;
        }
        if (diff == 0) {
            logger::warn("HandleConsume: Nothing to remove.");
            return;
        }
        
        logger::trace("HandleConsume: Adjusting registered count");

        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        std::sort(instances_candidates.begin(), instances_candidates.end(),
                  [curr_time](StageInstance* a, StageInstance* b) { 
                return a->GetElapsed(curr_time) > b->GetElapsed(curr_time);  // eat the older stuff but at same stage
            });

        for (auto& instance : instances_candidates) {
            if (diff <= instance->count) {
                instance->count -= diff;
                break;
			} else {
                diff -= instance->count;
                instance->count = 0;
			}
		}

        UpdateStages(player_ref);

        source->CleanUpData();
        logger::trace("HandleConsume: updated.");

    }
    
    // TAMAM
    // giris noktasi RegisterAndGo uzerinden
    void HandleBuy(const FormID bought_formid, const Count bought_count, const RefID vendor_chest_refid){
        ENABLE_IF_NOT_UNINSTALLED
		if (!bought_formid) {
			logger::warn("HandleBuy: Formid is null.");
			return;
		}
		if (!bought_count) {
			logger::warn("HandleBuy: Count is 0.");
			return;
		}
		if (!vendor_chest_refid) {
			logger::warn("HandleBuy: Vendor chest refid is null.");
			return;
		}

		logger::trace("HandleBuy: Formid {} , Count {} , Vendor chest refid {}", bought_formid, bought_count, vendor_chest_refid);
        if (IsExternalContainer(bought_formid,vendor_chest_refid)) {
            UnLinkExternalContainer(bought_formid, bought_count,vendor_chest_refid);
		} else RegisterAndGo(bought_formid, bought_count, player_refid);

    }

    // TAMAM
    void HandleCraftingEnter(unsigned int bench_type) {
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleCraftingEnter. bench_type: {}", bench_type);

        if (handle_crafting_instances.size() > 0) {
            logger::warn("HandleCraftingEnter: Crafting instances already exist.");
            return;
        }

        if (!Settings::qform_bench_map.contains(bench_type)) {
			logger::warn("HandleCraftingEnter: Bench type not found.");
			return;
		}

        const auto& q_form_types = Settings::qform_bench_map.at(bench_type);
        
        // trusting that the player will leave the crafting menu at some point and everything will be reverted
        std::map<FormID,int> to_remove;
        const auto player_inventory = player_ref->GetInventory();
        for (auto& src : sources) {
            if (src.data.empty()) {
				logger::warn("HandleCraftingEnter: Source data is empty.");
				continue;
			}
            if (!Utilities::Functions::Vector::HasElement<std::string>(q_form_types, src.qFormType)) {
                logger::trace("HandleCraftingEnter: qFormType mismatch: {} , {}", src.qFormType, bench_type);
                continue;
            }
            src.PrintData();
            // just to align reality and registries:
            logger::info("HandleCraftingEnter: Just to align reality and registries");
            //AlignRegistries({player_refid});
            if (!src.data.contains(player_refid)) continue;
            for (auto& st_inst : src.data[player_refid]) {
                if (!st_inst.xtra.crafting_allowed) continue;
                const auto stage_formid = st_inst.xtra.form_id;
                const FormFormID temp = {src.formid, stage_formid}; // formid1: source formid, formid2: stage formid
                if (!handle_crafting_instances.contains(temp)) {
                    const auto stage_bound = st_inst.GetBound();
                    if (!stage_bound) return RaiseMngrErr("HandleCraftingEnter: Stage bound is null.");
                    const auto src_bound = src.GetBoundObject();
                    const auto it = player_inventory.find(src_bound);
                    const auto count_src = it != player_inventory.end() ? it->second.first : 0;
                    handle_crafting_instances[temp] = {st_inst.count, count_src};
                } 
                else handle_crafting_instances[temp].first += st_inst.count;
                if (!faves_list.contains(stage_formid)) faves_list[stage_formid] = Utilities::FunctionsSkyrim::Inventory::IsFavorited(stage_formid,player_refid);
                else if (!faves_list[stage_formid]) faves_list[stage_formid] = Utilities::FunctionsSkyrim::Inventory::IsFavorited(stage_formid,player_refid);
                if (!equipped_list.contains(stage_formid)) equipped_list[stage_formid] = Utilities::FunctionsSkyrim::Inventory::IsEquipped(stage_formid);
				else if (!equipped_list[stage_formid]) equipped_list[stage_formid] = Utilities::FunctionsSkyrim::Inventory::IsEquipped(stage_formid);
            }
        }
        
        if (handle_crafting_instances.empty()) {
            logger::warn("HandleCraftingEnter: No instances found.");
            return;
        }
        for (auto& [formids, counts] : handle_crafting_instances) {
            RemoveItemReverse(player_ref, nullptr, formids.form_id2, counts.first, RE::ITEM_REMOVE_REASON::kRemove);
            AddItem(player_ref, nullptr, formids.form_id1, counts.first);
            logger::trace("Crafting item updated in inventory.");
        }
        // print handle_crafting_instances
        for (auto& [formids, counts] : handle_crafting_instances) {
			logger::info("HandleCraftingEnter: Formid1: {} , Formid2: {} , Count1: {} , Count2: {}", formids.form_id1, formids.form_id2, counts.first, counts.second);
		}

    }

    // TAMAM
    void HandleCraftingExit() {
        ENABLE_IF_NOT_UNINSTALLED
        logger::trace("HandleCraftingExit");

        if (handle_crafting_instances.empty()) {
			logger::warn("HandleCraftingExit: No instances found.");
            faves_list.clear();
            equipped_list.clear();
			return;
		}

        logger::trace("Crafting menu closed");
        for (auto& [formids, counts] : handle_crafting_instances) {
            logger::info("HandleCraftingExit: Formid1: {} , Formid2: {} , Count1: {} , Count2: {}", formids.form_id1,
                         formids.form_id2, counts.first, counts.second);
        }

        // need to figure out how many items were used up in crafting and how many were left

        const auto player_inventory = player_ref->GetInventory();
        for (auto& [formids, counts] : handle_crafting_instances) {
            const auto src_bound = Utilities::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formids.form_id1);
            const auto it = player_inventory.find(src_bound);
            const auto inventory_count = it != player_inventory.end() ? it->second.first : 0;
            const auto expected_count = counts.first + counts.second;
            auto diff = expected_count - inventory_count; // crafta kullanilan item sayisi
            const auto to_be_removed_added = inventory_count - counts.second;
            if (to_be_removed_added > 0) {
                RemoveItemReverse(player_ref, nullptr, formids.form_id1, to_be_removed_added,
                                  RE::ITEM_REMOVE_REASON::kRemove);
                AddItem(player_ref, nullptr, formids.form_id2, to_be_removed_added);
                bool __faved = faves_list[formids.form_id2];
                if (__faved) Utilities::FunctionsSkyrim::Inventory::FavoriteItem(formids.form_id2, player_refid);
                bool __equipped = equipped_list[formids.form_id2];
                if (__equipped) Utilities::FunctionsSkyrim::Inventory::EquipItem(formids.form_id2, false);
			}
            if (diff<=0) continue;
            HandleConsume(formids.form_id2);
        }


        handle_crafting_instances.clear();
        faves_list.clear();
        equipped_list.clear();

    }

    // TAMAM
    [[nodiscard]] const bool IsExternalContainer(const FormID stage_formid, const RefID refid) {
        if (!refid) return false;
        auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(refid);
        if (!ref) return false;
        if (!ref->HasContainer()) return false;
        const auto src = GetSource(stage_formid);
        if (!src) return false;
        if (src->formid==stage_formid && !IsStage(stage_formid,src)) return false;
        if (src->data.contains(refid)) return true;
        return false;
    }

    // TAMAM
    [[nodiscard]] const bool IsExternalContainer(const RE::TESObjectREFR* external_ref) {
        if (!external_ref) return false;
        if (!external_ref->HasContainer()) return false;
        const auto external_refid = external_ref->GetFormID();
        for (const auto& src : sources) {
			if (src.data.contains(external_refid)) return true;
		}
        return false;
    }

    [[nodiscard]] const bool IsExternalContainer(const RefID refid) {
		if (!refid) return false;
		auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(refid);
		if (!ref) return false;
		return IsExternalContainer(ref);
	}

    // TAMAM
    void LinkExternalContainer(const FormID some_formid, Count item_count, const RefID externalcontainer) {
        ENABLE_IF_NOT_UNINSTALLED
        if (!some_formid) {
			logger::error("Fake formid is null.");
			return;
		}
        if (!item_count) {
            logger::error("Item count is 0.");
            return;
        }

        const auto external_ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(externalcontainer);
        if (!external_ref) {
            logger::critical("External ref is null.");
            return;
        }
        if (!external_ref->HasContainer() || externalcontainer==player_refid) {
            logger::error("External container does not have a container or is player ref.");
            return;
        }
        const auto source = GetSource(some_formid);
        if (!source) {
            logger::trace("LinkExternalContainer: Source not found.");
			return;
        } else if (source->formid == some_formid && !IsStage(some_formid,source)) {
            logger::trace("LinkExternalContainer: Source same formid and not stage");
            return;
        }


        logger::trace("Linking external container.");

        if (const auto rest_ = source->MoveInstances(player_refid, externalcontainer, some_formid, item_count, true)) {
            logger::warn("LinkExternalContainer: Rest count: {}", rest_);
            RegisterAndGo(source->formid, rest_, externalcontainer);
		}

        source->CleanUpData();

   //     if (Utilities::FunctionsSkyrim::Inventory::IsFavorited(stage_formid, externalcontainer))
    }

    // TAMAM
    void UnLinkExternalContainer(const FormID some_formid, Count count, const RefID externalcontainer) { 
        // bu itemla bu containera linked olduunu varsayuyo
        ENABLE_IF_NOT_UNINSTALLED

        if (!some_formid) {
			logger::error("Fake formid is null.");
			return;
		}
        if (!count) {
			logger::error("Item count is 0.");
			return;
		}

        if (!externalcontainer) {
            logger::critical("External container is null.");
            return;
        }

        const auto source = GetSource(some_formid);
        if (!source) {
            logger::trace("LinkExternalContainer: Source not found.");
            return;
        } else if (source->formid == some_formid && !IsStage(some_formid,source)) {
            logger::trace("LinkExternalContainer: Source same formid and not stage");
            return;
        }

        logger::trace("Unlinking external container. Formid {} , Count {} , External container refid {}", some_formid, count, externalcontainer);
        // if there is count left, then we should register it in the player's inventory
        if (const auto rest_ = source->MoveInstances(externalcontainer, player_refid, some_formid, count, false)) {
            logger::warn("UnLinkExternalContainer: Rest count: {}", rest_);
            RegisterAndGo(some_formid, rest_, player_refid);
		}
        
        source->CleanUpData();
        logger::trace("Unlinked external container.");

        // TODO: add delayer stuff here and prolly also updating

        logger::trace("Unlinked external container.");
        //     if (Utilities::FunctionsSkyrim::Inventory::IsFavorited(stage_formid, externalcontainer))
    }

    // TAMAM
    // queued time modulator updates, update stages, update time modulators in inventory
    bool UpdateStages(RE::TESObjectREFR* ref) {
        // assumes that the ref is registered
        logger::trace("Manager: Updating stages.");
        if (!ref) {
            logger::critical("UpdateStages: ref is null.");
            return false;
        }
        if (sources.empty()) {
			logger::warn("UpdateStages: Sources is empty.");
			return false;
		}
        
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        
        // time modulator updates
        const bool is_inventory = ref->HasContainer() || ref->IsPlayerRef();
        bool update_took_place = false;
        if (is_inventory) {
            // if there are time modulators which can also evolve, they need to be updated first
            const bool temp_update_took_place = _UpdateTimeModulators(ref, curr_time);
            if (!update_took_place) update_took_place = temp_update_took_place;
        }
        else if (!worldobjectsevolve) return false;

        // need to handle queued_time_modulator_updates
        // order them by GetRemainingTime method of QueuedTModUpdate
        const bool temp_update_took_place = _UpdateStagesOfRef(ref, curr_time, is_inventory);
        if (!update_took_place) update_took_place = temp_update_took_place;
        if (!update_took_place) logger::trace("No update1");

        return update_took_place;
    }

    // TAMAM
    bool UpdateStages(RefID loc_refid) {
        logger::trace("Manager: Updating stages for loc_refid {}.",loc_refid);
        if (!loc_refid) {
			logger::critical("UpdateStages: loc_refid is null.");
			return false;
		}
        auto loc_ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(loc_refid);
        if (!loc_ref) {
            logger::critical("UpdateStages: loc_ref is null.");
            return false;
        }
        return UpdateStages(loc_ref);
    }

    // only necessary for the world objects who has fake counterparts
    // suan sadece npc pick up icin kullaniliyor
    // TAMAM
    void SwapWithStage(RE::TESObjectREFR* wo_ref) {
        // registered olduunu varsayiyoruz
        logger::trace("SwapWithStage");
        if (!wo_ref) return RaiseMngrErr("Ref is null.");
        const auto st_inst = GetWOStageInstance(wo_ref);
        if (!st_inst) {
        	logger::warn("SwapWithStage: Source not found.");
			return;
        }
        // remove the extra data that cause issues when dropping
        Utilities::FunctionsSkyrim::xData::PrintObjectExtraData(wo_ref);
        for (const auto& i : Settings::xRemove)
            wo_ref->extraList.RemoveByType(static_cast<RE::ExtraDataType>(i));
        Utilities::FunctionsSkyrim::WorldObject::SwapObjects(wo_ref, st_inst->GetBound(), false);
    }

    [[nodiscard]] const bool IsTimeModulator(const FormID formid) {
        for (const auto& src : sources) {
            if (src.IsTimeModulator(formid)) return true;
        }
        return false;
    }

    void Reset() {
		ENABLE_IF_NOT_UNINSTALLED
        logger::info("Resetting manager...");
        for (auto& src : sources) src.Reset();
        sources.clear();
        external_favs.clear();         // we will update this in ReceiveData
        handle_crafting_instances.clear();
        faves_list.clear();
        Clear();
        setListenMenuOpenClose(true);
        setListenActivate(true);
        setListenContainerChange(true);
        setListenCrosshair(true);
        setUninstalled(false);
        logger::info("Manager reset.");
	}

   // void SendData() {
   //     ENABLE_IF_NOT_UNINSTALLED
   //         // TODO: dont serialize empty sources!
   //     // std::lock_guard<std::mutex> lock(mutex);
   //     logger::info("--------Sending data---------");
   //     Print();
   //     Clear();
   //     for (auto& src : sources) {
   //         Utilities::Types::SaveDataLHS lhs{src.formid, src.editorid};
   //         Utilities::Types::SaveDataRHS rhs = src.data;
   //         for (auto& st_inst : rhs) {
   //             st_inst.xtra.is_favorited = st_inst.location == player_refid && 
   //                 Utilities::FunctionsSkyrim::IsPlayerFavorited(st_inst.GetBound());
   //             st_inst.xtra.is_favorited = external_favs.contains(st_inst.location) && 
   //                 external_favs[st_inst.location].contains(st_inst.xtra.form_id);
			//}
   //         SetData(lhs, rhs);
   //     }
   //     logger::info("Data sent.");
   // };

   // void ReceiveData() {
   //     logger::info("--------Receiving data---------");

   //     // std::lock_guard<std::mutex> lock(mutex);

   //     if (!empty_mgeff) return RaiseMngrErr("ReceiveData: Empty mgeff not there!");

   //     setListenContainerChange(false);

   //     for (const auto& [lhs, rhs] : m_Data) {
   //         const auto source_form = Utilities::FunctionsSkyrim::GetFormByID(lhs.form_id, lhs.editor_id);
   //         if (!source_form) {
   //             logger::critical("ReceiveData: Source form not found. Saved formid: {}, editorid: {}", lhs.form_id, lhs.editor_id);
			//	return RaiseMngrErr("ReceiveData: Source form not found.");
			//}
   //         if (GetSource(source_form->GetFormID())){
   //         	logger::critical("ReceiveData: Source already exists. Formid {}", source_form->GetFormID());
			//	return RaiseMngrErr("ReceiveData: Source already exists.");
   //         }
   //         Source src(source_form->GetFormID(), "", empty_mgeff);
   //         if (src.init_failed) {
			//	logger::critical("ReceiveData: Source init failed. Formid {}", source_form->GetFormID());
   //             return RaiseMngrErr("ReceiveData: Source init failed.");
   //         }
			//for (const auto& st_inst : rhs) {
   //             const auto formid = st_inst.xtra.form_id;
   //             const auto editorid = st_inst.xtra.editor_id;
   //             const auto form = Utilities::FunctionsSkyrim::GetFormByID(formid, editorid);
   //             if (!form) {
   //                 if (st_inst.xtra.is_fake) {
   //                     logger::info("ReceiveData: Fake form not found. Formid {} Editorid {}", formid, editorid);
   //                     logger::info("Replacing with new fake form.");
   //                 }
   //                 logger::critical("ReceiveData: Form not found. Formid {} Editorid {}", formid, editorid);
			//		return RaiseMngrErr("ReceiveData: Form not found.");
			//	}
   //             StageInstance copyInst(st_inst);
   //             if (!src.InsertNewInstance(copyInst)) {
			//		logger::critical("ReceiveData: InsertNewInstance failed. Formid {} Editorid {}", lhs.form_id, lhs.editor_id);
			//		Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Failed to receive one of the instances from your save. \
   //                     This is expected if you changed/deleted things in your config. Marking this instance as decayed.");
			//	}
			//}
   //     
   //     }
   //     setListenContainerChange(true);
   // }

    void Print() {
        for (auto& src : sources) {
		    src.PrintData();
        }
    }

#undef ENABLE_IF_NOT_UNINSTALLED
};