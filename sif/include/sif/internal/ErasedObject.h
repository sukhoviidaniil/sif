/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:       sukhovii.daniil@gmail.com
 * Created:       2026-01-15
 *
 * Disclaimer:
 *   This file is part of Render_Engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ERASEDOBJECT_H
#define RENDER_ENGINE_ERASEDOBJECT_H

#include "TypeId.h"

namespace sif::intrnl {
/**
 * @brief A move-only container that erases its value's type at runtime,
 * checked, not assumed.
 *
 * The same problem std::any solves, solved by hand: store a value of any
 * type T behind one common, non-templated type, so heterogeneous values
 * can live in the same container or cross an interface that cannot itself
 * be templated. The comment on the private members puts it plainly -
 * "Implementation of unique_ptr, but for a dynamic type."
 *
 * Two things a bare `void*` cannot give you on its own, ErasedObject
 * provides:
 *
 * @par Correct destruction
 * A `void*` alone does not know how to destroy what it points to -
 * deleting it as anything other than its real type is undefined
 * behaviour. The constructor captures a type-correct deleter
 * (`destroy_impl<T>`) at the moment T is still known, so the erased
 * object is still destroyed correctly later, however far the type has
 * since been forgotten.
 *
 * @par A checked way back
 * Casting a `void*` back to some assumed type is exactly as unsafe as
 * the destruction problem above, just silent instead of a crash - the
 * wrong assumption compiles and reads garbage. get<T>() checks the
 * stored TypeId first and aborts immediately on a mismatch, so a wrong
 * type parameter fails at the call site, not as a stale value read
 * three functions later.
 *
 * @note The type check is a hand-rolled TypeId (the address of a
 * per-template-instantiation static local), not RTTI - cheaper than
 * typeid(), and available even if RTTI is disabled. The trade-off
 * against std::any is the failure mode: a wrong get<T>() aborts the
 * process outright rather than throwing something recoverable like
 * std::bad_any_cast, since a type mismatch here means the calling code
 * itself is wrong, not that it hit bad input worth handling gracefully.
 *
 * @warning Copy-disabled by design, move-only - it owns a heap
 * allocation, the same as the unique_ptr it stands in for.
 */
class ErasedObject {
public:
    ErasedObject(const ErasedObject&) = delete;
    ErasedObject& operator=(const ErasedObject&) = delete;

    ErasedObject();

    /**
     *
     * @tparam T Movement and the move constructor should be allowed.
     * @param value
     */
    template <typename T>
    explicit ErasedObject(T value);

    ErasedObject(ErasedObject&& other) noexcept;

    ErasedObject& operator=(ErasedObject&& other) noexcept;

    ~ErasedObject();

    template <typename T>
    T& get();

    template <typename T>
    const T& get() const;

    [[nodiscard]] TypeId type() const;

    [[nodiscard]] bool valid() const;

private:
    void reset();

    template <typename T>
    static void destroy_impl(void* ptr);

    // Implementation of unique_ptr, but for a dynamic type
    void* storage_;
    void (*destroy_)(void*);

    TypeId type_;
};
} // namespace sif::intrnl

#include "sif/internal/ErasedObject.inl"

#endif // RENDER_ENGINE_ERASEDOBJECT_H