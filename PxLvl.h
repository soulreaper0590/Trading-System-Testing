#pragma once

#include "DoubleLinkedList.h"
#include "OrderNode.h"
#include "Utils.h

struct PxLvl : public DoubleLinkedList<OrderNode> {
public:
    explicit PxLvl(ordPx_t aPx = 0, exchange_ticker aTicker = 0) : px_(aPx), ticker_(aTicker) {}
    ~PxLvl() { deleteAllOrders(); }

    inline exchange_ticker ticker() const { return ticker_; }
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

    inline void matchOrders(OrderNode* aTaker,
                            std::vector<Trade>& aTrades,
                            std::vector<ordId_t>& aFilledOrderIds) {
        auto myIt = DoubleLinkedList<OrderNode>::begin();
        int  myConsumedPrefix = 0;

        while (myIt != DoubleLinkedList<OrderNode>::end() && aTaker->sz() > 0) {
            OrderNodePtr myMaker  = *myIt;
            ordSz_t myTradeSz = myMaker->sz() < aTaker->sz() ? myMaker->sz() : aTaker->sz();

            // Reduce both orders by the traded quantity (sizes updated in place).
            myMaker->fill(myTradeSz);
            aTaker->fill(myTradeSz);
            cumSz_ -= myTradeSz;

            // Print a line for BOTH orders; aggressor side is the same in both.
            printTrade(myMaker->id(), myTradeSz, aTaker->side());
            printTrade(aTaker->id(),  myTradeSz, aTaker->side());
            aTrades.push_back(Trade{aTaker->id(), myMaker->id(), px_, myTradeSz, aTaker->side()});

            if (myMaker->sz() > 0) break;   // maker partial => aggressor exhausted

            // Maker fully consumed; keep going down the queue.
            aFilledOrderIds.push_back(myMaker->id());
            myConsumedPrefix++;
            ++myIt;
        }

        // Detach the fully consumed prefix from the list (nodes deleted by book).
        if (myIt == DoubleLinkedList<OrderNode>::end()) {
            DoubleLinkedList<OrderNode>::clear();
        } else if (myConsumedPrefix > 0) {
            OrderNodePtr myNewHead = *myIt;
            myNewHead->prev = nullptr;
            head_ = myNewHead;
            size_ -= myConsumedPrefix;
        }
    }

    // Emitted for BOTH orders involved in a fill (the resting maker and the
    // aggressing taker). AggrSide is the aggressor's side in both lines.
    // Format: "Trade: Ticker OrderId TradeSize TradePrice, AggrSide=B"
    inline void printTrade(ordId_t aOrderId, ordSz_t aTradeSz, Side aAggrSide) const {
        printf("Trade: %lld %ld %ld %g, AggrSide=%c\n",
               (long long)ticker_, aOrderId, aTradeSz, px_, sideChar(aAggrSide));
    }

    void print(FILE* aStream = stdout) {
        fprintf(aStream, "  px=%g cumSz=%ld count=%d : ",
                px_, cumSz_, DoubleLinkedList<OrderNode>::size());
        for (auto myIt = DoubleLinkedList<OrderNode>::begin();
             myIt != DoubleLinkedList<OrderNode>::end(); ++myIt) {
            (*myIt)->print(aStream);
            fprintf(aStream, " ");
        }
        fprintf(aStream, "\n");
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
    const ordPx_t         px_;
    ordSz_t               cumSz_  = 0;
    const exchange_ticker ticker_ = 0;
};

using PxLvlPtr = PxLvl*;
