#include "Manager.h"

Manager* M = nullptr;

class OurEventSink : public RE::BSTEventSink<RE::TESEquipEvent>,
                     public RE::BSTEventSink<RE::TESActivateEvent>,
                     public RE::BSTEventSink<SKSE::CrosshairRefEvent>,
                     public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                     public RE::BSTEventSink<RE::TESContainerChangedEvent> {

    OurEventSink() = default;
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;


    bool block_droptake = false;
    RE::UI* ui = RE::UI::GetSingleton();

    FormID fake_equipped_id;  // set in equip event only when equipped and used in container event (consume)
    RefID picked_up_refid;
    float picked_up_time;
    bool activate_eat = false;

public:
    static OurEventSink* GetSingleton() {
        static OurEventSink singleton;
        return &singleton;
    }


    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
         if (!event) return RE::BSEventNotifyControl::kContinue;
         if (!event->actor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        
         if (!M->IsStage(event->baseObject)) return RE::BSEventNotifyControl::kContinue;

         fake_equipped_id = event->equipped ? event->baseObject : 0;
         logger::trace("Fake equipped: {}", fake_equipped_id);
        
        // if (event->equipped) {
	    //     logger::trace("Item {} was equipped. equipped: {}", event->baseObject);
        // } else {
        //     logger::trace("Item {} was unequipped. equipped: {}", event->baseObject);
        // }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) {
        
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->objectActivated) return RE::BSEventNotifyControl::kContinue;
        if (event->objectActivated == RE::PlayerCharacter::GetSingleton()->GetGrabbedRef()) return RE::BSEventNotifyControl::kContinue;
        if (!event->actionRef->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        if (!M->IsItem(event->objectActivated.get())) return RE::BSEventNotifyControl::kContinue;
        
        if (M->getPO3UoTInstalled()) {
            if (auto base = event->objectActivated->GetBaseObject()) {
                RE::BSString str;
                base->GetActivateText(RE::PlayerCharacter::GetSingleton(), str);
                if (Utilities::Functions::includesWord(str.c_str(), {"Eat", "Drink"})) activate_eat = true;
            }
        }
            
        picked_up_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        picked_up_refid = event->objectActivated->GetFormID();
        logger::trace("Picked up: {} at time {}", picked_up_refid, picked_up_time);


        return RE::BSEventNotifyControl::kContinue;
    }

    // to disable ref activation and external container-fake container placement
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event,
                                          RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {

        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->crosshairRef) return RE::BSEventNotifyControl::kContinue;
        if (!M->getListenCrosshair()) return RE::BSEventNotifyControl::kContinue;

        logger::trace("NAME: {}", event->crosshairRef->GetName());
        // extradata
		for (auto& x : event->crosshairRef->extraList) {
            logger::trace("ExtraData: {}", Utilities::ExtraDataTypeNames[static_cast<int>(x.GetType())]);
		}

        if (event->crosshairRef->extraList.HasType(RE::ExtraDataType::kLinkedRef)) {
            auto linked_ref = event->crosshairRef->extraList.GetByType<RE::ExtraLinkedRef>();
            for (auto& ref : linked_ref->linkedRefs) {
                if (ref.refr) {
				    logger::trace("LinkedRef NAME: {}", ref.refr->GetName());
				    logger::trace("LinkedRef ID: {}", ref.refr->GetFormID());
                }
                if (ref.keyword) logger::trace("LinkedRef KW: {}", ref.keyword->GetName());
			}
        }
        if (event->crosshairRef->extraList.HasType(RE::ExtraDataType::kMissingLinkedRefIDs)) {
            auto linked_ref = event->crosshairRef->extraList.GetByType<RE::ExtraMissingLinkedRefIDs>();
            for (auto& ref : linked_ref->entries) {
                logger::trace("MissingLinkedRefID: {}", ref.linkedRefID);
                if (auto refff = RE::TESForm::LookupByID(ref.linkedRefID))
                    logger::trace("MissingLinkedRef Name 1: {}", refff->GetName());
                else if (auto reff = RE::TESForm::LookupByID<RE::TESObjectREFR>(ref.linkedRefID))
                    logger::trace("MissingLinkedRef Name 2: {}", reff->GetName());

            }
        }

        if (event->crosshairRef->extraList.HasType(RE::ExtraDataType::kLastFinishedSequence)) {
            auto last_finished_seq = event->crosshairRef->extraList.GetByType<RE::ExtraLastFinishedSequence>();
            logger::trace("last_finished_seq->lastSequenceName: {}",last_finished_seq->lastSequenceName.c_str());
        }

        if (event->crosshairRef->extraList.GetOwner())
            logger::trace("{}", event->crosshairRef->extraList.GetOwner()->GetName());

        return RE::BSEventNotifyControl::kContinue;
        

        if (M->IsExternalContainer(event->crosshairRef.get())) M->UpdateSpoilage(event->crosshairRef.get());

        if (!M->IsItem(event->crosshairRef.get())) return RE::BSEventNotifyControl::kContinue;
        
        
        if (!M->RefIsRegistered(event->crosshairRef->GetFormID())) {
            logger::trace("Item not registered.");
            M->RegisterWorldObject(event->crosshairRef.get());
        }
        else M->UpdateSpoilage(event->crosshairRef.get());
        
        return RE::BSEventNotifyControl::kContinue;
    }
    
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
        
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->opening) return RE::BSEventNotifyControl::kContinue;
        if (!ui->IsMenuOpen(RE::FavoritesMenu::MENU_NAME) &&
            !ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) &&
            !ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) return RE::BSEventNotifyControl::kContinue;

        auto menuname = event->menuName.c_str();
        // return if menu is not favorite menu, container menu, barter menu or inventory menu
        if (menuname == RE::FavoritesMenu::MENU_NAME) {
            logger::trace("Favorites menu is open.");
            if (M->UpdateSpoilage(20)) {
                if (const auto queue = RE::UIMessageQueue::GetSingleton()) {
                    queue->AddMessage(RE::FavoritesMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    queue->AddMessage(RE::FavoritesMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
                }
            }
            logger::trace("Spoilage updated.");
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::InventoryMenu::MENU_NAME) {
            logger::trace("Inventory menu is open.");
            if (M->UpdateSpoilage(20)) {
                if (const auto queue = RE::UIMessageQueue::GetSingleton()) {
                    queue->AddMessage(RE::InventoryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    queue->AddMessage(RE::InventoryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
                }
            }
            logger::trace("Spoilage updated.");
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::BarterMenu::MENU_NAME){
            logger::trace("Barter menu is open.");
            if (M->UpdateSpoilage(20)) {
                if (const auto queue = RE::UIMessageQueue::GetSingleton()) {
                    queue->AddMessage(RE::BarterMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    queue->AddMessage(RE::BarterMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
        
        return RE::BSEventNotifyControl::kContinue;
        
    }
    
    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
        
        logger::trace("ListenContainerChange: {}",
                      M->getListenContainerChange());
        if (!event) return RE::BSEventNotifyControl::kContinue;
        logger::trace("Item count: {}", event->itemCount);
        logger::trace("BaseObj: {}", event->baseObj);
        logger::trace("OldContainer: {}", event->oldContainer);
        logger::trace("NewContainer: {}", event->newContainer);
        logger::trace("Reference: {}", Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(event->reference));
        logger::trace("uniqueID: {}", event->uniqueID);
        logger::trace("pad16: {}", event->pad16);

        if (!M->getListenContainerChange()) return RE::BSEventNotifyControl::kContinue;
        if (!event->itemCount) return RE::BSEventNotifyControl::kContinue;
        if (!event->baseObj) return RE::BSEventNotifyControl::kContinue;
        if (!M->IsItem(event->baseObj)) return RE::BSEventNotifyControl::kContinue;

   //     if (M->RefIsRegistered(event->reference.native_handle()) && event->newContainer) {
   //         auto external_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->newContainer);
   //         if (external_ref && external_ref->HasContainer()) {
   //             M->HandlePickUp(event->baseObj, event->itemCount, event->reference.native_handle(),false,external_ref);
   //         }
			//else logger::trace("ExternalRef not found.");
   //     }
   //     else if (event->reference.get() && M->RefIsRegistered(event->reference.get()->GetFormID()) && event->newContainer) {
   //         logger::trace("Ref id: {}", event->reference.get()->GetFormID());
   //         auto external_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->newContainer);
   //         if (external_ref && external_ref->HasContainer()) {
   //             logger::trace("ExternalRef: {}", external_ref->GetFormID());
   //             M->LinkExternalContainer(event->baseObj, event->itemCount, event->newContainer,
   //                                      event->reference.get()->GetFormID());
   //         } else logger::trace("ExternalRef not found.");
   //     }
        
        if (event->oldContainer != player_refid && event->newContainer != player_refid)
            return RE::BSEventNotifyControl::kContinue;
        
        logger::trace("Container change event.");
        logger::trace("IsStage: {}", M->IsStage(event->baseObj));

        // to player inventory <-
        if (event->newContainer == player_refid) {
            logger::trace("Item entered player inventory.");
            if (M->IsExternalContainer(event->baseObj,event->oldContainer)) {
                M->UnLinkExternalContainer(event->baseObj,event->itemCount,event->oldContainer);
            }
            else if (!event->oldContainer) {
                if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME) && !M->IsStage(event->baseObj)) {
                    logger::info("Bought new item.");
                    M->Register(event->baseObj, event->itemCount, player_refid);
                } 
                else {
                    auto reference_ = event->reference;
                    logger::trace("Reference: {}", reference_.native_handle());
                    auto ref_ = Utilities::FunctionsSkyrim::TryToGetRefFromHandle(reference_);
                    if (!ref_) {
                        logger::info("Could not find reference");
                        ref_ = RE::TESForm::LookupByID<RE::TESObjectREFR>(picked_up_refid);
                        if (std::abs(picked_up_time - RE::Calendar::GetSingleton()->GetHoursPassed())>0.01f) {
                            logger::warn("Picked up time: {}, calendar time: {}", picked_up_time, RE::Calendar::GetSingleton()->GetHoursPassed());
                        }
                        if (!ref_) {
                            logger::error("Could not find reference with RefID {}", picked_up_refid);
                            return RE::BSEventNotifyControl::kContinue;
                        } 
                        else {
                            logger::trace("PickedUp: {}", ref_->GetName());
                            picked_up_refid = 0;
                            picked_up_time = 0;
                        }
                    }
                    else logger::trace("Reference found: {}", ref_->GetFormID());
                    M->HandlePickUp(event->baseObj,event->itemCount,ref_->GetFormID(),activate_eat);
                    activate_eat = false;
                }
            }
            // NPC: you dropped this...
            else if (auto ref_id__ = Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(event->reference)) {
                logger::trace("Reference handle refid: {}", ref_id__);
                if (auto npc_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->oldContainer)) {
                    M->HandlePickUp(event->baseObj, event->itemCount, ref_id__, false, npc_ref);
                } //else M->HandlePickUp(event->baseObj, event->itemCount, ref_id__, false);
            } 
            else if (!M->IsStage(event->baseObj)) {
                M->Register(event->baseObj, event->itemCount, player_refid);
            }
            else {
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported!");
				logger::error("Unsupported!");
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        // from player inventory ->
        if (event->oldContainer == 20 && M->IsStage(event->baseObj)) {
            // a fake container left player inventory
            logger::trace("Fake container left player inventory.");
            // drop event
            if (!event->newContainer) {
                logger::trace("Dropped.");
                M->setListenCrosshair(false);
                RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(event->reference.native_handle());
                if (ref) logger::trace("Dropped ref name: {}", ref->GetBaseObject()->GetName());
                if (!ref || ref->GetBaseObject()->GetFormID() != event->baseObj) {
                    // iterate through all objects in the cell................
                    logger::info("Iterating through all references in the cell.");
                    auto player_cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
                    auto cell_runtime_data = player_cell->GetRuntimeData();
                    for (auto& ref_ : cell_runtime_data.references) {
                        if (!ref_) continue;
                        if (ref_.get()->GetBaseObject()->GetFormID() == event->baseObj) {
                            ref = ref_.get();
                            break;
                        }
                    }
                } 
                if (ref) M->HandleDrop(event->baseObj,event->itemCount,ref);
                else if (event->baseObj == fake_equipped_id) {
                    M->HandleConsume(event->baseObj, event->itemCount);
                    fake_equipped_id = 0;
                }
                else logger::warn("Ref not found at HandleDrop! Hopefully due to consume.");
            }
            // Barter transfer
            else if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME) && Utilities::FunctionsSkyrim::IsCONT(event->newContainer)) {
                logger::info("Sold container.");
                M->HandleSell();
            }
            // container transfer
            else if (RE::UI::GetSingleton()->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) {
                logger::trace("Container menu is open.");
                M->LinkExternalContainer(event->baseObj,event->itemCount,event->newContainer);
            }
            else {
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported behaviour. Please put back the item(s) you removed from your inventory.");
                logger::error("Unsupported. Please put back the item(s) you removed from your inventory.");
            }

            M->setListenCrosshair(true);
        }



        return RE::BSEventNotifyControl::kContinue;
    }

};


void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Start
        auto sources = std::vector<Source>();
        M = Manager::GetSingleton(sources);
        auto mgeff = RE::TESForm::LookupByID<RE::EffectSetting>(0x0003eb42);
        if (mgeff) logger::trace("{}", mgeff->FORMTYPE);
    }
    if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        // Post-load
        if (!M) return;
        // EventSink
        auto* eventSink = OurEventSink::GetSingleton();
        auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
        //eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
        //eventSourceHolder->AddEventSink<RE::TESActivateEvent>(eventSink);
        //eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
         //eventSourceHolder->AddEventSink<RE::TESFurnitureEvent>(eventSink);
        //RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(OurEventSink::GetSingleton());
        // RE::BSInputDeviceManager::GetSingleton()->AddEventSink(eventSink);
        SKSE::GetCrosshairRefEventSource()->AddEventSink(eventSink);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    // InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}