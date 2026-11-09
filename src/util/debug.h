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

// Calling this will report a qFatal and quit Mixxx, possibly disgracefully. Use
// very sparingly! A modal message box will be issued to the user which allows
// the Qt event loop to continue processing. This means that you must not call
// this function from a code section which is not re-entrant (e.g. paintEvent on
// a QWidget).
void reportFatalErrorAndQuit(const QString& message);

// Calling this will report a qCritical and quit Mixxx, possibly
// disgracefully. Use very sparingly! A modal message box will be issued to the
// user which allows the Qt event loop to continue processing. This means that
// you must not call this function from a code section which is not re-entrant
// (e.g. paintEvent on a QWidget).
void reportCriticalErrorAndQuit(const QString& message);
