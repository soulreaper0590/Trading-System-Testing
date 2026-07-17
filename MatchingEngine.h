#include <Utils.h>

#pragma once

#include "OrderBook.h"
#include "Utils.h"
#include <unordered_map>
class MatchingEngine {
public:
    template<enum RequestType, bool IS_BUY>
    void process_message(const Message& aMsg) {
        OrderBook* myOrderBook = findBookFromTicker(aMsg.exchangeTicker);
        if constexpr (ReqType == RequestType::NEW) {
            myOrderBook->addOrder<IS_BUY>(msg.orderId, msg.side, msg.px, msg.qty);
        } 
        else if constexpr (ReqType == RequestType::CANCEL) {
            myOrderBook->cancelOrder(msg.orderId);
        } 
        else if constexpr (ReqType == RequestType::AMEND) {
            myOrderBook->amendOrder<IS_BUY>(msg.orderId, msg.px, msg.sz);
        }
    }

    void print_books() const {
        for (const auto& [id, book] : books_) {
            std::cout << "=== Order Book for Ticker: " << id << " ===\n";
            book->print();
        }
    }

private:
    std::unordered_map<ExchangeTicker, OrderBook*> books_;



    OrderBook* findBookFromTicker(long int aTicker){
        auto myIter = books_.find(aTicker);
        if(myIter != books_.end()){
            return myIter->second;
        }
        OrderBook* myOrderBook = new OrderBook(aTicker);
        books_[aTicker] = myOrderBook; 
        return myOrderBook;
    }
};

