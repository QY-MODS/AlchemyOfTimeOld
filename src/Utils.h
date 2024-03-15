#pragma once

//#include <chrono>
#include "logger.h"
#include <windows.h>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include "SimpleIni.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <mutex>  // for std::mutex
#include <algorithm>
#include <ClibUtil/editorID.hpp>


namespace Utilities{

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";

    const auto no_src_msgbox = std::format(
        "{}: You currently do not have any container set up. Check your ini file or see the mod page for instructions.",
        mod_name);

    const auto general_err_msgbox = std::format("{}: Something went wrong. Please contact the mod author.", mod_name);
    
    const auto init_err_msgbox = std::format("{}: The mod failed to initialize and will be terminated.", mod_name);
        std::string dec2hex(int dec) {
        std::stringstream stream;
        stream << std::hex << dec;
        std::string hexString = stream.str();
        return hexString;
    };

    namespace Types {

        //using EditorID = std::string;
        using NameID = std::string;
        using FormID = RE::FormID;
        using RefID = RE::FormID;
        using Count = RE::TESObjectREFR::Count;
        using Duration = float;
        using StageNo = unsigned int;
        using StageName = std::string;


        struct Stage{
            FormID formid=0; // with which item is it represented
            Duration duration; // duration of the stage
            StageNo no; // used for sorting when multiple stages are present
            StageName name; // name of the stage
            
            Stage(){};
            Stage(FormID f, Duration d, StageNo s, StageName n)
                : formid(f), duration(d) , no(s), name(n) {
                    logger::trace("Stage: FormID {}, Duration {}, StageNo {}, Name {}", formid, duration, no, name);
                }

            bool operator<(const Stage& other) const {
                if (formid < other.formid) return true;
                if (other.formid < formid) return false;
                return no < other.no;
            }

            bool operator==(const Stage& other) const {
                return no == other.no && formid == other.formid && duration == other.duration;
            }
        };

        using Stages = std::vector<Stage>;
        using StageDict = std::map<StageNo,Stage>;
        struct SomethingWithStages {
            StageDict stages; // consider just using formid
            float start_time;
            StageNo current_stage;
            Count count;
            
            SomethingWithStages(){};
            SomethingWithStages(Stages s, float time, Count c, StageNo stage=0)
                : start_time(time), count(c) , current_stage(stage){
                    for (auto& stage : s) {
                        stages[stage.no] = stage;
                    }
                    logger::trace("Something with stages: Start time {}, Count {}, Current stage {}", start_time, count, current_stage);
                }

            // Define comparison operator for sorting in maps
            bool operator<(const SomethingWithStages& other) const {
                // Compare each member in the desired order
                if (current_stage < other.current_stage) return true;
                if (other.current_stage < current_stage) return false;
                if (start_time < other.start_time) return true;
                if (start_time > other.start_time) return false;
                return count < other.count;
            }

            bool operator==(const SomethingWithStages& other) {
                // Compare each member for equality
                if (stages.size() != other.stages.size()) return false;
                for (const auto& [other_no,other_stage] : other.stages) {
                    if (stages.find(other_no) == stages.end()) return false;
                    if (other_stage != stages[other_no]) return false;
                }
                return  start_time == other.start_time &&
                        current_stage == other.current_stage &&
                        count == other.count;
            }

            [[nodiscard]] const bool IsDue() {
                const auto curr_stage = GetCurrentStage();
                const auto duration_allowed = curr_stage.duration;
                if (!duration_allowed) return false;
                const auto time_passed = RE::Calendar::GetSingleton()->GetHoursPassed() - start_time;
                return time_passed > duration_allowed;
            }

            [[nodiscard]] const bool Update() {
                if (IsAtLastStage()){
                    logger::warn("Update: At last stage, this should not happen!");
                    return false;
                } 
                if (!IsDue()) return false;
                const auto curr_stage = GetCurrentStage();
                // need to see how many stages do i need to jump
                const auto curr_time = RE::Calendar::GetSingleton()->GetHoursPassed();
                const auto time_passed = curr_time - start_time;
                Duration cumulative_stage_duration = curr_stage.duration;
                while (time_passed > cumulative_stage_duration){
                    current_stage++;
                    cumulative_stage_duration += stages[current_stage].duration;
                }
                start_time = curr_time;
                return true;
                
            }

            [[nodiscard]] const bool isInitialized() const {
                return stages.size() > 0;
            }

            Stage GetCurrentStage() {
                return stages[current_stage];
            }

            [[nodiscard]] const bool IsAtLastStage() {
                // get stage with highest stage_no
                const auto curr_stage = GetCurrentStage();
                if (curr_stage.duration == 0) return true;
                unsigned int maxKey = 0;
                bool flag = false;
                // Iterate through the map to find the highest key
                for (const auto& pair : stages) {
                    if (pair.first > maxKey) {
                        maxKey = pair.first;
                        flag = true;
                    }
                }
                if (!flag){
                    logger::critical("IsAtLastStage: No stages found!");
                    return false;
                }
                if (current_stage == maxKey){
                    logger::warn("IsAtLastStage: At last stage but duration is not ZERO!");
                    return true;
                }
                return false;
            }
        };


        using SourceDataKey = SomethingWithStages; //
        using SourceDataVal = RefID; // RefID of the container where the fake food is stored or the real food itself when it is out in the world
        
        struct SourceDataKeyVal {
            SourceDataKey instance;
            SourceDataVal refid;

            bool operator==(const SourceDataKeyVal& other) {
                // Compare each member for equality
                // return instance.start_time==other.instance.start_time &&
                //         instance.count==other.instance.count &&
                //         instance.stages==other.instance.stages &&
                //         refid==other.refid;
                return instance==other.instance &&
                        refid==other.refid;
            }
        };
        using SourceData = std::vector<SourceDataKeyVal>; // 
        using SaveDataLHS = SourceDataKey;
        using SaveDataRHS = SourceDataVal;
    }

    namespace FunctionsSkyrim {

        RE::TESForm* GetFormByID(const RE::FormID& id, const std::string& editor_id="") {
            auto form = RE::TESForm::LookupByID(id);
            if (form)
                return form;
            else if (!editor_id.empty()) {
                form = RE::TESForm::LookupByEditorID(editor_id);
                if (form) return form;
            }
            return nullptr;
        };

        template <class T = RE::TESForm>
        static T* GetFormByID(const RE::FormID& id, const std::string& editor_id="") {
            T* form = RE::TESForm::LookupByID<T>(id);
            if (form)
                return form;
            else if (!editor_id.empty()) {
                form = RE::TESForm::LookupByEditorID<T>(editor_id);
                if (form) return form;
            }
            return nullptr;
        };

        std::size_t GetExtraDataListLength(const RE::ExtraDataList* dataList) {
            std::size_t length = 0;

            for (auto it = dataList->begin(); it != dataList->end(); ++it) {
                // Increment the length for each element in the list
                ++length;
            }

            return length;
        }


        bool FormIsOfType(RE::TESForm* form, RE::FormType type) {
		    return form->GetFormType() == type;
	    }

        bool IsFoodItem(RE::TESForm* form) {
            if (FormIsOfType(form,RE::AlchemyItem::FORMTYPE)){
            RE::AlchemyItem* form_as_ = form->As<RE::AlchemyItem>();
            if (!form_as_) return false;
            if (!form_as_->IsFood()) return false;
            }
            else if (FormIsOfType(form,RE::IngredientItem::FORMTYPE)){
                RE::IngredientItem* form_as_ = form->As<RE::IngredientItem>();
                if (!form_as_) return false;
                if (!form_as_->IsFood()) return false;

            }
            else if(FormIsOfType(form,RE::MagicItem::FORMTYPE)){
                RE::MagicItem* form_as_ = form->As<RE::MagicItem>();
                if (!form_as_) return false;
                if (!form_as_->IsFood()) return false;
            }
            else return false;
            return true;
        }

        bool IsCONT(Types::RefID refid) {
            return RE::FormTypeToString(
                   RE::TESForm::LookupByID<RE::TESObjectREFR>(refid)->GetObjectReference()->GetFormType()) == "CONT";
        }



        template <typename T>
        struct FormTraits {
            static float GetWeight(T* form) {
                // Default implementation, assuming T has a member variable 'weight'
                return form->weight;
            }

            static void SetWeight(T* form, float weight) {
                // Default implementation, set the weight if T has a member variable 'weight'
                form->weight = weight;
            }
    
            static int GetValue(T* form) {
			    // Default implementation, assuming T has a member variable 'value'
			    return form->value;
		    }

            static void SetValue(T* form, int value) {
                form->value = value;
            }
        };

        template <>
        struct FormTraits<RE::AlchemyItem> {
            static float GetWeight(RE::AlchemyItem* form) { 
                return form->weight;
            }

            static void SetWeight(RE::AlchemyItem* form, float weight) { 
                form->weight = weight;
            }

            static int GetValue(RE::AlchemyItem* form) {
        	    return form->GetGoldValue();
            }
            static void SetValue(RE::AlchemyItem* form, int value) { 
                logger::trace("CostOverride: {}", form->data.costOverride);
                form->data.costOverride = value;
            }
        };

	    // https:// github.com/Exit-9B/Dont-Eat-Spell-Tomes/blob/7b7f97353cc6e7ccfad813661f39710b46d82972/src/SpellTomeManager.cpp#L23-L32
        template <typename T>
        RE::TESObjectREFR* GetMenuOwner() {
            RE::TESObjectREFR* reference = nullptr;
            const auto ui = RE::UI::GetSingleton();
            const auto menu = ui ? ui->GetMenu<T>() : nullptr;
            const auto movie = menu ? menu->uiMovie : nullptr;

            if (movie) {
                // Parapets: "Menu_mc" is stored in the class, but it's different in VR and we haven't RE'd it yet
                RE::GFxValue isViewingContainer;
                movie->Invoke("Menu_mc.isViewingContainer", &isViewingContainer, nullptr, 0);

                if (isViewingContainer.GetBool()) {
                    auto refHandle = menu->GetTargetRefHandle();
                    RE::TESObjectREFRPtr refr;
                    RE::LookupReferenceByHandle(refHandle, refr);
                    return refr.get();
                }
            }
            return reference;
	    }

        // credits to Qudix on xSE RE Discord for this
        void OpenContainer(RE::TESObjectREFR* a_this, std::uint32_t a_openType) {
            // a_openType is probably in alignment with RE::ContainerMenu::ContainerMode enum
            using func_t = decltype(&OpenContainer);
            REL::Relocation<func_t> func{RELOCATION_ID(50211, 51140)};
            func(a_this, a_openType);
        }

        const bool HasItemEntry(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner, bool nonzero_entry_check=false) {
            if (!item) {
                logger::warn("Item is null");
                return 0;
            }
            if (!inventory_owner) {
                logger::warn("Inventory owner is null");
                return 0;
            }
            auto inventory = inventory_owner->GetInventory();
            auto it = inventory.find(item);
            bool has_entry = it != inventory.end();
            if (nonzero_entry_check) return has_entry && it->second.first > 0;
            return has_entry;
		}

        const std::int32_t GetItemCount(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
            if (HasItemEntry(item, inventory_owner, true)) {
				auto inventory = inventory_owner->GetInventory();
				auto it = inventory.find(item);
				return it->second.first;
			}
            return 0;
        }

        const bool HasItem(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner) {
            if (HasItemEntry(item, item_owner, true)) return true;
            return false;
        }

        void FavoriteItem(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
            if (!item) return;
            if (!inventory_owner) return;
            auto inventory_changes = inventory_owner->GetInventoryChanges();
            auto entries = inventory_changes->entryList;
            for (auto it = entries->begin(); it != entries->end(); ++it) {
                auto formid = (*it)->object->GetFormID();
                if (!formid) logger::critical("Formid is null");
                if (formid == item->GetFormID()) {
                    logger::trace("Favoriting item: {}", item->GetName());
                    bool no_extra_ = (*it)->extraLists->empty();
                    logger::trace("asdasd");
                    if (no_extra_) {
                        logger::trace("No extraLists");
                        inventory_changes->SetFavorite((*it), nullptr);
                    } else {
                        logger::trace("ExtraLists found");
                        inventory_changes->SetFavorite((*it), (*it)->extraLists->front());
                    }
                    return;
                }
            }
            logger::error("Item not found in inventory");
        }

        
    [[nodiscard]] const bool IsFavorited(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
            if (!item) {
                logger::warn("Item is null");
                return false;
            }
            if (!inventory_owner) {
                logger::warn("Inventory owner is null");
                return false;
            }
            auto inventory = inventory_owner->GetInventory();
            auto it = inventory.find(item);
            if (it != inventory.end()) {
                if (it->second.first <= 0) logger::warn("Item count is 0");
                return it->second.second->IsFavorited();
            }
            return false;
        }
    };

        // Utility functions
    namespace Functions {

        template <typename Key, typename Value>
        bool containsValue(const std::map<Key, Value>& myMap, const Value& valueToFind) {
            for (const auto& pair : myMap) {
                if (pair.second == valueToFind) {
                    return true;
                }
            }
            return false;
        }

        std::string GetPluginVersion(const unsigned int n_stellen) {
            const auto fullVersion = SKSE::PluginDeclaration::GetSingleton()->GetVersion();
            unsigned int i = 1;
            std::string version = std::to_string(fullVersion.major());
            if (n_stellen == i) return version;
            version += "." + std::to_string(fullVersion.minor());
            if (n_stellen == ++i) return version;
            version += "." + std::to_string(fullVersion.patch());
            if (n_stellen == ++i) return version;
            version += "." + std::to_string(fullVersion.build());
            return version;

            
        }

    };


    namespace MsgBoxesNotifs {

        // https://github.com/SkyrimScripting/MessageBox/blob/ac0ea32af02766582209e784689eb0dd7d731d57/include/SkyrimScripting/MessageBox.h#L9
        class SkyrimMessageBox {
            class MessageBoxResultCallback : public RE::IMessageBoxCallback {
                std::function<void(unsigned int)> _callback;

            public:
                ~MessageBoxResultCallback() override {}
                MessageBoxResultCallback(std::function<void(unsigned int)> callback) : _callback(callback) {}
                void Run(RE::IMessageBoxCallback::Message message) override {
                    _callback(static_cast<unsigned int>(message));
                }
            };

        public:
            static void Show(const std::string& bodyText, std::vector<std::string> buttonTextValues,
                             std::function<void(unsigned int)> callback) {
                auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
                auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
                auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(
                    uiStringHolder->messageBoxData);  // "MessageBoxData" <--- can we just use this string?
                auto* messagebox = factory->Create();
                RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback =
                    RE::make_smart<MessageBoxResultCallback>(callback);
                messagebox->callback = messageCallback;
                messagebox->bodyText = bodyText;
                for (auto& text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());
                messagebox->QueueMessage();
            }
        };

        void ShowMessageBox(const std::string& bodyText, std::vector<std::string> buttonTextValues,
                            std::function<void(unsigned int)> callback) {
            SkyrimMessageBox::Show(bodyText, buttonTextValues, callback);
        }

        namespace Windows {

            int GeneralErr() {
                MessageBoxA(nullptr, general_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            };
        };

        namespace InGame {

            void IniCreated() { 
                RE::DebugMessageBox("INI created. Customize it to your liking."); 
            };

            void InitErr() { RE::DebugMessageBox(init_err_msgbox.c_str()); };

            void GeneralErr() { RE::DebugMessageBox(general_err_msgbox.c_str()); };

            void NoSourceFound() { RE::DebugMessageBox(no_src_msgbox.c_str()); };

            void FormTypeErr(RE::FormID id) {
				RE::DebugMessageBox(
					std::format("{}: The form type of the item with FormID ({}) is not supported. Please contact the mod author.",
						Utilities::mod_name, Utilities::dec2hex(id)).c_str());
			};
            
            void FormIDError(RE::FormID id) {
                RE::DebugMessageBox(
                    std::format("{}: The ID ({}) could not have been found.",
                                Utilities::mod_name, Utilities::dec2hex(id))
                        .c_str());
            }

            void EditorIDError(std::string id) {
                RE::DebugMessageBox(
                    std::format("{}: The ID ({}) could not have been found.",
                                Utilities::mod_name, id)
                        .c_str());
            }

            void ProblemWithContainer(std::string id) {
                RE::DebugMessageBox(
					std::format("{}: Problem with one of the items with the form id ({}). This is expected if you have changed the list of containers in the INI file between saves. Corresponding items will be returned to your inventory. You can suppress this message by changing the setting in your INI.",
                        								Utilities::mod_name, id)
						.c_str());
            };

            void UninstallSuccessful() {
				RE::DebugMessageBox(
					std::format("{}: Uninstall successful. You can now safely remove the mod.",
								Utilities::mod_name)
						.c_str());
			};

            void UninstallFailed() {
                RE::DebugMessageBox(
                    std::format("{}: Uninstall failed. Please contact the mod author.", Utilities::mod_name).c_str());
            };

            void CustomErrMsg(const std::string& msg) { RE::DebugMessageBox((mod_name + ": " + msg).c_str()); };
        };
    };

    // https :  // github.com/ozooma10/OSLAroused/blob/29ac62f220fadc63c829f6933e04be429d4f96b0/src/PersistedData.cpp
    // template <typename T,typename U>
    // // BaseData is based off how powerof3's did it in Afterlife
    // class BaseData {
    // public:
    //     float GetData(T formId, T missing) {
    //         Locker locker(m_Lock);
    //         // if the plugin version is less than 0.7 need to handle differently
    //         // if (SKSE::PluginInfo::version)
    //         if (auto idx = m_Data.find(formId) != m_Data.end()) {
    //             return m_Data[formId];
    //         }
    //         return missing;
    //     }

    //     void SetData(T formId, U value) {
    //         Locker locker(m_Lock);
    //         m_Data[formId] = value;
    //     }

    //     virtual const char* GetType() = 0;

    //     virtual bool Save(SKSE::SerializationInterface*, std::uint32_t,
    //                       std::uint32_t) {return false;};
    //     virtual bool Save(SKSE::SerializationInterface*) {return false;};
    //     virtual bool Load(SKSE::SerializationInterface*) {return false;};

    //     void Clear() {
    //         Locker locker(m_Lock);
    //         m_Data.clear();
    //     };

    //     virtual void DumpToLog() = 0;

    // protected:
    //     std::map<T,U> m_Data;

    //     using Lock = std::recursive_mutex;
    //     using Locker = std::lock_guard<Lock>;
    //     mutable Lock m_Lock;
    // };

    // class SaveLoadData : public BaseData<Types::SaveDataLHS,Types::SaveDataRHS> {
    // public:
    //     void DumpToLog() override {
    //         Locker locker(m_Lock);
    //         for (const auto& [formId, value] : m_Data) {
    //             logger::info(
    //                 "Dump Row From {}",
    //                 GetType());
    //         }
    //         // sakat olabilir
    //         logger::info("{} Rows Dumped For Type {}", m_Data.size(), GetType());
    //     }

    //     [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface) override {
    //         assert(serializationInterface);
    //         Locker locker(m_Lock);

    //         const auto numRecords = m_Data.size();
    //         if (!serializationInterface->WriteRecordData(numRecords)) {
    //             logger::error("Failed to save {} data records", numRecords);
    //             return false;
    //         }

    //         for (const auto& [formId, value] : m_Data) {
    //             if (!serializationInterface->WriteRecordData(formId)) {
    //                 logger::error("Failed to save data");
    //                 return false;
    //             }

    //             if (!serializationInterface->WriteRecordData(value)) {
    //                 logger::error("Failed to save value data");
    //                 return false;
    //             }
    //         }
    //         return true;
    //     }

    //     [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
    //                        std::uint32_t version) override {
    //         if (!serializationInterface->OpenRecord(type, version)) {
    //             logger::error("Failed to open record for Data Serialization!");
    //             return false;
    //         }

    //         return Save(serializationInterface);
    //     }

    //     [[nodiscard]] bool Load(SKSE::SerializationInterface* serializationInterface) override {
    //         assert(serializationInterface);

    //         std::size_t recordDataSize;
    //         serializationInterface->ReadRecordData(recordDataSize);
    //         logger::trace("Loading data from serialization interface with size: {}", recordDataSize);

    //         Locker locker(m_Lock);
    //         m_Data.clear();

    //         Types::SaveDataLHS formId;
    //         Types::SaveDataRHS value;

    //         for (auto i = 0; i < recordDataSize; i++) {
    //             logger::trace("Loading data from serialization interface.");
    //             logger::trace("ReadRecordData:{}", serializationInterface->ReadRecordData(formId));
    //             auto formid_formid = formId.GetCurrentStage().formid;
    //             if (!serializationInterface->ResolveFormID(formid_formid,formid_formid)) {
    //                 logger::error("Failed to resolve form ID, 0x{:X}.", formid_formid);
    //                 continue;
    //             }
    //             logger::trace("Reading value...");
    //             logger::trace("ReadRecordData: {}", serializationInterface->ReadRecordData(value));
    //             m_Data[formId] = value;
    //             logger::trace("Loaded data for FormRefID: {}", formid_formid);
    //         }
    //         return true;
    //     }
    // };


}
    
