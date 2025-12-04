#pragma once
/**
 * @file UiIdAllocator.hpp
 * @brief Provides unique incremental integer IDs for UI objects such as
 *        nodes, pins, and links in the node editor.
 */

namespace of::ui {

/**
 * @class UiIdAllocator
 * @brief Simple monotonic integer ID generator.
 *
 * ImNodes requires unique integer IDs for UI nodes, pins, and links.
 * This allocator ensures a clean, explicit, collision-free generation of IDs.
 */
class UiIdAllocator {
   public:
    /**
     * @brief Allocate a new unique UI ID.
     * @return A freshly allocated integer ID.
     */
    int Allocate() {
        return ++m_Counter;
    }

    /**
     * @brief Reset the allocator back to zero.
     */
    void Reset() {
        m_Counter = 0;
    }

   private:
    int m_Counter = 0;
};

}  // namespace of::ui
