#include "Manager.h"

Manager* M = nullptr;

class OurEventSink : public RE::BSTEventSink<RE::TESEquipEvent>,
                     public RE::BSTEventSink<RE::TESActivateEvent>,
                     public RE::BSTEventSink<SKSE::CrosshairRefEvent>,
                     public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                     public RE::BSTEventSink<RE::TESFurnitureEvent>,
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
    bool furniture_entered = false;

    RE::NiPointer<RE::TESObjectREFR> furniture = nullptr;

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
        if (event->objectActivated->IsActivationBlocked()) return RE::BSEventNotifyControl::kContinue;

        if (!Settings::IsItem(event->objectActivated.get())) return RE::BSEventNotifyControl::kContinue;
        
        if (!event->actionRef->IsPlayerRef()) {
            logger::trace("Object activated: {} by {}", event->objectActivated->GetName(), event->actionRef->GetName());
            if (M->RefIsRegistered(event->objectActivated.get()->GetFormID()))
                M->SwapWithStage(event->objectActivated.get());
            return RE::BSEventNotifyControl::kContinue;
        }
        
        if (M->getPO3UoTInstalled()) {
            if (auto base = event->objectActivated->GetBaseObject()) {
                RE::BSString str;
                base->GetActivateText(RE::PlayerCharacter::GetSingleton(), str);
                if (Utilities::Functions::includesWord(str.c_str(), {"Eat"})) activate_eat = true;
            }
        }
            
        picked_up_time = RE::Calendar::GetSingleton()->GetHoursPassed();
        picked_up_refid = event->objectActivated->GetFormID();
        logger::trace("Picked up: {} at time {}, count: {}", picked_up_refid, picked_up_time, event->objectActivated->extraList.GetCount());

        M->SwapWithStage(event->objectActivated.get());
        return RE::BSEventNotifyControl::kContinue;
    }

    // to disable ref activation and external container-fake container placement
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event,
                                          RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {

        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->crosshairRef) return RE::BSEventNotifyControl::kContinue;
        if (!M->getListenCrosshair()) return RE::BSEventNotifyControl::kContinue;

        if (M->IsExternalContainer(event->crosshairRef.get())) M->UpdateSpoilage(event->crosshairRef.get());

        if (!Settings::IsItem(event->crosshairRef.get())) return RE::BSEventNotifyControl::kContinue;


        if (event->crosshairRef->extraList.GetOwner() && !event->crosshairRef->extraList.GetOwner()->IsPlayer()){
            logger::trace("Not player owned.");
            return RE::BSEventNotifyControl::kContinue;
        }

        if (event->crosshairRef->extraList.HasType(RE::ExtraDataType::kStartingPosition)) {
            logger::trace("has Starting position.");
            auto starting_pos = event->crosshairRef->extraList.GetByType<RE::ExtraStartingPosition>();
            if (starting_pos->location) {
                logger::trace("has location.");
                logger::trace("Location: {}", starting_pos->location->GetName());
                logger::trace("Location: {}", starting_pos->location->GetFullName());
                return RE::BSEventNotifyControl::kContinue;
            }
			/*logger::trace("Position: {}", starting_pos->startPosition.pos.x);
			logger::trace("Position: {}", starting_pos->startPosition.pos.y);
			logger::trace("Position: {}", starting_pos->startPosition.pos.z);*/
        }

        if (!M->RefIsRegistered(event->crosshairRef->GetFormID())) {
            logger::trace("Item not registered.");
            M->RegisterAndUpdate(event->crosshairRef.get());
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
            !ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) &&
            !ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) return RE::BSEventNotifyControl::kContinue;

        auto menuname = event->menuName.c_str();
        // return if menu is not favorite menu, container menu, barter menu or inventory menu
        if (menuname == RE::FavoritesMenu::MENU_NAME) {
            logger::trace("Favorites menu is open.");
            if (M->UpdateSpoilage(player_refid)) Utilities::FunctionsSkyrim::RefreshMenu(menuname);
            logger::trace("Spoilage updated.");
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::InventoryMenu::MENU_NAME) {
            logger::trace("Inventory menu is open.");
            if (M->UpdateSpoilage(player_refid)) Utilities::FunctionsSkyrim::RefreshMenu(menuname);
            logger::trace("Spoilage updated.");
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::BarterMenu::MENU_NAME){
            logger::trace("Barter menu is open.");
            if (M->UpdateSpoilage(player_refid) || 
                M->UpdateSpoilage(Utilities::FunctionsSkyrim::GetVendorChestFromMenu()->GetFormID())) {
                Utilities::FunctionsSkyrim::RefreshMenu(menuname);
            }
            return RE::BSEventNotifyControl::kContinue;
        } else if (menuname == RE::ContainerMenu::MENU_NAME) {
            logger::trace("Container menu is open.");
            if (auto container = Utilities::FunctionsSkyrim::GetContainerFromMenu()) {
                if (M->UpdateSpoilage(player_refid) || M->UpdateSpoilage(container->GetFormID())) {
                    Utilities::FunctionsSkyrim::RefreshMenu(menuname);
                }
            }
                
        }
        
        return RE::BSEventNotifyControl::kContinue;
        
    }
    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent* event,
                                          RE::BSTEventSource<RE::TESFurnitureEvent>*) {
        
        
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->actor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        if (furniture_entered && event->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter)
            return RE::BSEventNotifyControl::kContinue;
        if (!furniture_entered && event->type == RE::TESFurnitureEvent::FurnitureEventType::kExit)
            return RE::BSEventNotifyControl::kContinue;
        if (event->targetFurniture->GetBaseObject()->formType.underlying() != 40)
            return RE::BSEventNotifyControl::kContinue;


        auto bench = event->targetFurniture->GetBaseObject()->As<RE::TESFurniture>();
        if (!bench) return RE::BSEventNotifyControl::kContinue;
        auto bench_type = static_cast<std::uint8_t>(bench->workBenchData.benchType.get());
        logger::trace("Furniture event: {}", bench_type);

        //if (bench_type != 2 && bench_type != 3 && bench_type != 7) return RE::BSEventNotifyControl::kContinue;

        std::string qformtype;
        // POPULATE THIS
        if (bench_type == 1) {
            qformtype = "FOOD";
        } else return RE::BSEventNotifyControl::kContinue;

        if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter) {
            logger::trace("Furniture event: Enter {}", event->targetFurniture->GetName());
            furniture_entered = true;
            furniture = event->targetFurniture;
            M->HandleCraftingEnter(qformtype);
        } else if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kExit) {
            logger::trace("Furniture event: Exit {}", event->targetFurniture->GetName());
            if (event->targetFurniture == furniture) {
                M->HandleCraftingExit();
                furniture_entered = false;
                furniture = nullptr;
            }
        } else logger::info("Furniture event: Unknown");

        return RE::BSEventNotifyControl::kContinue;
    }

    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
        
        logger::trace("ListenContainerChange: {}",
                      M->getListenContainerChange());
        if (!M->getListenContainerChange()) return RE::BSEventNotifyControl::kContinue;
        if (furniture_entered) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->itemCount) return RE::BSEventNotifyControl::kContinue;
        if (!event->baseObj) return RE::BSEventNotifyControl::kContinue;
        if (!Settings::IsItem(event->baseObj)) return RE::BSEventNotifyControl::kContinue;

        if (event->oldContainer != player_refid && event->newContainer != player_refid && event->reference &&
            M->RefIsRegistered(Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(event->reference)) && event->newContainer) {
            auto external_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->newContainer);
            if (external_ref && external_ref->HasContainer()) {
                M->HandlePickUp(event->baseObj, event->itemCount, event->reference.native_handle(),false,external_ref);
            }
			else logger::trace("ExternalRef not found.");
        }
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
            if (!M->IsStage(event->baseObj)) {
                M->RegisterAndUpdate(event->baseObj, event->itemCount, player_refid);
                if (auto container_menu = RE::UI::GetSingleton()->GetMenu<RE::ContainerMenu>())
                    container_menu->GetRuntimeData().itemList->Update();
            }
            else if (M->IsExternalContainer(event->baseObj,event->oldContainer)) {
                M->UnLinkExternalContainer(event->baseObj,event->itemCount,event->oldContainer);
            }
            // stage deilse registered da deil
            else if (!event->oldContainer) {
                if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME) && M->IsStage(event->baseObj)) {
                    if (auto vendor_chest = Utilities::FunctionsSkyrim::GetVendorChestFromMenu()) {
                        M->UnLinkExternalContainer(event->baseObj, event->itemCount, vendor_chest->GetFormID());
					}
					else {
						logger::error("Could not get vendor chest");
						Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Could not get vendor chest.");
                    }
                }
                else { // demek ki world object
                    auto reference_ = event->reference;
                    logger::trace("Reference: {}", reference_.native_handle());
                    auto ref_id = Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(reference_);
                    if (!ref_id) {
                        logger::info("Could not find reference");
                        ref_id = picked_up_refid;
                        if (std::abs(picked_up_time - RE::Calendar::GetSingleton()->GetHoursPassed())>0.01f) {
                            logger::warn("Picked up time: {}, calendar time: {}", picked_up_time, RE::Calendar::GetSingleton()->GetHoursPassed());
                        }
                        if (!ref_id) {
                            logger::error("Could not find reference with RefID {}", picked_up_refid);
                            return RE::BSEventNotifyControl::kContinue;
                        }
                    }
                    logger::trace("Reference found: {}", ref_id);
                    picked_up_refid = 0;
                    picked_up_time = 0;
                    M->HandlePickUp(event->baseObj, event->itemCount, ref_id, activate_eat);
                }
            }
            // NPC: you dropped this...
            else if (auto ref_id__ = Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(event->reference)) {
                logger::info("NPC: you dropped this...Reference handle refid: {}", ref_id__);
                //if (auto npc_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->oldContainer)) {
                //    M->HandlePickUp(event->baseObj, event->itemCount, ref_id__, false, npc_ref);
                //} //else M->HandlePickUp(event->baseObj, event->itemCount, ref_id__, false);
            } 
            else {
				logger::error("Unsupported!");
#ifndef NDEBUG
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported!");
#endif  // !NDEBUG
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        // from player inventory ->
        if (event->oldContainer == player_refid && M->IsStage(event->baseObj)) {
            // a fake container left player inventory
            logger::trace("Fake container left player inventory.");
            // drop event
            if (!event->newContainer) {
                logger::trace("Dropped.");
                M->setListenCrosshair(false);
                auto reference_ = event->reference;
                logger::trace("Reference: {}", reference_.native_handle());
                RE::TESObjectREFR* ref = Utilities::FunctionsSkyrim::TryToGetRefFromHandle(reference_);
                if (ref) logger::trace("Dropped ref name: {}", ref->GetBaseObject()->GetName());
                if (!ref) {
                    // iterate through all objects in the cell................
                    logger::info("Iterating through all references in the cell.");
                    ref = Utilities::FunctionsSkyrim::TryToGetRefInCell(event->baseObj,event->itemCount);
                } 
                if (ref) {
                    M->HandleDrop(event->baseObj, event->itemCount, ref);
                }
                else if (event->baseObj == fake_equipped_id) {
                    M->HandleConsume(event->baseObj);
                    fake_equipped_id = 0;
                } 
                else if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME)){
                    if (auto vendor_chest = Utilities::FunctionsSkyrim::GetVendorChestFromMenu()){
                        M->LinkExternalContainer(event->baseObj, event->itemCount, event->newContainer);
                    } else {
                        logger::error("Could not get vendor chest");
						Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Could not get vendor chest.");
					
                    }
                }
                else logger::warn("Ref not found at HandleDrop! Hopefully due to consume.");
                M->setListenCrosshair(true);
            }
            // Barter transfer
            else if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME)) {
                logger::info("Sold container.");
                M->LinkExternalContainer(event->baseObj, event->itemCount, event->newContainer);
            }
            // container transfer
            else if (RE::UI::GetSingleton()->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) {
                logger::trace("Container menu is open.");
                M->LinkExternalContainer(event->baseObj,event->itemCount,event->newContainer);
            }
            else {
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Food got removed from player inventory due to unknown reason.");
                // remove from one of the instances
                M->HandleConsume(event->baseObj);
            }

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
        eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESActivateEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESFurnitureEvent>(eventSink);
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(OurEventSink::GetSingleton());
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