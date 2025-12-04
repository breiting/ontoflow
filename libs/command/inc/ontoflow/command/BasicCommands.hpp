#pragma once
#include <ontoflow/command/ICommand.hpp>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>

namespace of::cmd {

/**
 * \brief A generic command to add a specific component to an entity.
 *
 * This command handles the addition of a component and its subsequent removal
 * during undo operations. It's designed to be generic for any component type `T`.
 *
 * Note: The command assumes the `Entity` ID `e` is already managed or will be
 * managed by the calling context (e.g., a tool creates an entity ID, then uses
 * this command to add components to it).
 *
 * \tparam T The type of the component to be added.
 */
template <typename T>
class CreateComponentCommand : public ICommand {
   public:
    /**
     * \brief Constructs a CreateComponentCommand.
     * \param e The entity ID to which the component will be added.
     * \param comp The component instance to add.
     * \param newEntity If true, indicates this command conceptually created a new entity
     *                  (though the entity ID might have been pre-allocated).
     */
    CreateComponentCommand(domain::Entity e, T comp, bool newEntity = false)
        : m_Entity(e), m_Component(comp), m_CreatedEntity(newEntity) {
    }

    /**
     * \brief Executes the command, adding the component to the specified entity.
     * \param registry The central ECS registry.
     * \param geom Not used in this command.
     */
    void Execute(domain::Registry& registry, domain::GeometrySystem& /*geom*/) override {
        // We just forcefully add the component, assuming m_Entity is a valid ID managed by the caller/registry logic.
        registry.AddComponent<T>(m_Entity, m_Component);
    }

    /**
     * \brief Undoes the command, removing the component from the specified entity.
     * \param registry The central ECS registry.
     * \param geom Not used in this command.
     */
    void Undo(domain::Registry& registry, domain::GeometrySystem& /*geom*/) override {
        LOG(Info) << "Undo CreateComponent: Removing component from Entity " << m_Entity;
        registry.RemoveComponent<T>(m_Entity);
        // Note: If m_CreatedEntity is true, we leave the "shell" entity in registry.
        // A proper system would mark it dead or use a CreateEntityCommand separately.
    }

    /**
     * \brief Returns a human-readable name for the command.
     * \return "Create Component"
     */
    std::string GetName() const override {
        return "Create Component";
    }

   private:
    domain::Entity m_Entity;  ///< The entity ID to which the component is added.
    T m_Component;            ///< The component data.
    bool m_CreatedEntity;     ///< Flag indicating if this command created the entity (for tracking).
};

}  // namespace of::cmd
