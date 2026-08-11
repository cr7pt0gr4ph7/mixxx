#include "library/trackset/basecratefeature.h"

#include "library/library.h"
#include "moc_basecratefeature.cpp"

BaseCrateFeature::BaseCrateFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& rootViewName,
        const QString& iconName)
        : BaseTrackSetFeature(pLibrary, pConfig, rootViewName, iconName) {
}
