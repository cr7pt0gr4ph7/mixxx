#include "library/trackcuemodel.h"

#include <QIcon>
#include <QStringBuilder>

#include "moc_trackcuemodel.cpp"
#include "util/duration.h"

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

QList<mixxx::CueInfo> TrackCueModel::getCues() const {
    return m_cues;
}

void TrackCueModel::setCues(const QList<mixxx::CueInfo>& cues) {
    beginResetModel();
    m_cues = cues;
    m_hiddenCues.clear();
    endResetModel();
}

void TrackCueModel::setShowHiddenCueTypes(bool showHiddenCueTypes) {
    m_showHiddenCueTypes = showHiddenCueTypes;
}

void TrackCueModel::setSampleRate(mixxx::audio::SampleRate sampleRate, bool isAccurate) {
    m_sampleRate = sampleRate;
    m_sampleRateIsAccurate = isAccurate;
}

void TrackCueModel::loadCuePoints(const QList<CuePointer>& cuePoints, mixxx::audio::SampleRate sampleRate) {
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
            m_cues.append(pCue->getCueInfo(sampleRate));
        } else {
            m_hiddenCues.append(pCue->getCueInfo(sampleRate));
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
    for (const auto& cueInfo : std::as_const(m_hiddenCues)) {
        newCues.append(CuePointer(new Cue(cueInfo, sampleRate, true)));
    }
    for (const auto& cueInfo : std::as_const(m_cues)) {
        newCues.append(CuePointer(new Cue(cueInfo, sampleRate, true)));
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

    const mixxx::CueInfo& cue = m_cues.at(row);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (column) {
        case CUE_COLUMN_TYPE: {
            return QVariant::fromValue(cue.getType());
        }
        case CUE_COLUMN_NUMBER: {
            std::optional<int> index = cue.getHotCueIndex();
            if (!index) {
                return QVariant();
            }
            return QVariant(*index + 1);
        }
        case CUE_COLUMN_COLOR: {
            return mixxx::RgbColor::toQVariant(cue.getColor());
        }
        case CUE_COLUMN_LABEL: {
            return cue.getLabel();
        }
        case CUE_COLUMN_START_POSITION: {
            std::optional<double> startTime = cue.getStartPositionMillis();
            if (!startTime) {
                return QVariant();
            }
            return mixxx::Duration::formatTime(
                    *startTime / mixxx::Duration::kMillisPerSecond,
                    mixxx::Duration::Precision::CENTISECONDS);
        }
        case CUE_COLUMN_END_POSITION: {
            std::optional<double> endTime = cue.getEndPositionMillis();
            if (endTime) {
                return mixxx::Duration::formatTime(
                        *endTime / mixxx::Duration::kMillisPerSecond,
                        mixxx::Duration::Precision::CENTISECONDS);
            }
            return QVariant();
        }
        case CUE_COLUMN_LENGTH: {
            std::optional<double> startTime = cue.getStartPositionMillis();
            std::optional<double> endTime = cue.getEndPositionMillis();
            if (!startTime || !endTime) {
                return QVariant();
            }
            double duration = *endTime - *startTime;
            return mixxx::Duration::formatTime(
                    duration / mixxx::Duration::kMillisPerSecond,
                    mixxx::Duration::Precision::CENTISECONDS);
        }
        case CUE_COLUMN_LENGTH_BEATS: {
            return QVariant();
        }
        }
    } else if (role == Qt::BackgroundRole) {
        switch (column) {
        case CUE_COLUMN_COLOR: {
            return mixxx::RgbColor::toQVariantColor(cue.getColor());
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

    mixxx::CueInfo& cueInfo = m_cues[row];

    switch (column) {
    case CUE_COLUMN_LABEL:
        cueInfo.setLabel(value.toString());
        // emit dataChanged(modelIndex, index(modelIndex.row(), CUE_COLUMN_LABEL));
        return true;
    }

    return false;
}

Qt::ItemFlags TrackCueModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.column() == CUE_COLUMN_LABEL) {
        defaultFlags |= Qt::ItemIsEditable;
    }
    return defaultFlags;
}
