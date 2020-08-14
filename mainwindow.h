#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <QtWidgets/QFileDialog>
#include <imgviewer.h>
#include <stdio.h>

using namespace std;


using namespace cv;

namespace Ui {
    class MainWindow;
}


struct pixelPoint
{
    Point p;                  //Punto (Coordenada)
    float valor;              //Valor de HARRIS

};

struct matchPoints{
    Point p1,p2;              //puntos para img izq e img der
    bool match = false;       //true si dos esquinas coinciden
};

//Comparador para ordenación de esquinas
struct ComparadorListaEsquinas {
  bool operator() (pixelPoint i,pixelPoint j) { return (i.valor>j.valor);}
};

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer timer;

    VideoCapture *cap;
    ImgViewer *visorS, *visorD, *visorII, *visorID;
    Mat colorImage, grayImage;
    Mat destColorImage, destGrayImage;
    bool winSelected;
    Rect imageWindow;

    //===========================  Variables para entrega 03 =========================//
    std::vector<pixelPoint> listaEsquinasI,listaEsquinasD;
    Mat imgEsquinasI,imgEsquinasD, imgCopy,imgCopyD;

    //===========================  Variables para entrega 04 =========================//
    QImage *imgS, *imgD;

    //===========================  Variables para entrega 05 =========================//
    std::vector<matchPoints> listaMatches;
    Mat imgPuntosFijos,imgDisparidad;
    Mat imageII, imageID;

    //Point coorPrimerPixel, primer NO añadido -> arriba a abajo y izquierda a derecha
      struct region{
            Point coorPrimerPixel;    //Coordenada del primer pixel añadido a la región
            int numPuntosReg=0;       //Contador de número de puntos de la región
            uchar gris;               //Nivel de gris para visualizar la región
            Vec3b color;              //COLORES: 0-BLUE | 1-GREEN | 2-RED
            std::vector <Point> listaPtsFrontera; //Lista de puntos frontera de región
            int disparidad;
            float dMedia;             //Disparidad media para la fase 2 de disparidad
            int nfijos;  //PILAR (02/05)
            region() {}
      };

      //imgRegiones y listRegiones están enlazadas por el identificador de región
      //No hay que guardar el identificador de regiones como atributo. El índice de la lista
      //es el identificador.
      std::vector<region> listRegiones;
      Mat imgRegiones;
      Mat cannyRegion;
      Mat mascara;
      int ANCHOoriginal;
      bool disparidadCalculada;
      //===================================================================================//

public slots:
    void compute();
    void start_stop_capture(bool start);
    void change_color_gray(bool color);
    void selectWindow(QPointF p, int w, int h);
    void deselectWindow();

private slots:
    void on_LoadImages_clicked();
    void Esquinas();
    void REGIONES();
    void calculoRegiones();
    void on_InitializeDisparity_Buttom_clicked();
    void etiquetarPixelSinRegion_8Vecinos();
    void calculoEsquinas();
    void compararVentanasEsquinas();
    void generarResultadoVisual();
    void pintarEsquinas();
    void pintarEsquinasCoincidentes();
    void calculoDisparidad();
    void on_LoadGroundTruth_Buttom_clicked();
    void asignacionDisparidadPtosNoFijos();
    void MostrarValoresPuntoLCD(QPointF puntoLCD);
};


#endif // MAINWINDOW_H
