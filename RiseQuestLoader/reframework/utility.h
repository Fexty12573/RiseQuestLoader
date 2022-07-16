#pragma once

#include "API.hpp"

#include <memory>
#include <string>

class SystemString;

template <class T> class SystemArray {
public:
    void* object_info;      // 0x0000
    uint32_t ref_count;     // 0x0008
    int32_t _0;             // 0x000C
    void* contained_type;   // 0x0010
    uint32_t _1;            // 0x0018
    uint32_t count;         // 0x001C
    T elements[1];          // 0x0020
};

template <class T> class REArray;


namespace utility {

void log(const std::string& msg);

template <class T>
REArray<T>* to_re_array(reframework::API::ManagedObject* obj) {
    return reinterpret_cast<REArray<T>*>(obj);
}

template <class T> const REArray<T>* to_re_array(const reframework::API::ManagedObject* obj) {
    return reinterpret_cast<const REArray<T>*>(obj);
}

template <class T>
reframework::API::ManagedObject* to_managed_object(REArray<T>* array) {
    return reinterpret_cast<reframework::API::ManagedObject*>(array);
}

template <class T> const reframework::API::ManagedObject* to_managed_object(const REArray<T>* array) {
    return reinterpret_cast<const reframework::API::ManagedObject*>(array);
}

template <class T = reframework::API::ManagedObject*, class... Args>
T call(reframework::API::ManagedObject* obj, std::string_view name, Args... args) {
    return obj->call<T>(name, reframework::API::get()->get_vm_context(), obj, args...);
}

template <class T = reframework::API::ManagedObject*, class... Args> T call(reframework::API::Method* method, Args... args) {
    return method->call<T>(reframework::API::get()->get_vm_context(), args...);
}

std::string narrow(std::wstring_view str);

std::wstring widen(std::string_view str);

template<class... Args> std::string str_call(reframework::API::ManagedObject* obj, std::string_view name, Args... args) {
    const SystemString* res = obj->call<SystemString*>(name, reframework::API::get()->get_vm_context(), obj, args...);
    return narrow(res->data);
}

template <class... Args> std::string str_call(reframework::API::Method* method, Args... args) {
    const SystemString* res = method->call<SystemString*>(reframework::API::get()->get_vm_context(), args...);
    return narrow(res->data);
}

template <class T = uint32_t> std::string get_enum_name(std::string_view name, T value) {
    const auto& api = reframework::API::get();
    const auto type = api->tdb()->find_type(name);
    const auto inst = type->create_instance();

    *inst->get_field<T>("value__") = value;
    return narrow(call<SystemString*>(inst, "ToString()")->data);
}

template <class T = uint32_t> std::string get_enum_name(reframework::API::TypeDefinition* type, T value) {
    const auto inst = type->create_instance();
    *inst->get_field<T>("value__") = value;

    return narrow(call<SystemString*>(inst, "ToString()")->data);
}

template <class T = uint32_t> std::string get_enum_name(reframework::API::ManagedObject* value) {
    return narrow(call<SystemString*>(value, "ToString()")->data);
}

reframework::API::ManagedObject* get_main_view();

reframework::API::ManagedObject* get_current_scene();

reframework::API::ManagedObject* get_primary_camera();

using Pattern = std::vector<std::int16_t>;

Pattern make_pattern(std::string_view pattern);

std::vector<void*> scanmem(const Pattern& pattern);

SystemString* create_managed_string(std::string_view string);
reframework::API::ManagedObject* create_managed_array(reframework::API::ManagedObject* runtime_type, size_t length);
reframework::API::ManagedObject* create_managed_array(std::string_view type, size_t length);
reframework::API::ManagedObject* create_managed_array(const reframework::API::TypeDefinition* type, size_t length);

} // namespace utility

class SystemString
{
public:
    void* object_info;  // 0x0000
    uint32_t ref_count; // 0x0008
    int16_t _0;         // 0x000C
    char _1[2];         // 0x000E
    int32_t size;       // 0x0010
    wchar_t data[256];  // 0x0014

    [[nodiscard]] std::string to_str() const { return utility::narrow(data); }
    [[nodiscard]] std::wstring to_wstr() const { return data; }
    [[nodiscard]] std::wstring_view to_view() const { return data; }
};

template <class T> class REArray : SystemArray<T> {
public:
    using iterator = T*;
    using const_iterator = const T*;

    [[nodiscard]] size_t size() const { return SystemArray<T>::count; }
    const T& at(size_t idx) const { return SystemArray<T>::elements[idx]; }
    T& at(size_t idx) { return SystemArray<T>::elements[idx]; }

    T get_item(uint32_t idx) const { return utility::call<T>(utility::to_managed_object(this), "get_Item", idx); }
    void set_item(uint32_t idx, const T& value) { return utility::call(utility::to_managed_object(this), "set_Item", idx, value); }

    iterator begin() { return &SystemArray<T>::elements[0]; }
    const_iterator begin() const { return &SystemArray<T>::elements[0]; }
    iterator end() { return SystemArray<T>::elements + size(); }
    const_iterator end() const { return SystemArray<T>::elements + size(); }

    [[nodiscard]] bool contains(const T& value) { return utility::call<bool>(utility::to_managed_object(this), "Contains", value); }

    const T& operator[](size_t idx) const { return at(idx); }
    T& operator[](size_t idx) { return at(idx); }
};
