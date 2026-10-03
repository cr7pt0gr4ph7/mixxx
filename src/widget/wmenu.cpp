

#include "widget/wmenu.h"

#include <QEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QStyleOption>

#include "moc_wmenu.cpp"

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

void WMenu::keyPressEvent(QKeyEvent* pEvent) {
    switch (pEvent->key()) {
    case Qt::Key_Space:
    case Qt::Key_Select:
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        // Custom logic to avoid closing the menu & modify popup behavior
        // when using our custom WCheckableAction menu items.
        QAction* pAction = activeAction();
        if (!pAction) {
            break;
        }

        WCheckableAction* pCheckableAction = qobject_cast<WCheckableAction*>(pAction);
        if (!pCheckableAction || !pCheckableAction->isCheckable()) {
            break;
        }

        // Toggle the status and repaint the menu item
        pEvent->accept();
        pCheckableAction->toggle();
        QRect actionRect = actionGeometry(pAction);
        if (actionRect.isValid()) {
            update(actionRect);
        }
    }
    default: {
        break;
    }
    }

    // Fall back to default behavior
    QMenu::keyPressEvent(pEvent);
}

void WMenu::mouseReleaseEvent(QMouseEvent* pEvent) {
    // Custom logic to avoid closing the menu & modify popup behavior
    // when using our custom WCheckableAction menu items.
    QAction* pAction = activeAction();
    if (!pAction) {
        // Fall back to default behavior
        QMenu::mouseReleaseEvent(pEvent);
        return;
    }

    WCheckableAction* pCheckableAction = qobject_cast<WCheckableAction*>(pAction);
    if (!pCheckableAction || !pCheckableAction->isCheckable()) {
        // Fall back to default behavior
        QMenu::mouseReleaseEvent(pEvent);
        return;
    }

    // Toggle the status and repaint the menu item
    pEvent->accept();
    pCheckableAction->toggleOrTrigger();
    QRect actionRect = actionGeometry(pAction);
    if (actionRect.isValid()) {
        update(actionRect);
    }
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
