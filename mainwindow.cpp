#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cap = new VideoCapture(0);
    ui->progreso->setValue(0);
    winSelected = false;
    disparidadCalculada=false;
    colorImage.create(240,320,CV_8UC3);
    grayImage.create(240,320,CV_8UC1);
    destColorImage.create(240,320,CV_8UC3);
    destGrayImage.create(240,320,CV_8UC1);
    imgCopy.create(240,320,CV_32FC1);
    imgEsquinasI.create(240,320,CV_8UC1);
    imgEsquinasD.create(240,320,CV_8UC1);
    imgRegiones.create(240,320,CV_32SC1); //PILAR: LA IMAGEN TIENE QUE SER DE TIPO int (CV_32SC1)
    cannyRegion.create(240,320,CV_8UC1);
    mascara.create(240,320,CV_8UC1);
    imgPuntosFijos.create(240,320,CV_8UC1);
    imgDisparidad.create(240, 320, CV_32FC1);
    imageII.create(240,320,CV_8UC1); //Imagen para mostrar
    imageID.create(240,320,CV_8UC1);

    //Inicialización de imágenes que se usarán
    grayImage.setTo(0);
    destGrayImage.setTo(0);
    imageII.setTo(0);
    imageID.setTo(0);

    //Creación de los cuatro visores usados en la práctica
    visorS = new ImgViewer(&grayImage, ui->imageFrameS);
    visorD = new ImgViewer(&destGrayImage, ui->imageFrameD);
    visorII = new ImgViewer(&imageII,ui->imageFrameII);
    visorID = new ImgViewer(&imageID,ui->imageFrameID);

    connect(&timer,SIGNAL(timeout()),this,SLOT(compute()));
    connect(ui->captureButton,SIGNAL(clicked(bool)),this,SLOT(start_stop_capture(bool)));
    connect(ui->colorButton,SIGNAL(clicked(bool)),this,SLOT(change_color_gray(bool)));
    connect(ui->InitializeDisparity_Buttom,SIGNAL(clicked(bool)),this,SLOT(on_InitializeDisparity_Buttom_clicked()));
    connect(visorS,SIGNAL(windowSelected(QPointF, int, int)),this,SLOT(selectWindow(QPointF, int, int)));
    connect(visorS,SIGNAL(pressEvent()),this,SLOT(deselectWindow()));
    //Señal asociada a la acción de pinchar en un punto para mostrar el valor de disparidad y mostrarla
    connect(visorII,SIGNAL(windowSelected(QPointF, int, int)),this,SLOT(MostrarValoresPuntoLCD(QPointF)));
    timer.start(30);
}

MainWindow::~MainWindow(){
    delete ui;
    delete cap;
    delete visorS;
    delete visorD;
    delete visorII;
    delete visorID;
    colorImage.release();
    grayImage.release();
    destColorImage.release();
    destGrayImage.release();
    imgEsquinasI.release();
    imgEsquinasD.release();
    mascara.release();
    imgRegiones.release();
    cannyRegion.release();
    imgPuntosFijos.release();
    imgDisparidad.release();
}

void MainWindow::compute(){

    //Captura de imagen
    if(ui->captureButton->isChecked() && cap->isOpened()){
        *cap >> colorImage;
        cv::resize(colorImage, colorImage, Size(320,240));
        cvtColor(colorImage, grayImage, COLOR_BGR2GRAY);
        cvtColor(colorImage, colorImage, COLOR_BGR2RGB);
    }

    //Si se activa 'showCorners' se lanza la detección de esquinas
    if(ui->showCorners->isChecked())
        pintarEsquinas();
    if(ui->showCorners_2->isChecked())
        pintarEsquinasCoincidentes();


    if(winSelected)
        visorS->drawSquare(QRect(imageWindow.x, imageWindow.y, imageWindow.width,imageWindow.height), Qt::green );

    //Actualización de los visores
    visorS->update();
    visorD->update();
    visorII->update(); //Visor Inferior Izquierdo
    visorID->update(); //Visor Inferior Derecho
}

void MainWindow::start_stop_capture(bool start){
    if(start)
        ui->captureButton->setText("Stop capture");
    else
        ui->captureButton->setText("Start capture");
}

void MainWindow::change_color_gray(bool color){

    if(color){
        ui->colorButton->setText("Gray image");
        visorS->setImage(&colorImage);
        visorD->setImage(&destColorImage);
    }else{
        ui->colorButton->setText("Color image");
        visorS->setImage(&grayImage);
        visorD->setImage(&destGrayImage);
    }
}

void MainWindow::selectWindow(QPointF p, int w, int h){
    QPointF pEnd;
    if(w>0 && h>0){
        imageWindow.x = p.x()-w/2;
        if(imageWindow.x<0)
            imageWindow.x = 0;
        imageWindow.y = p.y()-h/2;
        if(imageWindow.y<0)
            imageWindow.y = 0;
        pEnd.setX(p.x()+w/2);
        if(pEnd.x()>=320)
            pEnd.setX(319);
        pEnd.setY(p.y()+h/2);
        if(pEnd.y()>=240)
            pEnd.setY(239);
        imageWindow.width = pEnd.x()-imageWindow.x+1;
        imageWindow.height = pEnd.y()-imageWindow.y+1;

        winSelected = true;
    }
}

void MainWindow::deselectWindow(){
    winSelected = false;
}


/**
 * @brief MainWindow::on_LoadImages_clicked
 * @details Carga una imagen en el visor
 */
void MainWindow::on_LoadImages_clicked(){
    start_stop_capture(false);                      //Detención de captura cam
    ui->captureButton->setChecked(false);
    ui->captureButton->setText("Start capture");    //El botón cambia de nombre

    //Estructura de String para gestionar las rutas de las dos imágenes seleccionadas, se utiliza el método
    //'getOpenFileNames' en lugar de 'getOpenFileName' para poder acceder a 2 imágenes durante la misma carga.
    QStringList fileName = QFileDialog::getOpenFileNames(this, tr("Seleccione dos imágenes (view1 y view5)"), "");
    Mat image1 =imread(fileName[0].toStdString(), IMREAD_COLOR); //Lectura de la 1º imagen seleccionada
    ANCHOoriginal = image1.cols;                                 //Guardado del ancho original de la imagen
    cv::resize(image1,image1,Size(320,240));                     //Redimensión al tamaño de la ventana (UI)
    cvtColor(image1, grayImage, COLOR_BGR2GRAY);
    cvtColor(image1, colorImage, COLOR_BGR2RGB);

    Mat image2 =imread(fileName[1].toStdString(), IMREAD_COLOR); //Lectura de la 2º imagen seleccionada
    cv::resize(image2,image2,Size(320,240));                     //Redimensión al tamaño de la ventana (UI)
    cvtColor(image2, destGrayImage, COLOR_BGR2GRAY);
    cvtColor(image2, destColorImage, COLOR_BGR2RGB);

}

//==========================================================================================================================
//=======================================       ESQUINAS (Show Corners)     ================================================
//==========================================================================================================================
/**
 * @brief MainWindow::pintarEsquinas
 * @details Vacía la listaEsquinas, a continuación, invoca al método Esquinas para introducir en la estructura
 * todas las esquinas existentes en la imagen, por último, recorre iterativamente todas ellas y las va pintando
 * de color ROJO con un tamaño de 5x5 en cada posición indicada por pixelPoint.
 */
void MainWindow::Esquinas(){

    listaEsquinasI.clear();   //Vacíamos la estructura de datos de posibles valores residuales
    listaEsquinasD.clear();
    calculoEsquinas();        //Invocación a Esquinas() para obtener todas las existentes
}


/**
 * @brief MainWindow::Esquinas
 * @details Esquinas detecta todas las esquinas existentes en una imagen, en primer lugar, las inserta
 * en la listaEsquinas existiendo así esquinas no deseadas por lo que, a continuación, se aplica el
 * algoritmo de supresión del no máximo, de esta forma, desechamos las esquinas que estén lo suficientemente
 * alejadas de las verdaderas esquinas. Esto se lleva a cabo realizando una ordenación usando como base el valor
 * de harris presente en el registro pixelPoint donde encontramos el punto concreto y su valor de harris.
 */
void MainWindow::calculoEsquinas(){

    //Comparador para ordenar la lista en función del valor de Harris
    ComparadorListaEsquinas comparador;

    //Invocación a la función cornerHarris para detección de esquinas
    cv::cornerHarris(grayImage,imgCopy,3,3,0.08);
    cv::cornerHarris(destGrayImage,imgCopyD,3,3,0.08);

    Point p;
    int i=0, j=0;
     ui->progreso->setValue(12);
    for(i=0;i<240;i++){
        for(j=0; j<320;j++){
            p.x = j;   //Referencia para coordenada x <-- j
            p.y = i;   //Referencia para coordenada y <-- i
            pixelPoint auxiliar,auxiliarD;
            auxiliar.valor = imgCopy.at<float>(p);    //Valor de Harris
            auxiliar.p=p;
            auxiliarD.valor = imgCopyD.at<float>(p);  //Valor de Harris
            auxiliarD.p=p;
            pixelPoint next;

            if(imgCopy.at<float>(i,j) > 0.000001){  //PILAR (02/05)
                listaEsquinasI.push_back(auxiliar); //Inserto el punto prometedor en listaEsquinas
            }
            if(imgCopyD.at<float>(i,j) > 0.000001){ //PILAR (02/05)
                listaEsquinasD.push_back(auxiliarD);
            }

        }
    }
     ui->progreso->setValue(14);

    //Ordenación de la lista utilizando la estructura 'comparador'
    std::sort(listaEsquinasI.begin(), listaEsquinasI.end(), comparador);
    std::sort(listaEsquinasD.begin(), listaEsquinasD.end(), comparador);

    //Aplicamos la supresión del NO MÁXIMO para limpiar la lista de valores no deseados
    pixelPoint pointt,auxPoint;
     ui->progreso->setValue(16);
    for(i=0;i<listaEsquinasI.size();i++){
        pointt=listaEsquinasI[i];

        for(j=i+1;j<listaEsquinasI.size();j++){
            auxPoint=listaEsquinasI[j];
             //Cálculo de la distancia euclídea
             float dist = sqrt(pow(pointt.p.x-auxPoint.p.x,2)+pow(pointt.p.y-auxPoint.p.y,2));
            //Si la distancia es mayor de un umbral determinado significa que ese punto no forma parte de la esquina por
            //encontrarse demasiado lejos uno del otro, por tanto, lo eliminamos de listaEsquinas.

             if(dist<6) {  //Este umbral de distancia puede modificarse dependiendo del caso, 6 suele dar buenos resultados
                listaEsquinasI.erase(listaEsquinasI.begin()+j);
                j--;
            }
        }
    }
     ui->progreso->setValue(18);
    //Supresion lista esquinas visor derecho
    for(i=0;i<listaEsquinasD.size();i++){
        pointt=listaEsquinasD[i];

        for(j=i+1;j<listaEsquinasD.size();j++){
            auxPoint=listaEsquinasD[j];
             //Cálculo de la distancia euclídea
             float dist = sqrt(pow(pointt.p.x-auxPoint.p.x,2)+pow(pointt.p.y-auxPoint.p.y,2));
            //Si la distancia es mayor de un umbral determinado significa que ese punto no forma parte de la esquina por
            //encontrarse demasiado lejos uno del otro, por tanto, lo eliminamos de listaEsquinas.

             if(dist<6) {  //Este umbral de distancia puede modificarse dependiendo del caso, 6 suele dar buenos resultados
                listaEsquinasD.erase(listaEsquinasD.begin()+j);
                j--;
            }
        }
    }

   imgEsquinasI.setTo(0);
   imgEsquinasD.setTo(0);

   for(i=0;i<listaEsquinasI.size();i++)     imgEsquinasI.at<uchar>(listaEsquinasI[i].p) = 1;

   for(i=0;i<listaEsquinasD.size();i++)     imgEsquinasD.at<uchar>(listaEsquinasD[i].p) = 1;
     ui->progreso->setValue(20);
}


void MainWindow::compararVentanasEsquinas(){

    Rect windowI,windowD;
    Mat result;
    int w = 11;
    float max;
    Point auxI,auxD;
    matchPoints pAux;
    windowI.width=w;
    windowI.height=w;
    windowD.width=w;
    windowD.height=w;
    bool first = true;
    listaMatches.clear();
    // recorrer lista de esquinas
    for(int i=0; i<listaEsquinasI.size(); i++){
        //Comprobar limites para creacion de ventana
        if(listaEsquinasI[i].p.x > w/2  && listaEsquinasI[i].p.y > w/2 && listaEsquinasI[i].p.x <(319-w/2)  && listaEsquinasI[i].p.y < (239-w/2)){
            //construir ventana (w x w) alrededor del punto en imagen Izq
            windowI.x=listaEsquinasI[i].p.x-(w/2);
            windowI.y=listaEsquinasI[i].p.y-(w/2);
            bool match = false;
            for(int j=0;j<320;j++){
                // Si corresponde a esquina en imagen derecha
                if(imgEsquinasD.at<uchar>(listaEsquinasI[i].p.y, j)==1){ ///////PILAR
                    if(j > w/2  && j <(319-w/2)) ////PILAR
                    {
                        windowD.x=j-(w/2);
                        windowD.y=listaEsquinasI[i].p.y-(w/2);
                        matchTemplate(grayImage(windowI),destGrayImage(windowD),result,TM_CCOEFF_NORMED);
                        if(first){
                             max=result.at<float>(0,0);
                             first = false;
                        }
                        //Actualizar puntos si supera maximo
                        if(result.at<float>(0,0) > 0.8 && result.at<float>(0,0) >= max ){
                            max = result.at<float>(0,0);
                            auxI.x=listaEsquinasI[i].p.x;
                            auxI.y=listaEsquinasI[i].p.y;
                            auxD.x=j;
                            auxD.y=listaEsquinasI[i].p.y;
                            match = true; ///////PILAR
                        }
                    }
                }
            }
            if(match) ///////PILAR
            {
                pAux.p1 =auxI;
                pAux.p2 =auxD;
                pAux.match=true;
                listaMatches.push_back(pAux);
            }
            first = true;
        }

    }

}

void MainWindow::calculoDisparidad(){

    float disp = 0.0;
    imgPuntosFijos.setTo(0);
    imgDisparidad.setTo(-1);

    //Actualizar a 1 la imagen de puntos fijos
    for(int i=0;i<listaMatches.size();i++)
        imgPuntosFijos.at<uchar>(listaMatches[i].p1) = 1;

    // Calcular el mapa de disparidad
    for(int i=0;i<listaMatches.size();i++){
        disp = listaMatches[i].p1.x - listaMatches[i].p2.x;
        imgDisparidad.at<float>(listaMatches[i].p1)=disp;
    }
}

void MainWindow::pintarEsquinas(){

    //Marca esquinas en rojo en cada visor
    for (pixelPoint point : listaEsquinasI)
        visorS->drawSquare(QPoint(point.p.x, point.p.y), 2, 2, Qt::red);

    for (pixelPoint point : listaEsquinasD)
        visorD->drawSquare(QPoint(point.p.x, point.p.y), 2, 2, Qt::red);

}

void MainWindow::pintarEsquinasCoincidentes(){

    //Marca esquinas coincidentes en verde en cada visor
    for(matchPoints point : listaMatches){
        visorS->drawSquare(QPoint(point.p1.x, point.p1.y), 2, 2, Qt::green);
        visorD->drawSquare(QPoint(point.p2.x, point.p2.y), 2, 2, Qt::green);
    }
}



//========================================================================================================================
//==============================                  SEGMENTACIÓN              ==============================================
//========================================================================================================================
/**
 * @brief MainWindow::STARTRegiones
 * @details La representación interna estará formada por una imagen (tipo CV_32SC1) que contendrá el identificador
 * de región de cada píxel (imgRegiones) y una lista de regiones (listRegiones) que almacenará los distintos atributos de una región.
 * Un valor negativo (-1) como identificador de región de un píxel indicará que el píxel no ha sido añadido a ninguna región.
 * Éste será, por lo tanto, el valor inicial de los píxeles de la imagen de regiones antes de realizar la segmentación.
 */
void MainWindow::REGIONES(){

    //destGrayImage.setTo(0); //Inicialmente, a 0
    imgRegiones.setTo(-1);  //Inicialmente, puntos a -1

    this->listRegiones.clear();  // vaciar lista de regiones
    Canny(grayImage,cannyRegion,40,120);
     ui->progreso->setValue(50);
    // Implementación de los algoritmos para cálculos de regiones
    calculoRegiones();

    //A partir del calculo, lo represento en la imagen (descomentar)
    //generarResultadoVisual();

}

/**
 * @brief MainWindow::calculoRegiones
 * @details
 *
 * En la llamada a floodFill para imágenes en color, los parámetros de tipo Scalar dónde almacenáis la máxima diferencia de valor R, G y B
 * tienen que incluir un Scalar de 3 valores: Scalar(MaxDifference, MaxDifference, MaxDifference). Cada valor de ese Scalar se refiere a
 * la máxima diferencia en R, G y B.
 */
void MainWindow::calculoRegiones(){


    int MaxDifference  = 10, newValParam = 1, nfijos=0;
    float dAcumulada=0;

    Point punto,punto2, mascaraP;
    Rect minRectRegionObtenida; //mínimo rectángulo que contiene a la región obtenida, lo retorna floodFill
    region reg;


    //Inicializo indicando como imagen origen la de bordes con Canny para generar un borde exterior en la imagen con el valor de píxel indicado.
    copyMakeBorder(cannyRegion,mascara,1,1,1,1,1,BORDER_CONSTANT);

    for(int alto=0;alto<240;alto++){     //Primero de arriba a bajo (0-240)
        for(int ancho=0;ancho<320;ancho++){    //Después de izquierda a derecha (0-320)
            punto.x=ancho;  punto.y=alto;

            //Mientras existan puntos que no sean bordes (-1) en imgRegiones
            if(this->imgRegiones.at<int>(punto)==-1 && cannyRegion.at<uchar>(punto)!=255){
                minRectRegionObtenida.x=ancho;   minRectRegionObtenida.y=alto;

                //Lanzar el crecimiento a partir del punto no borde mediante floodFill - finaliza en los puntos de la imagen de máscara != 0
                //FLOODFILL_FIXED_RANGE cuando el usuario quiera usar la comparación de rango fijo
                floodFill(grayImage, mascara,punto,newValParam,&minRectRegionObtenida,Scalar(MaxDifference),Scalar(MaxDifference),4|FLOODFILL_MASK_ONLY|newValParam<<8);

                //---------------------------------------------- Añado la nueva REGIÓN-----------------------------------------------------

                    reg.numPuntosReg=0;
                    float valorColor1=0.0;  ////PILAR: HAY QUE INICIALIZARLO DENTRO DEL BUCLE, POR CADA NUEVA REGIÓN

                    //PILAR (01/05): hay que inicializar nfijos y dAcumulada por cada nueva región
                    nfijos = 0;
                    dAcumulada = 0.;

                    //Recorro mínimo rectángulo que contiene a la región obtenida, un rectángulo con los puntps
                    //que han sido modificados. Se recorre cada punto de la ventana
                    for(int j=minRectRegionObtenida.y;j<minRectRegionObtenida.height+minRectRegionObtenida.y;j++){
                        for(int i=minRectRegionObtenida.x;i<minRectRegionObtenida.width+minRectRegionObtenida.x;i++){
                            punto2.x=i;  punto2.y=j;  mascaraP.x=i+1;  mascaraP.y=j+1;

                            //Se busca si alguno de los puntos de la ventana tiene valor -1 = No añadido a región
                            if(imgRegiones.at<int>(punto2)==-1 && mascara.at<uchar>(mascaraP)==newValParam){

                                //Añado el punto desde el que se lanzó el crecimiento
                                if(reg.numPuntosReg==0) reg.coorPrimerPixel=punto;

                                //Marcado de puntos mediante el identificador de región
                                imgRegiones.at<int>(punto2)= listRegiones.size();  //Identificador

                                reg.numPuntosReg++;  //Incremento el número de ppuntos de la región

                                valorColor1 += grayImage.at<uchar>(punto2);     //Acumulo valor Gris

                                //----------------------------------- Calculo disparidad Media--------------------------------------
                                //Acumulamos el valor de disparidad de los puntos fijos
                                if (imgPuntosFijos.at<uchar>(punto2)==1 ){  //Si es un punto fijo  PILAR(02/05): la comprobación correcta es ==1
                                    nfijos++;
                                    dAcumulada+=imgDisparidad.at<float>(punto2); //PILAR(02/05): disparidad de punto2
                                }
                            }
                        }
                    }

                    if(nfijos>0)
                        reg.dMedia = dAcumulada/nfijos;
                    else
                        reg.dMedia=0;

                    reg.nfijos = nfijos; //PILAR (02/05)

                    if(reg.numPuntosReg>0){  //Si existen puntos en la región, los actualizo
                         reg.gris=valorColor1/reg.numPuntosReg;  //Calculo nivel Gris
                         reg.disparidad=0;
                         listRegiones.push_back(reg);  //Añado esta región a la lista de regiones

                }
            }
        }
    }
    ui->progreso->setValue(60);
    //Invocación a etiquetarPixelSinRegion_8Vecinos para gestionar los píxeles sin región en relación a sus vecinos que si tienen una región asignada.
    etiquetarPixelSinRegion_8Vecinos();
    ui->progreso->setValue(70);
}


/**
 * @brief MainWindow::etiquetarPixelSinRegion_8Vecinos
 * @details Una vez que se han calculado las regiones hay que filtrar los resultados más prometedores, para ello,
 * volveremos a recorrer imgRegiones etiquetando cada uno de los píxeles que no tengan región. Si un pixel tiene un
 * valor = -1, se calcula el valor de identificación de sus 8 vecinos similares.
 * Para etiquetar un píxel sin región, se recorre su entorno de 3x3 en la imagen de grises (grayImage) o en
 * la imagen de color (colorImage). Comparáis el atributo de gris o color del píxel con el de cada uno de los vecinos
 * que sí tengan región asignada y se le asigna el identificador de región del píxel del entorno más parecido, es decir,
 * para el que existe menor diferencia en gris o color.
 */
void MainWindow::etiquetarPixelSinRegion_8Vecinos(){

    double dif1=0, dif2=0;
       Point m;

       //Recorremos imagen
       for(int i=0; i < 240; ++i){
           for(int j=0; j < 320; ++j){
               //Comprobamos que en las coordenadas el punto no tiene region
               if(imgRegiones.at<int>(i,j)==-1){
                   bool first = true;
                   // Calcular diferencia con adyacentes
                   for(int x=-1; x<2; x++){
                       for(int y=-1; y<2; y++){
                           //Comprobar que los puntos se encuentran dentro de rango
                           if(((x+j) >= 0 && (x+j) < 320) && ((y+i) >= 0 && (y+i) < 240)){

                                   if(imgRegiones.at<int>(i+y,j+x) != -1){ // Comprobar que pertenece a region
                                       if(first){
                                           m.x=j+x;
                                           m.y=i+y;
                                           first=false;
                                       }else{
                                           dif1 = abs(grayImage.at<uchar>(i+y,j+x) - grayImage.at<uchar>(i,j));
                                           dif2 = abs(grayImage.at<uchar>(i,j) - grayImage.at<uchar>(m));
                                           if(dif1 <=dif2){
                                               m.x=j+x;
                                               m.y=i+y;
                                       }
                                    }
                                }
                            }
                       }
                   }
                   // Actualizar el valor de la posicion con el que hemos obtenido
                   imgRegiones.at<int>(i,j) = imgRegiones.at<int>(m);
                   int id = imgRegiones.at<int>(i,j);   //PILAR (02/05)
                   if(imgPuntosFijos.at<uchar>(i,j)==1){ //PILAR (02/05)
                        listRegiones[id].dMedia = (listRegiones[id].dMedia*listRegiones[id].nfijos + imgDisparidad.at<float>(i,j))/(listRegiones[id].nfijos+1);
                        listRegiones[id].nfijos++;
                   }
                   }
              }
       }


}

/**
 * @brief MainWindow::generarResultadoVisual
 * @details Generación del resultado visual a partir de los algorítmos de cálculo de regiones
 * en la imagen de grises (destGrayImage) o la de color (destColorImage). Primero recorremos
 * en alto y ancho de la imagen, cuando tratamos una región hay dos casos:
 *   - En las imágenes grises colocamos (fijándonos en cada identificador) el nivel de gris medio
 *     de la región, es decir, el atributo gris de región.
 *   - En las imágenes en color necesitamos almacenar en un vector los valores medios RGB de la misma forma
 *     y asignarlo.
 */
void MainWindow::generarResultadoVisual(){
    int alto=0, ancho=0;
    for(alto=0;alto<240;alto++){            //Recorrido de arriba a bajo
        for(ancho=0;ancho<320;ancho++){     //Recorrido de izquierda a derecha
            if(imgRegiones.at<int>(alto, ancho) >= 0){
               imageII.at<uchar>(alto, ancho)=listRegiones[imgRegiones.at<int>(alto, ancho)].gris;  //PILAR: para pruebas mostrad el resultado en el visor de la posición inferior izquierda
            }
        }
    }
}

//==========================================================================================================================
//==========================================================================================================================
//==============================       IMPLEMENTACIONES DE VISIÓN ESTÉREO     ==============================================
//==========================================================================================================================
//==========================================================================================================================

/**
 * @brief MainWindow::on_InitializeDisparity_Buttom_clicked
 * @details Botón [Initialize Disparity]: Cuando se realiza el evento de click sobre este botón comienza el proceso de
 * inicialización de disparidad.
 */
void MainWindow::on_InitializeDisparity_Buttom_clicked(){
    ui->progreso->setValue(10);
    //Primero se calculan las esquinas (entrega03) -  -  - Primera fase
    Esquinas();
    ui->progreso->setValue(20);
    compararVentanasEsquinas();
    ui->progreso->setValue(30);
    calculoDisparidad();
     ui->progreso->setValue(40);

    //Segundo se calcula las regiones  (entrega04) -  -  - Primera fase
    REGIONES();

    //Calculo de la disparidad media de cada región -  -  - Segunda fase
    //Asignación de disparidad a los puntos no fijos
     ui->progreso->setValue(80);
    asignacionDisparidadPtosNoFijos();
     ui->progreso->setValue(100);
    disparidadCalculada=true;

}
/**
 * @brief MainWindow::on_LoadGroundTruth_Buttom_clicked
 * @details A partir de la ruta de la anterior carga de imágenes, cargamos la imagen de disparidad
 */
void MainWindow::on_LoadGroundTruth_Buttom_clicked(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Seleccione una IMAGEN (disp 1)"), "", tr("All Files (*)"));
    imageID =imread(fileName.toStdString(), IMREAD_COLOR);       // Read the file
    cv::resize(imageID,imageID,Size(320,240));                   //Redimensión al tamaño de la ventana (UI)
    cvtColor(imageID, imageID, COLOR_BGR2GRAY);
}



/**
 * @brief MainWindow::asignacionDisparidadPtosNoFijos
 * @details  En primer lugar, una vez s eha realizado el cálculo de la disparidad media durante el proceso de segmentaación trás la
 * llamada a floodFill, se realiza el proceso de asignación a la disparidad referente a los puntos no fijos de la imagen de disparidad.
 * Para ello, recorremos el alto y ancho de la imagen y, si encontramos un punto NO fijo, se actualiza la imagen de disparidad al valor
 * de disparidad media.
 * La imagen de disparidad de tipo float es la disparidad calculada. Es decir, para un píxel de la imagen izquierda,
 * su valor de disparidad representa la distancia en columna del píxel homólogo de la imagen derecha. Se mostrará la imageII, imagen
 * de tipo uchar (CV_8UC1), donde cada píxel contiene un nivel de gris asociado con la disparidad de tipo float del punto.
 */
void MainWindow::asignacionDisparidadPtosNoFijos(){

    Point punto;
    int alto, ancho;

    for(alto=0;alto<240;alto++){           //Primero de arriba a bajo (0-240)
        for(ancho=0;ancho<320;ancho++){    //Después de izquierda a derecha (0-320)
            punto.x=ancho;  punto.y=alto;

            //Si un punto NO es fijo en la posición punto
            if(imgPuntosFijos.at<uchar>(punto)==0 )  //Si NO es un punto fijo
                imgDisparidad.at<float>(punto) = listRegiones[imgRegiones.at<int>(punto)].dMedia;
        }
    }

    //Una vez asignaados los valores de disparidad, pintamos en imageII esa representación de valores
    for(alto=0; alto<240;alto++){
        for(ancho=0; ancho<320;ancho++){
            punto.x=ancho;  punto.y=alto;
            imageII.at<uchar>(punto)=(3*imgDisparidad.at<float>(punto)*ANCHOoriginal)/320;

        }
    }
}


/**
 * @brief MainWindow::MostrarValoresPuntoLCD
 * @param puntoLCD
 * @details Si la disparidad ha sido calculada, cuando se "pinche" en una coordenada concreta del visor inferior izquierdo, este
 * imprimirá en el LCD el valor de disparidad estimada, y el valor de disparidad real. En caso de que aún no se ha calculado la
 * disparidad, se mantendrá el LCD en 0.
 */
void MainWindow::MostrarValoresPuntoLCD(QPointF puntoLCD){

    if(disparidadCalculada){
        ui->LCDEstimated->display(imageII.at<uchar>(puntoLCD.y(), puntoLCD.x()));
        ui->LCTrue->display(imageID.at<uchar>(puntoLCD.y(), puntoLCD.x()));
    }else{
        ui->LCDEstimated->display(imageII.at<uchar>(0,0));
        ui->LCTrue->display(imageID.at<uchar>(0,0));
    }
}
