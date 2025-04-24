#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <iostream>
#include <fstream>
using namespace std;

// Aplica operación XOR entre dos arreglos lineales de imagenes
unsigned char* xorImagen(const unsigned char* img1, const unsigned char* img2, int totalBytes) {
    unsigned char* resultado = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; ++i) {
        resultado[i] = img1[i] ^ img2[i];
    }
    return resultado;
}

// Rota n_bits posiciones hacia la izquierda en cada byte
unsigned char rotarBitsDerecha(unsigned char b, int n_bits) {
    n_bits %= 8;
    return (b << n_bits) | (b >> (8 - n_bits));
}

// Rota n_bits posiciones hacia la derecha en cada byte
unsigned char rotarBitsIzquierda(unsigned char b, int n_bits) {
    n_bits %= 8;
    return (b >> n_bits) | (b << (8 - n_bits));
}

// Carga un BMP (RGB888) en un arreglo lineal [R,G,B,...]
unsigned char* loadPixels(const QString& ruta, int& ancho, int& alto) {
    QImage img(ruta);
    if (img.isNull()) {
        cout << "Error cargando " << ruta.toStdString() << endl;
        return nullptr;
    }
    img = img.convertToFormat(QImage::Format_RGB888);
    ancho = img.width();
    alto  = img.height();

    int total = ancho * alto * 3;
    unsigned char* pix = new unsigned char[total];
    for (int y = 0; y < alto; ++y) {
        memcpy(pix + y * ancho * 3, img.scanLine(y), ancho * 3);
    }
    return pix;
}

// Genera una imagen BMP (RGB888) y lo guarda en una ruta del portatil
bool exportImage(unsigned char* datos, int ancho, int alto, const QString& ruta) {
    QImage img(ancho, alto, QImage::Format_RGB888);
    for (int y = 0; y < alto; ++y) {
        memcpy(img.scanLine(y), datos + y * ancho * 3, ancho * 3);
    }
    if (!img.save(ruta, "BMP")) {
        cout << "Error guardando " << ruta.toStdString() << endl;
        return false;
    }
    cout << "Guardado: " << ruta.toStdString() << endl;
    return true;
}

// Lee semilla + tríos RGB desde un TXT
// Formato del TXT:
//  semilla
//  R G B
//  R G B
//  ...
unsigned int* loadMaskData(const char* ruta, int& semilla, int& pixeles) {
    ifstream archivo(ruta);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir " << ruta << endl;
        return nullptr;
    }
    archivo >> semilla;
    int r, g, b;
    // contamos cuántos tríos hay
    while (archivo >> r >> g >> b) pixeles++;
    archivo.clear();
    archivo.seekg(0);
    archivo >> semilla; // volvemos a leer la semilla

    unsigned int* data = new unsigned int[pixeles * 3];
    for (int i = 0; i < pixeles * 3; i += 3) {
        archivo >> r >> g >> b;
        data[i]     = r;
        data[i + 1] = g;
        data[i + 2] = b;
    }
    archivo.close();
    return data;
}

// Compara byte a byte: grande[offset..offset+cnt-1] vs pequeño[0..cnt-1]
bool comparar(const unsigned char* grande,
              const unsigned char* pequeño,
              int offset, int cantidad)
{
    for (int i = 0; i < cantidad; ++i) {
        if (grande[offset + i] != pequeño[i])
            return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // --- RUTAS (ajusta a tu carpeta) ---
    QString rutaOrig    = ".../I_D.bmp";
    QString rutaGauss   = ".../I_M.bmp";
    QString rutaMascBMP = ".../M.bmp";
    const char* rutaTxt = ".../M2.txt";
    QString salidaBMP   = ".../resultado.bmp";

    // --- Cargo las imágenes ---
    int w1,h1, w2,h2, wm,hm;
    unsigned char* imgOrig  = loadPixels(rutaOrig,    w1, h1);
    unsigned char* imgGauss = loadPixels(rutaGauss,   w2, h2);
    unsigned char* imgMasc  = loadPixels(rutaMascBMP, wm, hm);
    if (imgOrig == nullptr || imgGauss == nullptr || imgMasc == nullptr || w1 != w2 || h1 != h2) {
        cout << "Error en carga o tamaños distintos." << endl;
        return -1;
    }

    int totalBytes   = w1 * h1 * 3;
    int totalMascara = wm * hm * 3;

    // --- Cargo TXT (semilla + datosMask) ---
    int semilla = 0, cantPix = 0;
    unsigned int* datosMask = loadMaskData(rutaTxt, semilla, cantPix);
    if (datosMask == nullptr) return -2;

    // --- Construyo el arreglo "esperado" = datosMask - máscaraBMP ---
    unsigned char* esperado = new unsigned char[totalMascara];
    for (int i = 0; i < totalMascara; ++i) {
        int resta = int(datosMask[i]) - int(imgMasc[i]);
        if (resta < 0) resta = 0;
        if (resta > 255) resta = 255;
        esperado[i] = (unsigned char)resta;
    }

    // --- Primero pruebo XOR ---
    unsigned char* resultado = xorImagen(imgOrig, imgGauss, totalBytes);
    bool ok = comparar(resultado, esperado, semilla, totalMascara);

    if (ok == false) {
        // --- Si XOR falla, pruebo rotaciones izq (1..8) ---
        for (int k = 1; k <= 8 && ok == false; ++k) {
            for (int i = 0; i < totalBytes; ++i)
                resultado[i] = rotarBitsDerecha(imgOrig[i], k);
            ok = comparar(resultado, esperado, semilla, totalMascara);
            if (ok) {
                cout << "Transformación usada: Rotación izquierda de " << k << " bits." << endl;
            }
        }

    //EN PROCESO DE DESARROLLO...

    // --- Libero todo ---
    delete[] imgOrig;
    delete[] imgGauss;
    delete[] imgMasc;
    delete[] datosMask;
    delete[] esperado;
    delete[] resultado;

    return 0;
}
