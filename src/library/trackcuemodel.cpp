#include "library/trackcuemodel.h"

#include <QIcon>
#include <QStringBuilder>

#include "moc_trackcuemodel.cpp"
#include "track/beats.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/duration.h"

namespace {

constexpr double kInvalidMilliseconds = std::numeric_limits<double>::quiet_NaN();
constexpr double kInvalidBeats = std::numeric_limits<double>::quiet_NaN();
constexpr double kInvalidFrameDiff = std::numeric_limits<mixxx::audio::FrameDiff_t>::quiet_NaN();

inline std::optional<double> positionFramesToMillis(
        mixxx::audio::FramePos position,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return std::nullopt;
    }
    if (!position.isValid()) {
        return std::nullopt;
    }
    // Try to avoid rounding errors
    return (position.value() * 1000) / sampleRate;
}

inline mixxx::audio::FramePos positionMillisToFrames(
        std::optional<double> positionMillis,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    if (!positionMillis) {
        return mixxx::audio::kInvalidFramePos;
    }

    return mixxx::audio::FramePos((*positionMillis * sampleRate) / 1000);
}
} // namespace

// static
TrackCueModel::CueTimeInfo TrackCueModel::CueTimeInfo::fromMilliseconds(double milliseconds, mixxx::audio::SampleRate sampleRate) {
    return {milliseconds, positionMillisToFrames(milliseconds, sampleRate), true};
}

// static
TrackCueModel::CueTimeInfo TrackCueModel::CueTimeInfo::fromFramePos(const mixxx::audio::FramePos& position, mixxx::audio::SampleRate sampleRate) {
    const auto milliseconds = positionFramesToMillis(position, sampleRate);
    return {milliseconds.value_or(kInvalidMilliseconds), position, milliseconds.has_value()};
}

// static
TrackCueModel::CueTimeInfo TrackCueModel::CueTimeInfo::fromMilliseconds(std::optional<double> milliseconds, mixxx::audio::SampleRate sampleRate) {
    return milliseconds ? fromMilliseconds(*milliseconds, sampleRate) : (CueTimeInfo{kInvalidMilliseconds, mixxx::audio::kInvalidFramePos, false});
}

// static
TrackCueModel::CueLengthInfo TrackCueModel::CueLengthInfo::fromStartEnd(const CueTimeInfo& start, const CueTimeInfo& end, const mixxx::BeatsPointer& pBeats) {
    CueLengthInfo info;
    if (start.valid && end.valid) {
        info.beats = pBeats ? pBeats->findDistanceInBeats(start.frames, end.frames) : kInvalidBeats;
        info.milliseconds = end.milliseconds - start.milliseconds;
        info.frames = end.frames - start.frames;
        info.valid = true;
    } else {
        info.beats = kInvalidBeats;
        info.milliseconds = kInvalidMilliseconds;
        info.frames = kInvalidFrameDiff;
        info.valid = false;
    }
    return info;
}

TrackCueModel::CuePointInfo::CuePointInfo(const mixxx::CueInfo& cueInfo, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate) {
    id = DbId();
    type = cueInfo.getType();
    const auto index = cueInfo.getHotCueIndex();
    number = index ? std::make_optional(*index + 1) : std::nullopt;
    start = CueTimeInfo::fromMilliseconds(cueInfo.getStartPositionMillis(), sampleRate);
    end = CueTimeInfo::fromMilliseconds(cueInfo.getEndPositionMillis(), sampleRate);
    length = CueLengthInfo::fromStartEnd(start, end, pBeats);
    label = cueInfo.getLabel();
    color = cueInfo.getColor();
    flags = cueInfo.flags();
}

TrackCueModel::CuePointInfo::CuePointInfo(const Cue& cue, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate) {
    id = cue.getId();
    type = cue.getType();
    const int index = cue.getHotCue();
    number = index == Cue::kNoHotCue ? std::nullopt : std::make_optional(index + 1);
    start = CueTimeInfo::fromFramePos(cue.getPosition(), sampleRate);
    end = CueTimeInfo::fromFramePos(cue.getEndPosition(), sampleRate);
    length = CueLengthInfo::fromStartEnd(start, end, pBeats);
    label = cue.getLabel();
    color = cue.getColor();
    flags = mixxx::CueFlag::None;
}

mixxx::CueInfo TrackCueModel::CuePointInfo::toCueInfo() const {
    return mixxx::CueInfo(
            type,
            start.valid ? std::make_optional(start.milliseconds) : std::nullopt,
            end.valid ? std::make_optional(end.milliseconds) : std::nullopt,
            number ? std::make_optional(*number - 1) : std::nullopt,
            label,
            color,
            flags);
}

CuePointer TrackCueModel::CuePointInfo::toCuePointer() const {
    return CuePointer(new Cue(
            id,
            type,
            start.frames,
            length.frames,
            number ? (*number - 1) : Cue::kNoHotCue,
            label,
            color.value_or(mixxx::PredefinedColorPalettes::kDefaultCueColor)));
}

TrackCueModel::TrackCueModel(QObject* pParent)
        : QAbstractTableModel(pParent),
          m_showHiddenCueTypes(false),
          m_sampleRateIsAccurate(false) {
    setHeaderData(CUE_COLUMN_TYPE, Qt::Horizontal, tr("Type"));
    setHeaderData(CUE_COLUMN_NUMBER, Qt::Horizontal, tr("Cue #"));
    setHeaderData(CUE_COLUMN_NUMBER, Qt::Horizontal, tr("Cue Number"), Qt::ToolTipRole);
    setHeaderData(CUE_COLUMN_COLOR, Qt::Horizontal, tr("Color"));
    setHeaderData(CUE_COLUMN_LABEL, Qt::Horizontal, tr("Label"));
    setHeaderData(CUE_COLUMN_START_POSITION, Qt::Horizontal, tr("Start"));
    setHeaderData(CUE_COLUMN_START_POSITION, Qt::Horizontal, tr("Start Position"), Qt::ToolTipRole);
    setHeaderData(CUE_COLUMN_END_POSITION, Qt::Horizontal, tr("End"));
    setHeaderData(CUE_COLUMN_END_POSITION, Qt::Horizontal, tr("End Position"), Qt::ToolTipRole);
    setHeaderData(CUE_COLUMN_LENGTH, Qt::Horizontal, tr("Length"));
    setHeaderData(CUE_COLUMN_LENGTH_BEATS, Qt::Horizontal, tr("Length in Beats"));
}

TrackCueModel::~TrackCueModel() {
}

void TrackCueModel::setShowHiddenCueTypes(bool showHiddenCueTypes) {
    m_showHiddenCueTypes = showHiddenCueTypes;
}

void TrackCueModel::setSampleRate(mixxx::audio::SampleRate sampleRate, bool isAccurate) {
    m_sampleRate = sampleRate;
    m_sampleRateIsAccurate = isAccurate;
}

void TrackCueModel::loadCuePoints(const QList<CuePointer>& cuePoints, const mixxx::BeatsPointer& pBeats, mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate == m_sampleRate) {
        qCritical() << "Loading cue points based on a possibly wrong sample rate"
                    << "(actual:" << sampleRate << "expected:" << m_sampleRate << ")";
    }

    beginResetModel();
    m_cues.clear();
    m_cues.reserve(cuePoints.size());
    m_hiddenCues.clear();

    // Hide non-hotcue cue points
    for (const auto& pCue : std::as_const(cuePoints)) {
        auto type = pCue->getType();
        if (m_showHiddenCueTypes || type == mixxx::CueType::HotCue || type == mixxx::CueType::Loop || type == mixxx::CueType::Jump) {
            m_cues.append(CuePointInfo(*pCue, pBeats, sampleRate));
        } else {
            m_cues.append(CuePointInfo(*pCue, pBeats, sampleRate));
        }
    }

    endResetModel();
}

QList<CuePointer> TrackCueModel::saveCuePoints(mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate == m_sampleRate) {
        qCritical() << "Saving cue points based on a possibly wrong sample rate"
                    << "(actual:" << sampleRate << "expected:" << m_sampleRate << ")";
    }

    QList<CuePointer> newCues;
    newCues.reserve(m_cues.size() + m_hiddenCues.size());
    for (const auto& cuePointInfo : std::as_const(m_hiddenCues)) {
        newCues.append(cuePointInfo.toCuePointer());
    }
    for (const auto& cuePointInfo : std::as_const(m_cues)) {
        newCues.append(cuePointInfo.toCuePointer());
    }
    return newCues;
}

int TrackCueModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_cues.size();
}

int TrackCueModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return NUM_CUE_COLUMNS;
}

QVariant TrackCueModel::data(const QModelIndex& index,
        int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_cues.size()) {
        return QVariant();
    }

    const CuePointInfo& cue = m_cues.at(row);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (column) {
        case CUE_COLUMN_TYPE: {
            switch (cue.type) {
            case mixxx::CueType::Invalid:
                return tr("Invalid");
            case mixxx::CueType::HotCue:
                return tr("Hotcue");
            case mixxx::CueType::MainCue:
                return tr("Main Cue");
            case mixxx::CueType::Beat:
                return tr("Beat");
            case mixxx::CueType::Loop:
                return tr("Loop");
            case mixxx::CueType::Jump:
                return tr("Jump");
            case mixxx::CueType::Intro:
                return tr("Intro");
            case mixxx::CueType::Outro:
                return tr("Outro");
            case mixxx::CueType::N60dBSound:
                return tr(">-60dB");
            }
            return QVariant::fromValue(cue.type);
        }
        case CUE_COLUMN_NUMBER: {
            return cue.number ? QVariant(*cue.number) : QVariant();
        }
        case CUE_COLUMN_COLOR: {
            return QVariant();
        }
        case CUE_COLUMN_LABEL: {
            return cue.label;
        }
        case CUE_COLUMN_START_POSITION: {
            return cue.start.valid
                    ? mixxx::Duration::formatTime(
                              cue.start.milliseconds / mixxx::Duration::kMillisPerSecond,
                              mixxx::Duration::Precision::CENTISECONDS)
                    : QVariant();
        }
        case CUE_COLUMN_END_POSITION: {
            return cue.end.valid
                    ? mixxx::Duration::formatTime(
                              cue.end.milliseconds / mixxx::Duration::kMillisPerSecond,
                              mixxx::Duration::Precision::CENTISECONDS)
                    : QVariant();
        }
        case CUE_COLUMN_LENGTH: {
            if (!cue.start.valid || !cue.end.valid) {
                return QVariant();
            }
            double duration = cue.end.milliseconds - cue.start.milliseconds;
            return mixxx::Duration::formatTime(
                    duration / mixxx::Duration::kMillisPerSecond,
                    mixxx::Duration::Precision::CENTISECONDS);
        }
        case CUE_COLUMN_LENGTH_BEATS: {
            return cue.length.valid ? cue.length.beats : QVariant();
        }
        }
    } else if (role == Qt::BackgroundRole) {
        switch (column) {
        case CUE_COLUMN_COLOR: {
            return mixxx::RgbColor::toQVariantColor(cue.color);
        }
        }
    } else if (role == Qt::ToolTipRole) {
        switch (column) {
        case CUE_COLUMN_COLOR: {
            return mixxx::RgbColor::toQVariantString(cue.color);
        }
        }
    }
    return QVariant();
}

bool TrackCueModel::setHeaderData(int section,
        Qt::Orientation orientation,
        const QVariant& value,
        int role) {
    int numColumns = columnCount();
    if (section < 0 || section >= numColumns) {
        return false;
    }

    if (orientation != Qt::Horizontal) {
        // We only care about horizontal headers.
        return false;
    }

    if (m_headerInfo.size() != numColumns) {
        m_headerInfo.resize(numColumns);
    }

    m_headerInfo[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
}

QVariant TrackCueModel::headerData(int section,
        Qt::Orientation orientation,
        int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        QVariant headerValue = m_headerInfo.value(section).value(role);
        if (!headerValue.isValid()) {
            // Try EditRole if DisplayRole wasn't present
            headerValue = m_headerInfo.value(section).value(Qt::EditRole);
        }
        if (!headerValue.isValid()) {
            headerValue = QVariant(section).toString();
        }
        return headerValue;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool TrackCueModel::setData(const QModelIndex& modelIndex,
        const QVariant& value,
        int role) {
    if (!modelIndex.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = modelIndex.row();
    int column = modelIndex.column();

    if (row < 0 || row >= m_cues.size()) {
        return false;
    }

    CuePointInfo& cue = m_cues[row];

    switch (column) {
    case CUE_COLUMN_NUMBER: {
        bool ok;
        const double newNumber = value.toInt(&ok);
        if (!ok) {
            return false;
        }
        cue.number = newNumber;
        break;
    }
    case CUE_COLUMN_LABEL: {
        cue.label = value.toString();
        // emit dataChanged(modelIndex, index(modelIndex.row(), CUE_COLUMN_LABEL));
        return true;
    }
    }

    return false;
}

Qt::ItemFlags TrackCueModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_cues.size()) {
        return Qt::ItemIsEnabled;
    }

    const CuePointInfo& cue = m_cues.at(row);
    const mixxx::CueType type = cue.type;
    const bool isUserDefined = type == mixxx::CueType::HotCue || type == mixxx::CueType::Loop || type == mixxx::CueType::Jump;

    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.column() == CUE_COLUMN_NUMBER ||
            index.column() == CUE_COLUMN_LABEL) {
        if (isUserDefined) {
            defaultFlags |= Qt::ItemIsEditable;
        }
    }
    return defaultFlags;
}
