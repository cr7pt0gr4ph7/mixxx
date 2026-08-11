#pragma once

#include "library/trackset/basetracksetfeature.h"
#include "preferences/usersettings.h"

// forward declaration(s)
class Library;

class BaseCrateFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    BaseTrackSetFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& rootViewName,
            const QString& iconName);
    ~BaseCrateFeature() override = default;
};
