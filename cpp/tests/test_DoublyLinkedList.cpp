#include "gtest/gtest.h"

#include "DoublyLinkedList.h"
#include "DLLTraverser.h"
#include "Node.h"

#include <memory>
#include <string>
#include <vector>

class DoublyLinkedListTest : public ::testing::Test {
protected:
    DoublyLinkedList list;
};

TEST_F(DoublyLinkedListTest, InitialStateIsEmpty) {
    std::vector<std::string> paths = list.get_all_paths();
    EXPECT_TRUE(paths.empty());
}

TEST_F(DoublyLinkedListTest, InsertFront) {
    auto node1 = std::make_shared<Node>("/path/1");
    auto node2 = std::make_shared<Node>("/path/2");
    
    list.insert_front(node1);
    list.insert_front(node2);
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], "/path/2");
    EXPECT_EQ(paths[1], "/path/1");
}

TEST_F(DoublyLinkedListTest, InsertBack) {
    auto node1 = std::make_shared<Node>("/path/1");
    auto node2 = std::make_shared<Node>("/path/2");
    
    list.insert_back(node1);
    list.insert_back(node2);
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], "/path/1");
    EXPECT_EQ(paths[1], "/path/2");
}

TEST_F(DoublyLinkedListTest, InsertBefore) {
    auto node1 = std::make_shared<Node>("/path/1");
    auto node2 = std::make_shared<Node>("/path/2");
    auto node3 = std::make_shared<Node>("/path/3");
    
    list.insert_back(node1);
    list.insert_back(node3);
    list.insert_before(node2, node3);
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 3);
    EXPECT_EQ(paths[0], "/path/1");
    EXPECT_EQ(paths[1], "/path/2");
    EXPECT_EQ(paths[2], "/path/3");
}

TEST_F(DoublyLinkedListTest, DeleteAt) {
    auto node1 = std::make_shared<Node>("/path/1");
    auto node2 = std::make_shared<Node>("/path/2");
    auto node3 = std::make_shared<Node>("/path/3");
    
    list.insert_back(node1);
    list.insert_back(node2);
    list.insert_back(node3);
    
    list.delete_at(node2);
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], "/path/1");
    EXPECT_EQ(paths[1], "/path/3");
}

TEST_F(DoublyLinkedListTest, DeleteDummyNodes) {
    // Attempt to delete dummy nodes should have no effect
    auto head = list.get_dummy_head();
    auto tail = list.get_dummy_tail();
    
    list.delete_at(head);
    list.delete_at(tail);
    
    // Add a node to verify list is still functional
    auto node = std::make_shared<Node>("/path/1");
    list.insert_back(node);
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 1);
    EXPECT_EQ(paths[0], "/path/1");
}

TEST_F(DoublyLinkedListTest, MixedOperations) {
    auto node1 = std::make_shared<Node>("/path/1");
    auto node2 = std::make_shared<Node>("/path/2");
    auto node3 = std::make_shared<Node>("/path/3");
    auto node4 = std::make_shared<Node>("/path/4");
    
    list.insert_back(node1);    // 1
    list.insert_front(node2);   // 2->1
    list.insert_back(node3);    // 2->1->3
    list.insert_before(node4, node3); // 2->1->4->3
    list.delete_at(node1);     // 2->4->3
    
    std::vector<std::string> paths = list.get_all_paths();
    ASSERT_EQ(paths.size(), 3);
    EXPECT_EQ(paths[0], "/path/2");
    EXPECT_EQ(paths[1], "/path/4");
    EXPECT_EQ(paths[2], "/path/3");
}

class DLLTraverserTest : public ::testing::Test {
protected:
    DoublyLinkedList list;
    std::shared_ptr<Node> node1;
    std::shared_ptr<Node> node2;
    std::shared_ptr<Node> node3;
    DLLTraverser* traverser;

    void SetUp() override {
        node1 = std::make_shared<Node>("/path/1");
        node2 = std::make_shared<Node>("/path/2");
        node3 = std::make_shared<Node>("/path/3");
        
        list.insert_back(node1);
        list.insert_back(node2);
        list.insert_back(node3);
        
        traverser = new DLLTraverser(list);
    }

    void TearDown() override {
        delete traverser;
    }
};

TEST_F(DLLTraverserTest, InitialPosition) {
    EXPECT_EQ(traverser->current(), "/path/1");
}

TEST_F(DLLTraverserTest, MoveNext) {
    traverser->move_next();
    EXPECT_EQ(traverser->current(), "/path/2");
    traverser->move_next();
    EXPECT_EQ(traverser->current(), "/path/3");
}

TEST_F(DLLTraverserTest, MovePrev) {
    traverser->reset_to_back();
    EXPECT_EQ(traverser->current(), "/path/3");
    traverser->move_prev();
    EXPECT_EQ(traverser->current(), "/path/2");
    traverser->move_prev();
    EXPECT_EQ(traverser->current(), "/path/1");
}

TEST_F(DLLTraverserTest, CycleForward) {
    traverser->move_next();
    traverser->move_next();
    traverser->move_next();
    EXPECT_EQ(traverser->current(), "/path/1");
}

TEST_F(DLLTraverserTest, CycleBackward) {
    traverser->move_prev();
    EXPECT_EQ(traverser->current(), "/path/3");
}

TEST_F(DLLTraverserTest, ResetToFront) {
    traverser->move_next();
    traverser->move_next();
    traverser->reset_to_front();
    EXPECT_EQ(traverser->current(), "/path/1");
}

TEST_F(DLLTraverserTest, ResetToBack) {
    traverser->reset_to_back();
    EXPECT_EQ(traverser->current(), "/path/3");
}

TEST_F(DLLTraverserTest, EmptyList) {
    DoublyLinkedList empty_list;
    DLLTraverser empty_traverser(empty_list);
    // No operations should cause crashes
    empty_traverser.move_next();
    empty_traverser.move_prev();
    empty_traverser.reset_to_front();
    empty_traverser.reset_to_back();
}