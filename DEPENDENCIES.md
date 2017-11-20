# Dependencias usadas por el OS1
El sistema fue testeado utilizando Ubuntu 16.04 LTS.

## Paquetes usados instalados mediante APT
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

## Librerías Utilizadas
- OpenCV 3.3.0
- Boost 1.64.0
- Pangolin
- Eigen 3.3.4
- DBoW2 (versión modificada, incluida en la carpeta Thirdparty)
- g2o (versión modificada, incluida en la carpeta Thirdparty)
- JSON lib: https://github.com/nlohmann/json

## DBoW2 and g2o installation
Para instalar DBoW2 y g2o solo hay que ejecutar el script "buildDependencies.sh" en el directorio "scripts"
