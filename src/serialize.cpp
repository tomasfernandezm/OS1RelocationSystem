/**
 * @file:serialize.cpp
 *
 * Concentra la definición de los métodos serialize de orb-slam2, para facilitar su adaptación a los muchos branches.
 * La declaración se realiza en el archivo hpp de cada clase, de este modo, en public:
 *
 * friend class boost::serialization::access;
 * template<class Archivo> void serialize(Archivo&, const unsigned int);
 *
 * y un constructor por defecto, con la debida inicialización de atributos no serializables.
 *
 * Sus hpp deben agregar includes:
 *
 *
 * Las implementaciones pueden distinguir la dirección del proceso (guardado o carga) con la macro GUARDANDO(ar).
 *
 *
 * Las clases a serializar son:
 * - Map
 * - MapPoint
 * - KeyFrame
 * - KeyFrameDatabase
 * - Mat
 * - KeyPoint
 *
 * Map inicia la carga sobre el objeto existente.  KeyFrameDatabase se reconstruye, no requiere serialización.
 * Se serializan KeyFrame y MapPoint con método serialize, y Mat y KeyPoint con función serialize.
 *
 *
 * Este archivo se divide en las siguietes partes:
 *
 * - includes: boost::serialization, y clases de orb-slam2
 * - defines: macros y typenames para facilitar el uso de boost::serialize
 * - funciones serialize: para objetos de opencv, que no se pueden modificar
 * - métodos serialize: para los objetos de orb-slam2
 *
 * El código de cada serialize fue adaptado de https://github.com/MathewDenny/ORB_SLAM2
 *
 *  Created on: 8/11/2016
 *      Author: alejandro
 *
 */

#include <typeinfo>

// boost::serialization
#include <boost/serialization/serialization.hpp>
#include <boost/archive/tmpdir.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// Clases de orb-slam2
#include "Map.h"	// Incluye MapPoint y KeyFrame, éste incluye a KeyFrameDatabase
#include "KeyFrame.h"
#include "KeyFrameDatabase.h"
#include "MapPoint.h"
#include "System.h"
#include "Tracking.h"

extern ORB_SLAM2::System* Sistema;

// Defines
#define TIPOS_ARCHIVOS(FORMATO) \
	typedef boost::archive::FORMATO##_iarchive ArchivoEntrada;\
	typedef boost::archive::FORMATO##_oarchive ArchivoSalida;
// Aquí se define el tipo de archivo: binary, text, xml...
TIPOS_ARCHIVOS(binary)

/**
 * La bifurcación consiste en distinguir si serializa carga o guarda.  Se activa definiendo BIFURCACION, pero requiere c++11 para typeinfo
 * En un método serialize(Archivo...) la macro GUARDANDO(Archivo) devuelve true si se está guardando, false si se está cargando.
 *
 * Comentar la siguiente línea para desactivar la capacidad de bifurcación
 */
#define BIFURCACION
#ifdef BIFURCACION
	#include <typeinfo>
	#define CARGANDO(ARCHIVO) ( typeid(ARCHIVO) == typeid(ArchivoEntrada) )
#endif

/** Instanciación explícita*/
#define INST_EXP(CLASE)\
template void CLASE::serialize<ArchivoEntrada>(ArchivoEntrada&,const unsigned int);\
template void CLASE::serialize<ArchivoSalida>(ArchivoSalida&,const unsigned int);




/**
 * OpencCV KeyPoint y Mat
 * Copiado de https://github.com/MathewDenny/ORB_SLAM2 MapPoint.h
 */

// Serialización no intrusiva fuera de la clase: Mat, KeyPoint
BOOST_SERIALIZATION_SPLIT_FREE(::cv::Mat)
namespace boost{namespace serialization{


// KeyPoint
template<class Archive> void serialize(Archive & ar, cv::KeyPoint& kf, const unsigned int version){
  ar & kf.angle;	// Usado en matcher
  //ar & kf.class_id;	// No usado
  ar & kf.octave;	// Usado en LocalMapping y otros
  //ar & kf.response;	// Solamente se usa durante la extracción en ORBextractor::DistributeOctTree
  ar & kf.pt.x;
  ar & kf.pt.y;
}

// Mat: save
template<class Archive> void save(Archive & ar, const ::cv::Mat& m, const unsigned int version){
  size_t elem_size = m.elemSize();
  size_t elem_type = m.type();

  ar & m.cols;
  ar & m.rows;
  ar & elem_size;
  ar & elem_type;

  const size_t data_size = m.cols * m.rows * elem_size;
  ar & boost::serialization::make_array(m.ptr(), data_size);
}

// Mat: load
template<class Archive> void load(Archive & ar, ::cv::Mat& m, const unsigned int version){
  int cols, rows;
  size_t elem_size, elem_type;

  ar & cols;
  ar & rows;
  ar & elem_size;
  ar & elem_type;

  m.create(rows, cols, elem_type);
  size_t data_size = m.cols * m.rows * elem_size;

  ar & boost::serialization::make_array(m.ptr(), data_size);
}


}}


// Definiciones de los métodos serialize de las clases Map, MapPoint y KeyFrame
namespace ORB_SLAM2{


// Map: usando set ========================================================
template<class Archivo> void Map::serialize(Archivo& ar, const unsigned int version){
	ar & mspMapPoints;
    ar & mspKeyFrames;
    ar & mvpKeyFrameOrigins;
    ar & const_cast<long unsigned int &> (mnMaxKFid);
    //ar & mvpReferenceMapPoints;	// Es un vector efímero, no hace falta guardarlo, se genera durante el tracking normal.
}
INST_EXP(Map)

void Map::save(char* archivo){
	// Abre el archivo
	std::ofstream os(archivo);
	ArchivoSalida ar(os, boost::archive::no_header);

	// Guarda mappoints y keyframes
	serialize<ArchivoSalida>(ar, 0);
}

void Map::load(char* archivo){
	// A este punto se llega co el mapa limpio y los sistemas suspendidos para no interferir.

	// Abre el archivo
	std::ifstream is(archivo);
	ArchivoEntrada ar(is, boost::archive::no_header);

	// Carga mappoints y keyframes en Map
	serialize<ArchivoEntrada>(ar, 0);

	/*
	 * Reconstruye KeyFrameDatabase luego que se hayan cargado todos los keyframes.
	 * Recorre la lista de keyframes del mapa para reconstruir su lista invertida mvInvertedFile con el método add.
	 * No puede asignar el puntero mpKeyFrameDB del keyframe porque es protegido.  Esto se hace en el constructor de keyframe.
	 *
	 * kfdb Única instancia de KeyFrameDatabase, cuyo mvInvertedFile se reconstruirá
	 * mapa Única instancia del mapa de cuyo mspKeyFrames se recorrerán los keyframes
	 */
	KeyFrameDatabase* kfdb = Sistema->mpKeyFrameDatabase;//Map::mpKeyFrameDatabase;
	//kfdb->clear();
	long unsigned int maxId = 0;

	// Recorre todos los keyframes cargados
	for(KeyFrame* kf : mspKeyFrames){
		// Agrega el keyframe a la base de datos de keyframes
		kfdb->add(kf);

		// UpdateConnections para reconstruir el grafo
		//kf->UpdateConnections();
	}
	//KeyFrame::nNextId = maxId + 1;
	KeyFrame::nNextId = mnMaxKFid + 1;	// mnMaxKFid es el id del último keyframe agregado al mapa

	//for(MapPoint* mp : mspMapPoints){
		// Reconstruye normal, profundidad y alguna otra cosa.  Requiere cargados los keyframes.
		//mp->UpdateNormalAndDepth();
	//}

	// Next id para MapPoint
	maxId = 0;
	for(auto mp : mspMapPoints) maxId = max(maxId, mp->mnId);
	MapPoint::nNextId = maxId + 1;


	/*
	// Inicializa el registro de trayectoria.
	Tracking* t = Sistema->mpTracker;
	cv::Mat uno = cv::Mat::eye(4, 4, CV_32F);
	t->mlRelativeFramePoses.push_back(uno);
	t->mlpReferences.push_back(NULL);
	t->mlFrameTimes.push_back(0.0);
	 */

}


/*
// Inicializa punteros a singleton
Map* Map::mpMap = NULL;
KeyFrameDatabase* Map::mpKeyFrameDatabase = NULL;
ORBVocabulary* Map::mpVocabulary = NULL;
//Tracking* Map::mpTracker = NULL;
*/



// MapPoint: usando map ========================================================

/**
 * Constructor por defecto de MapPoint para el serializador.
 * Se encarga de inicializar las variables const, para que el compilador no chille.
 */
MapPoint::MapPoint():
nObs(0), mnTrackReferenceForFrame(0),
mnLastFrameSeen(0), mnBALocalForKF(0), mnFuseCandidateForKF(0), mnLoopPointForKF(0), mnCorrectedByKF(0),
mnCorrectedReference(0), mnBAGlobalForKF(0),mnVisible(1), mnFound(1), mbBad(false),
mpReplaced(static_cast<MapPoint*>(NULL)), mfMinDistance(0), mfMaxDistance(0), mpMap(Sistema->mpMap)//Map::mpMap)
{}

/**
 * Serializador de MapPoint
 */
template<class Archivo> void MapPoint::serialize(Archivo& ar, const unsigned int version){

	//if(mbBad) return;	// Evita guardar puntos inútiles

	ar & const_cast<long unsigned int &> (mnId);
	ar & const_cast<cv::Mat &> (mWorldPos);
	ar & const_cast<long int &> (mnFirstKFid);
	ar & mpRefKF;
	ar & const_cast<int &> (nObs);
	ar & mObservations;
	ar & const_cast<cv::Mat &> (mDescriptor);	// Reconstruíble con mp->ComputeDistinctiveDescriptors() (quizás requiere primero haber cargado los keyframes.), pero significa mucho trabajo para pocos datos.

	ar & const_cast<int &> (mnVisible);
	ar & const_cast<int &> (mnFound);
	ar & const_cast<bool &> (mbBad);

	//Posiblemente efímeros
	ar & const_cast<float &> (mTrackProjX);
	ar & const_cast<float &> (mTrackProjY);
	ar & const_cast<float &> (mTrackProjXR);
	ar & const_cast<bool &> (mbTrackInView);
	ar & const_cast<int &> (mnTrackScaleLevel);
	ar & const_cast<float &> (mTrackViewCos);
	ar & const_cast<long unsigned int &> (mnTrackReferenceForFrame);
	ar & const_cast<long unsigned int &> (mnLastFrameSeen);
	ar & const_cast<long unsigned int &> (mnBALocalForKF);
	ar & const_cast<long unsigned int &> (mnFuseCandidateForKF);
	ar & const_cast<long unsigned int &> (mnLoopPointForKF);
	ar & const_cast<long unsigned int &> (mnCorrectedByKF);
	ar & const_cast<long unsigned int &> (mnCorrectedReference);
	ar & const_cast<cv::Mat &> (mPosGBA);
	ar & const_cast<long unsigned int &> (mnBAGlobalForKF);


	//ar & nNextId;	// Propiedad estática, se ajusta luego de cargar, en Map::load
	//ar & const_cast<long int &> (mnFirstFrame);	//Inútil

	// Reconstruíbles con mp->UpdateNormalAndDepth();
	ar & const_cast<cv::Mat &> (mNormalVector); // Reconstruíble con mp->UpdateNormalAndDepth();
	ar & const_cast<float &> (mfMinDistance); // Reconstruíble con mp->UpdateNormalAndDepth();
	ar & const_cast<float &> (mfMaxDistance); // Reconstruíble con mp->UpdateNormalAndDepth();



	if(CARGANDO(ar)){
		//UpdateNormalAndDepth();  // luego de cargar los keyframes
	}
}
INST_EXP(MapPoint)

/*
 * De https://github.com/shomin/ORB_SLAM2/blob/master/src/Map.cc#L349 load:

 // MapPoints descriptors
  for(auto mp: amp) {
	mp->ComputeDistinctiveDescriptors();
	mp->UpdateNormalAndDepth();
  }


 */



// KeyFrame ========================================================
/**
 * Constructor por defecto para KeyFrame
 * Se ocupa de inicializar los atributos const, para que el compilador no chille.
 * Entre ellos inicializa los atributos no serializables (todos punteros a singletons).
 * Luego serialize se encarga de cambiarle los valores, aunque sean const.
 */
KeyFrame::KeyFrame():
	// Públicas
    mnFrameId(0),  mTimeStamp(0.0), mnGridCols(FRAME_GRID_COLS), mnGridRows(FRAME_GRID_ROWS),
    mfGridElementWidthInv(Frame::mfGridElementWidthInv),
    mfGridElementHeightInv(Frame::mfGridElementHeightInv),

    mnTrackReferenceForFrame(0), mnFuseTargetForKF(0), mnBALocalForKF(0), mnBAFixedForKF(0),
    mnLoopQuery(0), mnLoopWords(0), mnRelocQuery(0), mnRelocWords(0), mnBAGlobalForKF(0),

    fx(Frame::fx), fy(Frame::fy), cx(Frame::cx), cy(Frame::cy), invfx(Frame::invfx), invfy(Frame::invfy),
    mbf(Sistema->mpTracker->mCurrentFrame.mbf),
    mb(Sistema->mpTracker->mCurrentFrame.mb),
    mThDepth(Sistema->mpTracker->mCurrentFrame.mThDepth),
    N(0), mnScaleLevels(Sistema->mpTracker->mCurrentFrame.mnScaleLevels),
    mfScaleFactor(Sistema->mpTracker->mCurrentFrame.mfScaleFactor),
    mfLogScaleFactor(Sistema->mpTracker->mCurrentFrame.mfLogScaleFactor),
    mvScaleFactors(Sistema->mpTracker->mCurrentFrame.mvScaleFactors),
    mvLevelSigma2(Sistema->mpTracker->mCurrentFrame.mvLevelSigma2),
    mvInvLevelSigma2(Sistema->mpTracker->mCurrentFrame.mvInvLevelSigma2),
    mnMinX(Frame::mnMinX), mnMinY(Frame::mnMinY), mnMaxX(Frame::mnMaxX), mnMaxY(Frame::mnMaxY),
    mK(Sistema->mpTracker->mCurrentFrame.mK),
    // Protegidas:
    mpKeyFrameDB(Sistema->mpKeyFrameDatabase),
    mpORBvocabulary(Sistema->mpVocabulary),
    mbFirstConnection(true),
    mpMap(Sistema->mpMap)
{}

/**
 * Serializador para KeyFrame
 * No guarda mpKeyFrameDB, que se debe asignar de otro modo.
 */
template<class Archive> void KeyFrame::serialize(Archive& ar, const unsigned int version){

	//if(mbToBeErased || mbBad) return;

	ar & mnId;	//ar & const_cast<long unsigned int &> (mnId);
	ar & const_cast<cv::Mat &> (Tcw);
	ar & const_cast<std::vector<cv::KeyPoint> &> (mvKeysUn);
	ar & const_cast<int &> (N);	// Reconstruible N=mvKeysUn.size()
	ar & const_cast<cv::Mat &> (mDescriptors);
	ar & const_cast<bool &> (mbBad);

	ar & mvpMapPoints;

	// Conexiones del grafo: no es reconstruible, UpdateConections sólo lo inicializa, pero el grafo sigue cambiando, agregando hijos.
	ar & mConnectedKeyFrameWeights;
	ar & mvpOrderedConnectedKeyFrames;
	ar & const_cast<std::vector<int> &>(mvOrderedWeights);
	ar & const_cast<bool &> (mbFirstConnection);
	ar & mpParent;
	ar & mspChildrens;
	ar & mspLoopEdges;


	// Tienen el mismo valor en todas las instancias
	//ar & const_cast<float &> (mbf);	// Mismo valor en todos los keyframes
	//ar & const_cast<float &> (mb);	// Mismo valor en todos los keyframes
	//ar & const_cast<float &> (mThDepth);	// Mismo valor en todos los keyframes
	//ar & const_cast<int &> (mnScaleLevels);	// Mismo valor en todos los keyframes
	//ar & const_cast<float &> (mfScaleFactor);	// Mismo valor en todos los keyframes
	//ar & const_cast<float &> (mfLogScaleFactor);	// Mismo valor en todos los keyframes
	//ar & const_cast<std::vector<float> &> (mvScaleFactors);	// Mismo valor en todos los keyframes
	//ar & const_cast<std::vector<float> &> (mvLevelSigma2);	// Mismo valor en todos los keyframes
	//ar & const_cast<std::vector<float> &> (mvInvLevelSigma2);	// Mismo valor en todos los keyframes
	ar & const_cast<cv::Mat &> (mK);	// Mismo valor en todos los keyframes

	/*// El constructor toma sus valores de variables estáticas de Frame
	ar & const_cast<float &>  (mfGridElementWidthInv);	// Mismo valor en todos los keyframes
	ar & const_cast<float &>  (mfGridElementHeightInv);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (fx);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (fy);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (cx);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (cy);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (invfx);	// Mismo valor en todos los keyframes
	ar & const_cast<float &> (invfy);	// Mismo valor en todos los keyframes
	ar & const_cast<int &> (mnMinX);	// Mismo valor en todos los keyframes
	ar & const_cast<int &> (mnMinY);	// Mismo valor en todos los keyframes
	ar & const_cast<int &> (mnMaxX);	// Mismo valor en todos los keyframes
	ar & const_cast<int &> (mnMaxY);	// Mismo valor en todos los keyframes
	*/


	/*// Pueden ser inútiles
	ar & const_cast<cv::Mat &> (mTcwBefGBA);	// Parece efímera
	ar & const_cast<long unsigned int &> (mnBAGlobalForKF);	// Parece efímera
	ar & const_cast<bool &> (mbNotErase);	// Parece efímero
	ar & const_cast<bool &> (mbToBeErased);	// Parece efímero
	 */




	//ar & const_cast<int &> (mnGridCols);	// Mismo valor en todos los keyframes, de una constante
	//ar & const_cast<int &> (mnGridRows);	// Mismo valor en todos los keyframes, de una constante
	//ar & nNextId;	// Propiedad de clase, se ajusta al final de load.
	//ar & const_cast<long unsigned int &> (mnFrameId);	// Inútil
	//ar & const_cast<double &> (mTimeStamp);		// Inútil
	//ar & const_cast<long unsigned int &> (mnTrackReferenceForFrame);	// Efímero, inicializado en el constructor
	//ar & const_cast<long unsigned int &> (mnFuseTargetForKF);	// Efímero, inicializado en el constructor
	//ar & const_cast<long unsigned int &> (mnBALocalForKF);	// Efímero, inicializado en el constructor
	//ar & const_cast<long unsigned int &> (mnBAFixedForKF);	// Efímero, inicializado en el constructor
	//ar & const_cast<long unsigned int &> (mnLoopQuery);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<int &> (mnLoopWords);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<float &> (mLoopScore);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<long unsigned int &> (mnRelocQuery);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<int &> (mnRelocWords);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<float &> (mRelocScore);	// Efímero, no hace falta guardar, se inicializa en cero.
	//ar & const_cast<cv::Mat &> (mTcwGBA);	// Efímera
	//ar & const_cast<std::vector<cv::KeyPoint> &> (mvKeys);	// No se usan
	//ar & const_cast<std::vector<float> &> (mvuRight);	// No usado
	//ar & const_cast<std::vector<float> &> (mvDepth);	// No usado
	//ar & const_cast<cv::Mat &> (mTcp);	// Inútil
	//ar & const_cast<float &> (mHalfBaseline);	// Sólo para visualización
	//ar & const_cast<cv::Mat &> (Cw);


	// Reconstruible con SetPose Tcw
	//ar & const_cast<cv::Mat &> (Twc);	// Reconstruible con SetPose Tcw
	//ar & const_cast<cv::Mat &> (Ow);	// Reconstruible con SetPose Tcw

	// Grilla reconstruible
	//ar & mGrid;	// Reconstruible



	// Sólo load
	if(CARGANDO(ar)){
		// Reconstrucciones
		//int n = const_cast<int &> (N);
		//n = mvKeysUn.size();

		ComputeBoW();	// Sólo actúa al cargar, porque si el keyframe ya tiene los datos no hace nada.
		SetPose(Tcw);
		// UpdateConnections sólo se puede invocar luego de cargados todos los keyframes


		// Reconstruir la grilla

		// Dimensiona los vectores por exceso
		std::vector<std::size_t> grid[FRAME_GRID_COLS][FRAME_GRID_ROWS];
	    int nReserve = 0.5f*N/(FRAME_GRID_COLS*FRAME_GRID_ROWS);
	    for(unsigned int i=0; i<FRAME_GRID_COLS;i++)
	        for (unsigned int j=0; j<FRAME_GRID_ROWS;j++)
	            grid[i][j].reserve(nReserve);

	    for(int i=0;i<N;i++){
	        const cv::KeyPoint &kp = mvKeysUn[i];
	        int posX = round((kp.pt.x-mnMinX)*mfGridElementWidthInv);
	        int posY = round((kp.pt.y-mnMinY)*mfGridElementHeightInv);

	        //Keypoint's coordinates are undistorted, which could cause to go out of the image
	        if(!(posX<0 || posX>=FRAME_GRID_COLS || posY<0 || posY>=FRAME_GRID_ROWS))
	            grid[posX][posY].push_back(i);
	    }

	    // Copia al vector final.  No sé si esta parte agrega valor.
	    mGrid.resize(mnGridCols);
	    for(int i=0; i<mnGridCols;i++)
	    {
	        mGrid[i].resize(mnGridRows);
	        for(int j=0; j<mnGridRows; j++)
	            mGrid[i][j] = grid[i][j];
	    }
	}

	// En load hay que construir mGrid con un método como Frame::AssignFeaturesToGrid
}
INST_EXP(KeyFrame)

}
