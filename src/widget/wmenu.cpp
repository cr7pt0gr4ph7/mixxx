

#include "widget/wmenu.h"

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

void WMenu::initStyleOption(QStyleOptionMenuItem* pOption, const QAction* pAction) const {
    QMenu::initStyleOption(pOption, pAction);

    const WCheckableAction* pCheckableAction = qobject_cast<const WCheckableAction*>(pAction);
    if (pCheckableAction) {
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
