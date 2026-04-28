#pragma once
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <mutex>

//! @brief Лёгкий потокобезопасный логгер для имитационного моделирования
class Logger {
public:
    static void info(const QString &msg) {
        std::lock_guard<std::mutex> lock(mutex());
        qDebug().noquote()
                << QString("[INFO] [%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz")).arg(msg);
    }

    static void event(unsigned int nodeId, const QString &type) {
        std::lock_guard<std::mutex> lock(mutex());
        qDebug().noquote() << QString("[EVENT] [%1] Node-%2: %3")
                                      .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                                      .arg(nodeId)
                                      .arg(type);
    }

    static void step(double time, double availability) {
        std::lock_guard<std::mutex> lock(mutex());
        qDebug().noquote()
                << QString("[STEP] T=%1 | Availability=%2").arg(time, 0, 'f', 1).arg(availability, 0, 'f', 3);
    }


private:
    static std::mutex &mutex() {
        static std::mutex m;
        return m;
    }
};
