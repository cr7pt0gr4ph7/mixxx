#pragma once

#include <QtDebug>
#include <QString>

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
#define DBG(value)                                                                         \
    (([](auto&& val) {                                                                     \
        qDebug().nospace() << #value " = " << val << " [" __FILE__ ":" << __LINE__ << "]"; \
        return val;                                                                        \
    })(value))
#else
#define DBG(value)                                                             \
    static_assert(false,                                                       \
            "DBG() is not allowed in builds without debug assertions, please " \
            "remove it!")
#endif

template<typename T>
QString toDebugString(const T& object) {
    QString output;
#ifndef QT_NO_DEBUG_OUTPUT
    QDebug deb(&output);
    deb << object;
#else
    Q_UNUSED(object);
#endif
    return output;
}

void reportFatalErrorAndQuit(const QString& message);
void reportCriticalErrorAndQuit(const QString& message);
