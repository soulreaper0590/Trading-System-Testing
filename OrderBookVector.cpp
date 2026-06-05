struct PxLevel{
    double px_;
    int totalSz_;
    PxLevel(double aPx, int aSz){px_ = aPx; totalSz_ = aSz;}
};

bool customCompareBids(const PxLevel* a, double aPx){
    return a->px_ > aPx;
}
bool customCompareAsks(const PxLevel* a, double aPx){
    return a->px_  < aPx;
}


class OrderBook{
public:
    void newOrder(double aPx, int aSz, bool isBuy){
        if(isBuy){
            addANewOrderBids(aPx, aSz);
        }else{
            addANewOrderAsks(aPx, aSz);
        }
    }
    void cancelOrder(double aPx, int aRemianingSz, int isBuy){
        if(isBuy){
            findPxLevelAndReduceSz<true>(aPx, aRemianingSz);
        }else{
            findPxLevelAndReduceSz<false>(aPx, aRemianingSz);
        }
    }
    void modifyOrder(double aOldPx, int aOldSz, bool isBuy, double aNewpx, double aNewSz){
        if(isBuy){
            findPxLevelAndReduceSz<true>(aOldPx, aOldSz);
            addANewOrderBids(aNewpx, aNewSz);
        }else{
            findPxLevelAndReduceSz<false>(aOldPx, aOldSz);
            addANewOrderAsks(aNewpx, aNewSz);
        }
    }
    void tradeOrder(double aPx, int aTradedSz, int isBuy){
        if(isBuy){
            findPxLevelAndReduceSz<true>(aPx, aTradedSz);
        }else{
            findPxLevelAndReduceSz<false>(aPx, aTradedSz);
        }
    }
    
    
    void printOrderBook(){
        // printf("||");
        for(int i = (int)bids_.size()- 1; i >= 0; i--){
            printf("--[%f,%d]--",bids_[i]->px_, bids_[i]->totalSz_);
        }
        printf("||");
        for(int i = 0; i < (int)asks_.size(); i++){
            printf("--[%f,%d]--",asks_[i]->px_, asks_[i]->totalSz_);
        }
        printf("\n");
    }

protected:
// std::map<double, PxLevel*> mapPxLevel;
std::vector<PxLevel*> bids_;
std::vector<PxLevel*> asks_;

    void addANewOrderBids(double aPx, int aSz){
        int myRemainingSz = matchWithAsks(aPx, aSz);
        if(myRemainingSz == 0 ){return;}
        auto iter = std::lower_bound(bids_.begin(), bids_.end(), aPx, &customCompareBids);
        if(iter != bids_.end()){
            if((*iter)->px_ == aPx){
                (*iter)->totalSz_ += myRemainingSz;
                return;
            }
        }
        PxLevel *myPxLevel = new PxLevel(aPx, myRemainingSz);
        bids_.insert(iter, myPxLevel);
        // mapPxLevel[aPx] = myPxLevel;
    }
    void addANewOrderAsks(double aPx, int aSz){
        int myRemainingSz = matchWithBids(aPx, aSz);
        if(myRemainingSz == 0 ){return;}
        auto iter = std::lower_bound(asks_.begin(), asks_.end(), aPx, &customCompareAsks);
        if(iter != asks_.end()){
            if((*iter)->px_ == aPx){
                (*iter)->totalSz_ += myRemainingSz;
                return;
            }
        }
        PxLevel *myPxLevel = new PxLevel(aPx, myRemainingSz);
        asks_.insert(iter, myPxLevel);
        // mapPxLevel[aPx] = myPxLevel;
    }
    
    template<bool IS_BIDS>
    void findPxLevelAndReduceSz( double aPx, int aSz){
        if constexpr (IS_BIDS){
            auto iter = std::lower_bound(bids_.begin(), bids_.end(), aPx, &customCompareBids);
            if(iter != bids_.end()){
                if((*iter)->px_ == aPx){
                    (*iter)->totalSz_ -= aSz;
                    if((*iter)->totalSz_ <= 0){bids_.erase(iter);}
                    return;
                }
            }
            
        }else{
            auto iter = std::lower_bound(asks_.begin(), asks_.end(), aPx, &customCompareAsks);
            if(iter != asks_.end()){
                if((*iter)->px_ == aPx){
                    (*iter)->totalSz_ -= aSz;
                    if((*iter)->totalSz_ <= 0){asks_.erase(iter);}
                    return;
                }
            }
        }
    }
    int matchWithAsks(double aPx, int aSz){
        if(asks_.size() == 0){return aSz;}
        int aFilledSz = 0;
        int i = 0;
        while(((asks_[i]->px_ < aPx)&(aFilledSz < aSz))){
            // printOrderBook();
            if((aSz- aFilledSz) >= asks_[i]->totalSz_){
                aFilledSz += asks_[i]->totalSz_;
                asks_.erase(asks_.begin());
                if(i == ((int)asks_.size()-1)){break;}
            }else{
                asks_[i]->totalSz_ -= aSz;;
                aFilledSz = aSz;
                return aSz - aFilledSz;
            }
        }
        return aSz - aFilledSz;
    }
    
    int matchWithBids(double aPx, int aSz){
        if(bids_.size() == 0){return aSz;}
        int aFilledSz = 0;
        int i = 0;
        while(((bids_[i]->px_ > aPx)&(aFilledSz < aSz))){
            if((aSz- aFilledSz) >= bids_[i]->totalSz_){
                aFilledSz += bids_[i]->totalSz_;
                bids_.erase(bids_.begin());
                if(i == ((int)bids_.size()-1)){break;}
            }else{
                bids_[i]->totalSz_ -= aSz;
                aFilledSz = aSz;
                return aSz - aFilledSz;
            }
        }
        return aSz - aFilledSz;
    }
    
};


int main() {
    OrderBook ob;

    // ==========================================
    // PHASE 1: RESTING LIQUIDITY SETUP
    // ==========================================
    std::cout << "--- Setup: Adding Initial Resting Liquidity ---\n";
     ob.newOrder(90.00, 50, true);  // Bid Level 1
    ob.printOrderBook();
    ob.newOrder(89.00, 30, true);   // Bid Level 2
    ob.printOrderBook();
    ob.newOrder(107.00, 40, false); // Ask Level 1
    ob.printOrderBook();
    ob.newOrder(108.00, 25, false); // Ask Level 2
    ob.printOrderBook();
    ob.newOrder(100.00, 50, true);  // Bid Level 1
    ob.printOrderBook();
    ob.newOrder(99.00, 30, true);   // Bid Level 2
    ob.printOrderBook();
    ob.newOrder(102.00, 40, false); // Ask Level 1
    ob.printOrderBook();
    ob.newOrder(103.00, 25, false); // Ask Level 2
    ob.printOrderBook();

    // ==========================================
    // PHASE 2: CANCELLATION TESTS
    // ==========================================
    std::cout << "\n--- Test 5: Cancel Order - Partial Reduction ---\n";
    std::cout << "Canceling 20 units from Bid 100.00...\n";
    ob.cancelOrder(100.00, 20, true);
    ob.printOrderBook();

    std::cout << "\n--- Test 6: Cancel Order - Exact Size (Remove Level) ---\n";
    std::cout << "Canceling remaining 30 units from Bid 100.00...\n";
    ob.cancelOrder(100.00, 30, true);
    ob.printOrderBook();

    std::cout << "\n--- Test 7: Cancel Order - Ask Side ---\n";
    std::cout << "Canceling 15 units from Ask 102.00...\n";
    ob.cancelOrder(102.00, 15, false);
    ob.printOrderBook();

    std::cout << "\n--- Test 8: Cancel Edge Case - Over-canceling Size ---\n";
    std::cout << "Attempting to cancel 50 units from Ask 103.00 (Current Sz: 25)...\n";
    ob.cancelOrder(103.00, 50, false);
    ob.printOrderBook();

    std::cout << "\n--- Test 9: Cancel Edge Case - Price Level Does Not Exist ---\n";
    std::cout << "Attempting to cancel at non-existent price 500.00...\n";
    ob.cancelOrder(500.00, 10, true);
    ob.printOrderBook();

    // ==========================================
    // PHASE 3: MODIFICATION TESTS
    // ==========================================
    std::cout << "\n--- Test 10: Modify Order - Change Size Only ---\n";
    // Current resting: Bid [99.00, 30] || Ask [102.00, 25]
    // Action: Modify Bid 99.00 to reduce size by 10 (OldSz: 10, NewSz: 20) at same price
    std::cout << "Modifying Bid 99.00 to reduce size from 30 to 20...\n";
    ob.modifyOrder(99.00, 30, true, 99.00, 20);
    ob.printOrderBook();

    std::cout << "\n--- Test 11: Modify Order - Price Shift (Passive) ---\n";
    // Action: Move remaining Ask 102.00 up to a more passive price of 104.00 for size 15
    std::cout << "Modifying Ask 102.00 (w/ size 25) -> Moving to 104.00 with size 15...\n";
    ob.modifyOrder(102.00, 25, false, 104.00, 15);
    ob.printOrderBook();

    std::cout << "\n--- Test 12: Modify Order - Aggressive Shift (Triggers Book Cross) ---\n";
    // Current resting: Bid [99.00, 20] || Ask [104.00, 15]
    // Action: Modify the Bid at 99.00 to become an aggressive bid at 105.00 for size 25
    // This will cross the book, sweep Ask 104.00 entirely, and leave 10 units resting on Bid 105.00.
    std::cout << "Modifying Bid 99.00 (Sz: 20) -> Aggressive Bid 105.00 with size 25...\n";
    ob.modifyOrder(99.00, 20, true, 105.00, 25);
    ob.printOrderBook();

    // ==========================================
    // PHASE 4: FINAL UPDATED BOOK STATUS
    // ==========================================
    std::cout << "\n=============================================\n";
    std::cout << "   FINAL UPDATED ORDERBOOK HEALTH CHECK      \n";
    std::cout << "=============================================\n";
    ob.printOrderBook();
    // Expected Output Summary:
    // The crossing modification in Test 12 swallowed the Ask at 104.00.
    // The remaining 10 units of the modified order should now rest on the Bid side at 105.00.
    // Final Expected: Bids: [105.000000, 10] || Asks: (Empty)

    return 0;
}
