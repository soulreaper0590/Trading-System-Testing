#include <Utils.h>

#pragma once

#include "OrderBook.h"
#include "Utils.h"
#include <unordered_map>
class MatchingEngine {
public:
    template<enum RequestType>
    void process_message(const Message& msg) {
        auto& book = books[msg.exchange_id];
        switch (msg.request_type) {
            case RequestType::New:
                if (msg.side == Side::Buy) {
                    book.match_and_add_bid(msg.order_id, msg.quantity, msg.price);
                } else {
                    book.match_and_add_ask(msg.order_id, msg.quantity, msg.price);
                }
                break;
            case RequestType::Cancel:
                book.handle_cancel(msg.order_id);
                break;
            case RequestType::Amend:
                book.handle_amend(msg.order_id, msg.side, msg.quantity, msg.price);
                break;
        }
    }

    void print_books() const {
        for (const auto& [id, book] : books) {
            std::cout << "=== Order Book for Ticker: " << id << " ===\n";
            book.print();
        }
    }

private:
    std::unordered_map<long int, OrderBook> books;
};

