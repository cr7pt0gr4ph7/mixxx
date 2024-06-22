#pragma once

#include <QEvent>
#include <QObject>
#include <QString>

class WNotificationEvent /*: public QEvent*/ {
  public:
    /// Specifies how long a notification should be visible to the user.
    /// Positive values specify a time in seconds, negative values
    /// have special meanings.
    typedef double Duration;

    static constexpr Duration DURATION_DEFAULT = 0.0;
    static constexpr Duration DURATION_SHORT = -1.0;
    static constexpr Duration DURATION_LONG = -2.0;

    WNotificationEvent(const QString& text,
            const Duration duration = DURATION_DEFAULT)
            : m_text(text), m_duration(duration) {
    }

    /// Gets the localized text of this notification
    inline const QString& text() {
        return m_text;
    }

    /// Gets how long this notification should be displayed
    /// before fading out. The time is specified in seconds.
    ///
    /// Negative values and zeroes have special meaning,
    /// see DURATION_DEFAULT, DURATION_SHORT and DURATION_LONG.
    inline Duration duration() {
        return m_duration;
    }

    // static QEvent::Type type() {
    //     // TODO(cr7pt0gr4ph7): This is what Qt recommends, but its not thread-safe....
    //     // ...unless we make sure it gets called by the main thread before
    //     // anybody else has the chance to call it.
    //     if (s_customEventType == QEvent::None)
    //     {
    //         int generatedType = QEvent::registerEventType();
    //         s_customEventType = static_cast<QEvent::Type>(generatedType);
    //     }
    //     return s_customEventType;
    // }

  private:
    // static QEvent::Type s_customEventType;
    QString m_text;
    Duration m_duration;
};

class WNotifications : public QObject {
    Q_OBJECT
  public:
    static void show(const QString& text,
            WNotificationEvent::Duration duration =
                    WNotificationEvent::DURATION_DEFAULT);

    /// Gets the default WNotifications broker for this application.
    static inline WNotifications* globalInstance() {
        // TODO(cr7pt0gr4ph7): This is not threadsafe (but should be)
        if (s_globalInstance) {
            return s_globalInstance;
        }
        s_globalInstance = new WNotifications();
        return s_globalInstance;
    }

  signals:
    void showNotification(WNotificationEvent e);

  private:
    static WNotifications* s_globalInstance;
};
