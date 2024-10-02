// Copyright 2024 Accenture.

/**
 * Contains estd::singleton.
 * \file
 * \ingroup estl_utils
 */
#ifndef GUARD_6773A554_F58F_408F_92C2_88CA85ADAA30
#define GUARD_6773A554_F58F_408F_92C2_88CA85ADAA30

#include "estd/assert.h"

namespace estd
{
/**
 * Base class for singletons.
 * \tparam  T   Any type that wants to expose the instance() interface.
 *
 * This class is designed to work as a generic base class for any class that wants to
 * provide a singleton interface. It'll also work for classes that do not have a
 * default constructor.
 *
 * \section singleton_usage Usage example
 * \code{.cpp}
 * class Origin
 * :   singleton<Origin>
 * {
 * public:
 *     Origin(int x, int y)
 *     :   singleton<Origin>(*this)
 *     {}
 *
 *     int getX() const;
 * } theOrigin(0, 0);
 *
 * int x = Origin::instance().getX();
 * \endcode
 *
 * \note
 * It is important, that a call to instance() will not create the instance of the class, it needs
 * to be created by the user before calling instance().
 */
template<class T>
class singleton
{
protected:
    /**
     * Constructs the instance of singleton.
     * \param   theInstance Reference to T, which will be returned when instance() is called.
     * \assert{No other instance of singleton<T> has been created}
     */
    explicit singleton(T& theInstance)
    {
        estd_assert(_self == nullptr); // make sure we only have one instance
        // theInstance does not have a greater lifetime as T inherits from singleton
        _self = &theInstance;
    }

    /**
     * Removes the internal reference to the instance passed in the constructor.
     */
    ~singleton() { _self = nullptr; }

public:
    /**
     * Returns a reference to the instance.
     * \assert{instantiated()}
     */
    static T& instance()
    {
        estd_assert(_self != nullptr);
        return *_self;
    }

    /**
     * Returns whether a instance has been attached to singleton<T> or not.
     */
    static bool instantiated() { return (_self != nullptr); }

private:
    static T* _self;
};

// No violation of one definition rule as this is a class template
template<class T>
T* singleton<T>::_self = nullptr;

} // namespace estd

#endif // GUARD_6773A554_F58F_408F_92C2_88CA85ADAA30
