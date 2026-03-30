#pragma once
#include "Block.h"

namespace UserCommon_211645361_000000000 {
    class Node{
    private:
        // Attributes:
        Block* nodeBlock;
        bool explored;
        Node* parent;

    public:
        // Constructors:
        explicit Node(Block* nodeBlock);

        // Get/Set:
        Block* getBlock();
        void markExplored();
        Node* getParent();
        void setParent(Node* parent);
        std::stack<Block*> getChildToParentStack();

        // Predicates:
        bool hasParent();
        [[nodiscard]] bool hasExplored() const;
    };
}

