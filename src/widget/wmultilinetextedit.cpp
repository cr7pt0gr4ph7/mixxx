#include "widget/wmultilinetextedit.h"

#include <QSizeF>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wmultilinetextedit.cpp"

WMultiLineTextEdit::WMultiLineTextEdit(QWidget* parent)
        : QPlainTextEdit(parent) {
    // The added viewportMargin is required to ensure the inner
    // content of the text editor does not accidentally overwrite
    // the focus frame of the whole control. The documentMargin is
    // reduced by an equal amount so the total amount of padding
    // stays the same.
    setViewportMargins(4, 4, 4, 4);
    document()->setDocumentMargin(0);
}

QSize WMultiLineTextEdit::minimumSizeHint() const {
    const int minLines = 2;
    return sizeHintImpl(minLines);
}

QSize WMultiLineTextEdit::sizeHint() const {
    const int minLines = 2;
    return sizeHintImpl(minLines);
}

QSize WMultiLineTextEdit::sizeHintImpl(const int minLines) const {
    const auto w = 0.0;
    const auto h = 2 * frameWidth() + 2 * document()->documentMargin() +
            QFontMetrics(font()).lineSpacing() * minLines;

    return QSizeF(w, h).toSize();
}

bool WMultiLineTextEdit::event(QEvent* e) {
    switch (e->type()) {
    case QEvent::Paint: {
        QStylePainter p(this);
        QStyleOptionFrame option;
        initStyleOption(&option);
        if (isReadOnly()) {
            option.state |= QStyle::State_ReadOnly;
        }
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &p, this);
        return true;
    }
    default: {
        return QPlainTextEdit::event(e);
    }
    }
}
