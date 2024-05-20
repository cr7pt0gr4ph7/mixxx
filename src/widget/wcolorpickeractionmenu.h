#pragma once

#include <QMenu>

#include "util/color/colorpalette.h"
#include "util/color/rgbcolor.h"
#include "widget/wcolorpicker.h"

class WColorPickerAction;

class WColorPickerActionMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WColorPickerActionMenu(
            WColorPicker::Options options,
            const ColorPalette& palette,
            QWidget* parent = nullptr);

    /// Sets whether the color picker should be able to get keyboard focus.
    void setAllowKeyboardFocus(bool enable);
    bool allowKeyboardFocus() const {
        return m_allowKeyboardFocus;
    }

    /// Set a new color palette for the underlying color picker.
    void setColorPalette(const ColorPalette& palette);
    void setSelectedColor(const mixxx::RgbColor::optional_t& color);
    void resetSelectedColor();

  signals:
    void colorPicked(const mixxx::RgbColor::optional_t& color);

  protected:
    bool focusNextPrevChild(bool next) override;
    void keyPressEvent(QKeyEvent* event) override;

  private:
    parented_ptr<WColorPickerAction> m_pColorPickerAction;
    bool m_allowKeyboardFocus;
};
