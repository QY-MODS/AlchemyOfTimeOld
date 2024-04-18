#include "Utils.h"

class DynamicFormTracker {
    
    std::map<std::pair<FormID, std::string>, std::set<FormID>> forms;


public:
    static DynamicFormTracker* GetSingleton() {
        static DynamicFormTracker singleton;
        return &singleton;
    }

    template <typename T>
    const RE::FormID Create(T* baseForm, const RE::FormID setFormID = 0) {
        logger::trace("CreateFakeContainer");

        if (!baseForm) {
            logger::error("Real form is null.");
            return 0;
        }

        const auto base_formid = baseForm->GetFormID();
        const auto base_editorid = clib_util::editorID::get_editorID(baseForm);
        if (!base_formid) {
            logger::error("Real form has no formid.");
            return 0;
        }

        if (base_editorid.empty()) {
            logger::error("Real form has no editorid.");
            return 0;
        }

        RE::TESForm* new_form = nullptr;

        auto factory = RE::IFormFactory::GetFormFactoryByType(baseForm->GetFormType());

        new_form = factory->Create();

        // new_form = baseForm->CreateDuplicateForm(true, (void*)new_form)->As<T>();

        if (!new_form) {
            logger::error("Failed to create new form.");
            return 0;
        }
        logger::trace("Original form id: {:x}", new_form->GetFormID());

        ReviveDynamicForm(new_form, baseForm, setFormID);

        const auto new_formid = new_form->GetFormID();

        logger::trace("Created form with type: {}, Base ID: {:x}, Ref ID: {:x}, Name: {}",

                      RE::FormTypeToString(new_form->GetFormType()), new_form->GetFormID(), new_form->GetFormID(),

                      new_form->GetName());

        forms[{base_formid, base_editorid}].insert(new_formid);

        return new_formid;
    }

    void ReviveDynamicForm(RE::TESForm* fake, RE::TESForm* base, const FormID setFormID) {
        using namespace Utilities::FunctionsSkyrim::DynamicForm;
        fake->Copy(base);
        auto weaponBaseForm = base->As<RE::TESObjectWEAP>();

        auto weaponNewForm = fake->As<RE::TESObjectWEAP>();

        auto bookBaseForm = base->As<RE::TESObjectBOOK>();

        auto bookNewForm = fake->As<RE::TESObjectBOOK>();

        auto ammoBaseForm = base->As<RE::TESAmmo>();

        auto ammoNewForm = fake->As<RE::TESAmmo>();

        if (weaponNewForm && weaponBaseForm) {
            weaponNewForm->firstPersonModelObject = weaponBaseForm->firstPersonModelObject;

            weaponNewForm->weaponData = weaponBaseForm->weaponData;

            weaponNewForm->criticalData = weaponBaseForm->criticalData;

            weaponNewForm->attackSound = weaponBaseForm->attackSound;

            weaponNewForm->attackSound2D = weaponBaseForm->attackSound2D;

            weaponNewForm->attackSound = weaponBaseForm->attackSound;

            weaponNewForm->attackFailSound = weaponBaseForm->attackFailSound;

            weaponNewForm->idleSound = weaponBaseForm->idleSound;

            weaponNewForm->equipSound = weaponBaseForm->equipSound;

            weaponNewForm->unequipSound = weaponBaseForm->unequipSound;

            weaponNewForm->soundLevel = weaponBaseForm->soundLevel;

            weaponNewForm->impactDataSet = weaponBaseForm->impactDataSet;

            weaponNewForm->templateWeapon = weaponBaseForm->templateWeapon;

            weaponNewForm->embeddedNode = weaponBaseForm->embeddedNode;

        } else if (bookBaseForm && bookNewForm) {
            bookNewForm->data.flags = bookBaseForm->data.flags;

            bookNewForm->data.teaches.spell = bookBaseForm->data.teaches.spell;

            bookNewForm->data.teaches.actorValueToAdvance = bookBaseForm->data.teaches.actorValueToAdvance;

            bookNewForm->data.type = bookBaseForm->data.type;

            bookNewForm->inventoryModel = bookBaseForm->inventoryModel;

            bookNewForm->itemCardDescription = bookBaseForm->itemCardDescription;

        } else if (ammoBaseForm && ammoNewForm) {
            ammoNewForm->GetRuntimeData().data.damage = ammoBaseForm->GetRuntimeData().data.damage;

            ammoNewForm->GetRuntimeData().data.flags = ammoBaseForm->GetRuntimeData().data.flags;

            ammoNewForm->GetRuntimeData().data.projectile = ammoBaseForm->GetRuntimeData().data.projectile;
        }
        /*else {
            new_form->Copy(baseForm);
        }*/

        copyComponent<RE::TESDescription>(base, fake);

        copyComponent<RE::BGSKeywordForm>(base, fake);

        copyComponent<RE::BGSPickupPutdownSounds>(base, fake);

        copyComponent<RE::TESModelTextureSwap>(base, fake);

        copyComponent<RE::TESModel>(base, fake);

        copyComponent<RE::BGSMessageIcon>(base, fake);

        copyComponent<RE::TESIcon>(base, fake);

        copyComponent<RE::TESFullName>(base, fake);

        copyComponent<RE::TESValueForm>(base, fake);

        copyComponent<RE::TESWeightForm>(base, fake);

        copyComponent<RE::BGSDestructibleObjectForm>(base, fake);

        copyComponent<RE::TESEnchantableForm>(base, fake);

        copyComponent<RE::BGSBlockBashData>(base, fake);

        copyComponent<RE::BGSEquipType>(base, fake);

        copyComponent<RE::TESAttackDamageForm>(base, fake);

        copyComponent<RE::TESBipedModelForm>(base, fake);

        if (setFormID != 0) fake->SetFormID(setFormID, false);
    }

    void Delete(const std::pair<FormID, std::string> base, const FormID dynamic_formid) {
        if (!Utilities::FunctionsSkyrim::DynamicForm::IsDynamicFormID(dynamic_formid)) return;
        if (auto newForm = RE::TESForm::LookupByID(dynamic_formid)) {
            if (auto* virtualMachine = RE::BSScript::Internal::VirtualMachine::GetSingleton()) {
                auto* handlePolicy = virtualMachine->GetObjectHandlePolicy();
                auto* bindPolicy = virtualMachine->GetObjectBindPolicy();

                if (handlePolicy && bindPolicy) {
                    auto newHandler = handlePolicy->GetHandleForObject(newForm->GetFormType(), newForm);

                    if (newHandler != handlePolicy->EmptyHandle()) {
                        auto* vm_scripts_hashmap = &virtualMachine->attachedScripts;
                        auto newHandlerScripts_it = vm_scripts_hashmap->find(newHandler);

                        if (newHandlerScripts_it != vm_scripts_hashmap->end()) {
                            vm_scripts_hashmap->clear();
                        }
                    }
                }
            }
            delete newForm;
        }
        forms[base].erase(dynamic_formid);
    }
};

DynamicFormTracker* DFT = nullptr;
