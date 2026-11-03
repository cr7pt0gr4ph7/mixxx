#pragma once

#include <QAbstractTableModel>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>

#include "track/beats_decl.h"
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

    TrackCueModel(QObject* pParent = NULL);
    virtual ~TrackCueModel();

    void setSampleRate(mixxx::audio::SampleRate sampleRate, bool isAccurate);
    void setShowHiddenCueTypes(bool showHiddenCueTypes);

    void loadCuePoints(const QList<CuePointer>& cuePoints, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate);
    QList<CuePointer> saveCuePoints(mixxx::audio::SampleRate sampleRate);

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  private:
    void checkValidity(bool sendChangeEvents);

  private:
    class CueTimeInfo {
      public:
        double milliseconds;
        mixxx::audio::FramePos frames;
        bool valid;

        static CueTimeInfo fromMilliseconds(double milliseconds, mixxx::audio::SampleRate sampleRate);
        static CueTimeInfo fromMilliseconds(std::optional<double> milliseconds, mixxx::audio::SampleRate sampleRate);
        static CueTimeInfo fromFramePos(const mixxx::audio::FramePos& position, mixxx::audio::SampleRate sampleRate);
    };

    class CueLengthInfo {
      public:
        double beats;
        double milliseconds;
        mixxx::audio::FrameDiff_t frames;
        bool valid;

        static CueLengthInfo fromStartEnd(const CueTimeInfo& start, const CueTimeInfo& end, const mixxx::BeatsPointer& pBeats);
    };

    class CuePointInfo {
      public:
        CuePointInfo(const mixxx::CueInfo& cueInfo, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate);
        CuePointInfo(const Cue& cue, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate);

        mixxx::CueInfo toCueInfo() const;
        CuePointer toCuePointer() const;

      public:
        DbId id;
        mixxx::CueType type;
        CueTimeInfo start;
        CueTimeInfo end;
        CueLengthInfo length;
        std::optional<int> number;
        std::optional<bool> numberValid;
        QString label;
        mixxx::RgbColor::optional_t color;
        mixxx::CueFlags flags;
    };

  private:
    QVector<QHash<int, QVariant>> m_headerInfo;

    QList<CuePointInfo> m_cues;
    QList<CuePointInfo> m_hiddenCues;
    bool m_showHiddenCueTypes;

    mixxx::audio::SampleRate m_sampleRate;
    bool m_sampleRateIsAccurate;
};
