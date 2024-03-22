#pragma once
#include "SimpleIni.h"
#include "Utils.h"

using namespace Utilities::Types;

namespace Settings {

    constexpr auto exclude_path = L"Data/SKSE/Plugins/FoodSpoilageNG_exclude.txt";
    constexpr auto defaults_path = "Data/SKSE/Plugins/FoodSpoilageNG_default.json";


    struct DefaultSettings {
        std::map<StageNo, FormID> items = {};
        std::map<StageNo, Duration> durations = {};
        std::map<StageNo, StageName> stage_names = {};
        std::map<StageNo, std::vector<StageEffect>> effects = {};
        std::vector<StageNo> numbers = {};
        FormID decayed_id = 0;

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
                if (!Utilities::Functions::VectorHasElement<StageNo>(numbers, i)) {
                    logger::error("Key {} not found in numbers.", i);
                    return false;
                }
                if (!items.count(i) || !items.count(i) || !durations.count(i) || !stage_names.count(i) ||
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

    std::vector<std::string> LoadExcludeList() {
        std::ifstream file(exclude_path);
        std::vector<std::string> strings;
        std::string line;
        while (std::getline(file, line)) {
            strings.push_back(line);
        }
        return strings;
    }

    std::vector<std::string> exlude_list = LoadExcludeList();

    [[nodiscard]] const bool IsInExclude(const FormID formid) {
        auto form = Utilities::FunctionsSkyrim::GetFormByID(formid);
        if (!form) {
            logger::warn("Form not found.");
            return false;
        }
        std::string form_string = std::string(form->GetName());
        if (Utilities::Functions::includesWord(form_string, exlude_list)) {
            logger::trace("Form is in exclude list.form_string: {}", form_string);
            return true;
        }
        return false;
    }

    [[nodiscard]] const bool IsItem(const FormID formid) {
        return Utilities::FunctionsSkyrim::IsFoodItem(formid) && !Settings::IsInExclude(formid);
    }

    [[nodiscard]] const bool IsItem(const RE::TESObjectREFR* ref) {
        if (!ref) return false;
        if (ref->IsDisabled()) return false;
        if (ref->IsDeleted()) return false;
        const auto base = ref->GetBaseObject();
        if (!base) return false;
        return IsItem(base->GetFormID());
    }
    
    DefaultSettings parseDefaults(const char* filename = defaults_path) {

        DefaultSettings settings;

        // Open the JSON file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return settings;
        }

        // Read the entire file into a string
        std::string jsonStr;
        file.seekg(0, std::ios::end);
        jsonStr.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
        jsonStr.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Parse the JSON string
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
        if (doc.HasParseError()) {
            std::cerr << "JSON parse error: " << doc.GetParseError() << std::endl;
            return settings;
        }

        // Access the "stages" array
        if (doc.HasMember("stages") && doc["stages"].IsArray()) {
            const rapidjson::Value& stages = doc["stages"];
            for (rapidjson::SizeType i = 0; i < stages.Size(); i++) {
                const rapidjson::Value& stage = stages[i];

                // Parse stage properties
                if (!stage.HasMember("no")) {
				    logger::error("No is missing for stage {}", i);
					return settings;
				}
                StageNo no = stage["no"].GetUint();
                FormID formid;
                auto temp_formid = Utilities::FunctionsJSON::GetFormEditorID(stage, "FormEditorID");
                if (temp_formid < 0) {
					logger::error("FormEditorID is missing for stage {}", no);
					return DefaultSettings();
                } else formid = temp_formid;
                Duration duration;
                if (stage.HasMember("duration")) duration = stage["duration"].GetUint();
                else {
                    logger::error("Duration is missing for stage {}", no);
                    return DefaultSettings();
                }
                StageName name;
                if (stage.HasMember("name")) name = stage["name"].GetString();
                else {
                    logger::error("name is missing for stage {}", no);
                    return DefaultSettings();
                }
                // Parse mgeffect
                std::vector<StageEffect> effects;
                if (stage.HasMember("mgeffect") && stage["mgeffect"].IsArray()) {
                    const rapidjson::Value& mgeffect = stage["mgeffect"];
                    for (rapidjson::SizeType j = 0; j < mgeffect.Size(); j++) {
                        const rapidjson::Value& effect = mgeffect[j];
                        FormID beffect;
                        temp_formid = Utilities::FunctionsJSON::GetFormEditorID(effect, "FormEditorID");
                        if (temp_formid < 0) continue;
                        else beffect = temp_formid;
                        if (!effect.HasMember("magnitude") || !effect.HasMember("duration")) {
							logger::error("Magnitude or duration is missing for effect {}", j);
							return DefaultSettings();
						}
                        float magnitude = effect["magnitude"].GetFloat();
                        Duration effectDuration = effect["duration"].GetUint();
                        effects.push_back(StageEffect(beffect, magnitude, effectDuration));
                    }
                }

                // Populate settings with parsed data
                // if no exist already raise error
                if (Utilities::Functions::VectorHasElement<StageNo>(settings.numbers, no)) {
                    logger::error("No {} already exists.", no);
                    return DefaultSettings();
                }
                settings.numbers.push_back(no);
                settings.items[no] = formid;
                settings.durations[no] = duration;
                settings.stage_names[no] = name;
                settings.effects[no] = effects;
            }
        }

        auto temp_decayed_id = Utilities::FunctionsJSON::GetFormEditorID(doc, "decayedFormEditorID");
        if (temp_decayed_id < 0) {
            logger::error("DecayedFormEditorID is missing.");
            return DefaultSettings();
        } else settings.decayed_id = temp_decayed_id;

        if (!settings.CheckIntegrity()) {
			logger::critical("Settings integrity check failed.");
			return DefaultSettings();
		}
        return settings;
    }


}

struct Source {
    // TODO: reconsider consts here
    FormID formid=0;
    std::string editorid="";
    StageDict stages; // spoilage stages
    SourceData data = {};
    RE::EffectSetting* empty_mgeff;
    Settings::DefaultSettings defaultsettings;

    bool init_failed = false;
    RE::FormType formtype;
    std::vector<StageNo> fake_stages = {};

    Source(const FormID id, const std::string id_str, RE::EffectSetting* e_m,StageDict sd = {})
        : formid(id), editorid(id_str), stages(sd), empty_mgeff(e_m){
        if (!formid) {
            auto form = RE::TESForm::LookupByEditorID<RE::TESForm>(editorid);
            if (form) {
                logger::trace("Found formid for editorid {}", editorid);
                formid = form->GetFormID();
            } else logger::error("Could not find formid for editorid {}", editorid);
        }

        RE::TESForm* form = Utilities::FunctionsSkyrim::GetFormByID(formid);
        auto bound_ = GetBoundObject();
        if (!form || !bound_) {
            InitFailed();
            return;
        }
        
        if (!Settings::IsItem(formid)) {
            InitFailed();
            return;
        }

        defaultsettings = Settings::parseDefaults();
        if (!defaultsettings.CheckIntegrity()) {
			InitFailed();
			return;
		}

        formtype = form->GetFormType();

        //make sure the keys in stages are 0 to length-1 with increment 1
        if (stages.size() == 0) {
            // get stages
            if (formtype == RE::FormType::AlchemyItem) GatherSpoilageStages<RE::AlchemyItem>();
			else if (formtype == RE::FormType::Ingredient) GatherSpoilageStages<RE::IngredientItem>();
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

        if (!CheckIntegrity()) {
            logger::error("CheckIntegrity failed");
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

    const std::vector<StageUpdate> UpdateAllStages(const std::vector<RefID>& filter = {}) {
        logger::trace("Updating all stages.");
        // save the updated instances
        std::vector<StageUpdate> updated_instances;
        if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return updated_instances;
		}
        for (auto& instance : data) {
            // if the refid is not in the filter, skip
            if (!filter.empty() && std::find(filter.begin(), filter.end(), instance.location) == filter.end()) {
                //logger::trace("Refid {} not in filter. Skipping.", instance.location);
                for (auto& reffid : filter) {
                    logger::trace("Refid in filter: {}", reffid);
                }
                continue;
            }
            const StageNo old_no = instance.no;
            if (_UpdateStage(instance)) updated_instances.emplace_back(old_no,instance.no,instance.count);
        }
        CleanUpData();
        return updated_instances;
    }

    [[nodiscard]] const bool IsFakeStage(const StageNo no) {
        return Utilities::Functions::VectorHasElement<StageNo>(fake_stages, no);
    }

  //  [[nodiscard]] const bool IsFakeStage(const FormID formid_) {
  //      for (auto& [key, value] : stages) {
  //          if (value.formid == formid_) return IsFakeStage(key);
		//}
  //      return false;
  //  }

    [[nodiscard]] const StageNo* GetStageNo(const FormID formid_) {
        for (auto& [key, value] : stages) {
            if (value.formid == formid_) return &key;
        }
        return nullptr;
    }
    

    void CleanUpData() {
		logger::trace("Cleaning up data.");
        // size before cleanup
        logger::trace("Size before cleanup: {}", data.size());
        // if there are instances with same stage no and location, and start_time, merge them
        for (auto it = data.begin(); it != data.end(); ++it) {
			for (auto it2 = it; it2 != data.end(); ++it2) {
				if (it == it2) continue;
                if (it2->count <= 0) continue;
				if (it->no == it2->no && it->location == it2->location && it->start_time == it2->start_time) {
					it->count += it2->count;
					it2->count = 0;
				}
			}
		}
		// erase instances with count <= 0
		for (auto it = data.begin(); it != data.end();) {
			if (it->count <= 0) {
				logger::trace("Erasing stage instance with count {}", it->count);
				it = data.erase(it);
            } else if (!stages.count(it->no)) {
				logger::trace("Erasing decayed stage instance with no {}", it->no);
				it = data.erase(it);
            }
            else {
				++it;
			}
		}
        logger::trace("Size after cleanup: {}", data.size());
	}

private:
    // counta karismiyor
    [[nodiscard]] const bool _UpdateStage(StageInstance& st_inst) {
        if (!stages.count(st_inst.no)) return false; //decayed
        const auto current_stage = stages[st_inst.no];
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        float diff = curr_time - st_inst.start_time;
        if (diff < 0) {
            logger::critical("Time difference is negative. This should not happen!!!!");
            return false;
        }
        bool updated = false;
        logger::trace("C«urrent time: {}, Start time: {}, Diff: {}, Duration: {}", curr_time, st_inst.start_time, diff, stages[st_inst.no].duration);
        while (diff > stages[st_inst.no].duration) {
            logger::trace("Updating stage {} to {}", st_inst.no, st_inst.no + 1);
			diff -= stages[st_inst.no].duration;
			st_inst.no++;
            updated = true;
            if (!stages.count(st_inst.no)) {
			    logger::trace("Decayed");
                st_inst.no++;
                // make decayed stage
                Stage decayed_stage;
                decayed_stage.formid = defaultsettings.decayed_id;
                stages[st_inst.no] = decayed_stage;
                break;
		    }
		}
        if (updated) st_inst.start_time = curr_time - diff;
        return updated;
    }

    template <typename T>
    void GatherSpoilageStages() {
        // for now use default stages
        if (!empty_mgeff) {
            logger::error("Empty mgeff is null.");
            return;
        }

        for (auto i = 0; i < defaultsettings.numbers.size(); i++) {
            // create fake form
            auto alch_item = GetBoundObject()->As<T>();
            FormID fake_formid;
            if (!defaultsettings.items[i]) {
                fake_formid = CreateFake(alch_item);
                fake_stages.push_back(defaultsettings.numbers[i]);
            } else {
                auto fake_form = Utilities::FunctionsSkyrim::GetFormByID<T>(defaultsettings.items[i], "");
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
                fake_stages.push_back(defaultsettings.numbers[i]);
            }
            auto duration = defaultsettings.durations[i];
            auto name = defaultsettings.stage_names[i];

            Stage stage(fake_formid, duration, i, name, defaultsettings.effects[i]);
            if (!stages.insert({i, stage}).second) {
                logger::error("Could not insert stage");
                return;
            }

            auto fake_form = Utilities::FunctionsSkyrim::GetFormByID<T>(fake_formid);
            if (!fake_form) {
                logger::error("Fake form is null.");
                return;
            }
            if (Utilities::Functions::VectorHasElement<StageNo>(fake_stages, i)) {
                // Update name of the fake form
                fake_form->fullName = std::string(fake_form->fullName.c_str()) + " (" + name + ")";
                logger::info("Updated name of fake form to {}", name);
                // Update value of the fake form
                if (i == 1)
                    Utilities::FunctionsSkyrim::FormTraits<T>::SetValue(fake_form, 1);
                else if (i > 1)
                    Utilities::FunctionsSkyrim::FormTraits<T>::SetValue(fake_form, 0);
            }

            if (defaultsettings.effects[i].empty()) continue;

            // change mgeff of fake form

            std::vector<RE::EffectSetting*> MGEFFs;
            std::vector<uint32_t*> pMGEFFdurations;
            std::vector<float*> pMGEFFmagnitudes;

            // i need this many empty effects
            int n_empties =
                static_cast<int>(fake_form->effects.size()) - static_cast<int>(defaultsettings.effects[i].size());
            if (n_empties < 0) n_empties = 0;

            for (int j = 0; j < defaultsettings.effects[i].size(); j++) {
                auto fake_mgeff_id = defaultsettings.effects[i][j].beffect;
                if (!fake_mgeff_id) {
                    MGEFFs.push_back(empty_mgeff);
                    pMGEFFdurations.push_back(nullptr);
                    pMGEFFmagnitudes.push_back(nullptr);
                } else {
                    RE::EffectSetting* fake_mgeffect =
                        Utilities::FunctionsSkyrim::GetFormByID<RE::EffectSetting>(fake_mgeff_id);
                    if (defaultsettings.effects[i][j].duration &&
                        !Utilities::Functions::includesString(std::string(fake_mgeffect->magicItemDescription.c_str()),
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
                    pMGEFFdurations.push_back(&defaultsettings.effects[i][j].duration);
                    pMGEFFmagnitudes.push_back(&defaultsettings.effects[i][j].magnitude);
                }
            }

            for (int j = 0; j < n_empties; j++) {
                MGEFFs.push_back(empty_mgeff);
                pMGEFFdurations.push_back(nullptr);
                pMGEFFmagnitudes.push_back(nullptr);
            }
            Utilities::FunctionsSkyrim::OverrideMGEFFs(fake_form->effects, MGEFFs, pMGEFFdurations, pMGEFFmagnitudes);

            // int mg_count = 0;
            // for (auto& mgeffect : fake_form->effects) {
            //     logger::trace("Updating mgeffect {}", mg_count);
            //     if (!mg_count) {
            //         mgeffect->baseEffect = fake_mgeffect;
            //         mgeffect->effectItem.magnitude = 10;
            //         mgeffect->effectItem.duration = 20;
            //     }
            //     else mgeffect->baseEffect = empty_mgeff;
            //     mg_count++;
            // }
            // fake_form->effects.push_back(fake_form->effects.front()); // works
        }
        // update mgeffs
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
            if (stages[i].formid == formid) {
                logger::error("Formid {} is the same as the source formid.", formid);
				return false;
            }
            if (!stages[i].CheckIntegrity()) {
				logger::error("Stage {} integrity check failed.", i);
				return false;
			}
        }

		if (!defaultsettings.CheckIntegrity()) {
            logger::error("Default settings integrity check failed.");
            return false;
        }
        return true;
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