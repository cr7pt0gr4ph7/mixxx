#pragma once

#include <QMenu>

/// This is a custom QMenu fixing some bugs/quirks that occur when
/// QCheckBox is packed into QWidgetAction. It works in tandem with
/// WMenuCheckBox.
class WMenu : public QMenu {
    Q_OBJECT

  public:
    explicit WMenu(QWidget* parent = nullptr);
    explicit WMenu(const QString& title, QWidget* parent = nullptr);

  protected:
    virtual void initStyleOption(QStyleOptionMenuItem* pOption, const QAction* pAction) const override;
};
