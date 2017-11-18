/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file main
 * main prepara el "sensor" de imágenes monocular e inicia el bucle principal, enviando una imagen Mat por vez.
 * Previamente inicializa el sistema al construir el objeto SLAM, pasándole como argumentos las rutas al vocabulario y configuración,
 * y el modo MONOCULAR.
 */


#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <System.h>
#include <Viewer.h>
#include <Tracking.h>
#include <Video.h>
#include <include/TcpSocketImageDecoder.h>
#include <json.hpp>

using namespace std;
using namespace cv;
using json = nlohmann::json;

ORB_SLAM2::System *Sistema;
bool exitFlag = false;
bool loadedMap = false;
bool bogusImage = false;

/**
 * Cambios hechos:
 *
 * - Agregué el código para que apretando la tecla m, imprima la matriz de rototraslación del
 * frame actual solo si está inicializado ORB_SLAM.
 *
 * - Arreglé el error de compilación de la SparseMatrix de Eigen, cambiando el SparseMatrix::Index por un
 * int en el field que usa Sparse::Matrix
 *
 * - Agregué que cuando relocaliza, imprima la matriz de rototraslación.
 *
 */

void loadMap(string rutaMapa);
Mat getMatFromSocket();
void sendLocation(std::string host, int port, std::string message);
Mat operate(const Mat &image, const string &mapRoute);
string getVectorAsString(const Mat &vector);
Mat loadInitialMatrix(string initialImageLocation, string MapRoute);
Mat calculateLocation(const Mat &initialMatrix, const Mat &relocMatrix, const Mat &initialVector, double factor);

int main(int argc, char **argv) {

    ifstream ifs("/home/toams/facultad/OS1RelocationSystem/config.json");
    json jsonConfig = json::parse(ifs);


    const string rutaConfiguracionORB = jsonConfig["ORBConfigRoute"];
    const string rutaVocabulario = jsonConfig["VocRoute"];
    const string mapRoute = jsonConfig["MapRoute"];
    const string initialImageLocation = jsonConfig["InitialImageLocation"];
    const double meterFactor = jsonConfig["MeterFactor"];
    
    ORB_SLAM2::System SLAM(rutaVocabulario, rutaConfiguracionORB, ORB_SLAM2::System::MONOCULAR, true);
    Sistema = &SLAM;

    ORB_SLAM2::Video video;
    new thread(&ORB_SLAM2::Video::Run, &video);

    cv::Mat initialMatrix = loadInitialMatrix(initialImageLocation, mapRoute);
    cv::Mat initialVector = initialMatrix * initialMatrix.col(3);

    while(!exitFlag) {
        cout << "Recibiendo imagen" << endl;
        Mat img = getMatFromSocket();
	    if(!img.data) cout << "La imagen es corrupt" << endl;
        cout << "Imagen recibida" << endl;
        Mat relocMatrix = operate(img, mapRoute);
        cout << relocMatrix << endl;
        Mat displacementVector;
        if(!bogusImage){
            displacementVector = calculateLocation(initialMatrix, relocMatrix, initialVector, meterFactor);
            displacementVector.at<float>(1, 0) = 1;
        }else{
            displacementVector = relocMatrix;
        }
        string message = getVectorAsString(displacementVector);
        cout << "The resultant vector is: " + message << endl;
        sendLocation("localhost", 7001, message);
    }

    cout << "Invocando shutdown." << endl;
    exit(0);

}

Mat operate(const Mat &image, const string &mapRoute){

    bool operate = true;
    Mat result;
    int maxFrames = 20;
    while (operate && maxFrames != 0) {
        // Pass the image to the SLAM system
        (*Sistema).TrackMonocular(image, 1);

        if ((*Sistema).mpTracker->mState == 2 && (*Sistema).mpTracker->mbOnlyTracking) {
            result = (*Sistema).mpTracker->mCurrentFrame.mTcw.inv();
            operate = false;
            bogusImage = false;
        }

        // Ver si hay señal para cargar el mapa, que debe hacerse desde este thread
        if (!loadedMap) {
            cout << "Cargando mapa" << endl;
            loadMap(mapRoute);
            loadedMap = true;
        }
        if((*Sistema).mpViewer->salir) {
            exitFlag = true;
            cout << "Quiero salir" << endl;
        }
        maxFrames--;
    }
    if(maxFrames == 0){
        result = Mat::zeros(3, 1, CV_32F);
        result.at<float>(0, 0) = 0;
        result.at<float>(1, 0) = -1;
        result.at<float>(2, 0) = 0;
        bogusImage = true;
    }
    return result;
}

cv::Mat calculateLocation(const Mat &initialMatrix, const Mat &relocMatrix,
                          const Mat &initialVector, const double factor){
    Mat relocVector = initialMatrix * (relocMatrix.col(3));
    Mat resultantVector = relocVector - initialVector;
    cout << resultantVector << endl;
    resultantVector = resultantVector * factor;
    return resultantVector;
}

string getVectorAsString(const Mat &vector){
    float x = vector.at<float>(0, 0);
    float y = vector.at<float>(1, 0);
    float z = vector.at<float>(2, 0);
    return to_string(x) + " " + to_string(y) + " " + to_string(z);
}

Mat loadInitialMatrix(const string initialImageLocation, const string mapRoute){
    Mat image = imread(initialImageLocation, CV_LOAD_IMAGE_GRAYSCALE);
    Mat result = operate(image, mapRoute);
    cout << result << endl;
    return result;
}

void loadMap(const string rutaMapa) {
    (*Sistema).mpViewer->cargarMapa = false;

    (*Sistema).mpTracker->mState = ORB_SLAM2::Tracking::NOT_INITIALIZED;
    // El reset subsiguiente requiere que LocalMapping no esté pausado.
    (*Sistema).mpLocalMapper->Release();

    // Limpia el mapa de todos los singletons
    (*Sistema).mpTracker->Reset();
    // En este punto el sistema está reseteado.

    // Espera a que se detenga LocalMapping y  Viewer
    (*Sistema).mpLocalMapper->RequestStop();
    (*Sistema).mpViewer->RequestStop();

    //FILE *f = popen("zenity --file-selection", "r");
    //fgets(charchivo, 1024, f);

    while (!(*Sistema).mpLocalMapper->isStopped()) usleep(1000);
    while (!(*Sistema).mpViewer->isStopped()) usleep(1000);

    //std::string nombreArchivo(charchivo);
    //nombreArchivo.pop_back();	// Quita el \n final
    cout << "Abriendo archivo " << rutaMapa << endl;
    (*Sistema).serializer->mapLoad(rutaMapa);
    cout << "Mapa cargado." << endl;


    (*Sistema).mpTracker->mState = ORB_SLAM2::Tracking::LOST;

    // Reactiva viewer.  No reactiva el mapeador, pues el sistema queda en sólo tracking luego de cargar.
    (*Sistema).mpViewer->Release();

    // Por las dudas, es lo que hace Tracking luego que el estado pase a LOST.
    // Como tiene un mutex, por las dudas lo invoco después de viewer.release.
    (*Sistema).mpFrameDrawer->Update((*Sistema).mpTracker);
    (*Sistema).mpTracker->mbOnlyTracking = true;
}

cv::Mat getMatFromSocket() {
    TcpSocketImageDecoder tcid;
    return tcid.receiveImage();
}

void sendLocation(std::string host, int port, std::string message) {
    TcpSocketImageDecoder tcid;
    tcid.sendLocation(host, port, message);
}
