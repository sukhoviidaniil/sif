/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-21
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_INPUT_H
#define RENDER_ENGINE_INPUT_H
#include <cstdint>
#include <string_view>

#include "sif/event/Event.h"

// clang-format off
/**
 * @brief Every key the engine knows, in enum order (X-macro).
 *
 * The parameter is called KEY, not the customary X, because X is also a
 * key: X(X) would expand to the macro's own name.
 *
 * One list drives the Key enum, key_name() and every backend's mapping
 * (see SFML_Event_Collector), so a key can never be added to one and
 * forgotten in another. The names are SFML 2.6's sf::Keyboard::Key
 * names on purpose, which lets the SFML backend map with this very list.
 *
 * The first fifteen are the keys the engine started with, in their
 * original order, so their numeric values do not change; everything
 * after them was added later.
 */
#define SIF_KEYS(KEY)                                                                                                  \
    KEY(W) KEY(A) KEY(S) KEY(D) KEY(Escape) KEY(Enter) KEY(Space) KEY(Left) KEY(Right) KEY(Up) KEY(Down) KEY(Num1)     \
    KEY(Num2) KEY(Num3) KEY(Backspace)                                                                                 \
    KEY(B) KEY(C) KEY(E) KEY(F) KEY(G) KEY(H) KEY(I) KEY(J) KEY(K) KEY(L) KEY(M) KEY(N) KEY(O) KEY(P) KEY(Q) KEY(R)    \
    KEY(T) KEY(U) KEY(V) KEY(X) KEY(Y) KEY(Z)                                                                          \
    KEY(Num0) KEY(Num4) KEY(Num5) KEY(Num6) KEY(Num7) KEY(Num8) KEY(Num9)                                              \
    KEY(F1) KEY(F2) KEY(F3) KEY(F4) KEY(F5) KEY(F6) KEY(F7) KEY(F8) KEY(F9) KEY(F10) KEY(F11) KEY(F12)                 \
    KEY(Tab) KEY(Home) KEY(End) KEY(PageUp) KEY(PageDown) KEY(Insert) KEY(Delete) KEY(Pause) KEY(Menu)                 \
    KEY(LShift) KEY(RShift) KEY(LControl) KEY(RControl) KEY(LAlt) KEY(RAlt) KEY(LSystem) KEY(RSystem)                  \
    KEY(Comma) KEY(Period) KEY(Slash) KEY(Backslash) KEY(Semicolon) KEY(Apostrophe) KEY(LBracket) KEY(RBracket)        \
    KEY(Grave) KEY(Equal) KEY(Hyphen)                                                                                  \
    KEY(Numpad0) KEY(Numpad1) KEY(Numpad2) KEY(Numpad3) KEY(Numpad4) KEY(Numpad5) KEY(Numpad6) KEY(Numpad7)            \
    KEY(Numpad8) KEY(Numpad9)                                                                                          \
    KEY(Add) KEY(Subtract) KEY(Multiply) KEY(Divide)
// clang-format on

namespace sif::event::input {
enum class Key : std::uint16_t {
    Unknown,
#define SIF_KEY_ENUMERATOR(name) name,
    SIF_KEYS(SIF_KEY_ENUMERATOR)
#undef SIF_KEY_ENUMERATOR
};

/**
 * @brief The key's name as written in the enum ("W", "F5", "PageUp").
 *
 * For key-binding menus, config files and logs; "Unknown" for Key::Unknown.
 */
[[nodiscard]] constexpr std::string_view key_name(const Key key) {
    switch (key) {
#define SIF_KEY_NAME(name) \
    case Key::name:        \
        return #name;
        SIF_KEYS(SIF_KEY_NAME)
#undef SIF_KEY_NAME
    case Key::Unknown:
    default:
        return "Unknown";
    }
}

struct KeyPressed {
    static constexpr EventMask mask = EventMask::Input;
    Key key;
    bool alt;
    bool control;
    bool shift;
    bool system;
};

/**
 * @brief A character the user typed.
 *
 * Separate from KeyPressed on purpose. A key is a *position* on the
 * keyboard; a character is what the OS produced from it after the
 * layout, the modifiers and any compose sequence. Deriving one from
 * the other in the engine would mean re-implementing the platform's
 * keyboard layout handling - and getting it wrong for every layout
 * that is not the developer's own.
 *
 * Backspace and Enter arrive as KeyPressed, not here: they are
 * editing commands rather than text.
 */
struct TextEntered {
    static constexpr EventMask mask = EventMask::Input;
    char32_t unicode = 0;

    /// @brief True for characters that make sense in a text field.
    [[nodiscard]] bool printable() const { return unicode >= 32 && unicode != 127; }
};

struct KeyReleased {
    static constexpr EventMask mask = EventMask::Input;
    Key key;
};
} // namespace sif::event::input

#endif // RENDER_ENGINE_INPUT_H