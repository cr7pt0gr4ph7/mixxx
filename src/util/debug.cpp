#include "util/debug.h"

#include "errordialoghandler.h"

void reportFatalErrorAndQuit(const QString& message) {
    QByteArray message_bytes = message.toLocal8Bit();
    qFatal("%s", message_bytes.constData());
    ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();
    if (dialogHandler) {
        dialogHandler->requestErrorDialog(DLG_FATAL, message, true);
    }
}

void reportCriticalErrorAndQuit(const QString& message) {
    qCritical() << message;
    ErrorDialogHandler* dialogHandler = ErrorDialogHandler::instance();
    if (dialogHandler) {
        dialogHandler->requestErrorDialog(DLG_CRITICAL, message, true);
    }
}
