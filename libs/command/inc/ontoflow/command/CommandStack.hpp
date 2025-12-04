#pragma once
#include <memory>
#include <ontoflow/command/ICommand.hpp>
#include <vector>

namespace of::cmd {

/**
 * \brief Manages a stack of commands, enabling undo and redo functionality.
 *
 * The CommandStack stores `ICommand` objects and provides an interface to
 * execute commands, undo them, redo them, and revert to a specific point
 * in the command history.
 */
class CommandStack {
   public:
    /**
     * \brief Constructs a CommandStack.
     * \param reg Reference to the central ECS registry.
     * \param geom Reference to the geometry system.
     */
    CommandStack(domain::Registry& reg, domain::GeometrySystem& geom);

    /**
     * \brief Pushes a new command onto the stack and executes it.
     *
     * If the current position in the history is not at the end, any
     * "redoable" commands are discarded.
     * \param cmd A unique pointer to the command to be executed and stored.
     */
    void Push(std::unique_ptr<ICommand> cmd);

    /**
     * \brief Undoes the last executed command on the stack.
     * Decrements the history index and calls `Undo()` on the command.
     */
    void Undo();

    /**
     * \brief Redoes the next command in the history.
     * Increments the history index and calls `Execute()` on the command.
     */
    void Redo();

    /**
     * \brief Returns the current index in the command history.
     * This index points to the slot where the next command would be added.
     * Useful for marking a "transaction start" point for tools.
     * \return The current history index.
     */
    size_t GetCurrentIndex() const;

    /**
     * \brief Undoes commands until a specific target index in the history is reached.
     * This is useful for cancelling multi-step tool operations.
     * \param targetIndex The index to undo to. Commands at and before this index will remain executed.
     */
    void UndoTo(size_t targetIndex);

   private:
    domain::Registry& m_Registry;        ///< Reference to the ECS registry.
    domain::GeometrySystem& m_Geometry;  ///< Reference to the geometry system.

    std::vector<std::unique_ptr<ICommand>> m_Stack;  ///< The actual stack of commands.
    size_t m_Index = 0;  ///< Current position in the command history (points to next command slot).
};

}  // namespace of::cmd
