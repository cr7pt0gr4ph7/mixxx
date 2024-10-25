#pragma once

#include <QList>
#include <QObject>
#include <QPair>
#include <memory>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class PlayerManagerInterface;
class QNetworkAccessManager;

class MetadataPushManager : public QObject {
    Q_OBJECT
  public:
    MetadataPushManager(QObject* pParent,
            UserSettingsPointer pConfig,
            PlayerManagerInterface* pPlayerManager);

    /// Returns true if publishing of "now playing" information is enabled.
    /// Note this only indicates whether publishing is enabled, not whether
    /// there is actually a listener connected.
    bool isEnabled();

    /// Set whether or not publishing of "now playing" metadata is enabled.
    void setEnabled(bool enabled);

  private slots:
    void slotPlayStateChanged(double v);

  private:
    QList<QPair<bool, TrackPointer>> getPlayingTracks();
    QByteArray buildJson(QList<QPair<bool, TrackPointer>> playingTracks);

  private:
    UserSettingsPointer m_pConfig;
    PlayerManagerInterface* m_pPlayerManager;
    QNetworkAccessManager* m_pNetworkAccessManager;

    std::unique_ptr<ControlPushButton> m_pPublishNowPlaying;

    QList<ControlProxy*> m_decks;

    DISALLOW_COPY_AND_ASSIGN(MetadataPushManager);
};
