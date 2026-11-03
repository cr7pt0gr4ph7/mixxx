#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>

#include "track/cue.h"
#include "track/cueinfo.h"

class TrackCueModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    enum CueColumn {
        CUE_COLUMN_TYPE = 0,
        CUE_COLUMN_NUMBER,
        CUE_COLUMN_COLOR,
        CUE_COLUMN_LABEL,
        CUE_COLUMN_START_POSITION,
        CUE_COLUMN_END_POSITION,
        CUE_COLUMN_LENGTH,
        CUE_COLUMN_LENGTH_BEATS,
        NUM_CUE_COLUMNS
    };

    TrackCueModel(QObject* pParent=NULL);
    virtual ~TrackCueModel();

    QList<mixxx::CueInfo> getCues() const;
    void setCues(const QList<mixxx::CueInfo>& cues);
    void setSampleRate(mixxx::audio::SampleRate sampleRate, bool isAccurate);
    void setShowHiddenCueTypes(bool showHiddenCueTypes);

    void loadCuePoints(const QList<CuePointer>& cuePoints, mixxx::audio::SampleRate sampleRate);
    QList<CuePointer> saveCuePoints(mixxx::audio::SampleRate sampleRate);

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);

  private:
    QVector<QHash<int, QVariant>> m_headerInfo;

    QList<mixxx::CueInfo> m_cues;
    QList<mixxx::CueInfo> m_hiddenCues;
    bool m_showHiddenCueTypes;

    mixxx::audio::SampleRate m_sampleRate;
    bool m_sampleRateIsAccurate;
};
