#ifndef MODELOS_H
#define MODELOS_H

#include <QString>
#include <QDebug>

class Dispositivo {
public:
    int id;
    QString nombre;
    QString tipo;
    QString estado;

    Dispositivo(int id, QString nombre, QString tipo)
        : id(id), nombre(nombre), tipo(tipo), estado("Apagado") {}
    virtual ~Dispositivo() {}

    virtual void encender() {
        estado = "Encendido";
        qDebug() << "[" << tipo << "]" << nombre << "ha sido ENCENDIDO.";
    }
    virtual void apagar() {
        estado = "Apagado";
        qDebug() << "[" << tipo << "]" << nombre << "ha sido APAGADO.";
    }
    virtual void simularFallo() {
        estado = "Fallo";
        qDebug() << "¡ALERTA! [" << tipo << "]" << nombre << "presenta un FALLO.";
    }
};

class Luz : public Dispositivo {
public:
    int brillo;
    Luz(int id, QString nombre) : Dispositivo(id, nombre, "Luz"), brillo(100) {}
};

class Persiana : public Dispositivo {
public:
    int apertura;
    Persiana(int id, QString nombre) : Dispositivo(id, nombre, "Persiana"), apertura(0) {}
};

#endif