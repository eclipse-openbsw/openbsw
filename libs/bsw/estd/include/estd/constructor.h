// Copyright 2024 Accenture.

/**
 * Contains estd::constructor.
 * \file
 * \ingroup estl_utils
 */
#ifndef GUARD_F79CF972_B58B_49CD_8234_3F21F3E27221
#define GUARD_F79CF972_B58B_49CD_8234_3F21F3E27221

#include <platform/estdint.h>

#include <new>
#include <utility>

namespace estd
{
namespace internal
{
template<class T>
class by_ref_wrapper
{
public:
    inline explicit by_ref_wrapper(T& obj) : _ref(obj) {}

    inline operator T&() const { return _ref; } // NOLINT(google-explicit-constructor)

private:
    T& _ref;
};

} // namespace internal

template<class T>
inline internal::by_ref_wrapper<T> by_ref(T& obj)
{
    return internal::by_ref_wrapper<T>(obj);
}

/**
 * A template class that provides a portable way to construct
 * objects from memory that is owned by a fixed-size container.
 * This class makes it possible to add objects that do not have
 * default constructors. It is used for all of the emplace methods.
 *
 * \tparam T The type of object to construct.
 *
 * \section constructor_vector_example Vector Example
 * \code{.cpp}
 * class MyClass
 * {
 *     public:
 *         MyClass(int i);
 * };
 *
 * void emplaceExample(estd::vector<MyClass>& v)
 * {
 *     for(int i = 0; i < 10; ++i)
 *     {
 *         // call the MyClass(int) constructor
 *         v.emplace_back().construct(i);
 *     }
 * }
 * \endcode
 *
 * \warning
 * Be careful using this constructor class. It does not support
 * any kind of move semantics, i.e. when copying an instance of
 * constructor you will end up having two instances pointing to
 * the same memory region. This is a compromise to have still
 * efficient runtime.
 */
template<class T>
class constructor
{
public:
    /**
     * Initializes this constructor object with the specified
     * memory address.
     */
    explicit constructor(void* const mem) : _memory(mem) {}

    /**
     * Initializes this constructor object with the specified
     * memory address.
     *
     * \param idx The offset in T's to the requested memory location.
     *
     */
    constructor(uint8_t* const mem, size_t const idx) : constructor(mem + (idx * sizeof(T))) {}

    /**
     * Constructs an object of type T, using the constructor appropriate for the provided argument
     * list. Uses placement new to construct the object into the memory passed in the constructor.
     *
     * \return A reference to the newly created object.
     */
    template<typename... Args>
    T& construct(Args&&... args)
    {
        return *new (_memory) T(::std::forward<Args>(args)...);
    }

    /**
     * Returns underlying pointer.
     *
     * This can be used to provide custom placement
     * new constructor calls if necessary.
     */
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator uint8_t*() { return reinterpret_cast<uint8_t*>(_memory); }

    /**
     * Returns underlying memory.
     * \deprecated
     */
    uint8_t* release() { return reinterpret_cast<uint8_t*>(_memory); }

    T* release_uninitialized_as() { return reinterpret_cast<T*>(_memory); }

    /**
     * Function to check if has memory to safely call construct.
     */
    bool has_memory() const { return _memory != nullptr; }

private:
    mutable void* _memory;
};

} // namespace estd

#endif // GUARD_F79CF972_B58B_49CD_8234_3F21F3E27221
