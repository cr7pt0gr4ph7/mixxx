#include <QFuture>
#include <QList>
#include <QMessageBox>
#include <QtDebug>

#include "library/http/httpserverfeature.h"
#include "library/http/httpserverconnection.h"
#include "library/http/httpserverplaylistmodel.h"
#include "library/library.h"
#include "library/treeitem.h"
#include "moc_httpserverfeature.cpp"
#include "track/track.h"

HttpServerFeature::HttpServerFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("crates")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_cancelImport(false) {
    qDebug() << "HttpServerFeature()";
    Q_UNUSED(pConfig);
    m_pHttpServerPlaylistModel = new HttpServerPlaylistModel(
            this, m_pLibrary->trackCollectionManager(), &m_connection);
    m_isActivated = false;
    m_title = tr("(loading) HTTP Server");
}

HttpServerFeature::~HttpServerFeature() {
    qDebug() << "~HttpServerFeature()";
    // stop import thread, if still running
    m_cancelImport = true;
    if (m_future.isRunning()) {
        qDebug() << "m_future still running";
        m_future.waitForFinished();
        qDebug() << "m_future finished";
    }

    delete m_pHttpServerPlaylistModel;
}

// static
bool HttpServerFeature::isSupported() {
    return true;
}

QVariant HttpServerFeature::title() {
    return m_title;
}

template <typename T>
auto test() -> q_no_char8_t::QUtf8StringView
{
  return __PRETTY_FUNCTION__;
}


void HttpServerFeature::activate() {
    qDebug() << "HttpServerFeature::activate()";

    if (!m_isActivated) {
        QString endpointUrl("http://host.docker.internal:9090");
        if (!m_connection.open(endpointUrl)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Error connecting to HTTP server"),
                    tr("Failed to connect to HTTP server at ") + endpointUrl);
            return;
        }

        qDebug() << "Using HTTP Protocol V" << m_connection.getProtocolVersion();

        m_isActivated =  true;

        QFuture<QList<HttpServerConnection::Playlist>> playlistsFuture = m_connection.getPlaylists();

        playlistsFuture.then([this, playlistsFuture] (auto playlists) {
          std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);

            for (const HttpServerConnection::Playlist& playlist: playlistsFuture.result()) {
                qDebug() << playlist.name;
                // append the playlist to the child model
                pRootItem->appendChild(playlist.name, playlist.playlistId);
            }
            m_pSidebarModel->setRootItem(std::move(pRootItem));

            if (m_isActivated) {
                activate();
            }
            qDebug() << "HTTP Server playlists loaded: success";

            //calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
            m_title = tr("HTTP Server");
            emit featureLoadingFinished(this);

            m_pHttpServerPlaylistModel->selectPlaylist(0); // Loads the main playlist
            emit showTrackModel(m_pHttpServerPlaylistModel);
         });
    }

    emit enableCoverArtDisplay(false);
}

void HttpServerFeature::activateChild(const QModelIndex& index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    int playlistID = item->getData().toInt();
    if (playlistID > 0) {
        qDebug() << "Activating " << item->getLabel();
        m_pHttpServerPlaylistModel->selectPlaylist(playlistID);
        emit showTrackModel(m_pHttpServerPlaylistModel);
        emit enableCoverArtDisplay(false);
    }
}

TreeItemModel* HttpServerFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void HttpServerFeature::appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist) {
    if (lastRightClickedIndex().isValid()) {
        TreeItem *item = static_cast<TreeItem*>(lastRightClickedIndex().internalPointer());
        *pPlaylist = item->getLabel();
        int playlistID = item->getData().toInt();
        qDebug() << "HttpServerFeature::appendTrackIdsFromRightClickIndex " << *pPlaylist << " " << playlistID;
        if (playlistID > 0) {
            HttpServerPlaylistModel* pPlaylistModelToAdd =
                    new HttpServerPlaylistModel(this,
                            m_pLibrary->trackCollectionManager(),
                            &m_connection);
            pPlaylistModelToAdd->selectPlaylist(playlistID);
            pPlaylistModelToAdd->select();

            // Copy Tracks
            int rows = pPlaylistModelToAdd->rowCount();
            for (int i = 0; i < rows; ++i) {
                QModelIndex index = pPlaylistModelToAdd->index(i,0);
                if (index.isValid()) {
                    //qDebug() << pPlaylistModelToAdd->getTrackUrl(index);
                    TrackPointer track = pPlaylistModelToAdd->getTrack(index);
                    trackIds->append(track->getId());
                }
            }
            delete pPlaylistModelToAdd;
        }
    }
}
