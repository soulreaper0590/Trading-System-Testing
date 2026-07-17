#include <bits/stdc++.h>


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
    ordPx_t px{0.0};
    ExchangeTicker exchangeTicker{0};
    ordId_t orderId{0};
    ordSz_t sz{0};
    Side side{Side::Buy};
    RequestType request_type{RequestType::New};

    void print() const {
        std::cout << "ExchangeId: " << exchangeTicker
                  << " | Type: " << static_cast<char>(request_type)
                  << " | ID: " << orderId
                  << " | Side: " << static_cast<char>(side)
                  << " | Qty: " << sz
                  << " | Price: " << px << "\n";
    }
};
using ordId_t = long int;  // exchange order id
using ordPx_t = double;    // price
using ordSz_t = long int;  // quantity
using ExchangeTicker = long long int;

inline char sideChar(Side aSide) {
    return aSide == Side::Buy ? 'B' : 'S';
}

// for some reason we need to print this twice
struct Trade {
    ordId_t takerId;   // incoming / aggressing order
    ordId_t makerId;   // resting order that was hit
    ordPx_t px;        // execution price (maker price)
    ordSz_t sz;        // executed quantity
    Side    takerSide; // side of the aggressor
};
