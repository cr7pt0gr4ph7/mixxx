#pragma once

#include <QObject>

#include "library/trackset/crate/cratefolderid.h"
#include "library/trackset/crate/crateid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class Crate;

class CrateFeatureHelper : public QObject {
    Q_OBJECT

  public:
    CrateFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~CrateFeatureHelper() override = default;

    CrateId createEmptyCrate(CrateFolderId folderId);
    CrateFolderId createEmptyFolder(CrateFolderId parentId);
    CrateId duplicateCrate(const Crate& oldCrate);

  private:
    QString proposeNameForNewCrate(
            CrateFolderId folderId,
            const QString& initialName = QString()) const;
    QString proposeNameForNewFolder(
            CrateFolderId parentId,
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
