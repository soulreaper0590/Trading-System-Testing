#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <charconv>
#include <vector>
#include <optional>
#include <algorithm>


enum class RequestType : char {
    New = 'N',
    Cancel = 'C',
    Amend = 'A'
};


enum class Side : char {
    Buy = 'B',
    Sell = 'S'
};

struct Message {
    double price{0.0};
    long int exchange_ticker{0};
    long int order_id{0};
    long int quantity{0};
    Side side{Side::Buy};
    RequestType request_type{RequestType::New};

    void print() const {
        std::cout << "ExchangeId: " << exchange_ticker
                  << " | Type: " << static_cast<char>(request_type)
                  << " | ID: " << order_id
                  << " | Side: " << static_cast<char>(side)
                  << " | Qty: " << quantity
                  << " | Price: " << price << "\n";
    }
};



