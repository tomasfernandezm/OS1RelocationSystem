# OS1 Relocation System

Sistema de relocalización con monocular slam. Proyecto para la materia "Introducción a la Computacion" 
de la Facultad de Ingeniería de la Universidad Austral.

Está basado en 2 proyectos:
- ORB-SLAM2 de Raúl Mur Artal de la Universidad de Zaragoza https://github.com/raulmur/ORB_SLAM2
- os1 de Alejandro Silvestir de la Universidad Austral https://github.com/AlejandroSilvestri/os1

# Estructura

## Servidor en Node JS

## Servidor de ORB-SLAM2

### Set
El servidor de ORB-SLAM2 inicializa con un punto inicial desde el cual calculará 
todas las distancias. Este punto inicial es obtenido a través de una foto 
inicialmente setteada.

### Funcionamiento
Arranca la ejecución del programa esperando por el envío de la imagen de la posición
actual. Procesa la imagen y manda al servidor un string con el vector (x, y, z)
desde el punto inicial.
El z representa la distancia de frente, el x de costado y el y es perpendicular
al plano de la cámara. Como el y no es usado, se utiliza para control de errores.
