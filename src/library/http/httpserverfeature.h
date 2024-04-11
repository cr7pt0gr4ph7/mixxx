// This feature queries tracks, playlists and crates from
// a HTTP API. This can be used to integrate alternative
// database backends with Mixxx.

#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <atomic>

#include "library/baseexternallibraryfeature.h"
#include "library/http/httpserverconnection.h"


class HttpServerPlaylistModel;

class HttpServerFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    HttpServerFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~HttpServerFeature() override;
    static bool isSupported();

    QVariant title() override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private:
    void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist) override;

    HttpServerPlaylistModel* m_pHttpServerPlaylistModel;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    QStringList m_playlists;

    HttpServerConnection m_connection;
    bool m_isActivated;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;
    std::atomic<bool> m_cancelImport;
};
