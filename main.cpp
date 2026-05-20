#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QListWidget>
#include <QTimer>
#include <QMessageBox>
#include "modelos.h"
#include "db_manager.h"

class DomoticaApp : public QWidget {
private:
    QLineEdit *entryNombre;
    QRadioButton *radioLuz;
    QRadioButton *radioPersiana;
    QListWidget *listaVisual;
    QTimer *timerConsumo;

public:
    DomoticaApp() {
        setWindowTitle("SmartHome - Panel de Control Inteligente (C++)");
        resize(640, 680);

        // Estilos CSS idénticos al Modo Oscuro de Python
        setStyleSheet(
            "QWidget { background-color: #121824; color: #f8fafc; font-family: Helvetica; }"
            "QFrame { background-color: #1e293b; border-radius: 8px; }"
            "QLineEdit { background-color: #121824; color: #f8fafc; border: 1px solid #334155; padding: 6px; border-radius: 4px; }"
            "QPushButton { font-weight: bold; border-radius: 4px; padding: 8px; }"
            "QListWidget { background-color: #121824; color: #f8fafc; border: 1px solid #334155; font-family: Consolas; padding: 5px; }"
        );

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(25, 15, 25, 15);

        QLabel *title = new QLabel("SISTEMA DE GESTIÓN DOMÓTICA");
        title->setStyleSheet("color: #38bdf8; font-size: 14px; font-weight: bold;");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        // ZONA 1: REGISTRO
        QFrame *panelRegistro = new QFrame();
        QVBoxLayout *regLayout = new QVBoxLayout(panelRegistro);
        
        QLabel *lblReg = new QLabel("1. Registrar Nuevo Dispositivo");
        lblReg->setStyleSheet("font-weight: bold; font-size: 11px; color: #f8fafc;");
        regLayout->addWidget(lblReg);

        QHBoxLayout *inputLayout = new QHBoxLayout();
        QLabel *lblNom = new QLabel("Nombre del Aparato:");
        lblNom->setStyleSheet("color: #94a3b8; font-size: 10px;");
        entryNombre = new QLineEdit();
        inputLayout->addWidget(lblNom);
        inputLayout->addWidget(entryNombre);
        regLayout->addLayout(inputLayout);

        QHBoxLayout *radioLayout = new QHBoxLayout();
        radioLuz = new QRadioButton("Luz");
        radioPersiana = new QRadioButton("Persiana");
        radioLuz->setChecked(true);
        radioLayout->addWidget(radioLuz);
        radioLayout->addWidget(radioPersiana);
        regLayout->addLayout(radioLayout);

        QPushButton *btnGuardar = new QPushButton("GUARDAR EN BASE DE DATOS");
        btnGuardar->setStyleSheet("background-color: #38bdf8; color: #0f172a;");
        regLayout->addWidget(btnGuardar);
        mainLayout->addWidget(panelRegistro);

        // ZONA 2: DASHBOARD
        QFrame *panelList = new QFrame();
        QVBoxLayout *listLayout = new QVBoxLayout(panelList);

        QLabel *lblDash = new QLabel("2. Dispositivos Activos y Consumo Energético");
        lblDash->setStyleSheet("font-weight: bold; font-size: 11px;");
        listLayout->addWidget(lblDash);

        listaVisual = new QListWidget();
        listLayout->addWidget(listaVisual);

        // ZONA 3: ACCIONES RÁPIDAS
        QLabel *lblAcc = new QLabel("3. Panel de Acciones Rápidas");
        lblAcc->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 10px;");
        listLayout->addWidget(lblAcc);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnOn = new QPushButton("ENCENDER / ABRIR");
        btnOn->setStyleSheet("background-color: #10b981; color: white;");
        QPushButton *btnOff = new QPushButton("APAGAR / CERRAR");
        btnOff->setStyleSheet("background-color: #ef4444; color: white;");
        QPushButton *btnFallo = new QPushButton("SIMULAR FALLO");
        btnFallo->setStyleSheet("background-color: #f59e0b; color: white;");

        btnLayout->addWidget(btnOn);
        btnLayout->addWidget(btnOff);
        btnLayout->addWidget(btnFallo);
        listLayout->addLayout(btnLayout);
        mainLayout->addWidget(panelList);

        // BOTÓN AUDITORÍA
        QPushButton *btnHistorial = new QPushButton("📊 VER HISTORIAL DE AUDITORÍA (LOGS)");
        btnHistorial->setStyleSheet("background-color: #475569; color: white; padding: 10px;");
        mainLayout->addWidget(btnHistorial);

        // Conectar eventos (Slots)
        connect(btnGuardar, &QPushButton::clicked, this, &DomoticaApp::registrarNuevo);
        connect(btnOn, &QPushButton::clicked, [this](){ accionar("encender"); });
        connect(btnOff, &QPushButton::clicked, [this](){ accionar("apagar"); });
        connect(btnFallo, &QPushButton::clicked, [this](){ accionar("fallo"); });
        connect(btnHistorial, &QPushButton::clicked, this, &DomoticaApp::verHistorial);

        // Temporizador del bono (cada 2 segundos)
        timerConsumo = new QTimer(this);
        connect(timerConsumo, &QTimer::timeout, this, &DomoticaApp::simularConsumo);
        timerConsumo->start(2000);

        actualizarLista();
    }

    void registrarNuevo() {
        QString nombre = entryNombre->text();
        QString tipo = radioLuz->isChecked() ? "Luz" : "Persiana";
        if (!nombre.isEmpty()) {
            registrarDispositivo(nombre, tipo);
            entryNombre->clear();
            actualizarLista();
        }
    }

    void actualizarLista() {
        int filaSeleccionada = listaVisual->currentRow();
        listaVisual->clear();
        QList<DeviceData> list = obtenerDispositivos();
        for (const auto &d : list) {
            QString itemText = QString("ID: %1  |  %2 (%3)  ->  Estado: %4  |  Gasto: %5 kWh")
                               .arg(d.id).arg(d.nombre).arg(d.tipo).arg(d.estado).arg(d.consumo, 0, 'f', 2);
            listaVisual->addItem(itemText);
        }
        if (filaSeleccionada >= 0 && filaSeleccionada < listaVisual->count()) {
            listaVisual->setCurrentRow(filaSeleccionada);
        }
    }

    void simularConsumo() {
        QList<DeviceData> list = obtenerDispositivos();
        for (const auto &d : list) {
            if (d.estado == "Encendido") {
                double gasto = (d.tipo == "Luz") ? 0.05 : 0.12;
                incrementarConsumo(d.id, gasto);
            }
        }
        actualizarLista();
    }

    void accionar(QString accion) {
        int row = listaVisual->currentRow();
        if (row < 0) return;
        QString text = listaVisual->item(row)->text();

        QStringList partes = text.split("  |  ");
        int id = partes[0].replace("ID: ", "").toInt();
        QString nombreStr = partes[1].split("  ->  ")[0];
        QString nombre = nombreStr.split(" (")[0];
        QString tipo = nombreStr.split(" (")[1].replace(")", "");

        if (accion == "encender") {
            if (tipo == "Luz") { Luz obj(id, nombre); obj.encender(); }
            else { Persiana obj(id, nombre); obj.encender(); }
            actualizarEstado(id, "Encendido");
            registrarEvento(id, "Se encendio la " + nombre);
        } else if (accion == "apagar") {
            if (tipo == "Luz") { Luz obj(id, nombre); obj.apagar(); }
            else { Persiana obj(id, nombre); obj.apagar(); }
            actualizarEstado(id, "Apagado");
            registrarEvento(id, "Se apago la " + nombre);
        } else if (accion == "fallo") {
            if (tipo == "Luz") { Luz obj(id, nombre); obj.simularFallo(); }
            else { Persiana obj(id, nombre); obj.simularFallo(); }
            actualizarEstado(id, "Fallo");
            registrarEvento(id, "FALLO critico en " + nombre);
        }
        actualizarLista();
    }

    void verHistorial() {
        QWidget *winLogs = new QWidget(nullptr, Qt::Window);
        winLogs->setWindowTitle("Historial de Auditoría del Sistema");
        winLogs->resize(550, 400);
        winLogs->setStyleSheet("background-color: #121824; color: #f8fafc; font-family: Helvetica;");

        QVBoxLayout *layout = new QVBoxLayout(winLogs);
        QLabel *lbl = new QLabel("HISTORIAL DE EVENTOS REGISTRADOS");
        lbl->setStyleSheet("color: #38bdf8; font-size: 12px; font-weight: bold;");
        lbl->setAlignment(Qt::AlignCenter);
        layout->addWidget(lbl);

        QListWidget *listLogs = new QListWidget();
        listLogs->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: none; font-family: Consolas;");
        QList<LogData> logs = obtenerHistorial();
        for (const auto &l : logs) {
            listLogs->addItem(QString(" [%1]  |  ID Disp: %2  -->  %3").arg(l.fecha_hora).arg(l.id_dispositivo).arg(l.accion));
        }
        layout->addWidget(listLogs);
        winLogs->show();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (!inicializarDb()) return -1;
    DomoticaApp ventana;
    ventana.show();
    return app.exec();
}