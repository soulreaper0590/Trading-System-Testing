static constexpr ordSz_t ORDER_NOT_FOUND = -1;

struct OrderNode {
    static constexpr ordId_t NULL_ORDER_ID = -1;

    OrderNode() = default;
    OrderNode(ordId_t aId, Side aSide, ordPx_t aPx, ordSz_t aSz, Timestamp aTs)
        : px_(aPx), sz_(aSz), id_(aId), ts_(aTs), side_(aSide) {}

    OrderNode(const OrderNode&)            = default;
    OrderNode& operator=(const OrderNode&) = delete;

    // ---- accessors ---------------------------------------------------------
    inline ordId_t   id()   const { return id_; }
    inline Side      side() const { return side_; }
    inline ordPx_t   px()   const { return px_; }
    inline ordSz_t   sz()   const { return sz_; }
    inline Timestamp ts()   const { return ts_; }

    // ---- mutators ----------------------------------------------------------
    inline void fill(ordSz_t aSz) { sz_ -= aSz; }          // reduce on a trade
    inline void setSz(ordSz_t aSz) { sz_ = aSz; }
    inline void setPx(ordPx_t aPx) { px_ = aPx; }
    inline void setTs(Timestamp aTs) { ts_ = aTs; }

    // Required by DoubleLinkedList.
    inline void reset() { prev = next = nullptr; }

    void print(FILE* aStream = stdout) const {
        fprintf(aStream, "[id=%ld %s px=%g sz=%ld ts=%llu]",
                id_, sideStr(side_), px_, sz_, (unsigned long long)ts_);
    }

    // intrusive list links (time priority within a PxLvl)
    OrderNode* prev = nullptr;
    OrderNode* next = nullptr;

private:
    ordPx_t   px_   = 0;
    ordSz_t   sz_   = 0;
    ordId_t   id_   = NULL_ORDER_ID;
    Timestamp ts_   = 0;
    Side      side_ = Side::BUY;
};

using OrderNodePtr = OrderNode*;
