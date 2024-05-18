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
    m_expirationTimer->setSingleShot(true);

    connect(m_expirationTimer,
            &QTimer::timeout,
            this,
            &WNotificationsContainer::slotHideNotification);

    // Track size changes of the parent to adapt our layout accordingly
    if (pParent) {
        pParent->installEventFilter(this);
    }

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
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WNotificationsContainer::slotShowNotification(WNotificationEvent event) {
    double duration = event.duration();
    if (duration > 0.0) {
        // Positive values specify a duration in seconds directly
    } else if (duration == WNotificationEvent::DURATION_SHORT) {
        duration = 3.0;
    } else if (duration == WNotificationEvent::DURATION_LONG) {
        duration = 8.0;
    } else { // DURATION_DEFAULT and all other negative values
        duration = 3.0;
    }

    qDebug() << "slotShowNotification" << duration << event.text() << size() << sizeHint();

    // Immediately replace the current notification with the new one,
    // and restart the expiration timer
    m_lastText = event.text();
    m_expirationTimer->start(std::chrono::milliseconds(int(1000.0 * duration)));
    update();
    show();
    raise();
}

void WNotificationsContainer::slotHideNotification() {
    qDebug() << "slotHideNotification";
    m_lastText = QString();
    update();
}

void WNotificationsContainer::paintEvent(QPaintEvent*) {
    QStylePainter painter(this);
    QStyleOptionFrame option;
    option.initFrom(this);

    if (m_lastText.isEmpty()) {
        // Nothing to display
        return;
    }

    // The full area of the parent
    const QSize parentSize = size();

    // Notifications should be shown horizontally centered and near the
    // bottom of the parent widget, but with a specific amount of spacing
    // between the bottom of the notification and the bottom of the parent
    const QSize drawAreaSize(
            qMin(parentSize.width(), qMax(200, int(parentSize.width() * 0.8))),
            qMin(option.fontMetrics.lineSpacing() * 3, parentSize.height()));
    QRect drawArea(QPoint(0, 0), drawAreaSize);

    // Calculate the area taken up by the text
    const int textFlags = Qt::TextWordWrap | Qt::AlignHCenter | Qt::AlignBottom;
    QRect textBounds = painter.boundingRect(drawArea, textFlags, m_lastText);
    QRect originalTextBound = textBounds;
    textBounds.moveCenter(QPoint(drawAreaSize.width() / 2, 0));
    // FIXME(cr7pt0gr4ph): Consider the margin set in the stylesheet
    textBounds.moveBottom(parentSize.height() - 80);

    // Draw the background
    // FIXME(cr7pt0gr4ph): Consider the padding set in the stylesheet
    const QRect borderBounds = textBounds.marginsAdded(QMargins(10, 10, 10, 10));
    const double borderRadius = borderBounds.height() / 2.0;
    painter.setBrush(QColorConstants::Black);
    painter.drawRoundedRect(borderBounds, borderRadius, borderRadius);

    // Draw the text
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColorConstants::White);
    // painter.setPen(option.palette.color(QPalette::Inactive, QPalette::Text));
    painter.drawText(textBounds, textFlags, m_lastText);

    qDebug() << "drawNotifications" << originalTextBound << textBounds << borderBounds << drawArea;
}

bool WNotificationsContainer::eventFilter(QObject* target, QEvent* event) {
    switch (event->type()) {
    case QEvent::Resize: {
        // Parent has been resized, update our size and position accordingly
        QWidget* p = qobject_cast<QWidget*>(parent());
        if (p && p == target) {
            QRect rect(QPoint(0, 0), p->size());
            setGeometry(rect);
        }
        break;
    }
    default: {
        break;
    }
    }
    return false;
}
