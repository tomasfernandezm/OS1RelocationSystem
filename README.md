# OS1 Relocation System

Sistema de relocalización con monocular slam. Proyecto para la materia "Introducción a la Computacion" 
de la Facultad de Ingeniería de la Universidad Austral.

Está basado en 2 proyectos:
- ORB-SLAM2 de Raúl Mur Artal de la Universidad de Zaragoza https://github.com/raulmur/ORB_SLAM2
- os1 de Alejandro Silvestir de la Universidad Austral https://github.com/AlejandroSilvestri/os1

## Estructura

### Servidor en Node JS
El servidor de Node JS provee la webapp por la cual se accede a la aplicación. Por la webapp se sube una foto a el servidor. Cuando llega, este la manda al servidor de OS1 y espera la respuesta. Cuando obtiene la respuesta le envía al usuario una imagen de un mapa con un punto en el.

### Servidor de OS1

Utiliza CMake como build tool para el código C++

#### Set
El servidor de OS1 inicializa con un punto inicial desde el cual calculará 
todas las distancias. Este punto inicial es obtenido a través de una foto 
inicialmente setteada.

#### Funcionamiento
Arranca la ejecución del programa esperando por el envío de la imagen de la posición
actual. Procesa la imagen y manda al servidor un string con el vector (x, y, z)
desde el punto inicial.
El z representa la distancia de frente, el x de costado y el y es perpendicular
al plano de la cámara. Como el y no es usado, se utiliza para control de errores.

## Tecnologías utilizadas.
- Express y Node JS: para la webapp
- Socket: comunicación entre los 2 servicios.
- OS1: procesamiento de imagen y localización.

## Cambios necesarios para el uso particular

### Webapp
1. Cambiar el mapa a mostrar
2. Adecuar el lugar de inicialización en el mapa

### OS1 System
1. Adecuar las rutas de las dependencias Boost en el CMakeLists.txt
2. En el config.json, adecuar las rutas de:
	- Archivo de configuración de ORB-SLAM
	- Archivo de Vocabulario
	- Archivo del mapa de ORB-SLAM a utilizar
	- Imagen del punto de vista del punto inicial
También debe estar el factor de conversión a metros que utiliza la aplicación para pasar de las unidades de OS1 a metros.


## Observaciones
- La cámara a utilizar para sacar las fotos usadas en el sistema tienen que haber sido sacadas con una cámara calibrada con openCV.
- En el archivo de configuración de ORB-SLAM debe ir la calibración de la cámara.
- Para sacar el factor de distancia de la cámara, se debe calcular la distancia en unidades de ORB-SLAM de 2 puntos que se sepa la distancia entre ellos. La división de las 2 distancias dará el factor de distancia.
