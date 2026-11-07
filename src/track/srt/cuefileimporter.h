#pragma once

#include <QTextStream>

#include "track/cuefileimporter.h"

namespace mixxx {

class SrtCueFileImporter : public CueFileImporter {
  public:
    SrtCueFileImporter(CueType type);
    ~SrtCueFileImporter() override = default;

    QList<CueInfo> importCueInfos(
            const QString& filePath,
            const QString& fileType,
            const audio::SignalInfo& signalInfo) const override;

  private:
    QList<CueInfo> importCueInfos(
            QTextStream& stream) const;

    CueType m_type;
};

} // namespace mixxx
