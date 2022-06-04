#pragma once

namespace utility {

class FunctionHook {
public:
    FunctionHook(void* target, void* hook);

    void create() const;
    void destroy() const;

    [[nodiscard]] bool is_valid() const { return m_original != nullptr; }

    template <class CallableT> CallableT get_original() const { return static_cast<CallableT>(m_original); }
    template <class Ret, class... Args> Ret call_original(Args... args) { return get_original<Ret (*)(Args...)>()(args...); }

private:
    void* m_target;
    void* m_original;
    void* m_hook;

    static bool s_minhook_initialized;
};

}

