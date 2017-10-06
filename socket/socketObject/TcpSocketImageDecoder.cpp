//
// Created by toams on 06/10/17.
//

#include "TcpSocketImageDecoder.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <opencv2/opencv.hpp>

cv::Mat TcpSocketImageDecoder::receiveImage() {
    try
    {
        boost::asio::io_service io_service;

        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 7000));

        string decoded_string = "";
        bool transmissionFinished = false;
        tcp::socket socket(io_service);
        acceptor.accept(socket);
        while(!transmissionFinished) {

            boost::array<char, 1024> buf;
            boost::system::error_code error;

            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            if(len != 0){
                string encoded_string = buf.data();
                decoded_string += base64_decode(encoded_string);
            }else{
                transmissionFinished = true;
            }
        }
        vector<uchar> data(decoded_string.begin(), decoded_string.end());

        cv::Mat img = imdecode(data, IMREAD_UNCHANGED);

        // left for Debugging purposes
        imshow("Image", img);
        waitKey();

        return img;

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

bool TcpSocketImageDecoder::is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string TcpSocketImageDecoder::base64_decode(std::string const &encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}