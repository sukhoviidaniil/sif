/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:       sukhovii.daniil@gmail.com
 * Created:       2026-01-15
 *
 * Disclaimer:
 *   This file is part of Render_Engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_CONCEPT_H
#define RENDER_ENGINE_CONCEPT_H
#include <algorithm>

namespace sif::intrnl {
    /**
     * @brief The generic root of the Concept/Model type-erasure idiom -
     * inheritance-based, as opposed to ErasedObject's cast-based approach.
     *
     * A different way to solve the same family of problem ErasedObject
     * solves, using virtual dispatch instead of a stored type tag. Concept
     * is the empty polymorphic root: on its own it declares no operations,
     * only the virtual destructor needed to delete a Model<T> through a
     * Concept* correctly. A concrete use is expected to derive its own
     * interface from Concept - adding the pure virtual operations it
     * actually needs - and pair it with its own Model<T>-shaped template
     * that implements those operations by forwarding to the wrapped `impl`.
     *
     * @par Model<T>
     * Wraps one value of any concrete type T behind that interface. Unlike
     * ErasedObject, getting the value back out is not the point - Model<T>
     * exists to be *called through*, via the virtual functions its own
     * concrete interface declares, never unwrapped and cast back to T. A
     * caller holding a `Concept*` never needs to know T at all, which is
     * exactly what makes this the useful half of the pattern: heterogeneous
     * types that all happen to support the same operations can be stored
     * and invoked uniformly, with no shared base type required of T itself.
     *
     * @note As shipped, Concept declares no operations of its own, so it is
     * a template for the idiom rather than a directly usable base - nothing
     * in sif currently derives from it. sif::event::EventConcept /
     * EventInstance (Event.h) is the same idiom, independently reimplemented
     * with the operations the event system actually needs (mask(), among
     * others) rather than extending this shared root - worth knowing if this
     * file is ever revisited, since collapsing the two into one would remove
     * a real duplication.
     */

    struct Concept {
        virtual ~Concept() = default;
    };

    template<typename T>
    struct Model final : Concept {
        explicit Model(T v) : impl(std::move(v)) {}
        T impl;
    };
} // namespace sif::intrnl

#endif // RENDER_ENGINE_CONCEPT_H