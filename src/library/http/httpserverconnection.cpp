#include <QtDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

#include "library/http/httpserverconnection.h"
#include "util/performancetimer.h"

HttpServerConnection::HttpServerConnection() {
    m_networkAccessManager.setAutoDeleteReplies(true);
}

HttpServerConnection::~HttpServerConnection() {
    // TODO: Check for race conditions when a request is still in flight on shutdown
    qDebug() << "Close HTTP server connection";
}

bool HttpServerConnection::open(const QString& endpointUrl) {
    m_endpointUrl = endpointUrl;
    return true;
}

int HttpServerConnection::getProtocolVersion() {
    return 1;
}

QFuture<QList<HttpServerConnection::Playlist>> HttpServerConnection::getPlaylists() {

    PerformanceTimer time;
    time.start();

    // Prepare the HTTP request parameters
    QUrl requestUrl(m_endpointUrl + "/collections");
    QUrlQuery requestUrlQuery(requestUrl);

    requestUrlQuery.addQueryItem("limit", "100");
    requestUrlQuery.addQueryItem("fields", "*");
    requestUrl.setQuery(requestUrlQuery);

    // Send the HTTP request to the endpoint
    QNetworkRequest request(requestUrl);
    auto reply = m_networkAccessManager.get(request);

    auto future = QtFuture::connect(reply, &QNetworkReply::finished)
        .then([reply, time] {
            QList<HttpServerConnection::Playlist> list;
            HttpServerConnection::Playlist playlist;

            QString replyText = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(replyText.toUtf8());

            // Our JSON schema is based on [JSON API](https://jsonapi.org/)
            // for the outer message envelope, which, for our purposes, looks like this:
            //
            // {
            //    "errors": [] // Only present on errors
            //    "data": {    // Only present on success
            //        ...our data
            //    }
            // }
            for(const QJsonValue& playlistJson: doc.object().value("data").toObject().value("collections").toArray()) {
                auto playlistObject = playlistJson.toObject();
                playlist.name = playlistObject.value("name").toString();
                playlist.playlistId = playlistObject.value("id").toString();
                list.append(playlist);
            }

            qDebug() << "HttpServerConnection::getPlaylists(), took"
                << time.elapsed().debugMillisWithUnit();

            return list;
        });

    return future;
}

QFuture<QList<HttpServerConnection::PlaylistEntry>> HttpServerConnection::getPlaylistEntries(
    int playlistId) {

    // Prepare the HTTP request parameters
    QUrl requestUrl(m_endpointUrl + "/tracks");
    QUrlQuery requestUrlQuery(requestUrl);

    if (playlistId != 0) {
        // 0 indicates the main playlist, i.e. the whole database
        requestUrlQuery.addQueryItem("collection", QString::number(playlistId));
    }
    requestUrlQuery.addQueryItem("query", "searchquery");
    requestUrlQuery.addQueryItem("limit", "100");
    requestUrlQuery.addQueryItem("fields", "*");
    requestUrl.setQuery(requestUrlQuery);

    PerformanceTimer time;
    time.start();

    // Send the HTTP request to the endpoint
    QNetworkRequest request(requestUrl);
    auto reply = m_networkAccessManager.get(request);

    auto future = QtFuture::connect(reply, &QNetworkReply::finished)
        .then([reply, time] {
            QList<HttpServerConnection::PlaylistEntry> list;

            qDebug() << "HttpServerConnection::getPlaylistEntries(), took"
                << time.elapsed().debugMillisWithUnit();

            return list;
        });

    return future;
}
