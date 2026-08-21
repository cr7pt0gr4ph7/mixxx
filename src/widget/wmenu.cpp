

#include "widget/wmenu.h"

#include "moc_wmenu.cpp"

WMenu::WMenu(QWidget* pParent)
        : WMenu(QString(), pParent) {
}

WMenu::WMenu(const QString& title, QWidget* pParent)
        : QMenu(title, pParent) {
}
