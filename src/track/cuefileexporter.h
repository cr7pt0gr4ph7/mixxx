#pragma once

#include <QList>
#include <QString>

#include "audio/signalinfo.h"
#include "track/cueinfo.h"

namespace mixxx {

/// Exporter class for cue files that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class CueFileExporter {
  public:
    CueFileExporter() = default;
    virtual ~CueFileExporter() = default;

    virtual void exportCueInfos(
            const QList<CueInfo>& cueInfos,
            const QString& filePath,
            const QString& fileType,
            const audio::SignalInfo& signalInfo) const = 0;
};

} // namespace mixxx
