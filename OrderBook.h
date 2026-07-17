#pragma once
#include "PxLvl.h"

class OrderBook {
public:
    using Px2PxLvl   = std::map<ordPx_t, PxLvlPtr>;
    using Id2OrderPtr = std::unordered_map<ordId_t, OrderNodePtr>;

    explicit OrderBook(exchange_ticker aTicker = 0) : ticker_(aTicker) {}
    ~OrderBook() {
        for (auto& kv : bids_) delete kv.second;
        for (auto& kv : asks_) delete kv.second;
    }

    inline exchange_ticker ticker() const { return ticker_; }

    OrderBook(const OrderBook&)            = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    // ---- queries -----------------------------------------------------------
    inline bool isOrdExist(ordId_t aId) const { return orderMap_.find(aId) != orderMap_.end(); }
    inline OrderNodePtr getOrder(ordId_t aId) const {
        auto myIt = orderMap_.find(aId);
        return myIt == orderMap_.end() ? nullptr : myIt->second;
    }

    // best bid = highest buy px; best ask = lowest sell px
    inline bool bestBid(ordPx_t& aOut) const {
        if (bids_.empty()) return false;
        aOut = bids_.rbegin()->first; return true;
    }
    inline bool bestAsk(ordPx_t& aOut) const {
        if (asks_.empty()) return false;
        aOut = asks_.begin()->first; return true;
    }

    /*
     * Add a new order. Returns the trades it generated (empty if fully passive).
     *
     * 1. Match against the opposite side while the prices cross.
     * 2. Any remaining quantity rests on this order's own side (time priority
     *    = arrival order within its price level).
     */
    std::vector<Trade> addOrder(ordId_t aId, Side aSide, ordPx_t aPx, ordSz_t aSz) {
        std::vector<Trade> myTrades;
        if (aSz <= 0 || isOrdExist(aId)) return myTrades;

        Timestamp myTs = nextTs();
        OrderNodePtr myOrd = new OrderNode(aId, aSide, aPx, aSz, myTs);

        // Aggressor size is reduced in place while matching.
        if (aSide == Side::BUY) matchAgainst(asks_, myOrd, /*takerIsBuy=*/true,  myTrades);
        else                    matchAgainst(bids_, myOrd, /*takerIsBuy=*/false, myTrades);

        if (myOrd->sz() > 0) {
            restOrder(myOrd);                 // remainder becomes a resting (maker) order
        } else {
            delete myOrd;                     // fully filled on arrival
        }
        return myTrades;
    }

    // Cancel a resting order. Returns true if it existed.
    bool cancelOrder(ordId_t aId) {
        auto myIt = orderMap_.find(aId);
        if (myIt == orderMap_.end()) return false;
        OrderNodePtr myOrd = myIt->second;
        removeResting(myOrd);                 // unlink + drop empty level + erase map
        delete myOrd;
        return true;
    }

    /*
     * Amend an order's price and/or size.
     *
     * Any amend REBUILDS time priority: we remove the resting order and
     * re-add it as if new. Consequently an amend to an aggressive price will
     * cross and generate trades just like a new order would.
     *
     * Returns the trades produced by the (possibly re-crossing) amend. If the
     * order id does not exist, returns empty and sets aOk=false.
     */
    std::vector<Trade> amendOrder(ordId_t aId, ordPx_t aNewPx, ordSz_t aNewSz, bool& aOk) {
        std::vector<Trade> myTrades;
        auto myIt = orderMap_.find(aId);
        if (myIt == orderMap_.end()) { aOk = false; return myTrades; }
        aOk = true;

        OrderNodePtr myOrd = myIt->second;
        Side mySide = myOrd->side();

        // Remove the old resting instance entirely.
        removeResting(myOrd);
        delete myOrd;

        if (aNewSz <= 0) return myTrades;     // amend-to-zero == cancel

        // Re-add with a fresh timestamp -> loses time priority, may re-cross.
        myTrades = addOrder(aId, mySide, aNewPx, aNewSz);
        return myTrades;
    }

    // ---- introspection -----------------------------------------------------
    void print(FILE* aStream = stdout) const {
        fprintf(aStream, "==== ORDER BOOK ====\n");
        fprintf(aStream, "ASKS (low->high):\n");
        for (auto it = asks_.begin(); it != asks_.end(); ++it) it->second->print(aStream);
        fprintf(aStream, "BIDS (high->low):\n");
        for (auto it = bids_.rbegin(); it != bids_.rend(); ++it) it->second->print(aStream);
        fprintf(aStream, "====================\n");
    }

private:
    exchange_ticker ticker_ = 0;  // instrument ticker (printed in trade lines)
    Px2PxLvl    bids_{};      // buy side
    Px2PxLvl    asks_{};      // sell side
    Id2OrderPtr orderMap_{};  // id -> resting node
    Timestamp   clock_ = 0;   // monotonic tick source for time priority

    inline Timestamp nextTs() { return ++clock_; }

    inline Px2PxLvl& sideMap(Side aSide) { return aSide == Side::BUY ? bids_ : asks_; }

    // Get-or-create the price level on the order's own side, then append.
    inline void restOrder(OrderNodePtr aOrd) {
        Px2PxLvl& myMap = sideMap(aOrd->side());
        PxLvlPtr myLvl;
        auto myIt = myMap.find(aOrd->px());
        if (myIt == myMap.end()) { myLvl = new PxLvl(aOrd->px(), ticker_); myMap[aOrd->px()] = myLvl; }
        else                     { myLvl = myIt->second; }
        myLvl->insert(aOrd);
        orderMap_[aOrd->id()] = aOrd;
    }

    // Unlink a resting order from its level, drop the level if empty, erase map.
    inline void removeResting(OrderNodePtr aOrd) {
        Px2PxLvl& myMap = sideMap(aOrd->side());
        auto myIt = myMap.find(aOrd->px());
        if (myIt != myMap.end()) {
            PxLvlPtr myLvl = myIt->second;
            myLvl->removeOrder(aOrd);
            if (myLvl->isEmpty()) { myMap.erase(myIt); delete myLvl; }
        }
        orderMap_.erase(aOrd->id());
    }

    /*
     * Walk the opposite side from its best price and match while it crosses.
     *   takerIsBuy  -> oppMap is asks_, cross while askPx <= taker px (ascending)
     *   !takerIsBuy -> oppMap is bids_, cross while bidPx >= taker px (descending)
     * Fully filled maker orders are removed from orderMap_ and deleted.
     */
    void matchAgainst(Px2PxLvl& aOppMap, OrderNodePtr aTaker,
                      bool aTakerIsBuy, std::vector<Trade>& aTrades) {
        std::vector<ordId_t> myFilledIds;

        while (aTaker->sz() > 0 && !aOppMap.empty()) {
            // Best opposite level: lowest ask, or highest bid.
            auto myLvlIt = aTakerIsBuy ? aOppMap.begin() : std::prev(aOppMap.end());
            PxLvlPtr myLvl = myLvlIt->second;
            ordPx_t  myLvlPx = myLvl->px();

            bool myCrosses = aTakerIsBuy ? (myLvlPx <= aTaker->px())
                                         : (myLvlPx >= aTaker->px());
            if (!myCrosses) break;

            myFilledIds.clear();
            myLvl->matchOrders(aTaker, aTrades, myFilledIds);

            // Drop fully-consumed maker nodes from the id map and free them.
            for (ordId_t myId : myFilledIds) {
                auto myMapIt = orderMap_.find(myId);
                if (myMapIt != orderMap_.end()) {
                    OrderNodePtr myDone = myMapIt->second;
                    orderMap_.erase(myMapIt);
                    delete myDone;
                }
            }

            if (myLvl->isEmpty()) { aOppMap.erase(myLvlIt); delete myLvl; }
        }
    }
};
