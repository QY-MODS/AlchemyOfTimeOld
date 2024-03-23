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
#include "rapidjson/document.h"



namespace Utilities{

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
    constexpr auto po3_UoTpath = "Data/SKSE/Plugins/po3_UseOrTake.dll";
    bool IsPo3Installed() { return std::filesystem::exists(po3path); };
    bool IsPo3_UoTInstalled() { return std::filesystem::exists(po3_UoTpath); };

    const auto po3_err_msgbox = std::format(
        "{}: If you are trying to use Editor IDs, but you must have powerofthree's Tweaks "
        "installed. See mod page for further instructions.",
        mod_name);

    const auto general_err_msgbox = std::format("{}: Something went wrong. Please contact the mod author.", mod_name);
    
    const auto init_err_msgbox = std::format("{}: The mod failed to initialize and will be terminated.", mod_name);
        std::string dec2hex(int dec) {
        std::stringstream stream;
        stream << std::hex << dec;
        std::string hexString = stream.str();
        return hexString;
    };

    std::string dec2hex(unsigned int dec) {
		    std::stringstream stream;
		    stream << std::hex << dec;
		    std::string hexString = stream.str();
		    return hexString;
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

            int Po3ErrMsg() {
                MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            };
        };

        namespace InGame {

            void IniCreated() { RE::DebugMessageBox("INI created. Customize it to your liking."); };

            void InitErr() { RE::DebugMessageBox(init_err_msgbox.c_str()); };

            void GeneralErr() { RE::DebugMessageBox(general_err_msgbox.c_str()); };

            void FormTypeErr(RE::FormID id) {
                RE::DebugMessageBox(std::format("{}: The form type of the item with FormID ({}) is not supported. "
                                                "Please contact the mod author.",
                                                Utilities::mod_name, Utilities::dec2hex(id))
                                        .c_str());
            };

            void FormIDError(RE::FormID id) {
                RE::DebugMessageBox(std::format("{}: The ID ({}) could not have been found.", Utilities::mod_name,
                                                Utilities::dec2hex(id))
                                        .c_str());
            }

            void EditorIDError(std::string id) {
                RE::DebugMessageBox(
                    std::format("{}: The ID ({}) could not have been found.", Utilities::mod_name, id).c_str());
            }

            void ProblemWithContainer(std::string id) {
                RE::DebugMessageBox(
                    std::format(
                        "{}: Problem with one of the items with the form id ({}). This is expected if you have changed "
                        "the list of containers in the INI file between saves. Corresponding items will be returned to "
                        "your inventory. You can suppress this message by changing the setting in your INI.",
                        Utilities::mod_name, id)
                        .c_str());
            };

            void UninstallSuccessful() {
                RE::DebugMessageBox(
                    std::format("{}: Uninstall successful. You can now safely remove the mod.", Utilities::mod_name)
                        .c_str());
            };

            void UninstallFailed() {
                RE::DebugMessageBox(
                    std::format("{}: Uninstall failed. Please contact the mod author.", Utilities::mod_name).c_str());
            };

            void CustomErrMsg(const std::string& msg) { RE::DebugMessageBox((mod_name + ": " + msg).c_str()); };
        };
    };

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
        
        template <typename T>
        std::size_t GetListLength(RE::BSSimpleList<T>* dataList) {
            std::size_t length = 0;

            for (auto it = dataList->begin(); it != dataList->end(); ++it) {
                // Increment the length for each element in the list
                ++length;
            }

            return length;
        }

        void PrintObjectExtraData(RE::TESObjectREFR* ref) {
            for (int i = 0; i < 191; i++) {
                if (ref->extraList.HasType(static_cast<RE::ExtraDataType>(i))) {
					logger::info("ExtraDataList type: {}", i);
				}
			}
        }


        bool FormIsOfType(RE::TESForm* form, RE::FormType type) {
            if (!form) return false;
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
            //else if(FormIsOfType(form,RE::MagicItem::FORMTYPE)){
            //    RE::MagicItem* form_as_ = form->As<RE::MagicItem>();
            //    if (!form_as_) return false;
            //    if (!form_as_->IsFood()) return false;
            //}
            else return false;
            return true;
        }

        bool IsFoodItem(const FormID formid) {
			return IsFoodItem(GetFormByID(formid));
		}

        bool IsCONT(RefID refid) {
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

        void RefreshMenu(const std::string_view menuname) {
            if (auto ui = RE::UI::GetSingleton()) {
                if (!ui->IsMenuOpen(menuname)) return;
            }
            if (const auto queue = RE::UIMessageQueue::GetSingleton()) {
                queue->AddMessage(menuname, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                queue->AddMessage(menuname, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            }
		}

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

        RE::TESObjectREFR* GetContainerFromMenu() {
            if (auto ui_refid = RE::UI::GetSingleton()->GetMenu<RE::ContainerMenu>()->GetTargetRefHandle()) {
                logger::trace("UI Reference id {}", ui_refid);
                if (auto ui_ref = RE::TESObjectREFR::LookupByHandle(ui_refid)) {
                    logger::trace("UI Reference name {}", ui_ref->GetDisplayFullName());
                    return ui_ref.get();
                }
            }
            return nullptr;
        }

        RE::TESObjectREFR* GetVendorChestFromMenu(){
            if (auto ui_refid = RE::UI::GetSingleton()->GetMenu<RE::BarterMenu>()->GetTargetRefHandle()) {
                logger::trace("UI Reference id {}", ui_refid);
                if (auto ui_ref = RE::TESObjectREFR::LookupByHandle(ui_refid)) {
                    logger::trace("UI Reference name {}", ui_ref->GetDisplayFullName());
                    if (auto barter_npc = ui_ref->GetBaseObject()->As<RE::TESNPC>()) {
                        for (auto& faction_rank : barter_npc->factions) {
                            if (auto merchant_chest = faction_rank.faction->vendorData.merchantContainer) {
                                logger::trace("Found chest with refid {}", merchant_chest->GetFormID());
                                return merchant_chest;
                            }
                        };
                    }
                }
            }
            return nullptr;
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

        [[nodiscard]] const bool HasItemPlusCleanUp(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner,RE::ExtraDataList* xList=nullptr) {
            logger::trace("HasItemPlusCleanUp");

            if (HasItem(item, item_owner)) return true;
            if (HasItemEntry(item, item_owner)) {
                item_owner->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kRemove, xList, nullptr);
                logger::trace("Item with zero count removed from player.");
            }
            return false;
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

        void OverrideMGEFFs(RE::BSTArray<RE::Effect*>& effect_array, std::vector<RE::EffectSetting*> new_effects,
                            std::vector<uint32_t*> durations, std::vector<float*> magnitudes) {
            
            const unsigned int n_current_effects = static_cast<unsigned int>(effect_array.size());
            if (!n_current_effects) {
				logger::warn("No effects found");
				return;
			}
            const unsigned int n_new_effects = static_cast<unsigned int>(new_effects.size());
            unsigned int i = 0;
            while (i<n_new_effects) {
                if (i < n_current_effects) {
					if (new_effects[i]) effect_array[i]->baseEffect = new_effects[i];
                    if (durations[i]) effect_array[i]->effectItem.duration = *durations[i];
                    if (magnitudes[i]) effect_array[i]->effectItem.magnitude = *magnitudes[i];
                } else {
                    effect_array.push_back(effect_array.front());
                    if (new_effects[i]) effect_array.back()->baseEffect = new_effects[i];
                    if (durations[i]) effect_array.back()->effectItem.duration = *durations[i];
                    if (magnitudes[i]) effect_array.back()->effectItem.magnitude = *magnitudes[i];
                }
                i++;
            }
            
        }

        void SetObjectCount(RE::TESObjectREFR* ref, Count count) {
            if (!ref) {
                logger::error("Ref is null.");
                return;
            }
            int max_try = 10;
            while (ref->extraList.HasType(RE::ExtraDataType::kCount) && max_try) {
                ref->extraList.RemoveByType(RE::ExtraDataType::kCount);
                max_try--;
            }
            // ref->extraList.SetCount(static_cast<uint16_t>(count));
            auto xCount = new RE::ExtraCount(static_cast<int16_t>(count));
            ref->extraList.Add(xCount);
        }

        const int16_t GetObjectCount(RE::TESObjectREFR* ref) {
            if (!ref) {
                logger::error("Ref is null.");
                return 0;
            }
            if (ref->extraList.HasType(RE::ExtraDataType::kCount)) {
                RE::ExtraCount* xCount = ref->extraList.GetByType<RE::ExtraCount>();
                return xCount->count;
            }
            return 0;
        }


        RE::TESObjectREFR* DropObjectIntoTheWorld(RE::TESBoundObject* obj, Count count, RE::ExtraDataList*,
                                                  bool owned = true) {
            auto player_ch = RE::PlayerCharacter::GetSingleton();
            auto player_pos = player_ch->GetPosition();
            // distance in the xy-plane
            const auto multiplier = 100.0f;
            player_pos += {multiplier, multiplier, 70};
            auto player_cell = player_ch->GetParentCell();
            auto player_ws = player_ch->GetWorldspace();
            if (!player_cell && !player_ws) {
                logger::critical("Player cell AND player world is null.");
                return nullptr;
            }
            auto newPropRef =
                RE::TESDataHandler::GetSingleton()
                                  ->CreateReferenceAtLocation(obj, player_pos, {0.0f, 0.0f, 0.0f}, player_cell,
                                                              player_ws, nullptr, nullptr, {}, false, false)
                    .get()
                    .get();
            if (!newPropRef) {
                logger::critical("New prop ref is null.");
                return nullptr;
            }
            SetObjectCount(newPropRef, count);
            if (owned) newPropRef->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
            return newPropRef;
        }

        void SwapObjects(RE::TESObjectREFR* a_from, RE::TESBoundObject* a_to, const bool apply_havok=true) {
            logger::trace("SwapObjects");
            if (!a_from) {
                logger::error("Ref is null.");
                return;
            }
            auto ref_base = a_from->GetBaseObject();
            if (!ref_base) {
                logger::error("Ref base is null.");
				return;
            }
            if (ref_base->GetFormID() == a_to->GetFormID()) {
				logger::warn("Ref and base are the same.");
				return;
			}
            a_from->SetObjectReference(a_to);
            a_from->Disable();
            a_from->Enable(false);
            if (!apply_havok) return;

            /*float afX = 100;
            float afY = 100;
            float afZ = 100;
            float afMagnitude = 100;*/
            /*auto args = RE::MakeFunctionArguments(std::move(afX), std::move(afY), std::move(afZ),
            std::move(afMagnitude)); vm->DispatchMethodCall(object, "ApplyHavokImpulse", args, callback);*/
            // Looked up here (wSkeever): https:  // www.nexusmods.com/skyrimspecialedition/mods/73607
            SKSE::GetTaskInterface()->AddTask([a_from]() {
                // auto player_ch = RE::PlayerCharacter::GetSingleton();
                // player_ch->StartGrabObject();
                auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
                auto policy = vm->GetObjectHandlePolicy();
                auto handle = policy->GetHandleForObject(a_from->GetFormType(), a_from);
                RE::BSTSmartPointer<RE::BSScript::Object> object = nullptr;
                vm->CreateObject2("ObjectReference", object);
                vm->BindObject(object, handle, false);
                if (!object) logger::warn("Object is null");
                auto args = RE::MakeFunctionArguments(std::move(0.f), std::move(0.f), std::move(0.f), std::move(0.f));
                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                if (vm->DispatchMethodCall(object, "ApplyHavokImpulse", args, callback)) logger::trace("FUSRODAH");
            });
        }


        RE::TESObjectREFR* TryToGetRefInCell(const FormID baseid, const Count count, unsigned int max_try=3) {
            auto player_cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
            auto cell_runtime_data = player_cell->GetRuntimeData();
            for (auto& ref : cell_runtime_data.references) {
                if (!ref) continue;
                if (ref->GetBaseObject()->GetFormID() == baseid &&
                    ref->extraList.GetCount() == count) {
                    logger::info("Ref found in cell: {}", ref->GetBaseObject()->GetName());
                    return ref.get();
                }
            }
            if (max_try) return TryToGetRefInCell(baseid, count, --max_try);
            return nullptr;
        }

        RE::TESObjectREFR* TryToGetRefFromHandle(RE::ObjectRefHandle& handle, unsigned int max_try = 3) {
            if (auto handle_ref = RE::TESObjectREFR::LookupByHandle(handle.native_handle())) {
                logger::trace("Handle ref found");
                return handle_ref.get();
                //if (!ref && handle.get()) ref = handle.get().get();
            }
            else if (handle.get()) {
                logger::warn("Handle native handle is null");
                return handle.get().get();
            } 
            else if (auto ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(handle.native_handle()))
            {
                logger::trace("Ref found");
                return ref;
            }
            if (max_try && handle) return TryToGetRefFromHandle(handle, --max_try);
            return nullptr;
		}

        RefID TryToGetRefIDFromHandle(RE::ObjectRefHandle handle) {
            if (handle.get() && handle.get()->GetFormID()) return handle.get()->GetFormID();
            if (handle.native_handle()
                //&& RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(handle.native_handle())
                ) return handle.native_handle();
            return 0;
        }
    };
    
    namespace FunctionsPapyrus {
     //   using VM = RE::BSScript::Internal::VirtualMachine;
	    //using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;

     //   // https:// github.com/clayne/GTS_Plugin/blob/master/src/utils/papyrusUtils.hpp#L12
     //   inline RE::VMHandle GetHandle(RE::TESForm* a_form) {
     //       auto vm = VM::GetSingleton();
     //       auto policy = vm->GetObjectHandlePolicy();
     //       return policy->GetHandleForObject(a_form->GetFormType(), a_form);
     //   }

     //   // https://github.com/clayne/GTS_Plugin/blob/master/src/utils/papyrusUtils.hpp#L19
     //   inline ObjectPtr GetObjectPtr(RE::TESForm* a_form, const char* a_class, bool a_create) {
     //       auto vm = VM::GetSingleton();
     //       auto handle = GetHandle(a_form);

     //       ObjectPtr object = nullptr;
     //       bool found = vm->FindBoundObject(handle, a_class, object);
     //       if (!found && a_create) {
     //           vm->CreateObject2(a_class, object);
     //           vm->BindObject(object, handle, false);
     //       }

     //       return object;
     //   }

     //   // https://github.com/clayne/GTS_Plugin/blob/master/src/utils/papyrusUtils.hpp#L35
     //   template <class... Args>
     //   inline void CallFunctionOn(RE::TESForm* a_form, std::string_view formKind, std::string_view function,
     //                              Args... a_args) {
     //       const auto skyrimVM = RE::SkyrimVM::GetSingleton();
     //       auto vm = skyrimVM ? skyrimVM->impl : nullptr;
     //       if (vm) {
     //           RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
     //           auto args = RE::MakeFunctionArguments(std::forward<Args>(a_args)...);
     //           auto objectPtr = GetObjectPtr(a_form, std::string(formKind).c_str(), false);
     //           if (!objectPtr) {
     //               logger::error("Could not bind form");
     //           }
     //           vm->DispatchMethodCall(objectPtr, std::string(function).c_str(), args, callback);
     //       }
     //   }

     //   typedef void (*_ApplyHavokImpulse)(RE::BSScript::VMState* registry, uint32_t stackID, RE::TESObjectREFR* target, float afX,
     //                                      float afY, float afZ, float magnitude);
     //   const _ApplyHavokImpulse ApplyHavokImpulse = (_ApplyHavokImpulse)0x00908260;
     //   RE::SkyrimVM*& g_skyrimVM = *(RE::SkyrimVM**)0x012E568C;
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

        template <typename T>
        std::vector<T> mergeVectors(const std::vector<T>& vec1, const std::vector<T>& vec2) {
            std::vector<T> mergedVec;

            // Reserve enough space to avoid frequent reallocation
            mergedVec.reserve(vec1.size() + vec2.size());

            // Insert elements from vec1
            mergedVec.insert(mergedVec.end(), vec1.begin(), vec1.end());

            // Insert elements from vec2
            mergedVec.insert(mergedVec.end(), vec2.begin(), vec2.end());

            return mergedVec;
        }

        std::string toLowercase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return result;
        }

        bool includesString(const std::string& input, const std::vector<std::string>& strings) {
            std::string lowerInput = toLowercase(input);

            for (const auto& str : strings) {
                std::string lowerStr = str;
                std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lowerInput.find(lowerStr) != std::string::npos) {
                    return true;  // The input string includes one of the strings
                }
            }
            return false;  // None of the strings in 'strings' were found in the input string
        }

        bool includesWord(const std::string& input, const std::vector<std::string>& strings) {
            std::istringstream iss(input);
            std::vector<std::string> inputWords{std::istream_iterator<std::string>{iss},
                                                std::istream_iterator<std::string>{}};

            for (const auto& str : strings) {
                std::istringstream iss2(str);
                std::vector<std::string> strWords{std::istream_iterator<std::string>{iss2},
                                                  std::istream_iterator<std::string>{}};

                for (const auto& word : strWords) {
                    std::string lowercaseWord = toLowercase(word);
                    if (std::find_if(inputWords.begin(), inputWords.end(),
                                     [&lowercaseWord](const std::string& inputWord) {
                                         return toLowercase(inputWord) == lowercaseWord;
                                     }) != inputWords.end()) {
                        return true;  // The input string includes one of the strings
                    }
                }
            }
            return false;  // None of the strings in 'strings' were found in the input string
        }


        template <typename T>
        bool VectorHasElement(const std::vector<T>& vec, const T& element) {
			return std::find(vec.begin(), vec.end(), element) != vec.end();
		}

        bool isValidHexWithLength7or8(const char* input) {
            std::string inputStr(input);
            std::regex hexRegex("^[0-9A-Fa-f]{7,8}$");  // Allow 7 to 8 characters
            bool isValid = std::regex_match(inputStr, hexRegex);
            return isValid;
        }
    };

    namespace FunctionsJSON {
        
        int GetFormEditorID(const rapidjson::Value& section, const char* memberName) {
            if (!section.HasMember(memberName)) {
                logger::error("Member {} not found", memberName);
                return -1;
            }
            if (section[memberName].IsString()) {
                const std::string formEditorId = section[memberName].GetString();
                if (Utilities::Functions::isValidHexWithLength7or8(formEditorId.c_str())) {
                    int form_id_;
                    std::stringstream ss;
                    ss << std::hex << formEditorId;
                    ss >> form_id_;
                    auto temp_form = FunctionsSkyrim::GetFormByID(form_id_, "");
                    if (temp_form) return temp_form->GetFormID();
                    else {
                        logger::error("Formid is null for editorid {}", formEditorId);
						return -1;
                    }
                }
                if (formEditorId.empty()) return 0;
                else if (!IsPo3Installed()) {
                    logger::error("Po3 is not installed.");
                    MsgBoxesNotifs::Windows::Po3ErrMsg();
                    return -1;
                }
                auto temp_form = FunctionsSkyrim::GetFormByID(0, formEditorId);
                if (temp_form) return temp_form->GetFormID();
                else {
                    logger::error("Formid is null for editorid {}", formEditorId);
                    return -1;
                }
            } 
            else if (section[memberName].IsInt()) return section[memberName].GetInt();
			return -1;
        }
    }

    namespace Types {

        // using EditorID = std::string;
        using NameID = std::string;
        using Duration = std::uint32_t;
        using StageNo = unsigned int;
        using StageName = std::string;

        struct StageEffect {
            FormID beffect;          // base effect
            float magnitude;         // in effectitem
            std::uint32_t duration;  // in effectitem

            StageEffect() : beffect(0), magnitude(0), duration(0) {}
            StageEffect(FormID be, float mag, Duration dur) : beffect(be), magnitude(mag), duration(dur) {}

            [[nodiscard]] const bool IsNull() { return beffect == 0; }
            [[nodiscard]] const bool HasMagnitude() { return magnitude != 0; }
            [[nodiscard]] const bool HasDuration() { return duration != 0; }
        };

        struct Stage {
            FormID formid = 0;  // with which item is it represented
            Duration duration;  // duration of the stage
            StageNo no;         // used for sorting when multiple stages are present
            StageName name;     // name of the stage
            std::vector<StageEffect> mgeffect;

            Stage(){};
            Stage(FormID f, Duration d, StageNo s, StageName n, std::vector<StageEffect> e)
                : formid(f), duration(d), no(s), name(n), mgeffect(e) {
                if (!formid)
                    logger::critical("FormID is null");
                else
                    logger::trace("Stage: FormID {}, Duration {}, StageNo {}, Name {}", formid, duration, no, name);
                if (e.empty()) mgeffect.clear();
            }

            bool operator<(const Stage& other) const {
                if (formid < other.formid) return true;
                if (other.formid < formid) return false;
                return no < other.no;
            }

            bool operator==(const Stage& other) const {
                return no == other.no && formid == other.formid && duration == other.duration;
            }

            RE::TESBoundObject* GetBound() { return FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid); };

            std::string GetEditorID() {
				if (auto bound = GetBound()) return bound->GetFormEditorID();
				return "";
			}

            [[nodiscard]] const bool CheckIntegrity() {
                if (!formid || !GetBound()) {
					logger::error("FormID or bound is null");
					return false;
				}
				return true;
            }
            
            RE::ExtraTextDisplayData* GetExtraText() {
                _textData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
                _textData->SetName(GetBound()->GetName());
                return _textData;
            }

        private:
            //[[maybe_unused]] RE::ExtraTextDisplayData* _textData = nullptr;
            RE::ExtraTextDisplayData* _textData = nullptr;

        };

        using Stages = std::vector<Stage>;
        using StageDict = std::map<StageNo, Stage>;

        struct StageInstance {
            float start_time;
            StageNo no;
            Count count;
            RefID location;  // RefID of the container where the fake food is stored or the real food itself when it is
                             // out in the world
            std::string editorid;

            StageInstance() : start_time(0), no(0), count(0), location(0) {}
            StageInstance(float st, StageNo n, Count c, RefID l, std::string ei)
                : start_time(st), no(n), count(c), location(l), editorid(ei) {}
        };

        struct StageUpdate {
            StageNo old_no;
            StageNo new_no;
            Count count;
        };

        using SourceData = std::vector<StageInstance>;  //
        using SaveDataLHS = FormID;
        using SaveDataRHS = StageInstance;

    }


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
    
