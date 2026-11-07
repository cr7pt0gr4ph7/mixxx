#pragma once

#include <QList>
#include <QString>

#include "audio/signalinfo.h"
#include "track/cueinfo.h"

namespace mixxx {

/// Importer class for cue files that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class CueFileImporter {
  public:
    CueFileImporter() = default;
    virtual ~CueFileImporter() = default;

    virtual QList<CueInfo> importCueInfos(
            const QString& filePath,
            const QString& fileType,
            const audio::SignalInfo& signalInfo) const = 0;
};

} // namespace mixxx
