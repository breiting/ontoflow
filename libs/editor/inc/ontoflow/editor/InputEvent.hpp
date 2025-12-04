#pragma once
#include <glm/vec2.hpp>
#include <ontoflow/domain/Types.hpp>  // For domain::vec2
#include <variant>

namespace of::editor {

/**
 * \brief Enumerates the types of input events.
 */
enum class InputEventType {
    Key,          ///< A keyboard key press or release event.
    MouseButton,  ///< A mouse button press or release event.
    MouseMove,    ///< A mouse movement event.
    Scroll        ///< A mouse scroll wheel event.
};

/**
 * \brief Enumerates mouse button identifiers.
 */
enum class MouseButton {
    Left,   ///< Left mouse button.
    Right,  ///< Right mouse button.
    Middle  ///< Middle mouse button.
};

/**
 * \brief Enumerates common key codes.
 */
enum class KeyCode {
    Unknown,  ///< An unknown or unhandled key code.
    Escape,   ///< The Escape key.
    Enter,    ///< The Enter/Return key.
    Space,    ///< The Space key.
    // Add more key codes as needed
};

/**
 * \brief Represents a keyboard key event.
 */
struct KeyEvent {
    KeyCode code = KeyCode::Unknown;  ///< The key code.
    bool pressed = false;             ///< True if the key was pressed, false if released.
    char text = 0;                    ///< ASCII character if a printable key was pressed.
    bool ctrl = false;                ///< True if Ctrl key was down.
    bool alt = false;                 ///< True if Alt key was down.
    bool shift = false;               ///< True if Shift key was down.
};

/**
 * \brief Represents a mouse button event.
 */
struct MouseButtonEvent {
    MouseButton button = MouseButton::Left;  ///< The mouse button involved.
    bool pressed = false;                    ///< True if the button was pressed, false if released.
    domain::vec2 position{0.0, 0.0};         ///< Mouse position when the event occurred.
};

/**
 * \brief Represents a mouse movement event.
 */
struct MouseMoveEvent {
    domain::vec2 position{0.0, 0.0};  ///< Current mouse position.
};

/**
 * \brief Represents a mouse scroll event.
 */
struct ScrollEvent {
    domain::vec2 offset{0.0, 0.0};  ///< Scroll offset (e.g., (0, 1) for scroll up).
};

/**
 * \brief A variant type holding any of the specific input event data structures.
 */
using InputEventData = std::variant<KeyEvent, MouseButtonEvent, MouseMoveEvent, ScrollEvent>;

/**
 * \brief General purpose input event structure.
 * Contains the event type and its associated data in a variant.
 */
struct InputEvent {
    InputEventType type;  ///< The type of input event.
    InputEventData data;  ///< The specific data for the event type.
};

// Convenience Helpers
/**
 * \brief Casts an InputEvent to a KeyEvent pointer if it is a keyboard event.
 * \param ev The InputEvent to cast.
 * \return A pointer to KeyEvent data if type is Key, otherwise nullptr.
 */
inline const KeyEvent* AsKey(const InputEvent& ev) {
    if (ev.type != InputEventType::Key)
        return nullptr;
    return std::get_if<KeyEvent>(&ev.data);
}

/**
 * \brief Casts an InputEvent to a MouseButtonEvent pointer if it is a mouse button event.
 * \param ev The InputEvent to cast.
 * \return A pointer to MouseButtonEvent data if type is MouseButton, otherwise nullptr.
 */
inline const MouseButtonEvent* AsMouseButton(const InputEvent& ev) {
    if (ev.type != InputEventType::MouseButton)
        return nullptr;
    return std::get_if<MouseButtonEvent>(&ev.data);
}

/**
 * \brief Casts an InputEvent to a MouseMoveEvent pointer if it is a mouse move event.
 * \param ev The InputEvent to cast.
 * \return A pointer to MouseMoveEvent data if type is MouseMove, otherwise nullptr.
 */
inline const MouseMoveEvent* AsMouseMove(const InputEvent& ev) {
    if (ev.type != InputEventType::MouseMove)
        return nullptr;
    return std::get_if<MouseMoveEvent>(&ev.data);
}

/**
 * \brief Casts an InputEvent to a ScrollEvent pointer if it is a scroll event.
 * \param ev The InputEvent to cast.
 * \return A pointer to ScrollEvent data if type is Scroll, otherwise nullptr.
 */
inline const ScrollEvent* AsScroll(const InputEvent& ev) {
    if (ev.type != InputEventType::Scroll)
        return nullptr;
    return std::get_if<ScrollEvent>(&ev.data);
}

}  // namespace of::editor
