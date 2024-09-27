#include "integrations/metadatapushmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_metadatapushmanager.cpp"
#include "track/track.h"

MetadataPushManager::MetadataPushManager(QObject* pParent,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager) {
    m_pNetworkAccessManager = new QNetworkAccessManager(this);

    const bool persist = true;
    m_pPublishNowPlaying = std::make_unique<ControlPushButton>(
            ConfigKey("[NowPlaying]", "enabled"), persist);
    m_pPublishNowPlaying->setButtonMode(mixxx::control::ButtonMode::Toggle);

    // TODO(cr7pt0gr4ph7) listen to signals from PlayerManager and add/remove as decks
    // are created.
    for (unsigned int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        QString group = PlayerManager::groupForDeck(i);
        auto* pProxy = new ControlProxy(group, "play", this);
        m_decks.append(pProxy);
        pProxy->connectValueChanged(this, &MetadataPushManager::slotPlayStateChanged);
    }
}

void MetadataPushManager::setEnabled(bool value) {
    m_pPublishNowPlaying->set(value);
}

bool MetadataPushManager::isEnabled() {
    return m_pPublishNowPlaying->toBool();
}

void MetadataPushManager::slotPlayStateChanged(double v) {
    Q_UNUSED(v);
    auto playingTracks = getPlayingTracks();
    auto body = buildJson(playingTracks);
    auto host = qgetenv("METADATA_PUSH_HOST");
    auto request = QNetworkRequest(QUrl(QStringLiteral("http://%1/now-playing/update").arg(host)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    m_pNetworkAccessManager->post(request, body);
}

QList<TrackPointer> MetadataPushManager::getPlayingTracks() {
    QList<TrackPointer> tracks;
    for (unsigned int i = 0; i < m_pPlayerManager->numberOfDecks(); ++i) {
        QString group = PlayerManager::groupForDeck(i);
        BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
        TrackPointer pTrack = pPlayer->getLoadedTrack();
        if (pTrack) {
            tracks.append(pTrack);
        }
    }
    return tracks;
}

QByteArray MetadataPushManager::buildJson(QList<TrackPointer> playingTracks) {
    QJsonArray tracks;

    for (auto trackPointer : playingTracks) {
        QJsonObject track{
                {"title", trackPointer->getTitle()},
                {"artist", trackPointer->getArtist()},
                {"album", trackPointer->getAlbum()},
                {"genre", trackPointer->getGenre()},
                {"grouping", trackPointer->getGrouping()},
                {"comment", trackPointer->getComment()},
                {"year", trackPointer->getYear()},
                {"location", trackPointer->getLocation()},
                {"url", trackPointer->getURL()}};
        tracks.append(track);
    }

    QJsonObject root = {
            {"tracks", tracks}};
    QJsonDocument doc(root);

    return doc.toJson(QJsonDocument::Compact);
}
