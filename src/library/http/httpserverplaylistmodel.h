#pragma once

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"

class HttpServerConnection;

class HttpServerPlaylistModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HttpServerPlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager, HttpServerConnection* pConnection);
    ~HttpServerPlaylistModel() final;

    void selectPlaylist(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const final;
    TrackId getTrackId(const QModelIndex& index) const final;
    QUrl getTrackUrl(const QModelIndex& index) const final;

    QString getTrackLocation(const QModelIndex& index) const final;
    bool isColumnInternal(int column) final;

    Qt::ItemFlags flags(const QModelIndex &index) const final;
    Capabilities getCapabilities() const final;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const final;

    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;
    QVariant getFieldVariant(const QModelIndex& index, const QString& fieldName) const;
    void dropTempTable();

    HttpServerConnection* m_pConnection;
    int m_playlistId;
    QString m_tempTableName;
};
