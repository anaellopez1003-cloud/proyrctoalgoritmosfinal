/**
 * @file db_manager.h
 * @brief Archivo de cabecera para la gestión de la base de datos del sistema domótico.
 * Contiene las estructuras de datos y las funciones necesarias para interactuar 
 * con SQLite3 usando las librerías de Qt5.
 */

#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QList>

/**
 * @struct DeviceData
 * @brief Estructura que almacena la información de cada dispositivo domótico.
 * Mapea directamente los registros de la tabla "dispositivos" de la base de datos.
 */
struct DeviceData {
    int id;             /**< Identificador único autoincrementable del dispositivo. */
    QString nombre;     /**< Nombre descriptivo (ej. "Lámpara Sala"). */
    QString tipo;       /**< Categoría del dispositivo. */
    QString estado;     /**< Estado actual (ej. "Encendido", "Apagado"). */
    double consumo;     /**< Consumo eléctrico acumulado. */
};

/**
 * @struct LogData
 * @brief Estructura que almacena el registro de eventos del sistema.
 * Mapea los registros de la tabla "historial" para llevar trazabilidad de acciones.
 */
struct LogData {
    int id_evento;      /**< Identificador único del registro en el historial. */
    QString fecha_hora; /**< Fecha y hora exacta en la que ocurrió el evento. */
    int id_dispositivo; /**< ID del dispositivo que generó el evento. */
    QString accion;     /**< Descripción de la acción (ej. "Encendido automático"). */
};

/**
 * @brief Inicializa la conexión con la base de datos SQLite.
 * Configura la base de datos local "domotica.db" y crea las tablas principales
 * (dispositivos e historial) en caso de que el archivo sea nuevo.
 * * @return true Si la base de datos se abrió y preparó correctamente.
 * @return false Si falló la conexión con el driver QSQLITE.
 */
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

/**
 * @brief Registra un nuevo dispositivo en el sistema.
 * Lo inserta en la base de datos con un estado inicial "Apagado" y consumo 0.0.
 * * @param nombre Nombre descriptivo del nuevo dispositivo.
 * @param tipo Categoría o clasificación del dispositivo.
 */
inline void registrarDispositivo(QString nombre, QString tipo) {
    QSqlQuery query;
    query.prepare("INSERT INTO dispositivos (nombre, tipo, estado, consumo) VALUES (?, ?, 'Apagado', 0.0)");
    query.addBindValue(nombre);
    query.addBindValue(tipo);
    query.exec();
}

/**
 * @brief Guarda un evento en el historial del sistema.
 * Genera automáticamente una marca de tiempo actual usando QDateTime.
 * * @param id_dispositivo El ID del dispositivo que ejecutó o sufrió la acción.
 * @param accion Descripción de lo que sucedió.
 */
inline void registrarEvento(int id_dispositivo, QString accion) {
    QSqlQuery query;
    QString fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    query.prepare("INSERT INTO historial (fecha_hora, id_dispositivo, accion) VALUES (?, ?, ?)");
    query.addBindValue(fecha);
    query.addBindValue(id_dispositivo);
    query.addBindValue(accion);
    query.exec();
}

/**
 * @brief Obtiene la lista completa de dispositivos registrados.
 * * @return QList<DeviceData> Una lista de Qt con las estructuras de todos los dispositivos.
 */
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

/**
 * @brief Actualiza el estado (ON/OFF) de un dispositivo específico.
 * * @param id El identificador único del dispositivo a modificar.
 * @param estado El nuevo estado que se le asignará.
 */
inline void actualizarEstado(int id, QString estado) {
    QSqlQuery query;
    query.prepare("UPDATE dispositivos SET estado = ? WHERE id = ?");
    query.addBindValue(estado);
    query.addBindValue(id);
    query.exec();
}

/**
 * @brief Incrementa el valor de consumo de un dispositivo.
 * Suma la cantidad indicada al total histórico de consumo del dispositivo.
 * * @param id El identificador único del dispositivo.
 * @param cantidad Valor numérico (double) a sumar al consumo actual.
 */
inline void incrementarConsumo(int id, double cantidad) {
    QSqlQuery query;
    query.prepare("UPDATE dispositivos SET consumo = consumo + ? WHERE id = ?");
    query.addBindValue(cantidad);
    query.addBindValue(id);
    query.exec();
}

/**
 * @brief Recupera el historial completo de eventos del sistema.
 * Los eventos se entregan ordenados desde el más reciente al más antiguo.
 * * @return QList<LogData> Una lista con el registro de eventos cronológicos.
 */
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