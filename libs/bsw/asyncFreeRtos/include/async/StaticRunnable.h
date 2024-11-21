// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_DACDF931_AF72_4275_BB52_C84A579EB25F
#define GUARD_DACDF931_AF72_4275_BB52_C84A579EB25F

#include "estd/functional.h"
#include "estd/slice.h"

namespace async
{
/**
 * This is a template class that acts as collecion (linked list)
 * of Tasks to be added to the FreeRTOS system.
 *
 * \tparam T The underlying implementing execute function.
 */

template<class T>
class StaticRunnable
{
protected:
    ~StaticRunnable() = default;

public:
    StaticRunnable();

    static void run();

private:
    T* _next;

    static T* _first;
};

template<class T>
T* StaticRunnable<T>::_first = nullptr;

/**
 * Class constructor.
 * On instance construction the new instance is added into
 * the inked list by adjusting the pointers _first and _next.
 */
template<class T>
StaticRunnable<T>::StaticRunnable() : _next(_first)
{
    _first = static_cast<T*>(this);
}

template<class T>
void StaticRunnable<T>::run()
{
    while (_first != nullptr)
    {
        T* const current = _first;
        _first           = _first->_next;
        current->execute();
    }
}

} // namespace async

#endif // GUARD_DACDF931_AF72_4275_BB52_C84A579EB25F
