#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QList>

struct DeviceData {
    int id;
    QString nombre;
    QString tipo;
    QString estado;
    double consumo;
};

struct LogData {
    int id_evento;
    QString fecha_hora;
    int id_dispositivo;
    QString accion;
};

inline bool inicializarDb() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("domotica.db");
    if (!db.open()) return false;

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS dispositivos ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "nombre TEXT NOT NULL, "
               "tipo TEXT NOT NULL, "
               "estado TEXT NOT NULL, "
               "consumo REAL DEFAULT 0.0)");

    query.exec("CREATE TABLE IF NOT EXISTS historial ("
               "id_evento INTEGER PRIMARY KEY AUTOINCREMENT, "
               "fecha_hora TEXT NOT NULL, "
               "id_dispositivo INTEGER, "
               "accion TEXT NOT NULL)");
    return true;
}

inline void registrarDispositivo(QString nombre, QString tipo) {
    QSqlQuery query;
    query.prepare("INSERT INTO dispositivos (nombre, tipo, estado, consumo) VALUES (?, ?, 'Apagado', 0.0)");
    query.addBindValue(nombre);
    query.addBindValue(tipo);
    query.exec();
}

inline void registrarEvento(int id_dispositivo, QString accion) {
    QSqlQuery query;
    QString fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    query.prepare("INSERT INTO historial (fecha_hora, id_dispositivo, accion) VALUES (?, ?, ?)");
    query.addBindValue(fecha);
    query.addBindValue(id_dispositivo);
    query.addBindValue(accion);
    query.exec();
}

inline QList<DeviceData> obtenerDispositivos() {
    QList<DeviceData> lista;
    QSqlQuery query("SELECT * FROM dispositivos");
    while (query.next()) {
        DeviceData d;
        d.id = query.value(0).toInt();
        d.nombre = query.value(1).toString();
        d.tipo = query.value(2).toString();
        d.estado = query.value(3).toString();
        d.consumo = query.value(4).toDouble();
        lista.append(d);
    }
    return lista;
}

inline void actualizarEstado(int id, QString estado) {
    QSqlQuery query;
    query.prepare("UPDATE dispositivos SET estado = ? WHERE id = ?");
    query.addBindValue(estado);
    query.addBindValue(id);
    query.exec();
}

inline void incrementarConsumo(int id, double cantidad) {
    QSqlQuery query;
    query.prepare("UPDATE dispositivos SET consumo = consumo + ? WHERE id = ?");
    query.addBindValue(cantidad);
    query.addBindValue(id);
    query.exec();
}

inline QList<LogData> obtenerHistorial() {
    QList<LogData> lista;
    QSqlQuery query("SELECT * FROM historial ORDER BY id_evento DESC");
    while (query.next()) {
        LogData l;
        l.id_evento = query.value(0).toInt();
        l.fecha_hora = query.value(1).toString();
        l.id_dispositivo = query.value(2).toInt();
        l.accion = query.value(3).toString();
        lista.append(l);
    }
    return lista;
}

#endif