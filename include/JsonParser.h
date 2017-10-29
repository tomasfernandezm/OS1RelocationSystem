//
// Created by toams on 29/10/17.
//

#ifndef ORB_SLAM2_JSONPARSER_H
#define ORB_SLAM2_JSONPARSER_H

#include <json.hpp>

using json = nlohmann::json;

class JsonReader {

public:
    json readJson(char* route);
};


#endif //ORB_SLAM2_JSONPARSER_H
