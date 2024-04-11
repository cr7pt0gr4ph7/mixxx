#pragma once

#include <QFuture>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

class HttpServerConnection
{
public:

    struct Playlist {
        QString playlistId;
        QString name;
    };

    struct Track {
        QString title;
        QUrl uri;
        int duration;
        int year;
        int rating;
        QString genre;
        QString grouping;
        int tracknumber;
        int dateadded;
        int bpm;
        int bitrate;
        QString comment;
        int playcount;
        QString composer;
    };

    struct Artist {
        QString name;
    };

    struct Album {
        QString title;
    };

    struct PlaylistEntry {
        int trackId;
        int viewOrder;
        struct Track* pTrack;
        struct Artist* pArtist;
        struct Album* pAlbum;
        struct Artist* pAlbumArtist;
    };

    HttpServerConnection();
    virtual ~HttpServerConnection();

    bool open(const QString& endpointUrl);
    int getProtocolVersion();
    QFuture<QList<Playlist>> getPlaylists();
    QFuture<QList<PlaylistEntry>> getPlaylistEntries(int playlistId);

private:
    QString m_endpointUrl;
    QNetworkAccessManager m_networkAccessManager;

};
