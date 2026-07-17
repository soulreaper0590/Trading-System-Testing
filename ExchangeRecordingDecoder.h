#pragma once
#include "Utils.h"


template<typename MatchinEngine>
class ExchangeRecordingDecoder{
    
    ExchangeRecordingDecoder(){
        fields_.clear();
        fields_.resize(6);
    }
    
    bool parseLineWithSubstrings(const std::string& line) {
        
        size_t start = 0;
        size_t end = line.find(',');

        // Extract the first 5 fields sequentially using .substr()
        for (int i = 0; i < 5; ++i) {
            if (end == std::string::npos) {
                return false; // Incomplete or malformed line
            }
            fields_[i] = trim(line.substr(start, end - start));
            start = end + 1;
            end = line.find(',', start);
        }
        
        // Strip trailing inline comments if they exist

        try {
            Message msg{};
            
            // 1. Exchange ID
            msg.exchangeTicker = std::stoll(fields_[0]);

            // 2. Request Type
            char myReqchar = fields_[1][0];
            if (myReqchar != 'N' & myReqchar != 'C' & myReqchar != 'A'){
                printf("Error with the messages that %s, can't process this type of messages", line.c_str());
                return false;
            }
            msg.requestType = static_cast<RequestType>(myReqchar);

            // 3. Order ID (Strip the leading 'A' if present)
            std::string myOrderIdStr = fields_[2];
            if (!myOrderIdStr.empty() & (myOrderIdStr[0] == 'A' | myOrderIdStr[0] == 'a')) {
                myOrderIdStr = myOrderIdStr.substr(1); // Drop the 'A' and keep the rest
            }
            msg.orderId = std::stol(myOrderIdStr);

            // 4. Side
            char mySideChar = fields_[3][0];
            if (mySideChar != 'B' & mySideChar != 'S') {
                printf("Error with the messages that %s, can't tell which side of the Order.", line.c_str());
                return false;
            }
            msg.side = static_cast<Side>(mySideChar);

            // 5. Quantity
            msg.sz = static_cast<ordSz_t>(std::stol(fields_[4]));

            // 6. Price
            msg.px = std::stod(fields_[5]);
            
            int myDec = (msg.side)


              
        } catch (const std::exception& e) {
            // Catches conversion exceptions (std::invalid_argument or std::out_of_range)
            return false;
        }
    }
protected:
std::vector<std::string> fields_{};
MatchinEngine* me_;

    std::string trim(const std::string& str) {
        const size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        const size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    void dispactToMatchinEngineWithTemplate(const Message& aMsg) {
        switch (aMsg.requestType) {
            case RequestType::New:
                if (msg.side == Side::Buy) {
                    me_->template process<RequestType::New, Side::Buy>(aMsg);
                } else {
                    me_->template process<RequestType::New, Side::Sell>(aMsg);
                }
                break;
            case RequestType::Cancel:{
                me_->template process<RequestType::Cancel>(aMsg);
                break;
            }
            case RequestType::Amend:{
                if (msg.side == Side::Buy) {
                    me_->template process<RequestType::Amend, Side::Buy>(aMsg);
                } else {
                    me_->template process<RequestType::Amend, Side::Sell>(aMsg);
                }
                break;
            }
        }
    }
};
