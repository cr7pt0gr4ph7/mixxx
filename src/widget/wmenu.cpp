

#include "widget/wmenu.h"

#include <QEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QStyleOption>

#include "moc_wmenu.cpp"
#include "widget/wcheckableaction.h"

WMenu::WMenu(QWidget* pParent)
        : WMenu(QString(), pParent) {
}

WMenu::WMenu(const QString& title, QWidget* pParent)
        : QMenu(title, pParent) {
}

bool WMenu::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ShortcutOverride) {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent *>(pEvent);
        if (pKeyEvent->key() == Qt::Key_Space) {
            pEvent->accept();
            return true;
        }
    }
    return QMenu::event(pEvent);
}

void WMenu::initStyleOption(QStyleOptionMenuItem* pOption, const QAction* pAction) const {
    QMenu::initStyleOption(pOption, pAction);

    const WCheckableAction* pCheckableAction = qobject_cast<const WCheckableAction*>(pAction);
    if (pCheckableAction) {
        // Clear existing state-related flags set by the base class
        pOption->state &= ~(QStyle::State_NoChange | QStyle::State_On | QStyle::State_Off);

        Qt::CheckState checkState = pCheckableAction->checkState();
        if (checkState == Qt::PartiallyChecked) {
            pOption->state |= QStyle::State_NoChange;
        } else if (checkState == Qt::Checked) {
            pOption->state |= QStyle::State_On;
        } else {
            pOption->state |= QStyle::State_Off;
        }
    }
}
