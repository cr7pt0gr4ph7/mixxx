#include "util/notifications.h"

#include <QtDebug>

#include "moc_notifications.cpp"

// QEvent::Type WNotificationEvent::s_customEventType = QEvent::None;

WNotifications* WNotifications::s_globalInstance;

void WNotifications::show(const QString& text, WNotificationEvent::Duration duration) {
    qInfo() << QStringLiteral("Notification (Duration = %1 seconds)").arg(duration) << text;
    emit globalInstance()->showNotification(WNotificationEvent(text, duration));
}
