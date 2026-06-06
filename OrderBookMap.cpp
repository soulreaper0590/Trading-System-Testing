/******************************************************************************

Welcome to GDB Online.
  GDB online is an online compiler and debugger tool for C, C++, Python, PHP, Ruby,
  C#, OCaml, VB, Perl, Swift, Prolog, Javascript, Pascal, COBOL, HTML, CSS, JS
  Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <bits/stdc++.h>


class PxLvl {
public:
	PxLvl(double aPx) {
		px_= aPx;
	}
	int increaseSz(int aSz) {
		sz_ += aSz;
		return sz_;
	}
	int reduceSz(int aSz) {
		sz_ -= aSz;
		return sz_;
	}
	// int getCumSz() {return sz_;}

	double px_;
	int sz_;

};



class OrderBook {
public:
	void newOrder(double aPx, int aSz, bool isBuy) {
		if(isBuy) {
			addANewOrderBids(aPx, aSz);
		} else {
			addANewOrderAsks(aPx, aSz);
		}
	}
	void cancelOrder(double aPx, int aRemianingSz, int isBuy) {
		if(isBuy) {
			findPxLevelAndReduceSz<true>(aPx, aRemianingSz);
		} else {
			findPxLevelAndReduceSz<false>(aPx, aRemianingSz);
		}
	}
	void modifyOrder(double aOldPx, int aOldSz, bool isBuy, double aNewpx, double aNewSz) {
		if(isBuy) {
			findPxLevelAndReduceSz<true>(aOldPx, aOldSz);
			addANewOrderBids(aNewpx, aNewSz);
		} else {
			findPxLevelAndReduceSz<false>(aOldPx, aOldSz);
			addANewOrderAsks(aNewpx, aNewSz);
		}
	}
	void tradeOrder(double aPx, int aTradedSz, int isBuy) {
		if(isBuy) {
			findPxLevelAndReduceSz<true>(aPx, aTradedSz);
		} else {
			findPxLevelAndReduceSz<false>(aPx, aTradedSz);
		}
	}


	void printOrderBook() {
		for(auto iter = PxToBidsPxLvlMap_.begin(); iter != PxToBidsPxLvlMap_.end(); iter++) {
			printf("-[%f, %d]-", (iter->second)->px_,(iter->second)->sz_);
		}
		printf("||");
		for(auto iter =  PxToAsksPxLvlMap_.begin(); iter != PxToAsksPxLvlMap_.end(); iter++) {
		    printf("-[%f, %d]-",  (iter->second)->px_,(iter->second)->sz_);
		}
		printf("\n");

	}
public:
	std::map<double, PxLvl*> PxToAsksPxLvlMap_;
	std::map<double, PxLvl*> PxToBidsPxLvlMap_;


	template <bool IS_BIDS>
	void findPxLevelAndReduceSz(double aPx, int aSz) {
		if constexpr (IS_BIDS) {
			auto iter = PxToBidsPxLvlMap_.find(aPx);
			if(iter != PxToBidsPxLvlMap_.end()) {
				int aRemainingSz = iter->second->reduceSz(aSz);
				if(aRemainingSz <= 0) {
					PxToBidsPxLvlMap_.erase(iter);
				}
			}
			return;
		} else {
			auto iter = PxToAsksPxLvlMap_.find(aPx);
			if(iter != PxToAsksPxLvlMap_.end()) {
				int aRemainingSz = iter->second->reduceSz(aSz);
				if(aRemainingSz <= 0) {
					PxToAsksPxLvlMap_.erase(iter);
				}
			}
			return;
		}
	}

	void addANewOrderAsks(double aPx, int aSz) {
		int myRemainingSz = matchWithBids(aPx, aSz);
		auto iter = PxToAsksPxLvlMap_.find(aPx);
		if(iter == PxToAsksPxLvlMap_.end()) {
            insertPxLevel<false>(aPx, myRemainingSz);
		}else{
		    iter->second->increaseSz(myRemainingSz);
		}
	}
	
	void addANewOrderBids(double aPx, int aSz) {
		int myRemainingSz = matchWithAsks(aPx, aSz);
		auto iter = PxToBidsPxLvlMap_.find(aPx);
		if(iter == PxToBidsPxLvlMap_.end()) {
            insertPxLevel<true>(aPx, myRemainingSz);
		}else{
		    iter->second->increaseSz(myRemainingSz);
		}
	}


	template<bool IS_BIDS>
	inline void insertPxLevel(double aPx, int aSz) {
		PxLvl* myNewPxLevel = new PxLvl(aPx);
		if constexpr (IS_BIDS) {
			PxToBidsPxLvlMap_[aPx] = myNewPxLevel;
		} else {
			PxToAsksPxLvlMap_[aPx] = myNewPxLevel;
		}
		myNewPxLevel->sz_ = aSz;
	}
	
	int matchWithAsks(double aPx, int aSz){
	    auto iter = PxToAsksPxLvlMap_.begin();
	    if(iter == PxToAsksPxLvlMap_.end()){return aSz;}
	    int fillledSz = 0;
	    while((iter->second->px_ <= aPx)&(fillledSz < aSz)){
	        if(iter->second->sz_ > (aSz - fillledSz)){
	            iter->second->reduceSz(aSz);
	            return 0;
	        }else{
	            fillledSz += iter->second->sz_;
	            iter = PxToAsksPxLvlMap_.erase(iter);
	            if(iter == PxToAsksPxLvlMap_.end()){
	                return aSz - fillledSz;
	            }
	        }
	    }
	    return aSz - fillledSz;
	}
	int matchWithBids(double aPx, int aSz){
	    auto iter = PxToBidsPxLvlMap_.rbegin();
	    if(iter == PxToBidsPxLvlMap_.rend()){return aSz;}
	   // int fillledSz = 0;
	    int fillledSz = 0;
	    while((iter->second->px_ > aPx)&(fillledSz < aSz)){
	        if(iter->second->sz_ > (aSz - fillledSz)){
	            iter->second->reduceSz(aSz);
	            return 0;
	        }else{
	            fillledSz += iter->second->sz_;
	            iter = std::map<double, PxLvl*>::reverse_iterator(PxToBidsPxLvlMap_.erase(std::next(iter).base()));
	            if(iter == PxToBidsPxLvlMap_.rend()){
	                return aSz - fillledSz;
	            }
	        }
	    }
	    return aSz - fillledSz;
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
    
    
    std::cout << "Modifying Bid 99.00 (Sz: 20) -> Aggressive Bid 105.00 with size 25...\n";
    ob.modifyOrder(107.00, 20, false, 104.00, 20);
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

