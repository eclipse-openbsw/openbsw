// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

namespace async
{
template<typename Node>
class Queue
{
public:
    Queue();

    void enqueue(Node& node);
    Node* dequeue();

    void clear();
    void remove(Node& node);

private:
    Node* _first;
    Node* _last;
};

/**
 * Inline implementations.
 */
template<typename Node>
Queue<Node>::Queue() : _first(nullptr), _last(nullptr)
{}

template<typename Node>
void Queue<Node>::enqueue(Node& node)
{
    node.enqueue();
    if (_last != nullptr)
    {
        _last->setNext(&node);
    }
    else
    {
        _first = &node;
    }
    _last = &node;
}

template<typename Node>
Node* Queue<Node>::dequeue()
{
    if (_first != nullptr)
    {
        Node* const node = _first;
        if (_last == node)
        {
            _last = nullptr;
        }
        _first = node->dequeue();
        return node;
    }
    return nullptr;
}

template<typename Node>
void Queue<Node>::clear()
{
    while (_first != nullptr)
    {
        _first = _first->dequeue();
    }
    _last = nullptr;
}

template<typename Node>
void Queue<Node>::remove(Node& node)
{
    Node* prevNode    = nullptr;
    Node* currentNode = _first;
    while (currentNode != nullptr)
    {
        if (currentNode == &node)
        {
            if (prevNode != nullptr)
            {
                prevNode->setNext(node.dequeue());
            }
            else
            {
                _first = node.dequeue();
            }
            if (_last == &node)
            {
                _last = prevNode;
            }
            break;
        }
        prevNode    = currentNode;
        currentNode = currentNode->getNext();
    }
}

} // namespace async
