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
#include "util/fpclassify.h"

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

    void loadCueInfos(const QList<mixxx::CueInfo>& cueInfos, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate);
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
      private:
        constexpr CueTimeInfo(double milliseconds, const mixxx::audio::FramePos& frames, bool valid)
                : m_milliseconds(milliseconds),
                  m_frames(frames),
                  m_valid(valid) {
        }

      public:
        constexpr CueTimeInfo()
                : m_milliseconds(kInvalidMilliseconds),
                  m_frames(kInvalidFramePos),
                  m_valid(false) {
        }

        inline bool hasTimePosition() const {
            return m_valid && !util_isnan(m_milliseconds);
        }

        inline double getPositionAsSeconds() const {
            return m_milliseconds / mixxx::Duration::kMillisPerSecond;
        }

        inline double getPositionAsMilliseconds() const {
            return m_milliseconds;
        }

        inline bool hasFramePosition() const {
            return m_valid && m_frames.isValid();
        }

        inline mixxx::audio::FramePos getPositionAsFrames() const {
            return m_frames;
        }

        constexpr std::optional<double> toMillisecondsOptional() const {
            return isValid() ? std::make_optional(m_milliseconds) : std::nullopt;
        }

        constexpr mixxx::audio::FramePos toFramePos() const {
            return m_frames;
        }

        constexpr bool isValid() const {
            return m_valid;
        }

      public:
        static constexpr double kInvalidMilliseconds = std::numeric_limits<double>::quiet_NaN();
        static constexpr mixxx::audio::FramePos kInvalidFramePos = mixxx::audio::kInvalidFramePos;

      private:
        double m_milliseconds;
        mixxx::audio::FramePos m_frames;
        bool m_valid;

      public:
        static CueTimeInfo fromMilliseconds(double milliseconds, mixxx::audio::SampleRate sampleRate);
        static CueTimeInfo fromMilliseconds(std::optional<double> milliseconds, mixxx::audio::SampleRate sampleRate);
        static CueTimeInfo fromFramePos(const mixxx::audio::FramePos& position, mixxx::audio::SampleRate sampleRate);
    };

    class CueLengthInfo {
      private:
        constexpr CueLengthInfo(double beats, double milliseconds, mixxx::audio::FrameDiff_t frames, bool valid)
                : m_beats(beats),
                  m_milliseconds(milliseconds),
                  m_frames(frames),
                  m_valid(valid) {
        }

      public:
        constexpr CueLengthInfo()
                : m_beats(kInvalidBeats),
                  m_milliseconds(kInvalidMilliseconds),
                  m_frames(kInvalidFrameDiff),
                  m_valid(false) {
        }

        inline bool hasBeats() const {
            return m_valid && !util_isnan(m_beats);
        }

        inline std::optional<double> getBeatsOptional() const {
            return hasBeats() ? std::make_optional(m_beats) : std::nullopt;
        }

        inline bool hasFrameDiff() const {
            return m_valid && !util_isnan(m_frames);
        }

        constexpr mixxx::audio::FrameDiff_t getFrameDiff() const {
            return m_frames;
        }

        inline std::optional<mixxx::audio::FrameDiff_t> getFrameDiffOptional() const {
            return hasFrameDiff() ? std::make_optional(m_frames) : std::nullopt;
        }

        constexpr mixxx::audio::FrameDiff_t toFrameDiff() const {
            return m_frames;
        }

        inline bool hasDuration() const {
            return m_valid && !util_isnan(m_milliseconds);
        }

        constexpr double getDurationAsSeconds() const {
            return m_milliseconds / mixxx::Duration::kMillisPerSecond;
        }

        constexpr double getDurationAsMilliseconds() const {
            return m_milliseconds;
        }

        inline std::optional<double> getDurationAsSecondsOptional() const {
            return hasDuration() ? std::make_optional(getDurationAsSeconds()) : std::nullopt;
        }

        inline std::optional<double> getDurationAsMillisecondsOptional() const {
            return hasDuration() ? std::make_optional(getDurationAsMilliseconds()) : std::nullopt;
        }

        constexpr bool isValid() const {
            return m_valid;
        }

      public:
        static constexpr double kInvalidMilliseconds = CueTimeInfo::kInvalidMilliseconds;
        static constexpr double kInvalidFrameDiff = std::numeric_limits<double>::quiet_NaN();
        static constexpr double kInvalidBeats = std::numeric_limits<double>::quiet_NaN();

      private:
        double m_beats;
        double m_milliseconds;
        mixxx::audio::FrameDiff_t m_frames;
        bool m_valid;

      public:
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
