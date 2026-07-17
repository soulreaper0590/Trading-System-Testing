#pragma once

#include "Utils.h"
#include "DoubleLinkedList.h"
#include "OrderNode.h"

struct PxLvl : public DoubleLinkedList<OrderNode> {
public:
    explicit PxLvl(ordPx_t aPx = 0, ExchangeTicker aTicker = 0) : px_(aPx), ticker_(aTicker) {}
    ~PxLvl() { deleteAllOrders(); }

    inline ExchangeTicker ticker() const { return ticker_; }
    inline ordPx_t px()        const { return px_; }
    inline ordSz_t cumSz()     const { return cumSz_; }
    inline int     orderCount()      { return DoubleLinkedList<OrderNode>::size(); }
    inline bool    isEmpty()         { return DoubleLinkedList<OrderNode>::empty(); }

    // Append at the back => new order gets lowest time priority (FIFO).
    inline void insert(OrderNode* aNode) {
        DoubleLinkedList<OrderNode>::push_back(aNode);
        cumSz_ += aNode->sz();
    }

    // Unlink and reduce the cached size (node is NOT deleted here).
    inline void removeOrder(OrderNode* aNode) {
        DoubleLinkedList<OrderNode>::erase(aNode);
        cumSz_ -= aNode->sz();
    }

    // Keep cumSz_ correct when a resting order's size changes in place.
    inline void adjustCumSz(ordSz_t aDelta) { cumSz_ += aDelta; }

    inline ordSz_t matchOrders(OrderNode* aArgessor, std::vector<ordId_t>& aMatchedOrders) { // will return if there is volume left or not.
        auto myIt = DoubleLinkedList<OrderNode>::begin();

        while (myIt != DoubleLinkedList<OrderNode>::end() & aArgessor->sz() > 0) {
            OrderNodePtr myPassiveOrder  = *myIt;
            ordSz_t myTradeSz = std::min(aArgessor->sz(), myPassiveOrder->sz());

            // Reduce both orders by the traded quantity (sizes updated in place).
            
            cumSz_ -= myTradeSz;
            // Print a line for BOTH orders; aggressor side is the same in both.
            aArgessor->fill(myTradeSz);
            printTrade(aArgessor->id(),  myTradeSz, aArgessor->side());
            printTrade(myPassiveOrder->id(), myTradeSz, aArgessor->side());
            ++myIt;
            if(myPassiveOrder->fill(myTradeSz)){
                removeOrder(myPassiveOrder);
                aMatchedOrders.push_back(myPassiveOrder->id());
            }
        }

        // Detach the fully consumed prefix from the list (nodes deleted by book).
        if (myIt == DoubleLinkedList<OrderNode>::end()) {
            DoubleLinkedList<OrderNode>::clear();
        }
        return aArgessor->sz();
    }

    // Emitted for BOTH orders involved in a fill (the resting maker and the
    // aggressing taker). AggrSide is the aggressor's side in both lines.
    // Format: "Trade: Ticker OrderId TradeSize TradePrice, AggrSide=B"
    inline void printTrade(ordId_t aOrderId, ordSz_t aTradeSz, Side aAggrSide) const {
        printf("Trade: %lld %ld %ld %g, AggrSide=%c\n",
               (long long)ticker_, aOrderId, aTradeSz, px_, sideChar(aAggrSide));
    }

    void print(FILE* aStream = stdout) {
        printf( "  px=%g cumSz=%ld",
                px_, cumSz_);
        for (auto myIt = DoubleLinkedList<OrderNode>::begin(); myIt != DoubleLinkedList<OrderNode>::end(); ++myIt) {
            (*myIt)->print();
            printf(" ");
        }
        printf("\n");
    }

    inline void deleteAllOrders() {
        auto myIt = DoubleLinkedList<OrderNode>::begin();
        while (myIt != DoubleLinkedList<OrderNode>::end()) {
            OrderNodePtr myToDelete = *myIt;
            ++myIt;
            delete myToDelete;
        }
        DoubleLinkedList<OrderNode>::clear();
        cumSz_ = 0;
    }

private:
    const ordPx_t px_; // this has to be 
    ordSz_t cumSz_  = 0;
    const ExchangeTicker ticker_ = 0;
};

using PxLvlPtr = PxLvl*;
