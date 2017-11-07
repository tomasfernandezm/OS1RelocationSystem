# OS1 Dependencies 
The system was tested with Ubuntu 16.04 LTS.

## Used APT Packages
- build-essential 
- wget 
- git 
- libgtk2.0-dev 
- pkg-config 
- libavcodec-dev 
- libavformat-dev 
- libswscale-dev 
- python-dev 
- python-numpy 
- libtbb2 
- libtbb-dev 
- libjpeg-dev 
- libpng-dev 
- libtiff-dev 
- libjasper-dev 
- libdc1394-22-dev

## Used Libraries
- OpenCV 3.3.0
- Boost 1.64.0
- Pangolin
- Eigen 3.3.4
- Modified version of DBoW2 (included in the Thirdparty folder)
- Modified version of g2o (included in the Thirdparty folder)
- JSON lib: https://github.com/nlohmann/json

## DBoW2 and g2o installation
To install DBoW2 and g2o execute buildDependencies.sh in the scripts directory