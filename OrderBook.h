#pragma once
#include "PxLvl.h"

class OrderBook {
public:
    using Px2PxLvl   = std::map<ordPx_t, PxLvlPtr>; // PxLvl maps bids and asks
    using Id2OrderPtr = std::unordered_map<ordId_t, OrderNodePtr>; // OrderId to OrderNode* map 

    explicit OrderBook(ExchangeTicker aTicker) : ticker_(aTicker) {}
    ~OrderBook() {
        for (auto& kv : bids_) delete kv.second;
        for (auto& kv : asks_) delete kv.second;
    }

    inline ExchangeTicker ticker() const { return ticker_; }

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
    template<bool IS_BUY>
    void addOrder(ordId_t aId, Side aSide, ordPx_t aPx, ordSz_t aSz) {
        OrderNodePtr myOrd = new OrderNode(aId, aSide, aPx, aSz);
        int myRemainingSz = aSz;
        // Aggressor size is reduced in place while matching.
        if constexpr (IS_BUY){matchAgainstAsks(myOrd);}
        else{matchAgainstBids(myOrd);}

        if (myOrd->sz() > 0) {
            addOrderToBook(myOrd);    // remainder becomes a resting (maker) order
        } else {
            delete myOrd;                     // fully filled on arrival
        }
        return;
    }

    // Cancel a resting order. Returns true if it existed.
    void cancelOrder(ordId_t aId) {
        cleanOrderExistenceFromBook(aId);
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
    template<bool IS_BUY>
    bool amendOrder(ordId_t aId, ordPx_t aNewPx, ordSz_t aNewSz) {
        
        // Remove the old resting instance entirely.
        clearOrderFromBook(myOrd);
        delete myOrd;

        if (aNewSz <= 0) return false;     // amend-to-zero == cancel

        // Re-add with a fresh timestamp -> loses time priority, may re-cross.
        addOrder<IS_BUY>(aId, mySide, aNewPx, aNewSz);
        return true;
    }

    // ---- introspection -----------------------------------------------------
    void print(FILE* aStream = stdout) const {
        fprintf(aStream, "==== ORDER BOOK ====\n");
        fprintf(aStream, "BIDS (low->high):\n");
        for (auto it = bids_.rbegin(); it != bids_.rend(); ++it){it->second->print(aStream);}
        fprintf(aStream, "ASKS (low->high):\n");
        for (auto it = asks_.begin(); it != asks_.end(); ++it){it->second->print(aStream);}
        fprintf(aStream, "====================\n");
    }
    Px2PxLvl& sideMap(bool aIsBuy){
        if(aIsBuy){return bids_;}
        else{asks_;}
    }

private:
    ExchangeTicker ticker_;  // instrument ticker (printed in trade lines)
    Px2PxLvl bids_{};      // buy side
    Px2PxLvl asks_{};      // sell side
    Id2OrderPtr orderMap_{};  // id -> resting nodei

    // Get-or-create the price level on the order's own side, then append.
    template<bool IS_BUY>
    inline void addOrderToBook(OrderNodePtr aOrd) {
        if constexpr (IS_BUY){
            addPxLvlToMap(bids_, aOrd);
        }else{
            addPxLvlToMap(asks_, aOrd);
        }
        orderMap_[aOrd->id()] = aOrd;
    }

    inline void addPxLvlToMap(Px2PxLvl& aPxLvlMap, OrderNodePtr aOrd){
        PxLvlPtr myPxLvl = nullptr;
        auto myIt = aPxLvlMap.find(aOrd->px());
        if (myIt == aPxLvlMap.end()) { myPxLvl = new PxLvl(aOrd->px(), ticker_); aPxLvlMap[aOrd->px()] = myPxLvl; }
        else {myPxLvl = myIt->second;}
        myPxLvl->insert(aOrd);
    }

    inline void cleanOrderExistenceFromBook(ordId_t aOrdId){
        auto myIt = orderMap_.find(aOrdId);
        if (myIt == orderMap_.end()) {
            printf("Arror Occured Could not Find Order Correspoding a Ticker %lld and ID %lld \n", ticker_, aOrdId);
            return;
        }
        OrderNodePtr myOrd = myIt->second;
        removeOrderFromPxMap(myOrd);
    }

    // Unlink a resting order from its level, drop the level if empty, erase map.
    inline void removeOrderFromPxMap(OrderNodePtr aOrd) {
        Px2PxLvl& myMap = sideMap(aOrd->side() == Side::Buy);
        auto myIt = myMap.find(aOrd->px());
        if (myIt != myMap.end()) {
            PxLvlPtr myLvl = myIt->second;
            myLvl->removeOrder(aOrd);
            if (myLvl->isEmpty()) { myMap.erase(myIt); delete myLvl; }
        }
        orderMap_.erase(aOrd->id());
    }
    
    void matchAgainstBids(OrderNodePtr aAgressor) {
        std::vector<ordId_t> myFilledIds;
	    int fillledSz = 0;
	    while((bids_.empty())&(aAgressor->sz()>0)){
            auto bestBid = bids_.rbegin();
            if (bestBid->first < aAgressor->px()){return;} // there is comparision of double happening here.
            bestBid->second->matchOrders(aAgressor, myFilledIds);
            eraseOrderIdsFromMap(myFilledIds); // raising trade here ?
            if(bestBid->second->isEmpty()){
                bids_.erase(bestBid->first);
                delete bestBid->second; // freeing this memory
            }else{
                return;
            }
	    }
	    return;
	}
    ordSz_t matchAgainstAsks(Px2PxLvl& aAskSideMap, OrderNodePtr aAgressor) {
        std::vector<ordId_t> myFilledIds;
        auto iter = asks_.begin();
	    if(iter == asks_.end()){return;}
	    while((asks_.empty())&(aAgressor->sz()>0)){
            auto myBestAsk = asks_.begin();
            if (myBestAsk->first > aAgressor->px()){return;} // there is comparision of double happening here.
            myBestAsk->second->matchOrders(aAgressor, myFilledIds); // raising trades here .. 
            eraseOrderIdsFromMap(myFilledIds);
            if(myBestAsk->second->isEmpty()){
                asks_.erase(myBestAsk->first);
                delete myBestAsk->second; // freeing this memory
            }
	    }
	    return;
    }


    void eraseOrderIdsFromMap(std::vector<ordId_t>& aOrdIdsToRemove){
        for (ordId_t id : aOrdIdsToRemove) {
            auto it = orderMap_.find(id);
            if (it != orderMap_.end()) {
                delete it->second; // Free the actual OrderNode memory
                orderMap_.erase(it); // remove the traded resting orders in the book from the map
            }
        }
        aOrdIdsToRemove.clear(); // lets just clear it here.. 
    }
};
