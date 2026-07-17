#include <bits/stdc++.h>



template <typename LinkedListNode>
class DoubleLinkedList {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;

        Iterator(LinkedListNode* aNode) : current_(aNode) {}

        LinkedListNode* operator*()  { return current_; }
        LinkedListNode* operator->() { return current_; }

        Iterator& operator++() { current_ = current_->next; return *this; }
        Iterator  operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.current_ == b.current_; }
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.current_ != b.current_; }

    private:
        LinkedListNode* current_;
    };

    Iterator begin() const { return Iterator(head_); }
    Iterator end()   const { return Iterator(nullptr); }

    DoubleLinkedList() {}

    inline int  size()  const { return size_; }
    inline bool empty() const { return size_ == 0; }
    inline LinkedListNode* head() const { return head_; }
    inline LinkedListNode* tail() const { return tail_; }

    // Append at tail => lowest time priority (FIFO).
    inline void push_back(LinkedListNode* aNewNode) {
        if (tail_ == nullptr) { aNewNode->reset(); head_ = tail_ = aNewNode; }
        else{ setTailToNewNode(aNewNode); }
        size_++;
    }

    inline void push_front(LinkedListNode* aNewNode) {
        if (tail_ == nullptr) { aNewNode->reset(); head_ = tail_ = aNewNode; }
        else {
            head_->prev   = aNewNode;
            aNewNode->next = head_;
            aNewNode->prev = nullptr;
            head_          = aNewNode;
        }
        size_++;
    }

    inline void clear() { head_ = tail_ = nullptr; size_ = 0; }

    // Move an existing node to the tail (used when an amend should lose priority
    // in-place). Kept for completeness.
    inline void moveBack(LinkedListNode* aNode) {
        LinkedListNode* myBefore = aNode->prev;
        LinkedListNode* myAfter  = aNode->next;
        int myCase = 2 * (myBefore == nullptr) + (myAfter == nullptr);
        switch (myCase) {
            case 0: myBefore->next = myAfter; myAfter->prev = myBefore; setTailToNewNode(aNode); break;
            case 1: break;                                   // already tail
            case 2: head_ = head_->next; head_->prev = nullptr; setTailToNewNode(aNode); break;
            case 3: break;                                   // only node
        }
    }

    inline void setTailToNewNode(LinkedListNode* aNode) {
        tail_->next  = aNode;
        aNode->prev  = tail_;
        tail_        = aNode;
        tail_->next  = nullptr;
    }

    std::vector<LinkedListNode*> getVector() {
        std::vector<LinkedListNode*> myRes;
        for (LinkedListNode* it = head_; it != nullptr; it = it->next) myRes.push_back(it);
        return myRes;
    }

    // Unlink (does NOT delete) a node from the list.
    inline void erase(LinkedListNode* aNode) {
        LinkedListNode* myBefore = aNode->prev;
        LinkedListNode* myAfter  = aNode->next;
        int myCase = 2 * (myBefore == nullptr) + (myAfter == nullptr);
        switch (myCase) {
            case 0: myBefore->next = myAfter; myAfter->prev = myBefore; break;
            case 1: tail_ = tail_->prev; tail_->next = nullptr; break;   // was tail
            case 2: head_ = head_->next; head_->prev = nullptr; break;   // was head
            case 3: head_ = tail_ = nullptr; break;                       // was only node
        }
        size_--;
    }

protected:
    LinkedListNode* head_ = nullptr;
    LinkedListNode* tail_ = nullptr;
    int             size_ = 0;
};
