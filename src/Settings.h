#pragma once
#include "SimpleIni.h"
#include "Utils.h"

using namespace Utilities::Types;

namespace Settings {

    namespace oldstuff {
    
      //  DefaultSettings _parseDefaults(const rapidjson::Value& owner) {
      //      DefaultSettings settings;

      //      // Access the "stages" array
      //      if (owner.HasMember("stages") && owner["stages"].IsArray()) {
      //          const rapidjson::Value& stages = owner["stages"];
      //          logger::trace("Stages size: {}", stages.Size());
      //          for (rapidjson::SizeType i = 0; i < stages.Size(); i++) {
      //              const rapidjson::Value& stage = stages[i];

      //              // Parse stage properties
      //              if (!stage.HasMember("no")) {
      //                  logger::error("No is missing for stage {}", i);
      //                  return settings;
      //              }
      //              // Parse no
      //              StageNo no = stage["no"].GetUint();
      //              // Parse formid
      //              FormID formid;
      //              auto temp_formid = Utilities::FunctionsJSON::GetFormEditorID(stage, "FormEditorID");
      //              if (temp_formid < 0) {
      //                  logger::error("FormEditorID is missing for stage {}", no);
      //                  return DefaultSettings();
      //              } else
      //                  formid = temp_formid;
      //              // Parse duration
      //              Duration duration;
      //              if (stage.HasMember("duration"))
      //                  duration = stage["duration"].GetUint();
      //              else {
      //                  logger::error("Duration is missing for stage {}", no);
      //                  return DefaultSettings();
      //              }
      //              // Parse name
      //              StageName name = "";
      //              if (stage.HasMember("name"))
      //                  name = stage["name"].GetString();
      //              else
      //                  logger::warn("name is missing for stage {}", no);

      //              // parse crafting eligibility
      //              bool crafting_allowed = false;
      //              if (stage.HasMember("crafting_allowed"))
      //                  crafting_allowed = stage["crafting_allowed"].GetBool();
      //              else
      //                  logger::warn("Crafting allowed is missing for stage {}", no);

      //              // Parse mgeffect
      //              std::vector<StageEffect> effects;
      //              if (stage.HasMember("mgeffect") && stage["mgeffect"].IsArray()) {
      //                  const rapidjson::Value& mgeffect = stage["mgeffect"];
      //                  for (rapidjson::SizeType j = 0; j < mgeffect.Size(); j++) {
      //                      const rapidjson::Value& effect = mgeffect[j];
      //                      FormID beffect;
      //                      temp_formid = Utilities::FunctionsJSON::GetFormEditorID(effect, "FormEditorID");
      //                      if (temp_formid < 0)
      //                          continue;
      //                      else
      //                          beffect = temp_formid;
      //                      if (!effect.HasMember("magnitude") || !effect.HasMember("duration")) {
      //                          logger::error("Magnitude or duration is missing for effect {}", j);
      //                          return DefaultSettings();
      //                      }
      //                      float magnitude = effect["magnitude"].GetFloat();
      //                      Duration effectDuration = effect["duration"].GetUint();
      //                      effects.push_back(StageEffect(beffect, magnitude, effectDuration));
      //                  }
      //              }

      //              // Populate settings with parsed data
      //              // if no exist already raise error
      //              if (Utilities::Functions::Vector::VectorHasElement<StageNo>(settings.numbers, no)) {
      //                  logger::error("No {} already exists.", no);
      //                  return DefaultSettings();
      //              }

      //              settings.numbers.push_back(no);
      //              settings.items[no] = formid;
      //              settings.durations[no] = duration;
      //              settings.stage_names[no] = name;
      //              settings.crafting_allowed[no] = crafting_allowed;
      //              settings.effects[no] = effects;
      //          }
      //      }

      //      auto temp_decayed_id = Utilities::FunctionsJSON::GetFormEditorID(owner, "decayedFormEditorID");
      //      if (temp_decayed_id < 0) {
      //          logger::error("DecayedFormEditorID is missing.");
      //          return DefaultSettings();
      //      } else
      //          settings.decayed_id = temp_decayed_id;

      //      if (!settings.CheckIntegrity()) {
      //          logger::critical("Settings integrity check failed.");
      //          return DefaultSettings();
      //      }
      //      return settings;

      //      // Populate settings with parsed data specific to this owner
      //  }

      //  DefaultSettings parseDefaults(const std::string& _type_) {

      //      DefaultSettings settings;
      //      const auto filename = std::string(defaults_path_) + _type_ + ".json";
      //      logger::trace("Filename: {}", filename);
      //      // Open the JSON file
      //      std::ifstream file(filename);
      //      if (!file.is_open()) {
      //          logger::error("Failed to open file: {}",filename);
      //          return settings;
      //      }

      //      // Read the entire file into a string
      //      std::string jsonStr;
      //      file.seekg(0, std::ios::end);
      //      jsonStr.reserve(file.tellg());
      //      file.seekg(0, std::ios::beg);
      //      jsonStr.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      //      file.close();

      //      // Parse the JSON string
      //      rapidjson::Document doc;
      //      doc.Parse(jsonStr.c_str());
      //      if (doc.HasParseError()) {
      //          logger::critical("JSON parse error: {}", doc.GetParseError());
      //          Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("JSON parse error: " + std::to_string(doc.GetParseError()));
      //          return settings;
      //      }

      //      // Access the "stages" array
      //      if (doc.HasMember("stages") && doc["stages"].IsArray()) {
      //          const rapidjson::Value& stages = doc["stages"];
      //          logger::trace("Stages size: {}", stages.Size());
      //          for (rapidjson::SizeType i = 0; i < stages.Size(); i++) {
      //              const rapidjson::Value& stage = stages[i];

      //              // Parse stage properties
      //              if (!stage.HasMember("no")) {
				  //      logger::error("No is missing for stage {}", i);
					 //   return settings;
				  //  }
      //              // Parse no
      //              StageNo no = stage["no"].GetUint();
      //              // Parse formid
      //              FormID formid;
      //              auto temp_formid = Utilities::FunctionsJSON::GetFormEditorID(stage, "FormEditorID");
      //              if (temp_formid < 0) {
					 //   logger::error("FormEditorID is missing for stage {}", no);
					 //   return DefaultSettings();
      //              } else formid = temp_formid;
      //              // Parse duration
      //              Duration duration;
      //              if (stage.HasMember("duration")) duration = stage["duration"].GetUint();
      //              else {
      //                  logger::error("Duration is missing for stage {}", no);
      //                  return DefaultSettings();
      //              }
      //              // Parse name
      //              StageName name = "";
      //              if (stage.HasMember("name")) name = stage["name"].GetString();
      //              else logger::warn("name is missing for stage {}", no);

      //              // parse crafting eligibility
      //              bool crafting_allowed = false;
      //              if (stage.HasMember("crafting_allowed")) crafting_allowed = stage["crafting_allowed"].GetBool();
      //              else logger::warn("Crafting allowed is missing for stage {}", no);
      //          
      //              // Parse mgeffect
      //              std::vector<StageEffect> effects;
      //              if (stage.HasMember("mgeffect") && stage["mgeffect"].IsArray()) {
      //                  const rapidjson::Value& mgeffect = stage["mgeffect"];
      //                  for (rapidjson::SizeType j = 0; j < mgeffect.Size(); j++) {
      //                      const rapidjson::Value& effect = mgeffect[j];
      //                      FormID beffect;
      //                      temp_formid = Utilities::FunctionsJSON::GetFormEditorID(effect, "FormEditorID");
      //                      if (temp_formid < 0) continue;
      //                      else beffect = temp_formid;
      //                      if (!effect.HasMember("magnitude") || !effect.HasMember("duration")) {
						//	    logger::error("Magnitude or duration is missing for effect {}", j);
						//	    return DefaultSettings();
						//    }
      //                      float magnitude = effect["magnitude"].GetFloat();
      //                      Duration effectDuration = effect["duration"].GetUint();
      //                      effects.push_back(StageEffect(beffect, magnitude, effectDuration));
      //                  }
      //              }

      //              // Populate settings with parsed data
      //              // if no exist already raise error
      //              if (Utilities::Functions::Vector::VectorHasElement<StageNo>(settings.numbers, no)) {
      //                  logger::error("No {} already exists.", no);
      //                  return DefaultSettings();
      //              }
      //          
      //              settings.numbers.push_back(no);
      //              settings.items[no] = formid;
      //              settings.durations[no] = duration;
      //              settings.stage_names[no] = name;
      //              settings.crafting_allowed[no] = crafting_allowed;
      //              settings.effects[no] = effects;
      //          }
      //      }

      //      auto temp_decayed_id = Utilities::FunctionsJSON::GetFormEditorID(doc, "decayedFormEditorID");
      //      if (temp_decayed_id < 0) {
      //          logger::error("DecayedFormEditorID is missing.");
      //          return DefaultSettings();
      //      } else settings.decayed_id = temp_decayed_id;

      //      if (!settings.CheckIntegrity()) {
			   // logger::critical("Settings integrity check failed.");
			   // return DefaultSettings();
		    //}
      //      return settings;
      //  }

      //  std::map<std::string, DefaultSettings, CompareByLength> parseCustoms(const std::string& _type_) {

      //      std::map<std::string, DefaultSettings, CompareByLength> _custom_settings;

      //      const auto filename = std::string(customs_path_) + _type_ + ".json";
      //      std::ifstream file(filename);
      //      if (!file.is_open()) {
      //          logger::error("Failed to open file: {}", filename);
      //          return _custom_settings;
      //      }

      //      // Read the entire file into a string
      //      std::string jsonStr;
      //      file.seekg(0, std::ios::end);
      //      jsonStr.reserve(file.tellg());
      //      file.seekg(0, std::ios::beg);
      //      jsonStr.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      //      file.close();

      //      // Parse the JSON string
      //      rapidjson::Document doc;
      //      doc.Parse(jsonStr.c_str());
      //      if (doc.HasParseError()) {
      //          logger::error("JSON parse error: {}", doc.GetParseError());
      //          _custom_settings.clear();
      //          return _custom_settings;
      //      }

      //      // Iterate over each owner
      //      for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
      //          const std::string& ownerName = it->name.GetString();
      //          const rapidjson::Value& owner = it->value;

      //          logger::trace("Owner: {}", ownerName);

      //          // Call your existing parsing function for stages of each owner
      //          DefaultSettings ownerSettings = _parseDefaults(owner);

      //          _custom_settings[ownerName] = ownerSettings;
      //      }
      //      return _custom_settings;
      //  }

      //  struct CompareByLength {
      //      bool operator()(const std::string& a, const std::string& b) const {
      //          if (a.length() == b.length()) {
      //              return a < b;  // If lengths are equal, use lexicographical comparison
      //          }
      //          return a.length() < b.length();  // Otherwise, compare strings by their lengths
      //      }
      //  };
    }

    constexpr std::uint32_t kSerializationVersion = 626;
    constexpr std::uint32_t kDataKey = 'QAOT';

    constexpr auto exclude_path_ = "Data/SKSE/Plugins/AoT_exclude"; //txt
    constexpr auto defaults_path_ = "Data/SKSE/Plugins/AoT_default";  // yml
    constexpr auto customs_path_ = "Data/SKSE/Plugins/AoT_custom";  // yml


    struct DefaultSettings {
        std::map<StageNo, FormID> items = {};
        std::map<StageNo, Duration> durations = {};
        std::map<StageNo, StageName> stage_names = {};
        std::map<StageNo,bool> crafting_allowed = {};
        std::map<StageNo,unsigned int> costoverrides = {};
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
        DefaultSettings settings;
         //we have:stages, decayedFormEditorID and delayers
        if (!config["stages"] || config["stages"].size() == 0) {
            logger::error("Stages are empty.");
            return settings;
        }
        for (const auto& stageNode : config["stages"]) {
            const auto temp_no = stageNode["no"].as<StageNo>();
            // add to numbers
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
            settings.items[temp_no] = temp_formid;
            // add to durations
            settings.durations[temp_no] = stageNode["duration"].as<Duration>();
            // add to stage_names
            settings.stage_names[temp_no] = stageNode["name"].as<StageName>();
            // add to crafting_allowed
            settings.crafting_allowed[temp_no] = stageNode["crafting_allowed"].as<bool>();
            // add to effects
            std::vector<StageEffect> effects;
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
                const auto temp_magnitude = effectNode["magnitude"].as<float>();
                const auto temp_duration = effectNode["duration"].as<Duration>();
                effects.push_back(StageEffect(temp_effect_formid, temp_magnitude, temp_duration));
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
        YAML::Node config = YAML::LoadFile(filename);
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
        try {
            for (const auto& _qftype: Settings::QFORMS) {
			    defaultsettings[_qftype] = parseDefaults(_qftype);
			    custom_settings[_qftype] = parseCustoms(_qftype);
                for (const auto& [key,_] : custom_settings[_qftype]) {
                    logger::trace("Key: {}", key.front());
                }
                exclude_list[_qftype] = LoadExcludeList(_qftype);
		    }
        } catch (const std::exception&) {
            logger::critical("Failed to load settings.");
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
    // TODO: reconsider consts here
    FormID formid=0;
    std::string editorid="";
    StageDict stages;
    SourceData data = {};
    RE::EffectSetting* empty_mgeff;
    Settings::DefaultSettings* defaultsettings = nullptr; // eigentlich sollte settings heissen

    bool init_failed = false;
    RE::FormType formtype;
    std::string qFormType;
    std::vector<StageNo> fake_stages = {};
    Stage decayed_stage;

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
        if (init_failed) {
            logger::critical("UpdateAllStages: Initialisation failed.");
            return {};
        }
        // save the updated instances
        std::vector<StageUpdate> updated_instances = {};
        if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return updated_instances;
		}
        for (auto& reffid : filter) {
            logger::trace("Refid in filter: {}", reffid);
        }
        for (auto& instance : data) {
            // if the refid is not in the filter, skip
            if (!Utilities::Functions::Vector::VectorHasElement<RefID>(filter, instance.location)) {
				//logger::trace("Refid {} not in filter. Skipping.", instance.location);
				continue;
			}
            Stage* old_stage = &stages[instance.no];
            Stage* new_stage = nullptr;
            if (_UpdateStageInstance(instance)) {
                if (instance.xtra.is_decayed || !stages.contains(instance.no)) {
                    new_stage = &decayed_stage;
				}
                else new_stage = &stages[instance.no];
                auto is_fake__ = IsFakeStage(instance.no);
                updated_instances.emplace_back(old_stage, new_stage, instance.count, instance.location, is_fake__);
            }
        }
        CleanUpData();
        return updated_instances;
    }

    [[nodiscard]] const bool IsFakeStage(const StageNo no) {
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

    [[nodiscard]] const bool InsertNewInstance(StageInstance& stage_instance) { 

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
        if (stage_instance.location == 0) {
			logger::error("Location is 0.");
			return false;
		}
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

        data.push_back(stage_instance);

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
        StageInstance new_instance(curr_time, n, c, l);
        new_instance.xtra.form_id = stages[n].formid;
        new_instance.xtra.editor_id = clib_util::editorID::get_editorID(stages[n].GetBound());
        new_instance.xtra.crafting_allowed = stages[n].crafting_allowed;
        if (IsFakeStage(n)) new_instance.xtra.is_fake = true;

        return InsertNewInstance(new_instance);
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
        UpdateTimeModulationInInventory(inventory_owner);
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

        std::vector<StageInstance*> instances_candidates = {};
        for (auto& st_inst : data) {
            if (st_inst.xtra.form_id == instance_formid && st_inst.location == from_ref)
                instances_candidates.push_back(&st_inst);
        }

        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        if (bias_direction) {
            std::sort(instances_candidates.begin(), instances_candidates.end(),
                      [curr_time](StageInstance* a, StageInstance* b) {
                          return a->GetElapsed(curr_time) > b->GetElapsed(curr_time);  // move the older stuff
                      });
        } 
        else {
			std::sort(instances_candidates.begin(), instances_candidates.end(),
					  [curr_time](StageInstance* a, StageInstance* b) {
						  return a->GetElapsed(curr_time) < b->GetElapsed(curr_time);  // move the newer stuff
					  });
		}

        if (instances_candidates.empty()) {
            logger::warn("No instances found for formid {} and location {}", instance_formid, from_ref);
            return 0;
        }

        for (auto& instance : instances_candidates) {
            if (!count) break;
            if (count <= instance->count) {
                instance->count -= count;
                StageInstance new_instance(*instance);
                new_instance.count = count;
                new_instance.location = to_ref;
                if (!InsertNewInstance(new_instance)) {
                    logger::error("InsertNewInstance failed.");
                    return 0;
                }
                count = 0;
                //break;
            } else {
                count -= instance->count;
                instance->location = to_ref;
            }
        }

        return count;

    }

    // always update before doing this
    void UpdateTimeModulationInInventory(RE::TESObjectREFR* inventory_owner){
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
        
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();

        // get the instances inside this inventory
        std::vector<StageInstance*> instances = {};
        for (auto& instance : data) {
			if (instance.location == inventory_owner_refid) instances.push_back(&instance);
		}
        if (instances.empty()) {
            logger::trace("No instances found for inventory owner {} and source {}", inventory_owner_refid, editorid);
            return;
        }

        const auto delayer_best = GetModulatorInInventory(inventory_owner);
        if (!delayer_best) {
			logger::trace("No delayer found in inventory.");
			// need to remove if there is a delayer
            for (auto& instance : instances) {
				if (instance->xtra.is_decayed) continue;
                if (instance->GetDelayMagnitude()==1) continue;
                instance->SetDelay(curr_time, 1, 0);
			}
		} 
		else {
            const auto new_delay_mag = defaultsettings->delayers[delayer_best];
            for (auto& instance : instances) {
                if (instance->xtra.is_decayed) continue;
                if (instance->GetDelayMagnitude() == new_delay_mag) continue;
                instance->SetDelay(curr_time, new_delay_mag, delayer_best);
            }
        }
    }

    void RemoveTimeModulationFromWO(const RefID world_object) {
        if (!world_object) {
			logger::error("World object is null.");
			return;
		}
		if (data.empty()) {
			logger::warn("No data found for source {}", editorid);
			return;
		}
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
		for (auto& instance : data) {
			if (instance.location == world_object) {
                if (instance.GetDelayMagnitude() == 1) break;
                instance.SetDelay(RE::Calendar::GetSingleton()->GetHoursPassed(), 1,0);
                break;
            }
		}
    }

    RE::ExtraTextDisplayData* GetTextData(const StageNo _no) {
        return stages[_no].GetExtraText();
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
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        for (auto it = data.begin(); it != data.end(); ++it) {
			for (auto it2 = it; it2 != data.end(); ++it2) {
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
		for (auto it = data.begin(); it != data.end();) {
			if (it->count <= 0) {
				logger::trace("Erasing stage instance with count {}", it->count);
				it = data.erase(it);
            } 
            else if (!stages.count(it->no) || it->xtra.is_decayed) {
				logger::trace("Erasing decayed stage instance with no {}", it->no);
				it = data.erase(it);
            }
            else {
				++it;
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
		for (auto& instance : data) {
            logger::trace("No: {}, Location: {}, Count: {}, Start time: {}", instance.no, instance.location,
                          instance.count, instance.start_time);
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
    [[nodiscard]] const bool _UpdateStageInstance(StageInstance& st_inst) {
        if (init_failed) {
        	logger::critical("_UpdateStage: Initialisation failed.");
            return false;
        }
        if (st_inst.xtra.is_decayed) return false;  // decayed
        
        const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
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
                fake_form->fullName = std::string(fake_form->fullName.c_str()) + " (" + name + ")";
                logger::info("Updated name of fake form to {}", name);
                // Update value of the fake form
                if (i == 1)
                    Utilities::FunctionsSkyrim::FormTraits<T>::SetValue(fake_form, 1);
                else if (i > 1)
                    Utilities::FunctionsSkyrim::FormTraits<T>::SetValue(fake_form, 0);
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
            //if (!stages.count(i)) {
            //    logger::error("Key {} not found in stages.", i);
            //    return false;
            //}
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

		if (!defaultsettings->CheckIntegrity()) {
            logger::error("Default settings integrity check failed.");
            return false;
        }
        return true;
	}


    void InitFailed(){
        logger::error("Initialisation failed.");
        Reset();
        init_failed = true;
    }
};