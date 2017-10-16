//
// Created by toams on 06/10/17.
//

#ifndef ORB_SLAM2_TCPSOCKETIMAGEDECODER_H
#define ORB_SLAM2_TCPSOCKETIMAGEDECODER_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <opencv2/opencv.hpp>

using boost::asio::ip::tcp;
using namespace std;
using namespace cv;

class TcpSocketImageDecoder {

private:
    static inline bool is_base64(unsigned char c);
    std::string base64_decode(std::string const& encoded_string);

public:
    cv::Mat receiveImage();
    void sendLocation(std::string host, int port, std::string message);
};


#endif //ORB_SLAM2_TCPSOCKETIMAGEDECODER_H
