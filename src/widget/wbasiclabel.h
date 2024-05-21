#pragma once

#include <QLabel>

/// Subclass of QLabel that allows the Up and Down arrow keys
/// to be used like the Tab/Backtab keys.
///
/// See WLabel when you need to connect to a ControlObject.
class WBasicLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WBasicLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit WBasicLabel(const QString& text,
            QWidget* parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

    /// Select all text in this widget.
    void selectAll();

  protected:
    void focusInEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};
