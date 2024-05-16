#pragma once

#include <QPlainTextEdit>

class WMultiLineTextEdit : public QPlainTextEdit {
    Q_OBJECT
  public:
    WMultiLineTextEdit(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;

    QSize sizeHint() const override;
};
