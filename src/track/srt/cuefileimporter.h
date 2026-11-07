#pragma once

#include <QTextStream>
#include <optional>

#include "track/cuefileimporter.h"

namespace mixxx {

class SrtCueFileImporter : public CueFileImporter {
  public:
    SrtCueFileImporter(std::optional<CueType> type = std::nullopt);
    ~SrtCueFileImporter() override = default;

    QList<CueInfo> importCueInfos(
            const QString& filePath,
            const QString& fileType,
            const audio::SignalInfo& signalInfo) const override;

  private:
    QList<CueInfo> importCueInfos(
            QTextStream& stream) const;

    std::optional<CueType> m_type;
};

} // namespace mixxx
