#pragma once
#include "SimpleIni.h"
#include "Utils.h"

using namespace Utilities::Types;

namespace Settings {

    bool failed_to_load = false;

    constexpr std::uint32_t kSerializationVersion = 626;
    constexpr std::uint32_t kDataKey = 'QAOT';

    constexpr auto exclude_path_ = "Data/SKSE/Plugins/AoT_exclude"; //txt
    constexpr auto defaults_path_ = "Data/SKSE/Plugins/AoT_default";  // yml
    constexpr auto customs_path_ = "Data/SKSE/Plugins/AoT_custom";  // yml


    struct DefaultSettings {
        std::map<StageNo, FormID> items = {};
        std::map<StageNo, Duration> durations = {};
        std::map<StageNo, StageName> stage_names = {};
        std::map<StageNo, bool> crafting_allowed = {};
        std::map<StageNo, int> costoverrides = {};
        std::map<StageNo, float> weightoverrides = {};
        std::map<StageNo, std::vector<StageEffect>> effects = {};
        std::vector<StageNo> numbers = {};
        FormID decayed_id = 0;

        std::map<FormID,float> delayers;

        [[nodiscard]] const bool CheckIntegrity() {
            if (items.empty() || durations.empty() || stage_names.empty() || effects.empty() || numbers.empty()) {
                logger::error("One of the maps is empty.");
                // list sizes of each
                logger::info("Items size: {}", items.size());
                logger::info("Durations size: {}", durations.size());
                logger::info("Stage names size: {}", stage_names.size());
                logger::info("Effects size: {}", effects.size());
                logger::info("Numbers size: {}", numbers.size());

                return false;
            }
            if (items.size() != durations.size() || items.size() != stage_names.size() || items.size() != numbers.size()) {
				logger::error("Sizes do not match.");
				return false;
			}
            for (auto i = 0; i < numbers.size(); i++) {
                if (!Utilities::Functions::Vector::VectorHasElement<StageNo>(numbers, i)) {
                    logger::error("Key {} not found in numbers.", i);
                    return false;
                }
                if (!items.count(i) || !crafting_allowed.count(i) || !durations.count(i) || !stage_names.count(i) ||
                    !effects.count(i)) {
					logger::error("Key {} not found in all maps.", i);
					return false;
				}
			}
            if (!decayed_id) {
                logger::error("Decayed id is 0.");
                return false;
            }
			return true;
        }
    };

    std::vector <std::string> QFORMS = {"FOOD"};
    // key: qfromtype ->
    using CustomSettings = std::map<std::vector<std::string>, DefaultSettings>;
    std::map<std::string,DefaultSettings> defaultsettings;
    std::map<std::string, CustomSettings> custom_settings;
    std::map <std::string,std::vector<std::string>> exclude_list;

    std::vector<std::string> LoadExcludeList(const std::string postfix) {
        const auto exclude_path = std::string(exclude_path_) + postfix + ".txt";
        logger::trace("Exclude path: {}", exclude_path);
        std::ifstream file(exclude_path);
        std::vector<std::string> strings;
        std::string line;
        while (std::getline(file, line)) {
            strings.push_back(line);
        }
        return strings;
    }

    [[nodiscard]] const bool IsQFormType(const FormID formid, const std::string& qformtype) {
        // POPULATE THIS
        if (qformtype == "FOOD") return Utilities::FunctionsSkyrim::IsFoodItem(formid);
        else return false;
    }

    std::string GetQFormType(const FormID formid) {
        for (const auto& q_ftype : QFORMS) {
            if (Settings::IsQFormType(formid,q_ftype)) return q_ftype;
		}
		return "";
    }

    [[nodiscard]] const bool IsInExclude(const FormID formid, std::string type = "") {
        auto form = Utilities::FunctionsSkyrim::GetFormByID(formid);
        if (!form) {
            logger::warn("Form not found.");
            return false;
        }
        
        if (type.empty()) type = GetQFormType(formid);
        if (type.empty()) {
            logger::trace("Type is empty. for formid: {}", formid);
			return false;
		}
        if (!Settings::exclude_list.count(type)) {
			logger::critical("Type not found in exclude list. for formid: {}", formid);
            return false;
        }

        std::string form_string = std::string(form->GetName());
        
        /*const auto exlude_list = LoadExcludeList(postfix);*/
        if (Utilities::Functions::String::includesWord(form_string, Settings::exclude_list[type])) {
            logger::trace("Form is in exclude list.form_string: {}", form_string);
            return true;
        }
        return false;
    }

    [[nodiscard]] const bool IsItem(const FormID formid, std::string type = "") {
        if (!formid) return false;
        if (Settings::IsInExclude(formid)) return false;
        if (type.empty()) {
            return !GetQFormType(formid).empty();
		} else return IsQFormType(formid, type);
            
    }

    [[nodiscard]] const bool IsItem(const RE::TESObjectREFR* ref, std::string type = "") {
        if (!ref) return false;
        if (ref->IsDisabled()) return false;
        if (ref->IsDeleted()) return false;
        const auto base = ref->GetBaseObject();
        if (!base) return false;
        return IsItem(base->GetFormID(),type);
    }

    DefaultSettings _parseDefaults(const YAML::Node& config) {
        logger::info("Parsing settings.");
        DefaultSettings settings;
         //we have:stages, decayedFormEditorID and delayers
        if (!config["stages"] || config["stages"].size() == 0) {
            logger::error("Stages are empty.");
            return settings;
        }
        for (const auto& stageNode : config["stages"]) {
            if (!stageNode["no"] || stageNode["no"].IsNull()) {
				logger::error("No is missing for stage.");
				return DefaultSettings();
			}
            const auto temp_no = stageNode["no"].as<StageNo>();
            // add to numbers
            logger::info("Stage no: {}", temp_no);
            settings.numbers.push_back(temp_no);
            const auto temp_formeditorid = stageNode["FormEditorID"].as<std::string>();
            const auto temp_formid = temp_formeditorid.empty()
                                         ? 0
                                         : Utilities::FunctionsSkyrim::GetFormEditorIDFromString(temp_formeditorid);
            if (temp_formid < 0) {
                logger::error("Formid is less than 0.");
                return DefaultSettings();
            }
            // add to items
            logger::info("Formid");
            settings.items[temp_no] = temp_formid;
            // add to durations
            logger::info("Duration");
            settings.durations[temp_no] = stageNode["duration"].as<Duration>();
            // add to stage_names
            logger::info("Name");
            if (!stageNode["name"].IsNull()) {
                const auto temp_name = stageNode["name"].as<StageName>();
                // if it is empty, or just whitespace, set it to empty
                if (temp_name.empty() || std::all_of(temp_name.begin(), temp_name.end(), isspace))
					settings.stage_names[temp_no] = "";
				else settings.stage_names[temp_no] = stageNode["name"].as<StageName>();
            } else settings.stage_names[temp_no] = "";
            // add to costoverrides
            logger::info("Cost");
            if (stageNode["value"] && !stageNode["value"].IsNull()) {
                settings.costoverrides[temp_no] = stageNode["value"].as<int>();
            } else settings.costoverrides[temp_no] = -1;  // Or whatever default value you prefer
            // add to weightoverrides
            logger::info("Weight");
            if (stageNode["weight"] && !stageNode["weight"].IsNull()) {
                settings.weightoverrides[temp_no] = stageNode["weight"].as<float>();
            } else settings.weightoverrides[temp_no] = -1.0f;  // Or whatever default value you prefer
            
            // add to crafting_allowed
            logger::info("Crafting");
            settings.crafting_allowed[temp_no] = stageNode["crafting_allowed"].as<bool>();


            // add to effects
            logger::info("Effects");
            std::vector<StageEffect> effects;
            if (!stageNode["mgeffect"] || stageNode["mgeffect"].size() == 0) {
				logger::info("Effects are empty.");
            } else {
                for (const auto& effectNode : stageNode["mgeffect"]) {
                    const auto temp_effect_formeditorid = effectNode["FormEditorID"].as<std::string>();
                    const auto temp_effect_formid =
                        temp_effect_formeditorid.empty()
                            ? 0
                            : Utilities::FunctionsSkyrim::GetFormEditorIDFromString(temp_effect_formeditorid);
                    if (temp_effect_formid < 0) {
                        logger::error("Effect Formid is less than 0.");
                        return DefaultSettings();
                    }
                    if (temp_effect_formid>0){
                        const auto temp_magnitude = effectNode["magnitude"].as<float>();
                        const auto temp_duration = effectNode["duration"].as<Duration>();
                        effects.push_back(StageEffect(temp_effect_formid, temp_magnitude, temp_duration));
                    } else effects.push_back(StageEffect(temp_effect_formid, 0, 0));
                }
            }
            settings.effects[temp_no] = effects;
        }
        // final formid
        const auto temp_decayed_id =
            Utilities::FunctionsSkyrim::GetFormEditorIDFromString(config["finalFormEditorID"].as<std::string>());
        if (temp_decayed_id < 0) {
            logger::error("Decayed Formid is less than 0.");
            return DefaultSettings();
        }
        settings.decayed_id = temp_decayed_id;
        // delayers
        logger::info("timeModulators");
        for (const auto& modulator : config["timeModulators"]) {
            const auto temp_formeditorid = modulator["FormEditorID"].as<std::string>();
            const auto temp_formid = Utilities::FunctionsSkyrim::GetFormEditorIDFromString(temp_formeditorid);
            if (temp_formid < 0) {
                logger::warn("Delayer Formid is less than 0.");
                continue;
            }
            settings.delayers[temp_formid] = modulator["magnitude"].as<float>();
        }
        if (!settings.CheckIntegrity()) {
            logger::critical("Settings integrity check failed.");
            return DefaultSettings();
        }
        return settings;
    }

    DefaultSettings parseDefaults(std::string _type){ 
        const auto filename = std::string(defaults_path_) + _type + ".yml";
        logger::info("Filename: {}", filename);
		YAML::Node config = YAML::LoadFile(filename);
        logger::info("asldkjfösadjn");
		return _parseDefaults(config);
    }

    CustomSettings parseCustoms(std::string _type){
        CustomSettings _custom_settings;
		const auto filename = std::string(customs_path_) + _type + ".yml";
        YAML::Node config = YAML::LoadFile(filename);

        for (const auto& _Node : config["ownerLists"]){
            // we have list of owners at each node or a scalar owner
            if (_Node["owners"].IsScalar()) {
                const auto ownerName = _Node["owners"].as<std::string>();
                _custom_settings[std::vector<std::string>{ownerName}] = _parseDefaults(_Node);
			} 
            else {
				std::vector<std::string> owners;
                for (const auto& owner : _Node["owners"]) {
					owners.push_back(owner.as<std::string>());
				}
                _custom_settings[owners] = _parseDefaults(_Node);
			}
        }
        return _custom_settings;
    }

    void LoadSettings() {
            for (const auto& _qftype: Settings::QFORMS) {
                try {
                    logger::info("Loading defaultsettings for {}", _qftype);
			        defaultsettings[_qftype] = parseDefaults(_qftype);
                } catch (const std::exception& ex) {
                    logger::critical("Failed to load default settings for {}: {}", _qftype, ex.what());
                    failed_to_load = true;
                    return;
                }
                try {
                    logger::info("Loading custom settings for {}", _qftype);
			        custom_settings[_qftype] = parseCustoms(_qftype);
                } catch (const std::exception&) {
                    logger::critical("Failed to load custom settings for {}", _qftype);
					failed_to_load = true;
                    return;
                }
                for (const auto& [key,_] : custom_settings[_qftype]) {
                    logger::trace("Key: {}", key.front());
                }
                try {
                    exclude_list[_qftype] = LoadExcludeList(_qftype);
                } catch (const std::exception&) {
                    logger::critical("Failed to load exclude list for {}", _qftype);
                    failed_to_load = true;
                    return;
                }
		    }
    }

    // 0x99 - ExtraTextDisplayData 
    // 0x3C - ExtraSavedHavokData
    // 0x0B - ExtraPersistentCell
    // 0x48 - ExtraStartingWorldOrCell

    // 0x21 - ExtraOwnership
    // 0x24 - ExtraCount
    // 0x0E - ExtraStartingPosition //crahes when removed
    
    // 0x70 - ExtraEncounterZone 112
    // 0x7E - ExtraReservedMarkers 126
    // 0x88 - ExtraAliasInstanceArray 136
    // 0x8C - ExtraPromotedRef 140 NOT OK
    std::vector<int> xRemove = {0x99, 0x3C, 0x0B, 0x48, 0x21, 0x24,
                                0x70, 0x7E, 0x88, 
        0x8C
    };


}

struct Source {
    
    FormID formid=0;
    std::string editorid="";
    StageDict stages;
    SourceData data; // change this to a map with refid as key and vector of instances as value
    RE::EffectSetting* empty_mgeff;
    Settings::DefaultSettings* defaultsettings = nullptr; // eigentlich sollte settings heissen

    bool init_failed = false;
    RE::FormType formtype;
    std::string qFormType;
    std::vector<StageNo> fake_stages = {};
    Stage decayed_stage;

    std::vector<StageInstance*> queued_time_modulator_updates;

    Source(const FormID id, const std::string id_str, RE::EffectSetting* e_m, Settings::DefaultSettings* sttngs=nullptr)
        : formid(id), editorid(id_str), empty_mgeff(e_m), defaultsettings(sttngs) {


        RE::TESForm* form = Utilities::FunctionsSkyrim::GetFormByID(formid, editorid);
        auto bound_ = GetBoundObject();
        if (!form || !bound_) {
            InitFailed();
            return;
        } 
        else {
            formid = form->GetFormID();
            editorid = clib_util::editorID::get_editorID(form);
        }
        
        if (!formid || editorid.empty()) {
			logger::error("Editorid is empty.");
			InitFailed();
			return;
		}

        if (!Settings::IsItem(formid)) {
            InitFailed();
            return;
        }

        qFormType = Settings::GetQFormType(formid);
        if (qFormType.empty()) {
            logger::error("Formtype is not one of the predefined types.");
            InitFailed();
            return;
        } else
            logger::info("Source initializing with QFormType: {}", qFormType);
        
        if (!defaultsettings) defaultsettings = &Settings::defaultsettings[qFormType];
        if (!defaultsettings->CheckIntegrity()) {
            logger::critical("Default settings integrity check failed.");
			InitFailed();
			return;
		}

        formtype = form->GetFormType();

        //make sure the keys in stages are 0 to length-1 with increment 1
        if (stages.size() == 0) {
            // get stages
            
            // POPULATE THIS
            if (qFormType == "FOOD") {
                if (formtype == RE::FormType::AlchemyItem) GatherStages<RE::AlchemyItem>();
			    else if (formtype == RE::FormType::Ingredient) GatherStages<RE::IngredientItem>();
            }
            else {
				InitFailed();
				return;
			}
        }
        else {
            // check if formids exist in the game
            for (auto& [key, value] : stages) {
                if (!Utilities::FunctionsSkyrim::GetFormByID(value.formid, "")) {
                    // make one and replace formid
					logger::warn("Formid {} for stage {} does not exist.", value.formid, key);
                    if (formtype == RE::FormType::AlchemyItem) value.formid = CreateFake<RE::AlchemyItem>(form->As<RE::AlchemyItem>());
                    else if (formtype == RE::FormType::Ingredient) value.formid = CreateFake<RE::IngredientItem>(form->As<RE::IngredientItem>());
                    //else if (formtype == RE::FormType::magi) value.formid = CreateFake<RE::MagicItem>(form->As<RE::MagicItem>());
					else {
						InitFailed();
						return;
					}
                    if (!value.formid) {
                        InitFailed();
                        return;
                    }
                    fake_stages.push_back(key);
				}
			}
        }

        // decayed stage
        decayed_stage = GetFinalStage();
        if (!decayed_stage.CheckIntegrity()) {
			logger::critical("Decayed stage integrity check failed.");
			InitFailed();
			return;
		}

        if (!CheckIntegrity()) {
            logger::critical("CheckIntegrity failed");
            InitFailed();
			return;
        }
    };

    [[maybe_unused]] const std::string_view GetName() {
        auto form = Utilities::FunctionsSkyrim::GetFormByID(formid, editorid);
        if (form) return form->GetName();
        else return "";
    };

    RE::TESBoundObject* GetBoundObject() {
        return Utilities::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid, editorid);
    };

    const std::map<RefID,std::vector<StageUpdate>> UpdateAllStages(const std::vector<RefID>& filter,const float curr_time) {
        logger::trace("Updating all stages.");
        if (init_failed) {
            logger::critical("UpdateAllStages: Initialisation failed.");
            return {};
        }
        // save the updated instances
        std::map<RefID, std::vector<StageUpdate>> updated_instances;
        if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return updated_instances;
		}
        for (auto& reffid : filter) {
            logger::trace("Refid in filter: {}", reffid);
            if (!data.count(reffid)) {
				logger::warn("Refid {} not found in data.", reffid);
				continue;
			}
            auto& instances = data[reffid];
            for (auto& instance : instances) {
                Stage* old_stage = &stages[instance.no];
                Stage* new_stage = nullptr;
                if (_UpdateStageInstance(instance, curr_time)) {
                    if (instance.xtra.is_decayed || !stages.contains(instance.no)) {
                        new_stage = &decayed_stage;
				    }
                    else new_stage = &stages[instance.no];
                    auto is_fake__ = IsFakeStage(instance.no);
                    updated_instances[reffid].emplace_back(old_stage, new_stage, instance.count, is_fake__);
                }
            }
        }
        CleanUpData();
        return updated_instances;
    }

    [[nodiscard]] const bool IsFakeStage(const StageNo no) const {
        return Utilities::Functions::Vector::VectorHasElement<StageNo>(fake_stages, no);
    }

    [[nodiscard]] const StageNo* GetStageNo(const FormID formid_) {
        if (init_failed) {
            logger::critical("GetStageNo: Initialisation failed.");
            return nullptr;
        }
        for (auto& [key, value] : stages) {
            if (value.formid == formid_) return &key;
        }
        return nullptr;
    }

    [[nodiscard]] const bool InsertNewInstance(StageInstance& stage_instance, const RefID loc) { 

        if (init_failed) {
            logger::critical("InsertData: Initialisation failed.");
            return false;
        }

        const auto n = stage_instance.no;
        if (!stages.count(n)) {
            logger::error("Stage {} does not exist.", n);
            return false;
        }
        if (stage_instance.count <= 0) {
            logger::error("Count is less than or equal 0.");
            return false;
        }
        /*if (stage_instance.location == 0) {
			logger::error("Location is 0.");
			return false;
		}*/
        if (stage_instance.xtra.form_id != stages[n].formid) {
			logger::error("Formid does not match the stage formid.");
			return false;
		}
        if (stage_instance.xtra.is_fake != IsFakeStage(n)) {
            logger::error("Fake status does not match the stage fake status.");
            return false;
        }
        if (stage_instance.xtra.is_decayed) {
			logger::error("Decayed status is true.");
			return false;
		}
        if (stage_instance.xtra.crafting_allowed != stages[n].crafting_allowed) {
			logger::error("Crafting allowed status does not match the stage crafting allowed status.");
			return false;
		}

        if (!data.count(loc)) {
            data[loc] = {};
        }
        data[loc].push_back(stage_instance);

        // fillout the xtra of the emplaced instance
        // get the emplaced instance
        /*auto& emplaced_instance = data.back();
        emplaced_instance.xtra.form_id = stages[n].formid;
        emplaced_instance.xtra.editor_id = clib_util::editorID::get_editorID(stages[n].GetBound());
        emplaced_instance.xtra.crafting_allowed = stages[n].crafting_allowed;
        if (IsFakeStage(n)) emplaced_instance.xtra.is_fake = true;*/

        return true;
    }

    [[nodiscard]] const bool InitInsertInstance(StageNo n, Count c, RefID l) {
        if (init_failed) {
            logger::critical("InitInsertInstance: Initialisation failed.");
            return false;
        }
        const float curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        StageInstance new_instance(curr_time, n, c);
        new_instance.xtra.form_id = stages[n].formid;
        new_instance.xtra.editor_id = clib_util::editorID::get_editorID(stages[n].GetBound());
        new_instance.xtra.crafting_allowed = stages[n].crafting_allowed;
        if (IsFakeStage(n)) new_instance.xtra.is_fake = true;

        return InsertNewInstance(new_instance,l);
    }

    // applies time modulation to all instances in the inventory
    [[nodiscard]] const bool InitInsertInstance(StageNo n, Count c, RE::TESObjectREFR* inventory_owner){
        if (!inventory_owner) {
            logger::error("Inventory owner is null.");
            return false;
        }
        const RefID inventory_owner_refid = inventory_owner->GetFormID();
        if (!inventory_owner->HasContainer() && inventory_owner_refid != player_refid) {
        	logger::error("Inventory owner is not a container.");
			return false;
        }
        
        if (!InitInsertInstance(n, c, inventory_owner_refid)) {
			logger::error("InitInsertInstance failed.");
			return false;
		}
        
        SetDelayOfInstance(data[inventory_owner_refid].back(), RE::Calendar::GetSingleton()->GetHoursPassed(),
                            inventory_owner);
        return true;
    }
    
    [[nodiscard]] const bool MoveInstance(const RefID from_ref, const RefID to_ref, StageInstance* st_inst) {
        // Check if the from_ref exists in the data map
        if (data.count(from_ref) == 0) {
            return false;
        }

        // Get the vector of instances from the from_ref key
        std::vector<StageInstance>& from_instances = data[from_ref];
        StageInstance new_instance(*st_inst);

        // Find the instance in the from_instances vector
        auto it = std::find(from_instances.begin(), from_instances.end(), *st_inst);
        if (it == from_instances.end()) {
            return false;
        }

        // Remove the instance from the from_instances vector
        from_instances.erase(it);

        // Add the instance to the to_ref key vector
        if (data.count(to_ref) == 0) {
			data[to_ref] = {};
		}
        data[to_ref].push_back(new_instance);

        return true;
    }

    Count MoveInstances(const RefID from_ref, const RefID to_ref, const FormID instance_formid, Count count, const bool bias_direction) {
        // bias_direction: true to move older instances first
        if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return 0;
		}
        if (init_failed) {
			logger::critical("MoveInstances: Initialisation failed.");
			return 0;
		}
        if (count <= 0) {
            logger::error("Count is less than or equal 0.");
            return 0;
        }
        if (!instance_formid) {
			logger::error("Instance formid is 0.");
			return 0;
		}
        if (from_ref == to_ref) {
			logger::error("From and to refs are the same.");
			return 0;
		}
        if (from_ref == 0 || to_ref == 0) {
            logger::error("Refid is 0.");
            return 0;
        }

        if (data.count(from_ref) == 0) {
			logger::error("From refid not found in data.");
            return count;
		}

        std::vector<size_t> instances_candidates = {};
        size_t index__ = 0;
        for (auto& st_inst : data[from_ref]) {
            if (st_inst.xtra.form_id == instance_formid) {
                instances_candidates.push_back(index__);
            }
            index__++;
        }

        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        if (bias_direction) {
            std::sort(instances_candidates.begin(), instances_candidates.end(),
                      [this, from_ref, curr_time](size_t a, size_t b) {
                          return this->data[from_ref][a].GetElapsed(curr_time) >
                                 this->data[from_ref][b].GetElapsed(curr_time);  // move the older stuff
                      });
        } 
        else {
            std::sort(instances_candidates.begin(), instances_candidates.end(),
                      [this, from_ref, curr_time](size_t a, size_t b) {
                          return this->data[from_ref][a].GetElapsed(curr_time) <
                                 this->data[from_ref][b].GetElapsed(curr_time);  // move the newer stuff
                      });
		}

        if (instances_candidates.empty()) {
            logger::warn("No instances found for formid {} and location {}", instance_formid, from_ref);
            return 0;
        }

        std::vector<size_t> removed_indices;
        for (size_t index: instances_candidates) {
            if (!count) break;
            
            StageInstance* instance = nullptr;
            if (removed_indices.empty()) {
                instance = &data[from_ref][index];
            } else {
                int shift = 0;
                for (size_t removed_index : removed_indices) {
                    if (index == removed_index) {
                        logger::critical("Index is equal to removed index.");
                        return count;
                    }
                    if (index > removed_index) shift++;
                }
                instance = &data[from_ref][index - shift];
            }

            if (count <= instance->count) {
                instance->count -= count;
                StageInstance new_instance(*instance);
                new_instance.count = count;
                if (!InsertNewInstance(new_instance, to_ref)) {
                    logger::error("InsertNewInstance failed.");
                    return 0;
                }
                count = 0;
                //break;
            } else {
                if (!MoveInstance(from_ref, to_ref, instance)) {
					logger::error("MoveInstance failed.");
                    return count;
				}
                count -= instance->count;
                removed_indices.push_back(index);
            }
        }

        return count;

    }
    
    /*void RemoveTimeModulationFromWO(const RefID world_object) {
        if (!world_object) {
			logger::error("World object is null.");
			return;
		}
		if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return;
		}
		for (auto& instance : data) {
			if (instance.location == world_object) {
                if (instance.GetDelayMagnitude() == 1) break;
                instance.SetDelay(RE::Calendar::GetSingleton()->GetHoursPassed(), 1,0);
                break;
            }
		}
    }*/
    
    [[nodiscard]] const bool IsTimeModulator(const FormID _form_id) const {
        if (!_form_id) return false;
        for (const auto& [_, instances] : data) {
			for (const auto& instance : instances) {
				if (instance.GetDelayerFormID() == _form_id) return true;
			}
		}
        return false;
    }

    // always update before doing this
    void UpdateTimeModulationInInventory(RE::TESObjectREFR* inventory_owner, const float curr_time) {
        if (!inventory_owner) {
            logger::error("Inventory owner is null.");
            return;
        }

        const RefID inventory_owner_refid = inventory_owner->GetFormID();
        if (!inventory_owner_refid) {
            logger::error("Inventory owner refid is 0.");
            return;
        }

        if (!inventory_owner->HasContainer() && inventory_owner_refid != player_refid) {
            logger::error("Inventory owner does not have a container.");
            return;
        }

        if (data.count(inventory_owner_refid) == 0) {
			logger::error("Inventory owner refid not found in data: {} and source {}", inventory_owner_refid, editorid);
			return;
		}

        if (data[inventory_owner_refid].empty()) {
            logger::trace("No instances found for inventory owner {} and source {}", inventory_owner_refid, editorid);
            return;
        }

        SetDelayOfInstances(inventory_owner_refid, curr_time, inventory_owner);
    }

    void CleanUpData() {
        if (init_failed) {
            logger::critical("CleanUpData: Initialisation failed.");
            return;
        }
        if (data.empty()) {
            logger::warn("No data found for source {}", editorid);
            return;
        }
		logger::trace("Cleaning up data.");
        PrintData();
        // size before cleanup
        logger::trace("Size before cleanup: {}", data.size());
        // if there are instances with same stage no and location, and start_time, merge them
        
        for (auto it = data.begin(); it != data.end();) {
            if (it->second.empty()) {
                logger::trace("Erasing key from data: {}", it->first);
                it = data.erase(it);
            } else {
                ++it;
            }
        }

        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        for (auto& [_, instances] : data) {
            for (auto it = instances.begin(); it != instances.end(); ++it) {
                for (auto it2 = it; it2 != instances.end(); ++it2) {
				    if (it == it2) continue;
                    if (it2->count <= 0) continue;
                    if (it->AlmostSameExceptCount(*it2, curr_time)) {
                        logger::trace("Merging stage instances with count {} and {}", it->count, it2->count);
					    it->count += it2->count;
					    it2->count = 0;
				    }
			    }
		    }
		    // erase instances with count <= 0
            for (auto it = instances.begin(); it != instances.end();) {
			    if (it->count <= 0) {
				    logger::trace("Erasing stage instance with count {}", it->count);
                    it = instances.erase(it);
                } 
                else if (!stages.count(it->no) || it->xtra.is_decayed) {
				    logger::trace("Erasing decayed stage instance with no {}", it->no);
                    it = instances.erase(it);
                }
                else {
				    ++it;
			    }
		    }
        }
        
        if (!CheckIntegrity()) {
			logger::critical("CheckIntegrity failed");
			InitFailed();
        }

        logger::trace("Size after cleanup: {}", data.size());
	}

    void PrintData() {
        logger::trace("Printing data for source {}", editorid);
		for (auto& [loc,instances] : data) {
            logger::trace("Location: {}", loc);
            for (auto& instance : instances) {
                logger::trace("No: {}, Count: {}, Start time: {}", instance.no,
                    //instance.location,
                              instance.count, instance.start_time);
            }
		}
	
    }

    void Reset() {
        formid = 0;
		editorid = "";
		stages.clear();
		data.clear();
		init_failed = false;
    }
    
private:

    // counta karismiyor
    [[nodiscard]] const bool _UpdateStageInstance(StageInstance& st_inst, const float curr_time) {
        if (init_failed) {
        	logger::critical("_UpdateStage: Initialisation failed.");
            return false;
        }
        if (st_inst.xtra.is_decayed) return false;  // decayed
        
        float diff = st_inst.GetElapsed(curr_time);
        bool updated = false;
        logger::trace("Current time: {}, Start time: {}, Diff: {}, Duration: {}", curr_time, st_inst.start_time, diff,stages[st_inst.no].duration);
        
        while (diff < 0) {
            logger::trace("Updating stage {} to {}", st_inst.no, st_inst.no - 1);
            if (st_inst.no > 0) {
                st_inst.no--;
			    diff += stages[st_inst.no].duration;
                updated = true;
            } else {
                diff = 0;
                break;
            }
        }
        while (diff > stages[st_inst.no].duration) {
            logger::trace("Updating stage {} to {}", st_inst.no, st_inst.no + 1);
			diff -= stages[st_inst.no].duration;
			st_inst.no++;
            updated = true;
            if (!stages.count(st_inst.no)) {
			    logger::trace("Decayed");
                st_inst.xtra.is_decayed= true;
                st_inst.xtra.form_id = decayed_stage.formid;
                st_inst.xtra.editor_id = clib_util::editorID::get_editorID(decayed_stage.GetBound());
                st_inst.xtra.is_fake = false;
                st_inst.xtra.crafting_allowed = false;
                break;
		    }
		}
        if (updated) {
            if (st_inst.xtra.is_decayed) {
                st_inst.xtra.form_id = decayed_stage.formid;
                st_inst.xtra.editor_id = clib_util::editorID::get_editorID(decayed_stage.GetBound());
                st_inst.xtra.is_fake = false;
                st_inst.xtra.crafting_allowed = false;
            } 
            else {
                st_inst.xtra.form_id = stages[st_inst.no].formid;
                st_inst.xtra.editor_id = clib_util::editorID::get_editorID(stages[st_inst.no].GetBound());
                st_inst.xtra.is_fake = IsFakeStage(st_inst.no);
                st_inst.xtra.crafting_allowed = stages[st_inst.no].crafting_allowed;
            }
            // as long as the delay start was before the ueberschreitung time this will work,
            // the delay start cant be strictly after the ueberschreitung time bcs we call update when a new delay
            // starts so the delay start will always be before the ueberschreitung time
            st_inst.SetNewStart(curr_time,diff);
        }
        return updated;
    }

    template <typename T>
    void GatherStages() {
        // for now use default stages
        if (!empty_mgeff) {
            logger::error("Empty mgeff is null.");
            return;
        }

        for (auto i = 0; i < defaultsettings->numbers.size(); i++) {
            // create fake form
            auto alch_item = GetBoundObject()->As<T>();
            FormID fake_formid;
            if (!defaultsettings->items[i]) {
                fake_formid = CreateFake(alch_item);
                fake_stages.push_back(defaultsettings->numbers[i]);
            } else {
                auto fake_form = Utilities::FunctionsSkyrim::GetFormByID<T>(defaultsettings->items[i], "");
                fake_formid = fake_form ? fake_form->GetFormID() : 0;
            }
            if (!fake_formid) {
                logger::error("Could not create fake form for stage {}", i);
                return;
            }
            // or if this fake_formid is already in the stages return error
            for (auto& [key, value] : stages) {
                if (fake_formid == value.formid) {
                    logger::error("Fake formid is already in the stages.");
                    return;
                }
            }
            if (fake_formid == formid) {
                logger::warn("Fake formid is the same as the real formid.");
                fake_formid = CreateFake(alch_item);
                fake_stages.push_back(defaultsettings->numbers[i]);
            }
            const auto duration = defaultsettings->durations[i];
            const StageName& name = defaultsettings->stage_names[i];

            Stage stage(fake_formid, duration, i, name, defaultsettings->crafting_allowed[i], defaultsettings->effects[i]);
            if (!stages.insert({i, stage}).second) {
                logger::error("Could not insert stage");
                return;
            }

            auto fake_form = Utilities::FunctionsSkyrim::GetFormByID<T>(fake_formid);
            if (!fake_form) {
                logger::error("Fake form is null.");
                return;
            }
            if (Utilities::Functions::Vector::VectorHasElement<StageNo>(fake_stages, i)) {
                // Update name of the fake form
                if (!name.empty()) {
                    fake_form->fullName = std::string(fake_form->fullName.c_str()) + " (" + name + ")";
                    logger::info("Updated name of fake form to {}", name);
                }
                // Update value of the fake form
                const auto temp_value = defaultsettings->costoverrides[i];
                if (temp_value >= 0) Utilities::FunctionsSkyrim::FormTraits<T>::SetValue(fake_form, temp_value);
                // Update weight of the fake form
                const auto temp_weight = defaultsettings->weightoverrides[i];
                if (temp_weight >= 0) Utilities::FunctionsSkyrim::FormTraits<T>::SetWeight(fake_form, temp_weight);
            }

            if (defaultsettings->effects[i].empty()) continue;

            // change mgeff of fake form
            // POPULATE THIS
            if (!Utilities::Functions::Vector::VectorHasElement<std::string>({"FOOD"}, qFormType)) {
                logger::trace("MGEFF not available for this form type {}", qFormType);
                return;
            }

            std::vector<RE::EffectSetting*> MGEFFs;
            std::vector<uint32_t*> pMGEFFdurations;
            std::vector<float*> pMGEFFmagnitudes;

            // i need this many empty effects
            int n_empties =
                static_cast<int>(fake_form->effects.size()) - static_cast<int>(defaultsettings->effects[i].size());
            if (n_empties < 0) n_empties = 0;

            for (int j = 0; j < defaultsettings->effects[i].size(); j++) {
                auto fake_mgeff_id = defaultsettings->effects[i][j].beffect;
                if (!fake_mgeff_id) {
                    MGEFFs.push_back(empty_mgeff);
                    pMGEFFdurations.push_back(nullptr);
                    pMGEFFmagnitudes.push_back(nullptr);
                } else {
                    RE::EffectSetting* fake_mgeffect =
                        Utilities::FunctionsSkyrim::GetFormByID<RE::EffectSetting>(fake_mgeff_id);
                    if (defaultsettings->effects[i][j].duration &&
                        !Utilities::Functions::String::includesString(std::string(fake_mgeffect->magicItemDescription.c_str()),
                                                              {"<dur>"}) &&
                        !fake_mgeffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kNoDuration)) {
                        auto descr_str = std::string(fake_mgeffect->magicItemDescription.c_str());
                        descr_str = descr_str.substr(0, descr_str.length() - 1);
                        RE::EffectSetting* new_form = nullptr;
                        new_form = fake_mgeffect->CreateDuplicateForm(true, (void*)new_form)->As<RE::EffectSetting>();

                        if (!new_form) {
                            logger::error("Failed to create new form.");
                            return;
                        }
                        new_form->Copy(fake_mgeffect);
                        new_form->fullName = fake_mgeffect->GetFullName();
                        new_form->magicItemDescription = (descr_str + " for <dur> second(s).").c_str();
                        fake_mgeffect = new_form;
                    }
                    MGEFFs.push_back(fake_mgeffect);
                    pMGEFFdurations.push_back(&defaultsettings->effects[i][j].duration);
                    pMGEFFmagnitudes.push_back(&defaultsettings->effects[i][j].magnitude);
                }
            }

            for (int j = 0; j < n_empties; j++) {
                MGEFFs.push_back(empty_mgeff);
                pMGEFFdurations.push_back(nullptr);
                pMGEFFmagnitudes.push_back(nullptr);
            }
            Utilities::FunctionsSkyrim::OverrideMGEFFs(fake_form->effects, MGEFFs, pMGEFFdurations, pMGEFFmagnitudes);
        }
    }
    
    [[nodiscard]] const Stage GetFinalStage() const {
        Stage dcyd_st;
        dcyd_st.formid = defaultsettings->decayed_id;
        return dcyd_st;
    }

    const FormID GetModulatorInInventory(RE::TESObjectREFR* inventory_owner) {
        const auto inventory = inventory_owner->GetInventory();
        for (const auto& [dlyr_fid, dlyr_mgntd] : defaultsettings->delayers) {
            if (const auto entry = inventory.find(RE::TESForm::LookupByID<RE::TESBoundObject>(dlyr_fid));
                entry != inventory.end() && entry->second.first > 0) {
                return dlyr_fid;
            }
        }
        return 0;
    }

    void SetDelayOfInstances(const RefID loc, const float curr_time,
                             RE::TESObjectREFR* inventory_owner) {
        if (!inventory_owner) {
			logger::error("Inventory owner is null.");
			return;
		}
        if (!data.count(loc)) {
            logger::error("Location {} does not exist.", loc);
            return;
        }
        const auto delayer_best = GetModulatorInInventory(inventory_owner);
        const float __delay = delayer_best == 0 ? 1 : defaultsettings->delayers[delayer_best];
        for (auto& instance : data[loc]) {
            instance.SetDelay(curr_time, __delay, delayer_best);
        }
	}

    void SetDelayOfInstance(StageInstance& instance, const float curr_time,
                             RE::TESObjectREFR* inventory_owner) {

        const auto delayer_best = GetModulatorInInventory(inventory_owner);
        const float __delay = delayer_best == 0 ? 1 : defaultsettings->delayers[delayer_best];
        instance.SetDelay(curr_time, __delay, delayer_best);
    }
        
    template <typename T>
    const FormID CreateFake(T* real) {
        logger::trace("CreateFakeContainer");
        if (!real) {
			logger::error("Real form is null.");
			return 0;
		}
        T* new_form = nullptr;
        new_form = real->CreateDuplicateForm(true, (void*)new_form)->As<T>();

        if (!new_form) {
            logger::error("Failed to create new form.");
            return 0;
        }
        new_form->Copy(real);

        new_form->fullName = real->GetFullName();
        logger::info("Created form with type: {}, Base ID: {:x}, Ref ID: {:x}, Name: {}",
                     RE::FormTypeToString(new_form->GetFormType()), new_form->GetFormID(), new_form->GetFormID(),
                     new_form->GetName());

        return new_form->GetFormID();
    }
   
    [[nodiscard]] const bool CheckIntegrity() {
        
        if (init_failed) {
			logger::error("CheckIntegrity: Initialisation failed.");
			return false;
		}

        if (!GetBoundObject()) {
			logger::error("Formid {} does not exist.", formid);
			return false;
		}

        if (formid == 0 || stages.empty()) {
			logger::error("One of the members is empty.");
			return false;
		}
        // stages must have keys [0,...,n-1]
        for (auto i = 0; i < stages.size(); i++) {
            if (!stages.count(i)) {
                logger::error("Key {} not found in stages.", i);
                return false;
            }
            // ayni formid olmicak
            /*if (stages[i].formid == formid) {
                logger::error("Formid {} is the same as the source formid.", formid);
				return false;
            }*/
            if (!stages[i].CheckIntegrity()) {
				logger::error("Stage {} integrity check failed.", i);
				return false;
			}
        }

		if (!defaultsettings->CheckIntegrity()) {
            logger::error("Default settings integrity check failed.");
            return false;
        }
        return true;
	}

    [[maybe_unused]] const bool WouldHaveBeenDecayed(StageInstance* st_inst){
        if (!st_inst) {
			logger::error("Stage instance is null.");
			return false;
		}
		if (st_inst->xtra.is_decayed) return true;
        StageNo curr_stageno = st_inst->no;
        if (!stages.contains(curr_stageno)) {
            logger::error("Stage {} does not exist.", curr_stageno);
            return true;
        }
		float diff = st_inst->GetElapsed(RE::Calendar::GetSingleton()->GetHoursPassed());
        const auto last_stage_no = stages.rbegin()->first;
        float total_duration = 0;
        while (curr_stageno <= last_stage_no) {
            total_duration += stages[curr_stageno].duration;
            curr_stageno+=1;
		}
        if (diff> total_duration) return true;
		return false;
    };

    void InitFailed(){
        logger::error("Initialisation failed.");
        Reset();
        init_failed = true;
    }
};