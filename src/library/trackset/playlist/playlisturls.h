#pragma once

#include <QUrl>
#include <QUrlQuery>

#include "library/dao/playlistdao.h"

class PlaylistURLs {
  private:
    static const QString kPlaylistUrl;
    static const QString kFolderUrl;

    static QUrl urlFromTemplate(const QString& urlTemplate, int id) {
        if (!id.isValid()) {
            return QUrl();
        }
        return QUrl(QStringLiteral("%1?id=%2").arg(urlTemplate, QString::number(id)));
    }

    static int idFromUrl(const QString& urlTemplate, const QUrl& url) {
        if (url.isEmpty()) {
            return kInvalidPlaylistId;
        }
        const QString urlString = url.adjusted(QUrl::RemoveFragment | QUrl::RemoveQuery).toString();
        if (urlString != urlTemplate) {
            return kInvalidPlaylistId;
        }
        const QUrlQuery query(url);
        const QString id = query.queryItemValue("id");
        if (id.isEmpty()) {
            return kInvalidPlaylistId;
        }
        // If id is a valid number, it will be automatically parsed while
        // coercing the QVariant to int.
        return QVariant(id).toInt();
    }

  public:
    // Returns the URL representing the specified, valid playlistId,
    // or an empty QUrl if playlistId is invalid.
    static QUrl toUrl(int playlistId) {
        return urlFromTemplate(kPlaylistUrl, playlistId);
    }

    // Returns the playlist id represented by the specified QUrl,
    // or an invalid playlist id if the URL does not represent a playlist.
    static int parsePlaylistUrl(const QUrl& url) {
        return idFromUrl(kPlaylistUrl, url);
    }

    // Parses the list of URLs, and returns the corresponding list of playlist ids.
    // Urls that do not represent playlist references are ignored.
    static QList<int> parsePlaylistURLs(const QList<QUrl>& urls) {
        QList<PlaylistId> ids;
        for (const QUrl& url : urls) {
            PlaylistId id = parsePlaylistUrl(url);
            if (id.isValid()) {
                ids.append(id);
            }
        }
        return ids;
    }
};

const QString PlaylistURLs::kPlaylistUrl = QStringLiteral("mixxx://library/playlists");
