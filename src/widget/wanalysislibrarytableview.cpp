#include "widget/wanalysislibrarytableview.h"

#include "moc_wanalysislibrarytableview.cpp"

WAnalysisLibraryTableView::WAnalysisLibraryTableView(
        QWidget* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard,
        double trackTableBackgroundColorOpacity)
        : WTrackTableView(parent,
                  pConfig,
                  pLibrary,
                  pKeyboard,
                  trackTableBackgroundColorOpacity,
                  true) {
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true); //Always enable drag for now (until we have a model that doesn't support this.)
}

void WAnalysisLibraryTableView::onSearch(const QString& text) {
    Q_UNUSED(text);
}
