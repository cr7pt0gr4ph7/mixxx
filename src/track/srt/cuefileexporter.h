#pragma once

#include <optional>

#include "track/cuefileexporter.h"

namespace mixxx {

class SrtCueFileExporter : public CueFileExporter {
  public:
    SrtCueFileExporter(
            std::optional<CueType> type = std::nullopt,
            QString lineEnding = QStringLiteral("\n"));
    ~SrtCueFileExporter() override = default;

    void exportCueInfos(
            const QList<CueInfo>& cueInfos,
            const QString& filePath,
            const QString& fileType,
            const audio::SignalInfo& signalInfo) const override;

  private:
    void exportCueInfos(
            const QList<CueInfo>& cueInfos,
            QTextStream& out) const;

    std::optional<CueType> m_type;
    QString m_lineEnding;
};

} // namespace mixxx
