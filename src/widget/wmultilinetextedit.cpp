#include "widget/wmultilinetextedit.h"

#include "moc_wmultilinetextedit.cpp"

WMultiLineTextEdit::WMultiLineTextEdit(QWidget* parent)
        : QPlainTextEdit(parent) {
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

    return QSize(w, h);
}
