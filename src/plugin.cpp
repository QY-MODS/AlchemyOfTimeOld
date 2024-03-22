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
        
         if (!M->IsFake(event->baseObject)) return RE::BSEventNotifyControl::kContinue;

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
        
        if (!M->IsItem(event->objectActivated.get())) return RE::BSEventNotifyControl::kContinue;
        
        if (!event->actionRef->IsPlayerRef()) {
            logger::trace("Object activated: {} by {}", event->objectActivated->GetName(), event->actionRef->GetName());
            if (M->RefIsRegistered(event->objectActivated.get()->GetFormID())) M->SwapWithStage(event->objectActivated.get());
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
        logger::trace("Picked up: {} at time {}", picked_up_refid, picked_up_time);

        M->SwapWithStage(event->objectActivated.get());
        return RE::BSEventNotifyControl::kContinue;
    }

    // to disable ref activation and external container-fake container placement
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event,
                                          RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {

        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->crosshairRef) return RE::BSEventNotifyControl::kContinue;
        if (!M->getListenCrosshair()) return RE::BSEventNotifyControl::kContinue;
<<<<<<< Updated upstream
        

        // buraya external cont muhabbeti gelcek
        if (M->IsExternalContainer(event->crosshairRef.get())) M->UpdateSpoilage(event->crosshairRef.get());

        if (!M->IsItem(event->crosshairRef.get())) return RE::BSEventNotifyControl::kContinue;
        
=======

        if (M->IsExternalContainer(event->crosshairRef.get())) M->UpdateSpoilage(event->crosshairRef.get());

        if (!M->IsItem(event->crosshairRef.get())) return RE::BSEventNotifyControl::kContinue;


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

>>>>>>> Stashed changes
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
            !ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) &&
            !ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) return RE::BSEventNotifyControl::kContinue;

        auto menuname = event->menuName.c_str();
        // return if menu is not favorite menu, container menu, barter menu or inventory menu
        if (menuname == RE::FavoritesMenu::MENU_NAME) {
            logger::trace("Favorites menu is open.");
            if (M->UpdateSpoilage(20)) Utilities::FunctionsSkyrim::RefreshMenu(menuname);
            logger::trace("Spoilage updated.");
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::InventoryMenu::MENU_NAME) {
            logger::trace("Inventory menu is open.");
            if (M->UpdateSpoilage(20)) Utilities::FunctionsSkyrim::RefreshMenu(menuname);
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
    
    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
        
        logger::trace("ListenContainerChange: {}",
                      M->getListenContainerChange());
        if (!M->getListenContainerChange()) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->itemCount) return RE::BSEventNotifyControl::kContinue;
        if (!event->baseObj) return RE::BSEventNotifyControl::kContinue;
        if (!M->IsItem(event->baseObj)) return RE::BSEventNotifyControl::kContinue;
<<<<<<< Updated upstream
=======

        if (event->oldContainer != player_refid && event->newContainer != player_refid && event->reference &&
            M->RefIsRegistered(event->reference.native_handle()) && event->newContainer) {
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
>>>>>>> Stashed changes
        
        if (event->oldContainer != 20 && event->newContainer != 20) return RE::BSEventNotifyControl::kContinue;
        
        logger::trace("Container change event.");
        logger::trace("IsFake: {}", M->IsFake(event->baseObj));

        // to player inventory <-
        if (event->newContainer == 20) {
            logger::trace("Item entered player inventory.");
            if (M->IsExternalContainer(event->baseObj,event->oldContainer)) {
                M->UnLinkExternalContainer(event->baseObj,event->itemCount,event->oldContainer);
            }
            else if (!event->oldContainer) {
<<<<<<< Updated upstream
                auto reference_ = event->reference;
                logger::trace("Reference: {}", reference_.native_handle());
                auto ref_ = RE::TESObjectREFR::LookupByHandle(reference_.native_handle()).get();
                if (!ref_) {
                    logger::info("Could not find reference for handle {}", reference_.native_handle());
                    ref_ = reference_.get().get();
=======
                if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME)) {
                    if (M->IsStage(event->baseObj)) {
                        if (auto vendor_chest = Utilities::FunctionsSkyrim::GetVendorChestFromMenu()) {
                            M->UnLinkExternalContainer(event->baseObj, event->itemCount, vendor_chest->GetFormID());
						}
						else {
							logger::error("Could not get vendor chest");
							Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Could not get vendor chest.");
                        }
					}
					else M->Register(event->baseObj, event->itemCount, player_refid);
                }
                else {
                    auto reference_ = event->reference;
                    logger::trace("Reference: {}", reference_.native_handle());
                    auto ref_ = Utilities::FunctionsSkyrim::TryToGetRefFromHandle(reference_);
>>>>>>> Stashed changes
                    if (!ref_) {
                        logger::info("Could not find reference");
                        ref_ = RE::TESForm::LookupByID<RE::TESObjectREFR>(picked_up_refid);
                        if (std::abs(picked_up_time - RE::Calendar::GetSingleton()->GetHoursPassed())>0.01f) {
                            logger::warn("Picked up time: {}, calendar time: {}", picked_up_time, RE::Calendar::GetSingleton()->GetHoursPassed());
                        }
                        if (!ref_) {
                            logger::error("Could not find reference with RefID {}", picked_up_refid);
                            return RE::BSEventNotifyControl::kContinue;
                        } else {
                            logger::trace("PickedUp: {}", ref_->GetName());
                            picked_up_refid = 0;
                            picked_up_time = 0;
                        }
                    }
                    else logger::trace("Reference found: {}", ref_->GetFormID());
                }
<<<<<<< Updated upstream
                else logger::trace("Reference found by handle: {}", ref_->GetFormID());
                M->HandlePickUp(event->baseObj,event->itemCount,ref_->GetFormID(),activate_eat);
                activate_eat = false;
            }
            else {
                // NPC: you dropped this...
                auto reference_ = event->reference;
                if (!reference_) {
                    Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported behaviour.");
                    logger::error("Unsupported!");
                } else if (!reference_.native_handle()) {
                    Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported behaviour.");
                    logger::error("Unsupported!");
                } else {
                    logger::trace("Reference handle: {}", reference_.native_handle());
                    if (auto npc_ref = RE::TESObjectREFR::LookupByID<RE::TESObjectREFR>(event->oldContainer)) {
                        M->HandlePickUp(event->baseObj, event->itemCount, reference_.native_handle(), false, npc_ref);
                    } else M->HandlePickUp(event->baseObj, event->itemCount, reference_.native_handle(), false);
                }
            }   
=======
            }
            else if (!M->IsStage(event->baseObj)) {
                M->Register(event->baseObj, event->itemCount, player_refid);
                //Utilities::FunctionsSkyrim::RefreshMenu(RE::ContainerMenu::MENU_NAME);
            }
            // NPC: you dropped this...
            else if (auto ref_id__ = Utilities::FunctionsSkyrim::TryToGetRefIDFromHandle(event->reference)) {
                logger::trace("Reference handle refid: {}", ref_id__);
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

>>>>>>> Stashed changes
            return RE::BSEventNotifyControl::kContinue;
        }

        // from player inventory ->
        if (event->oldContainer == 20 && M->IsFake(event->baseObj)) {
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
                } else if (RE::UI::GetSingleton()->IsMenuOpen(RE::BarterMenu::MENU_NAME)){
                    if (auto vendor_chest = Utilities::FunctionsSkyrim::GetVendorChestFromMenu()){
                        M->LinkExternalContainer(event->baseObj, event->itemCount, event->newContainer);
                    } else {
                        logger::error("Could not get vendor chest");
						Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Could not get vendor chest.");
					
                    }
                }
                else logger::warn("Ref not found at HandleDrop! Hopefully due to consume.");
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
        eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESActivateEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
         //eventSourceHolder->AddEventSink<RE::TESFurnitureEvent>(eventSink);
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