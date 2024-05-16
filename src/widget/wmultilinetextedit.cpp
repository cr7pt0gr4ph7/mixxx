#include "widget/wmultilinetextedit.h"

#include "moc_wmultilinetextedit.cpp"

WMultiLineTextEdit::WMultiLineTextEdit(QWidget* parent)
        : QPlainTextEdit(parent) {
}

QSize WMultiLineTextEdit::minimumSizeHint() const {
    return QSize(0, 0);
}

QSize WMultiLineTextEdit::sizeHint() const {
    const int minLines = 2;

    const auto w = 0.0;
    const auto h = 2 * frameWidth() + 2 * document()->documentMargin() +
            QFontMetrics(font()).lineSpacing() * minLines;

    return QSize(w, h);
}
