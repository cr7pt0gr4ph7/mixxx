#pragma once

#include <QString>
#include <QTimer>
#include <QWidget>

#include "util/notifications.h"
#include "util/parented_ptr.h"

class QDomNode;
class SkinContext;

/// Displays notifications sent via WNotifications::show
class WNotificationsContainer : public QWidget {
    Q_OBJECT
  public:
    explicit WNotificationsContainer(QWidget* pParent = nullptr, WNotifications* pSource = nullptr);

    virtual void setup(const QDomNode& node, const SkinContext& context);

  protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* target, QEvent* event) override;

  private slots:
    void slotShowNotification(WNotificationEvent event);
    void slotHideNotification();

  private:
    parented_ptr<QTimer> m_expirationTimer;
    QString m_lastText;
};
