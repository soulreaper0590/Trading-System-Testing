#pragma once
#include "Utils.h"
static constexpr ordSz_t ORDER_NOT_FOUND = -1;

struct OrderNode {
    static constexpr ordId_t NULL_ORDER_ID = -1;

    OrderNode() = default;
    OrderNode(ordId_t aId, Side aSide, ordPx_t aPx, ordSz_t aSz)
        : px_(aPx), sz_(aSz), id_(aId), side_(aSide) {}

    OrderNode(const OrderNode&) = default;
    OrderNode& operator=(const OrderNode&) = delete;

    // ---- accessors ---------------------------------------------------------
    inline ordId_t id()const { return id_; }
    inline Side side()const { return side_; }
    inline ordPx_t px()const { return px_; }
    inline ordSz_t sz()const { return sz_; }

    // ---- mutators ----------------------------------------------------------
    inline bool fill(ordSz_t aSz) { sz_ -= aSz; return sz_<=0;}          // reduce on a trade, return true if completely fille
    inline void setSz(ordSz_t aSz) { sz_ = aSz; }
    inline void setPx(ordPx_t aPx) { px_ = aPx; }

    // Required by DoubleLinkedList.
    inline void reset() { prev = next = nullptr; }

    void print() const {
        printf("[id=%ld. side = %c,px=%g sz=%ld]", id_, sideChar(side_), px_, sz_);
    }

    // intrusive list links (time priority within a PxLvl)
    OrderNode* prev = nullptr;
    OrderNode* next = nullptr;

private:
    ordPx_t px_   = 0;
    ordSz_t sz_   = 0;
    ordId_t id_   = NULL_ORDER_ID;
    Side side_ = Side::Buy;
};

using OrderNodePtr = OrderNode*;
