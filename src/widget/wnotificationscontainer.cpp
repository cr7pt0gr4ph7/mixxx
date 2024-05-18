#include "widget/wnotificationscontainer.h"

#include <QStyleOption>
#include <QStylePainter>

#include "moc_wnotificationscontainer.cpp"

WNotificationsContainer::WNotificationsContainer(QWidget* pParent, WNotifications* pSource)
        : QWidget(pParent) {
    // The notifications appear on top of everything else,
    // but do not receive any events.
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_expirationTimer = make_parented<QTimer>(this);

    connect(m_expirationTimer,
            &QTimer::timeout,
            this,
            &WNotificationsContainer::slotHideNotification);

    // Connect to the notifications broker
    //
    // This should only be done after everything else has been
    // set up, because notifications can arrive immediately after
    // registration, due to cross-thread notifications
    if (!pSource) {
        pSource = WNotifications::globalInstance();
    }

    connect(pSource,
            &WNotifications::showNotification,
            this,
            &WNotificationsContainer::slotShowNotification,
            Qt::QueuedConnection);
}

void WNotificationsContainer::setup(const QDomNode& node, const SkinContext& context) {
}

void WNotificationsContainer::slotShowNotification(WNotificationEvent event) {
    double duration = event.duration();
    if (duration > 0.0) {
        // Positive values specify a duration in seconds directly
    } else if (duration == WNotificationEvent::DURATION_SHORT) {
        duration = 1.0;
    } else if (duration == WNotificationEvent::DURATION_LONG) {
        duration = 5.0;
    } else { // DURATION_DEFAULT and all other negative values
        duration = 1.0;
    }

    // Immediately replace the current notification with the new one,
    // and restart the expiration timer
    m_lastText = event.text();
    m_expirationTimer->start(std::chrono::milliseconds(int(1000.0 * duration)));
    update();
}

void WNotificationsContainer::slotHideNotification() {
    m_lastText = QString();
    update();
}

void WNotificationsContainer::paintEvent(QPaintEvent*) {
    QStylePainter painter(this);
    QStyleOptionFrame option;
    option.initFrom(this);

    if (!m_lastText.isEmpty()) {
        painter.setPen(option.palette.color(QPalette::Inactive, QPalette::Text));
        painter.drawText(0, 0, m_lastText);
    }
}
