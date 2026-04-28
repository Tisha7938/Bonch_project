#pragma once
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <mutex>

//! @brief Утилитарный логгер для имитационного моделирования
//! Выводит сообщения в консоль (qDebug) и опционально в текстовый файл(мейби на будущее).
class Logger {
public:
    //! @brief Получить синглтон
    static Logger &instance() {
        static Logger inst;
        return inst;
    }

    //! @brief Записать лог
    //! @param msg Текст сообщения
    //! @param level Уровень лога (INFO, EVENT, WARN, STEP)
    void log(const QString &msg, const QString &level = "INFO") {
        std::lock_guard<std::mutex> lock(m_mutex);
        QString line =
                QString("[%1] [%2] %3").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz")).arg(level).arg(msg);
        qDebug().noquote() << line;
        if (m_file.isOpen()) {
            m_stream << line << Qt::endl;
        }
    }

    //! @brief Открыть файл для записи логов
    void openFile(const QString &path) {
        m_file.setFileName(path);
        if (m_file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_stream.setDevice(&m_file);
        }
    }

    //! @brief Закрыть файл логов
    void close() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.isOpen())
            m_file.close();
    }

private:
    Logger() = default;
    QFile m_file;
    QTextStream m_stream;
    std::mutex m_mutex;
};
