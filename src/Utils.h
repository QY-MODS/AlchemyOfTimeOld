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

    const char* ExtraDataTypeNames[]{
        "kNone",                     // 0x00
        "kHavok",                    // 0x01 - ExtraHavok
        "kCell3D",                   // 0x02 - ExtraCell3D
        "kCellWaterType",            // 0x03 - ExtraCellWaterType
        "kRegionList",               // 0x04 - ExtraRegionList
        "kSeenData",                 // 0x05 - ExtraSeenData
        "kEditorID",                 // 0x06 - ExtraEditorID
        "kCellMusicType",            // 0x07 - ExtraCellMusicType
        "kCellSkyRegion",            // 0x08 - ExtraCellSkyRegion
        "kProcessMiddleLow",         // 0x09 - ExtraProcessMiddleLow
        "kDetachTime",               // 0x0A - ExtraDetachTime
        "kPersistentCell",           // 0x0B - ExtraPersistentCell
        "kUnk0C",                    // 0x0C
        "kAction",                   // 0x0D - ExtraAction
        "kStartingPosition",         // 0x0E - ExtraStartingPosition
        "kUnk0F",                    // 0x0F
        "kAnimGraphManager",         // 0x10 - ExtraAnimGraphManager
        "kBiped",                    // 0x11 - ExtraBiped
        "kUsedMarkers",              // 0x12 - ExtraUsedMarkers
        "kDistantData",              // 0x13 - ExtraDistantData
        "kRagDollData",              // 0x14 - ExtraRagDollData
        "kContainerChanges",         // 0x15 - ExtraContainerChanges
        "kWorn",                     // 0x16 - ExtraWorn
        "kWornLeft",                 // 0x17 - ExtraWornLeft
        "kPackageStartLocation",     // 0x18 - ExtraPackageStartLocation
        "kPackage",                  // 0x19 - ExtraPackage
        "kTresPassPackage",          // 0x1A - ExtraTresPassPackage
        "kRunOncePacks",             // 0x1B - ExtraRunOncePacks
        "kReferenceHandle",          // 0x1C - ExtraReferenceHandle
        "kFollower",                 // 0x1D - ExtraFollower
        "kLevCreaModifier",          // 0x1E - ExtraLevCreaModifier
        "kGhost",                    // 0x1F - ExtraGhost
        "kOriginalReference",        // 0x20 - ExtraOriginalReference
        "kOwnership",                // 0x21 - ExtraOwnership
        "kGlobal",                   // 0x22 - ExtraGlobal
        "kRank",                     // 0x23 - ExtraRank
        "kCount",                    // 0x24 - ExtraCount
        "kHealth",                   // 0x25 - ExtraHealth
        "kUnk26",                    // 0x26
        "kTimeLeft",                 // 0x27 - ExtraTimeLeft
        "kCharge",                   // 0x28 - ExtraCharge
        "kLight",                    // 0x29 - ExtraLight
        "kLock",                     // 0x2A - ExtraLock
        "kTeleport",                 // 0x2B - ExtraTeleport
        "kMapMarker",                // 0x2C - ExtraMapMarker
        "kLeveledCreature",          // 0x2D - ExtraLeveledCreature
        "kLeveledItem",              // 0x2E - ExtraLeveledItem
        "kScale",                    // 0x2F - ExtraScale
        "kMissingLinkedRefIDs",      // 0x30 - ExtraMissingLinkedRefIDs
        "kMagicCaster",              // 0x31 - ExtraMagicCaster
        "kNonActorMagicTarget",      // 0x32 - NonActorMagicTarget
        "kUnk33",                    // 0x33
        "kPlayerCrimeList",          // 0x34 - ExtraPlayerCrimeList
        "kUnk35",                    // 0x35
        "kEnableStateParent",        // 0x36 - ExtraEnableStateParent
        "kEnableStateChildren",      // 0x37 - ExtraEnableStateChildren
        "kItemDropper",              // 0x38 - ExtraItemDropper
        "kDroppedItemList",          // 0x39 - ExtraDroppedItemList
        "kRandomTeleportMarker",     // 0x3A - ExtraRandomTeleportMarker
        "kUnk3B",                    // 0x3B
        "kSavedHavokData",           // 0x3C - ExtraSavedHavokData
        "kCannotWear",               // 0x3D - ExtraCannotWear
        "kPoison",                   // 0x3E - ExtraPoison
        "kMagicLight",               // 0x3F - ExtraMagicLight
        "kLastFinishedSequence",     // 0x40 - ExtraLastFinishedSequence
        "kSavedAnimation",           // 0x41 - ExtraSavedAnimation
        "kNorthRotation",            // 0x42 - ExtraNorthRotation
        "kSpawnContainer",           // 0x43 - ExtraSpawnContainer
        "kFriendHits",               // 0x44 - ExtraFriendHits
        "kHeadingTarget",            // 0x45 - ExtraHeadingTarget
        "kUnk46",                    // 0x46
        "kRefractionProperty",       // 0x47 - ExtraRefractionProperty
        "kStartingWorldOrCell",      // 0x48 - ExtraStartingWorldOrCell
        "kHotkey",                   // 0x49 - ExtraHotkey
        "kEditorRef3DData",          // 0x4A - ExtraEditorRef3DData
        "kEditorRefMoveData",        // 0x4B - ExtraEditorRefMoveData
        "kInfoGeneralTopic",         // 0x4C - ExtraInfoGeneralTopic
        "kHasNoRumors",              // 0x4D - ExtraHasNoRumors
        "kSound",                    // 0x4E - ExtraSound
        "kTerminalState",            // 0x4F - ExtraTerminalState
        "kLinkedRef",                // 0x50 - ExtraLinkedRef
        "kLinkedRefChildren",        // 0x51 - ExtraLinkedRefChildren
        "kActivateRef",              // 0x52 - ExtraActivateRef
        "kActivateRefChildren",      // 0x53 - ExtraActivateRefChildren
        "kCanTalkToPlayer",          // 0x54 - ExtraCanTalkToPlayer
        "kObjectHealth",             // 0x55 - ExtraObjectHealth
        "kCellImageSpace",           // 0x56 - ExtraCellImageSpace
        "kNavMeshPortal",            // 0x57 - ExtraNavMeshPortal
        "kModelSwap",                // 0x58 - ExtraModelSwap
        "kRadius",                   // 0x59 - ExtraRadius
        "kUnk5A",                    // 0x5A
        "kFactionChanges",           // 0x5B - ExtraFactionChanges
        "kDismemberedLimbs",         // 0x5C - ExtraDismemberedLimbs
        "kActorCause",               // 0x5D - ExtraActorCause
        "kMultiBound",               // 0x5E - ExtraMultiBound
        "kMultiBoundMarkerData",     // 0x5F - MultiBoundMarkerData
        "kMultiBoundRef",            // 0x60 - ExtraMultiBoundRef
        "kReflectedRefs",            // 0x61 - ExtraReflectedRefs
        "kReflectorRefs",            // 0x62 - ExtraReflectorRefs
        "kEmittanceSource",          // 0x63 - ExtraEmittanceSource
        "kUnk64",                    // 0x64
        "kCombatStyle",              // 0x65 - ExtraCombatStyle
        "kUnk66",                    // 0x66
        "kPrimitive",                // 0x67 - ExtraPrimitive
        "kOpenCloseActivateRef",     // 0x68 - ExtraOpenCloseActivateRef
        "kAnimNoteReceiver",         // 0x69 - ExtraAnimNoteReceiver
        "kAmmo",                     // 0x6A - ExtraAmmo
        "kPatrolRefData",            // 0x6B - ExtraPatrolRefData
        "kPackageData",              // 0x6C - ExtraPackageData
        "kOcclusionShape",           // 0x6D - ExtraOcclusionShape
        "kCollisionData",            // 0x6E - ExtraCollisionData
        "kSayTopicInfoOnceADay",     // 0x6F - ExtraSayTopicInfoOnceADay
        "kEncounterZone",            // 0x70 - ExtraEncounterZone
        "kSayTopicInfo",             // 0x71 - ExtraSayToTopicInfo
        "kOcclusionPlaneRefData",    // 0x72 - ExtraOcclusionPlaneRefData
        "kPortalRefData",            // 0x73 - ExtraPortalRefData
        "kPortal",                   // 0x74 - ExtraPortal
        "kRoom",                     // 0x75 - ExtraRoom
        "kHealthPerc",               // 0x76 - ExtraHealthPerc
        "kRoomRefData",              // 0x77 - ExtraRoomRefData
        "kGuardedRefData",           // 0x78 - ExtraGuardedRefData
        "kCreatureAwakeSound",       // 0x79 - ExtraCreatureAwakeSound
        "kUnk7A",                    // 0x7A
        "kHorse",                    // 0x7B - ExtraHorse
        "kIgnoredBySandbox",         // 0x7C - ExtraIgnoredBySandbox
        "kCellAcousticSpace",        // 0x7D - ExtraCellAcousticSpace
        "kReservedMarkers",          // 0x7E - ExtraReservedMarkers
        "kWeaponIdleSound",          // 0x7F - ExtraWeaponIdleSound
        "kWaterLightRefs",           // 0x80 - ExtraWaterLightRefs
        "kLitWaterRefs",             // 0x81 - ExtraLitWaterRefs
        "kWeaponAttackSound",        // 0x82 - ExtraWeaponAttackSound
        "kActivateLoopSound",        // 0x83 - ExtraActivateLoopSound
        "kPatrolRefInUseData",       // 0x84 - ExtraPatrolRefInUseData
        "kAshPileRef",               // 0x85 - ExtraAshPileRef
        "kCreatureMovementSound",    // 0x86 - ExtraCreatureMovementSound
        "kFollowerSwimBreadcrumbs",  // 0x87 - ExtraFollowerSwimBreadcrumbs
        "kAliasInstanceArray",       // 0x88 - ExtraAliasInstanceArray
        "kLocation",                 // 0x89 - ExtraLocation
        "kUnk8A",                    // 0x8A
        "kLocationRefType",          // 0x8B - ExtraLocationRefType
        "kPromotedRef",              // 0x8C - ExtraPromotedRef
        "kAnimationSequencer",       // 0x8D - ExtraAnimationSequencer
        "kOutfitItem",               // 0x8E - ExtraOutfitItem
        "kUnk8F",                    // 0x8F
        "kLeveledItemBase",          // 0x90 - ExtraLeveledItemBase
        "kLightData",                // 0x91 - ExtraLightData
        "kSceneData",                // 0x92 - ExtraSceneData
        "kBadPosition",              // 0x93 - ExtraBadPosition
        "kHeadTrackingWeight",       // 0x94 - ExtraHeadTrackingWeight
        "kFromAlias",                // 0x95 - ExtraFromAlias
        "kShouldWear",               // 0x96 - ExtraShouldWear
        "kFavorCost",                // 0x97 - ExtraFavorCost
        "kAttachedArrows3D",         // 0x98 - ExtraAttachedArrows3D
        "kTextDisplayData",          // 0x99 - ExtraTextDisplayData
        "kAlphaCutoff",              // 0x9A - ExtraAlphaCutoff
        "kEnchantment",              // 0x9B - ExtraEnchantment
        "kSoul",                     // 0x9C - ExtraSoul
        "kForcedTarget",             // 0x9D - ExtraForcedTarget
        "kUnk9E",                    // 0x9E
        "kUniqueID",                 // 0x9F - ExtraUniqueID
        "kFlags",                    // 0xA0 - ExtraFlags
        "kRefrPath",                 // 0xA1 - ExtraRefrPath
        "kDecalGroup",               // 0xA2 - ExtraDecalGroup
        "kLockList",                 // 0xA3 - ExtraLockList
        "kForcedLandingMarker",      // 0xA4 - ExtraForcedLandingMarker
        "kLargeRefOwnerCells",       // 0xA5 - ExtraLargeRefOwnerCells
        "kCellWaterEnvMap",          // 0xA6 - ExtraCellWaterEnvMap
        "kCellGrassData",            // 0xA7 - ExtraCellGrassData
        "kTeleportName",             // 0xA8 - ExtraTeleportName
        "kInteraction",              // 0xA9 - ExtraInteraction
        "kWaterData",                // 0xAA - ExtraWaterData
        "kWaterCurrentZoneData",     // 0xAB - ExtraWaterCurrentZoneData
        "kAttachRef",                // 0xAC - ExtraAttachRef
        "kAttachRefChildren",        // 0xAD - ExtraAttachRefChildren
        "kGroupConstraint",          // 0xAE - ExtraGroupConstraint
        "kScriptedAnimDependence",   // 0xAF - ExtraScriptedAnimDependence
        "kCachedScale",              // 0xB0 - ExtraCachedScale
        "kRaceData",                 // 0xB1 - ExtraRaceData
        "kGIDBuffer",                // 0xB2 - ExtraGIDBuffer
        "kMissingRefIDs",            // 0xB3 - ExtraMissingRefIDs
        "kUnkB4",                    // 0xB4
        "kResourcesPreload",         // 0xB5 - ExtraResourcesPreload
        "kUnkB6",                    // 0xB6
        "kUnkB7",                    // 0xB7
        "kUnkB8",                    // 0xB8
        "kUnkB9",                    // 0xB9
        "kUnkBA",                    // 0xBA
        "kUnkBB",                    // 0xBB
        "kUnkBC",                    // 0xBC
        "kUnkBD",                    // 0xBD
        "kUnkBE",                    // 0xBE
        "kUnkBF"                     // 0xBF
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

    namespace Types {

        //using EditorID = std::string;
        using NameID = std::string;
        using FormID = RE::FormID;
        using RefID = RE::FormID;
        using Count = RE::TESObjectREFR::Count;
        using Duration = std::uint32_t;
        using StageNo = unsigned int;
        using StageName = std::string;

        struct StageEffect {
            FormID beffect; // base effect
            float magnitude; // in effectitem
            std::uint32_t duration; // in effectitem

            StageEffect() : beffect(0), magnitude(0), duration(0) {}
            StageEffect(FormID be, float mag, Duration dur) : beffect(be), magnitude(mag), duration(dur) {}

            [[nodiscard]] const bool IsNull() {return beffect == 0;}
            [[nodiscard]] const bool HasMagnitude() { return magnitude == 0; }
            [[nodiscard]] const bool HasDuration() { return duration == 0; }
        };

        struct Stage {
            FormID formid=0; // with which item is it represented
            Duration duration; // duration of the stage
            StageNo no; // used for sorting when multiple stages are present
            StageName name; // name of the stage
            std::vector<StageEffect> mgeffect;
            
            Stage(){};
            Stage(FormID f, Duration d, StageNo s, StageName n, std::vector<StageEffect> e)
                : formid(f), duration(d) , no(s), name(n), mgeffect(e){
                    if (!formid) logger::critical("FormID is null");
                    else logger::trace("Stage: FormID {}, Duration {}, StageNo {}, Name {}", formid, duration, no, name);
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
        };

        using Stages = std::vector<Stage>;
        using StageDict = std::map<StageNo,Stage>;


        struct StageInstance {
            float start_time;
            StageNo no;
            Count count;
            RefID location;  // RefID of the container where the fake food is stored or the real food itself when it is out in the world

            StageInstance() : start_time(0), no(0), count(0), location(0) {}
            StageInstance(float st, StageNo n, Count c, RefID l) : start_time(st), no(n), count(c), location(l) {}
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


        RE::TESObjectREFR* TryToGetRefFromHandle(RE::ObjectRefHandle handle) {
            if (!handle) return nullptr;
            if (!handle.get()) return nullptr;
            if (auto ref = RE::TESObjectREFR::LookupByHandle(handle.native_handle()).get()) return ref;
            if (!handle.get().get()) return nullptr;
            if (auto ref = handle.get().get()) return ref;
            return nullptr;
        }

        Types::RefID TryToGetRefIDFromHandle(RE::ObjectRefHandle handle) {
            if (!handle) return 0;
            if (!handle.get()) return handle.native_handle();
            if (auto ref = handle.get()->GetFormID()) return ref;
            if (auto ref = handle.native_handle()) return ref;
            return 0;
        }


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

        bool IsFoodItem(const Types::FormID formid) {
			return IsFoodItem(GetFormByID(formid));
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


        RE::TESObjectREFR* DropObjectIntoTheWorld(RE::TESBoundObject* obj, Types::Count count, RE::ExtraDataList*) {
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
                                                player_ws, nullptr, nullptr, {}, true, false)
                    .get()
                    .get();
            if (!newPropRef) {
                logger::critical("New prop ref is null.");
                return nullptr;
            }
            newPropRef->extraList.SetCount(static_cast<uint16_t>(count));
            newPropRef->extraList.SetOwner(RE::TESForm::LookupByID<RE::TESForm>(0x07));
            return newPropRef;
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
    
