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
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <System.h>
#include <Viewer.h>
#include <Tracking.h>
#include <FrameDrawer.h>
#include <Serializer.h>
#include <Video.h>
#include <include/TcpSocketImageDecoder.h>

using namespace std;

ORB_SLAM2::System *Sistema;

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

cv::Mat processImage(cv::Mat initial, cv::Mat relocated, int factor);

void loadMap(char *rutaMapa);

cv::Mat getMatFromSocket();

void sendLocation(std::string host, int port, std::string message);

int main(int argc, char **argv) {

    cout << "Iniciando ORB-SLAM.  Línea de comando:" << endl
         << "os1 [archivo de configuración yaml [ruta al archivo de video]]\nSin argumentos para usar la webcam, con configuración en webcam.yaml"
         << endl;

    // Parámetros de la línea de comando

    const char *rutaConfiguracionORB = "/home/toams/facultad/os1/webcamNacho.yaml";
    const char *rutaVocabulario = "/home/toams/facultad/os1/orbVoc.bin";

    // Inicializa el sistema SLAM.
    // Mi versión de archivo binario con el vocabulario, que carga mucho más rápido porque evita el análisis sintáctico.
    ORB_SLAM2::System SLAM(rutaVocabulario, rutaConfiguracionORB, ORB_SLAM2::System::MONOCULAR, true);

    // Puntero global al sistema singleton
    Sistema = &SLAM;

    ORB_SLAM2::Viewer *visor = SLAM.mpViewer;

    // Arranca el hilo de Video
    ORB_SLAM2::Video video;
    new thread(&ORB_SLAM2::Video::Run, &video);

    cout << "Recibiendo imagen" << endl;
    cv::Mat img = getMatFromSocket();
    cout << "Imagen recibida" << endl;

    bool hasSentMatrix = false;
    bool mapaCargado = false;
    cv::Mat initialMatrix;
    bool operate = true;

    while (operate) {

        // Pass the image to the SLAM system
        if (SLAM.mpTracker->mState == 2
            && SLAM.mpTracker->mbOnlyTracking && !hasSentMatrix) {
            hasSentMatrix = !hasSentMatrix;
            cv::Mat relocationMatrix = SLAM.mpTracker->mCurrentFrame.mTcw.inv();
            cout << "El vector de traslación es: " << endl;
            /**
             * vector traslación = matriz inicial * (cuarta columna de matriz de relocalización)
             */
            cv::Mat vector = initialMatrix * (relocationMatrix.col(3)).rowRange(0, 2);
            cout << vector << endl;

            float x = vector.at<float>(0, 0);
            float y = vector.at<float>(1, 0);
            float z = vector.at<float>(2, 0);

            std::string str = to_string(x) + " " + to_string(y) + " " + to_string(z);
            /* Escribir respuesta acá */
            sendLocation("localhost", 7001, str);
            /* Coordenada Z para adelante, X para la derecha e Y para abajo*/
        }

        SLAM.TrackMonocular(img, (double) video.posCuadro);

        // Ver si hay señal para cargar el mapa, que debe hacerse desde este thread
        if (!mapaCargado) {
            loadMap("/home/toams/facultad/os1/Mapa_Pasillo_A10.osMap");
            mapaCargado = true;
        }
        if(visor->salir) operate = !operate;
    }
    cout << "Invocando shutdown." << endl;

    exit(0);
    // Stop all threads
    SLAM.Shutdown(); /** No sirve de nada */

    cout << "Terminado." << endl;

    return 0;
}

void loadMap(char *rutaMapa) {
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
    std::string nombreArchivo(rutaMapa);
    cout << "Abriendo archivo " << nombreArchivo << endl;
    (*Sistema).serializer->mapLoad(nombreArchivo);
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
