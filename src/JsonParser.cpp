//
// Created by toams on 29/10/17.
//

#include <JsonParser.h>
#include <fstream>

json readJson(char* route){
    std::ifstream ifs(route);
    return json::parse(ifs);
}