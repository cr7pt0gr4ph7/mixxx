#pragma once

#include <QAction>

/// This is a custom checkable QAction that also offers an "indetermined"
/// state. This avoids the bugs/quirks that occur when QCheckBox is packed
/// into QWidgetAction. It works in tandem with WMenu.
class WCheckableAction : public QAction {
    Q_OBJECT

    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate);

  public:
    explicit WCheckableAction(QObject *parent = nullptr);
    explicit WCheckableAction(const QString &text, QObject *parent = nullptr);
    explicit WCheckableAction(const QIcon &icon, const QString &text, QObject *parent = nullptr);

    // Toggle the check state without closing the parent menu.
    void toggle();

    // Calls toggle() for checkable actions, and trigger() for normal actions
    void toggleOrTrigger();

    void setTristate(bool value = true) {
        m_bTriState = value;
    }

    bool isTristate() const {
        return m_bTriState;
    }

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

  signals:
    void checkStateChanged(Qt::CheckState state);

  private slots:
    void onToggled(bool checked);

  private:
    bool m_bTriState;
    bool m_bNoChange;
    bool m_bInSetCheckState;
    Qt::CheckState m_ePublishedState;
};
