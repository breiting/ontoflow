#pragma once

#include <string>

namespace of::ui {

/**
 * @class StatusBar
 * @brief Displays transient notifications and status messages at the bottom of the UI.
 */
class StatusBar {
   public:
    void ShowMessage(const std::string& message, float duration = 3.0f);
    void Draw(float dt);

   private:
    std::string m_Message;
    float m_Timer = 0.0f;
};

}  // namespace of::ui
