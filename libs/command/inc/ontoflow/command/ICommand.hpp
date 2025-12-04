#pragma once

#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <string>

namespace of::cmd {

/**
 * \brief The base interface for all commands in the CAD system.
 *
 * Commands encapsulate an action that can be performed (executed) and subsequently
 * undone. This is crucial for implementing undo/redo functionality.
 * Each command operates on the central `Registry` (ECS) and `GeometrySystem`.
 */
class ICommand {
   public:
    /// \brief Virtual destructor to ensure proper cleanup of derived command classes.
    virtual ~ICommand() = default;

    /**
     * \brief Executes the command, applying its changes to the system.
     * \param registry The central ECS registry.
     * \param geom The geometry system for advanced operations (e.g., OCCT calls).
     */
    virtual void Execute(domain::Registry& registry, domain::GeometrySystem& geom) = 0;

    /**
     * \brief Reverts the changes made by the command.
     * \param registry The central ECS registry.
     * \param geom The geometry system.
     */
    virtual void Undo(domain::Registry& registry, domain::GeometrySystem& geom) = 0;

    /**
     * \brief Provides a human-readable name for the command.
     * This can be used for UI elements (e.g., in an undo history list).
     * \return A string representing the command's name.
     */
    virtual std::string GetName() const {
        return "Command";
    }
};

}  // namespace of::cmd
