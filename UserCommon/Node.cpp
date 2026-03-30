#include "Node.h"

namespace UserCommon_211645361_000000000 {
// Constructors:
    Node::Node(Block *nodeBlock) {
        this->nodeBlock = nodeBlock;
        this->explored = false;
        this->parent = nullptr;
    }


// Get/Set:
    Block *Node::getBlock() {
        return this->nodeBlock;
    }

    void Node::markExplored() {
        this->explored = true;
    }

    Node *Node::getParent() {
        return this->parent;
    }

    void Node::setParent(Node *newParent) {
        this->parent = newParent;
    }


// Predicates:
    bool Node::hasExplored() const {
        return this->explored;
    }

    bool Node::hasParent() {
        return this->parent != nullptr;
    }

    std::stack<Block*> Node::getChildToParentStack(){
        std::stack<Block*> stack;
        Node* node = this;
        while(node->hasParent()){
            stack.push(node->getBlock());
            node = node->getParent();
        }
        return stack;
    }
}
