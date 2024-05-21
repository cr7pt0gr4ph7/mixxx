#include "widget/wbasiclabel.h"

#include <QFocusEvent>
#include <QKeyEvent>

#include "moc_wbasiclabel.cpp"

WBasicLabel::WBasicLabel(QWidget* parent, Qt::WindowFlags f)
        : QLabel(parent, f) {
}
WBasicLabel::WBasicLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
        : QLabel(text, parent, f) {
}

void WBasicLabel::focusInEvent(QFocusEvent* event) {
    auto focusReason = event->reason();
    if (focusReason == Qt::TabFocusReason ||
            focusReason == Qt::BacktabFocusReason ||
            focusReason == Qt::ShortcutFocusReason ||
            focusReason == Qt::OtherFocusReason) {
        selectAll();
    }
}

void WBasicLabel::selectAll() {
    auto flags = textInteractionFlags();
    if (flags & (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard)) {
        this->setSelection(0, text().size());
    }
}

void WBasicLabel::keyPressEvent(QKeyEvent* event) {
    bool noModifiersPressed = !(event->modifiers() &
            (Qt::ControlModifier | Qt::AltModifier |
                    Qt::ShiftModifier | Qt::MetaModifier));

    if (event->key() == Qt::Key_Up && noModifiersPressed) {
        if (focusPreviousChild()) {
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Down && noModifiersPressed) {
        if (focusNextChild()) {
            event->accept();
            return;
        }
    }

    QLabel::keyPressEvent(event);
}
