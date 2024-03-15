#include "Manager.h"

Manager* M = nullptr;
bool listen_crosshair_ref = true;
bool block_eventsinks = false;
bool block_droptake = false;


FormID fake_equipped_id; // set in equip event only when equipped and used in container event (consume)
RefID picked_up_refid;

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

public:
    static OurEventSink* GetSingleton() {
        static OurEventSink singleton;
        return &singleton;
    }


    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
        // if (block_eventsinks) return RE::BSEventNotifyControl::kContinue;
        // if (!event) return RE::BSEventNotifyControl::kContinue;
        // if (!event->actor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        
        // if (!M->IsItem(event->baseObject)) return RE::BSEventNotifyControl::kContinue;

        // fake_equipped_id = event->equipped ? event->baseObject : 0;
        // logger::trace("Fake container equipped: {}", fake_equipped_id);
        
        // if (event->equipped) {
	    //     logger::trace("Item {} was equipped. equipped: {}", event->baseObject);
        // } else {
        //     logger::trace("Item {} was unequipped. equipped: {}", event->baseObject);
        // }
        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) {
        
        if (block_eventsinks) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->objectActivated) return RE::BSEventNotifyControl::kContinue;
        if (!event->actionRef->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        if (event->objectActivated == RE::PlayerCharacter::GetSingleton()->GetGrabbedRef()) return RE::BSEventNotifyControl::kContinue;
        if (!M->listen_activate) return RE::BSEventNotifyControl::kContinue;
        if (!M->IsItem(event->objectActivated.get())) return RE::BSEventNotifyControl::kContinue;
        
        logger::trace("Item {} was activated.", event->objectActivated->GetFormID());
        picked_up_refid = event->objectActivated->GetFormID();

        if (!event->objectActivated->extraList.HasType<RE::ExtraTextDisplayData>()) {
            auto textDisplayData = RE::BSExtraData::Create<RE::ExtraTextDisplayData>();
            textDisplayData->SetName((std::string(event->objectActivated->GetDisplayFullName()) + " (Fresh)").c_str());
            event->objectActivated->extraList.Add(textDisplayData);
		}

        return RE::BSEventNotifyControl::kContinue;
    }

    // to disable ref activation and external container-fake container placement
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event,
                                          RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {

        // if (block_eventsinks) return RE::BSEventNotifyControl::kContinue;
        // if (!event->crosshairRef) return RE::BSEventNotifyControl::kContinue;
        // if (!listen_crosshair_ref) return RE::BSEventNotifyControl::kContinue;


        // if (!M->IsItem(event->crosshairRef.get())) {
            
        //     // if the fake items are not in it we need to place them (this happens upon load game)
        //     M->listen_container_change = false;
        //     listen_crosshair_ref = false; 
        //     M->HandleFakePlacement(event->crosshairRef.get());
        //     M->listen_container_change = true;
        //     listen_crosshair_ref = true;

        //     return RE::BSEventNotifyControl::kContinue;
        
        // }

        // if (event->crosshairRef->IsActivationBlocked() && !M->isUninstalled) return RE::BSEventNotifyControl::kContinue;

        
        // if (M->isUninstalled) {
        //     event->crosshairRef->SetActivationBlocked(0);
        // } else {
        //     event->crosshairRef->SetActivationBlocked(1);
        // }
        return RE::BSEventNotifyControl::kContinue;
    }
    
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
        
        if (block_eventsinks) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->opening) return RE::BSEventNotifyControl::kContinue;

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
        else if (menuname == RE::FavoritesMenu::MENU_NAME || menuname == RE::InventoryMenu::MENU_NAME) {
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
            // auto meanu_owner = Utilities::
            M->UpdateSpoilage(20);
            // add here the barter menu owner
            return RE::BSEventNotifyControl::kContinue;
        }
        else if (menuname == RE::ContainerMenu::MENU_NAME){
            M->UpdateSpoilage(20);
            // add here the container menu owner
            return RE::BSEventNotifyControl::kContinue;
        }
        
        return RE::BSEventNotifyControl::kContinue;
        
    }
    
    
    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
        
        if (block_eventsinks) return RE::BSEventNotifyControl::kContinue;
        if (!M->listen_container_change) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!listen_crosshair_ref) return RE::BSEventNotifyControl::kContinue;
        if (!event->itemCount) return RE::BSEventNotifyControl::kContinue;
        if (event->oldContainer != 20 && event->newContainer != 20) return RE::BSEventNotifyControl::kContinue;

        // to player inventory <-
        if (event->newContainer == 20 && M->IsItem(event->baseObj)) {
            if (M->IsExternalContainer(event->oldContainer)) {
                M->UnLinkExternalContainer(event->baseObj,event->itemCount,event->oldContainer,20);
            }
            else if (!event->oldContainer) {
                auto reference_ = event->reference;
                logger::trace("Reference: {}", reference_.native_handle());
                auto ref_ = RE::TESObjectREFR::LookupByHandle(reference_.native_handle()).get();
                if (!ref_) {
                    logger::error("Could not find reference for handle {}", reference_.native_handle());
                    ref_ = reference_.get().get();
                    if (!ref_) {
                        logger::error("Could not find reference");
                        ref_ = RE::TESForm::LookupByID<RE::TESObjectREFR>(picked_up_refid);
                        if (!ref_) {
                            logger::error("Could not find reference with RefID {}", picked_up_refid);
                            return RE::BSEventNotifyControl::kContinue;
                        }
                        else logger::trace("PickedUp: {}", ref_->GetName());
                    }
                    else logger::trace("Reference found: {}", ref_->GetFormID());
                }
                else logger::trace("Reference found by handle: {}", ref_->GetFormID());
                M->HandlePickUp(event->baseObj,event->itemCount,ref_->GetFormID());
            }
            else {
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported behaviour. Please put back the container you removed from your inventory.");
                logger::error("Unsupported. Please put back the container you removed from your inventory.");
            }   
            return RE::BSEventNotifyControl::kContinue;
        }


        // from player inventory ->
        if (event->oldContainer == 20 && M->IsItem(event->baseObj)) {
            // a fake container left player inventory
            logger::trace("Fake container left player inventory.");
            // drop event
            if (!event->newContainer) {
                logger::trace("Dropped.");
                M->HandleDrop(event->baseObj);
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
                Utilities::MsgBoxesNotifs::InGame::CustomErrMsg("Unsupported behaviour. Please put back the container you removed from your inventory.");
                logger::error("Unsupported. Please put back the container you removed from your inventory.");
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
        // eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESActivateEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
        // eventSourceHolder->AddEventSink<RE::TESFurnitureEvent>(eventSink);
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(OurEventSink::GetSingleton());
        // RE::BSInputDeviceManager::GetSingleton()->AddEventSink(eventSink);
        // SKSE::GetCrosshairRefEventSource()->AddEventSink(eventSink);
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