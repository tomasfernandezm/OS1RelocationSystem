//
// Created by toams on 06/10/17.
//

#ifndef ORB_SLAM2_TCPSOCKETIMAGEDECODER_H
#define ORB_SLAM2_TCPSOCKETIMAGEDECODER_H

using boost::asio::ip::tcp;

class TcpSocketImageDecoder {

private:
    static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "0123456789+/";
    static inline bool is_base64(unsigned char c);
    std::string base64_decode(std::string const& encoded_string);

public:
    cv::Mat receiveImage();
};


#endif //ORB_SLAM2_TCPSOCKETIMAGEDECODER_H
