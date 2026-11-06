#pragma once

#include "track/cuefileexporter.h"

namespace mixxx {

class SrtCueFileExporter : public CueFileExporter {
  public:
    SrtCueFileExporter(CueType type, QString lineEnding = QStringLiteral("\n"));
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

    CueType m_type;
    QString m_lineEnding;
};

} // namespace mixxx
