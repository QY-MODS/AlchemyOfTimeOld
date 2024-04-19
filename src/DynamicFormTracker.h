#include "Utils.h"

class DynamicFormTracker {
    
    std::map<std::pair<FormID, std::string>, std::set<FormID>> forms; // Create populates this
    std::map<FormID, uint32_t> customIDforms; // Create populates this

    std::set<FormID> active_forms; // Fetch populates this
    std::set<FormID> deleted_forms;


    std::mutex mutex;


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

    template <typename T>
    const FormID Create(T* baseForm, const RE::FormID setFormID = 0) {

        //std::lock_guard<std::mutex> lock(mutex);

        if (!baseForm) {
            logger::error("Real form is null for baseForm.");
            return 0;
        }

        const auto base_formid = baseForm->GetFormID();
        const auto base_editorid = clib_util::editorID::get_editorID(baseForm);

        if (base_editorid.empty()) {
			logger::error("Failed to get editorID for baseForm.");
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

        if (forms[{base_formid, base_editorid}].contains(setFormID)) {
        	logger::warn("Form with ID {:x} already exist for baseid {} and editorid {}.", setFormID, base_formid, base_editorid);
            ReviveDynamicForm(new_form, baseForm, 0);
        } else ReviveDynamicForm(new_form, baseForm, setFormID);

        const auto new_formid = new_form->GetFormID();

        logger::trace("Created form with type: {}, Base ID: {:x}, Ref ID: {:x}, Name: {}",

                      RE::FormTypeToString(new_form->GetFormType()), new_form->GetFormID(), new_form->GetFormID(),

                      new_form->GetName());

        if (!forms[{base_formid, base_editorid}].insert(new_formid).second) {
            logger::error("Failed to insert new form into forms.");
            _delete({base_formid, base_editorid}, new_formid);
            return 0;
        };

        return new_formid;
    }

    const std::set<FormID> GetFormSet(const FormID base_formid, std::string base_editorid = ""){
        if (base_editorid.empty()) {
			base_editorid = Utilities::FunctionsSkyrim::GetEditorID(base_formid);
            if (base_editorid.empty()) {
                return {};
			}
		}
        const std::pair<FormID, std::string> key = {base_formid, base_editorid};
        if (forms.contains(key)) return forms[key];
        return {};
    };

    const FormID GetByCustomID(const uint32_t custom_id, const FormID base_formid, std::string base_editorid) {
        const auto formset = GetFormSet(base_formid, base_editorid);
        if (formset.empty()) return 0;
        for (const auto _formid : formset) {
            if (customIDforms.contains(_formid) && customIDforms[_formid] == custom_id) return _formid;
		}
        return 0;
    }

    const bool IsActive(const FormID a_formid) {
        return active_forms.contains(a_formid);
	}

    const RE::TESForm* _yield(const FormID dynamic_formid, RE::TESForm* base_form) {
        if (auto newForm = RE::TESForm::LookupByID(dynamic_formid)) {
            if (std::strlen(newForm->GetName()) == 0) {
                ReviveDynamicForm(newForm, base_form, 0);
			}
            active_forms.insert(dynamic_formid);
			return newForm;
		}
		return nullptr;
	}

    void _delete(const std::pair<FormID, std::string> base, const FormID dynamic_formid) {
        if (!forms.contains(base)) return;

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
        customIDforms.erase(dynamic_formid);
        active_forms.erase(dynamic_formid);
        deleted_forms.insert(dynamic_formid);
    }

public:
    static DynamicFormTracker* GetSingleton() {
        static DynamicFormTracker singleton;
        return &singleton;
    }

    void Delete(const FormID dynamic_formid) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& [base, formset] : forms) {
			if (formset.contains(dynamic_formid)) {
				_delete(base, dynamic_formid);
			}
		}
	}


    template <typename T>
    const FormID Fetch(const FormID baseFormID, const std::string baseEditorID, const std::optional<uint32_t> customID) {

        auto* base_form = Utilities::FunctionsSkyrim::GetFormByID<T>(baseFormID, baseEditorID);
        
        if (!base_form) {
			logger::error("Failed to get base form.");
			return 0;
		}

        if (customID.has_value()) {
            const auto new_formid = GetByCustomID(customID.value(), baseFormID, baseEditorID);
            if (const auto dyn_form = _yield(new_formid, base_form)) return dyn_form->GetFormID();
        }
        else if (const auto formset = GetFormSet(baseFormID, baseEditorID); !formset.empty()) {
		    for (const auto _formid : formset) {
                if (IsActive(_formid)) continue;
                if (const auto dyn_form = _yield(_formid, base_form)) return dyn_form->GetFormID();
                //else if (!Utilities::FunctionsSkyrim::GetFormByID(_formid)) Delete({baseFormID, baseEditorID}, _formid);
		    }
        }


        if (const auto dyn_form = _yield(Create<T>(base_form), base_form)) {
            const auto new_formid = dyn_form->GetFormID();
            if (customID.has_value()) customIDforms[new_formid] = customID.value();
            return new_formid;
        }

        return 0;
    
    }
};

DynamicFormTracker* DFT = nullptr;
