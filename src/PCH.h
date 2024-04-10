#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

using namespace std::literals;

const uint32_t player_refid = 20;

using FormID = RE::FormID;
using RefID = RE::FormID;
using Count = RE::TESObjectREFR::Count;

// rot scale "0100306D"

//https:// github.com/clayne/GTS_Plugin/blob/17986520ede68988d772dfcba1c8024e8d7b1023/src/utils/papyrusUtils.hpp#L19
//using VM = RE::BSScript::Internal::VirtualMachine;
//using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
//inline RE::VMHandle GetHandle(RE::TESForm* a_form) {
//    auto vm = VM::GetSingleton();
//    auto policy = vm->GetObjectHandlePolicy();
//    return policy->GetHandleForObject(a_form->GetFormType(), a_form);
//}
//inline ObjectPtr GetObjectPtr(RE::TESForm* a_form, const char* a_class, bool a_create) {
//    auto vm = VM::GetSingleton();
//    auto handle = GetHandle(a_form);
//
//    ObjectPtr object = nullptr;
//    bool found = vm->FindBoundObject(handle, a_class, object);
//    if (!found && a_create) {
//        vm->CreateObject2(a_class, object);
//        vm->BindObject(object, handle, false);
//    }
//
//    return object;
//}