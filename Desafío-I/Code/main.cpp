//-- PARTE DEL CÓDIGO INICIAL:
#include <QCoreApplication>
#include <QImage>
#include <QFile>
#include <QString>
#include <iostream>
                             using namespace std;

// ✅ Función proporcionada por los profes
unsigned char* loadPixels(QString input, int &width, int &height){
    QImage imagen(input);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr;
    }

    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    width = imagen.width();
    height = imagen.height();

    int dataSize = width * height * 3;
    unsigned char* pixelData = new unsigned char[dataSize];

    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);
        unsigned char* dstLine = pixelData + y * width * 3;
        memcpy(dstLine, srcLine, width * 3);
    }

    return pixelData;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 📂 Rutas
    QString rutaImgOriginal  = "C:/Users/buelv/Downloads/Desafio I (Informatica II)/DesafioI/Caso 1/I_D.bmp";
    QString rutaImgGaussiana = "C:/Users/buelv/Downloads/Desafio I (Informatica II)/DesafioI/Caso 1/I_M.bmp";
    QString rutaMascara      = "C:/Users/buelv/Downloads/Desafio I (Informatica II)/DesafioI/Caso 1/M.bmp";

    int w1, h1, w2, h2, wm, hm;

    // 📥 Cargar imágenes usando loadPixels()
    unsigned char* imgOriginal  = loadPixels(rutaImgOriginal, w1, h1);
    unsigned char* imgGaussiana = loadPixels(rutaImgGaussiana, w2, h2);
    if (!imgOriginal || !imgGaussiana || w1 != w2 || h1 != h2) {
        cout << "Error al cargar imágenes o tamaños incompatibles." << endl;
        return -1;
    }

    // 🔐 XOR (imgGaussiana ^ imgOriginal)
    int total = w1 * h1 * 3;
    unsigned char* resultado = new unsigned char[total];
    for (int i = 0; i < total; ++i) {
        resultado[i] = imgGaussiana[i] ^ imgOriginal[i];
    }

    // 🎭 Cargar la máscara
    unsigned char* mascara = loadPixels(rutaMascara, wm, hm);
    int totalMascara = wm * hm * 3;
    if (!mascara) {
        cout << "Error al cargar la máscara." << endl;
        return -2;
    }

    // 🧹 Limpiar memoria
    delete[] imgOriginal;
    delete[] imgGaussiana;
    delete[] resultado;
    delete[] mascara;

    return 0;
}
