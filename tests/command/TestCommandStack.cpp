#include <gtest/gtest.h>

#include <ontoflow/command/CommandStack.hpp>
#include <ontoflow/command/ICommand.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/IGeometryBackend.hpp>
#include <ontoflow/domain/Registry.hpp>

using namespace of::cmd;
using namespace of::domain;

// --- Mocks ---

class MockBackend : public IGeometryBackend {
   public:
    BackendShapeHandle CreateExtrudedBody(const Polygon&, double) override {
        return 0;
    }
    BackendShapeHandle CreateBox(double, double, double) override {
        return 0;
    }
    BackendShapeHandle CreateCylinder(double, double) override {
        return 0;
    }
    bool ExportShapeToSTEP(BackendShapeHandle, const std::string&) const override {
        return true;
    }
    bool ExportShapeToSTL(BackendShapeHandle, const std::string&, double) const override {
        return true;
    }
    Mesh GetMeshFromShape(BackendShapeHandle) override {
        return {};
    }
};

// A simple command for testing: increments a counter in Execute, decrements in Undo
class TestCommand : public ICommand {
   public:
    TestCommand(int& counter) : m_Counter(counter) {
    }

    void Execute(Registry&, GeometrySystem&) override {
        m_Counter++;
    }

    void Undo(Registry&, GeometrySystem&) override {
        m_Counter--;
    }

    std::string GetName() const override {
        return "TestCommand";
    }

   private:
    int& m_Counter;
};

class CommandStackTest : public ::testing::Test {
   protected:
    Registry reg;
    MockBackend backend;
    GeometrySystem geom{reg, backend};
    CommandStack stack{reg, geom};
};

// --- Tests ---

TEST_F(CommandStackTest, ExecuteIncrements) {
    int counter = 0;
    auto cmd = std::make_unique<TestCommand>(counter);

    stack.Push(std::move(cmd));

    EXPECT_EQ(counter, 1);
    EXPECT_EQ(stack.GetCurrentIndex(), 1);
}

TEST_F(CommandStackTest, UndoDecrements) {
    int counter = 0;
    stack.Push(std::make_unique<TestCommand>(counter));
    EXPECT_EQ(counter, 1);

    stack.Undo();
    EXPECT_EQ(counter, 0);
    EXPECT_EQ(stack.GetCurrentIndex(), 0);
}

TEST_F(CommandStackTest, RedoIncrementsAgain) {
    int counter = 0;
    stack.Push(std::make_unique<TestCommand>(counter));
    stack.Undo();
    EXPECT_EQ(counter, 0);

    stack.Redo();
    EXPECT_EQ(counter, 1);
    EXPECT_EQ(stack.GetCurrentIndex(), 1);
}

TEST_F(CommandStackTest, MultipleCommands) {
    int c1 = 0;
    int c2 = 0;

    stack.Push(std::make_unique<TestCommand>(c1));
    stack.Push(std::make_unique<TestCommand>(c2));

    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 1);
    EXPECT_EQ(stack.GetCurrentIndex(), 2);

    stack.Undo();  // Undo c2
    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 0);

    stack.Undo();  // Undo c1
    EXPECT_EQ(c1, 0);
    EXPECT_EQ(c2, 0);
}

TEST_F(CommandStackTest, PushClearsRedoStack) {
    int c1 = 0;
    int c2 = 0;

    stack.Push(std::make_unique<TestCommand>(c1));
    stack.Undo();  // Index is now 0

    // Push new command, should overwrite the redoable c1 command (conceptually)
    // Actually CommandStack keeps unique_ptrs.
    // If we push at index 0, the vector is resized to 0, deleting old c1 command.
    stack.Push(std::make_unique<TestCommand>(c2));

    EXPECT_EQ(c1, 0);  // Remains undone
    EXPECT_EQ(c2, 1);  // New command executed
    EXPECT_EQ(stack.GetCurrentIndex(), 1);

    // Try Redo -> Should do nothing because we are at tip
    stack.Redo();
    EXPECT_EQ(c1, 0);
    EXPECT_EQ(c2, 1);
}

TEST_F(CommandStackTest, UndoToTarget) {
    int counter = 0;
    // Push 3 commands
    stack.Push(std::make_unique<TestCommand>(counter));
    stack.Push(std::make_unique<TestCommand>(counter));
    stack.Push(std::make_unique<TestCommand>(counter));

    EXPECT_EQ(counter, 3);
    EXPECT_EQ(stack.GetCurrentIndex(), 3);

    // Undo until index 1 (so 1 command remains applied)
    stack.UndoTo(1);

    EXPECT_EQ(counter, 1);
    EXPECT_EQ(stack.GetCurrentIndex(), 1);
}
