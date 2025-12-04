#include <ontoflow/command/CommandStack.hpp>
#include <ontoflow/core/Logger.hpp>

namespace of::cmd {

CommandStack::CommandStack(domain::Registry& reg, domain::GeometrySystem& geom) 
    : m_Registry(reg), m_Geometry(geom) {}

void CommandStack::Push(std::unique_ptr<ICommand> cmd) {
    // If we are not at the tip, discard redo history
    if (m_Index < m_Stack.size()) {
        m_Stack.resize(m_Index);
    }

    cmd->Execute(m_Registry, m_Geometry);
    m_Stack.push_back(std::move(cmd));
    m_Index++;
}

void CommandStack::Undo() {
    if (m_Index > 0) {
        m_Stack[--m_Index]->Undo(m_Registry, m_Geometry);
    }
}

void CommandStack::Redo() {
    if (m_Index < m_Stack.size()) {
        m_Stack[m_Index]->Execute(m_Registry, m_Geometry);
        m_Index++;
    }
}

size_t CommandStack::GetCurrentIndex() const { return m_Index; }

void CommandStack::UndoTo(size_t targetIndex) {
    LOG(Info) << "UndoTo: target=" << targetIndex << ", current=" << m_Index;
    while (m_Index > targetIndex) {
        Undo();
    }
}

} // namespace of::cmd
