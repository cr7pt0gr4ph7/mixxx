#pragma once

#include "preferences/usersettings.h"
#include "widget/wtracktableview.h"

class WAnalysisLibraryTableView : public WTrackTableView {
    Q_OBJECT
  public:
    WAnalysisLibraryTableView(
            QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard,
            double trackTableBackgroundColorOpacity);

    void onSearch(const QString& text) override;
};
