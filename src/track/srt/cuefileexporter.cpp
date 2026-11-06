#include "track/srt/cuefileexporter.h"

#include <QFile>
#include <QRegularExpression>
#include <QTime>

#include "track/srt/srt_defs.h"
#include "util/duration.h"
#include "util/fpclassify.h"

namespace {

const QRegularExpression kLineBreakRegex(
        QStringLiteral("(?:\\n\\r?|\\r)"));

QString formatTimeCode(double dMilliseconds) {
    double dSeconds = dMilliseconds / mixxx::Duration::kMillisPerSecond;
    if (dSeconds < 0.0 || !util_isfinite(dSeconds)
            // Use >= instead of >: 2^63-1 (qint64) is rounded to 2^63 (double)
            || dSeconds >= static_cast<double>(std::numeric_limits<qint64>::max())) {
        // negative durations and infinity or isNaN values are not supported
        return mixxx::Duration::kInvalidDurationString;
    }

    const qint64 days = static_cast<qint64>(std::floor(dSeconds)) / mixxx::Duration::kSecondsPerDay;
    dSeconds -= days * mixxx::Duration::kSecondsPerDay;

    // NOTE(uklotzde): QTime() constructs a 'null' object, but
    // we need 'zero' here.
    QTime t = QTime(0, 0).addMSecs(static_cast<int>(dSeconds * mixxx::Duration::kMillisPerSecond));

    return QString::number(days * 24 + t.hour()) + QChar(':') + t.toString(QStringLiteral("hh:mm:ss,zzz"));
}

} // namespace

mixxx::SrtCueFileExporter::SrtCueFileExporter(
        std::optional<mixxx::CueType> type,
        QString lineEnding)
        : m_type(type),
          m_lineEnding(lineEnding) {
    DEBUG_ASSERT(lineEnding == QStringLiteral("\n") ||
            lineEnding == QStringLiteral("\n\r") ||
            lineEnding == QStringLiteral("\r"));
}

void mixxx::SrtCueFileExporter::exportCueInfos(
        const QList<mixxx::CueInfo>& cueInfos,
        const QString& filePath,
        const QString& fileType,
        const mixxx::audio::SignalInfo& signalInfo) const {
    // TODO: First write to temporary location, then replace the original file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;

    QTextStream out(&file);

    // Ignore cues that do not have a start position
    QList<mixxx::CueInfo> sortedCueInfos;
    sortedCueInfos.reserve(cueInfos.size());
    for (const mixxx::CueInfo& cueInfo : std::as_const(cueInfos)) {
        if ((m_type ? cueInfo.getType() == m_type : cueInfo.getType() != mixxx::CueType::Invalid) &&
                cueInfo.getStartPositionMillis()) {
            sortedCueInfos.append(cueInfo);
        }
    }

    // Sort cues by start position
    std::sort(
            sortedCueInfos.begin(),
            sortedCueInfos.end(),
            [](const mixxx::CueInfo& a, const mixxx::CueInfo& b) {
                return *a.getStartPositionMillis() < *b.getStartPositionMillis();
            });

    exportCueInfos(sortedCueInfos, out);
}

void mixxx::SrtCueFileExporter::exportCueInfos(
        const QList<mixxx::CueInfo>& cueInfos,
        QTextStream& out) const {
    int sequenceNumber = 0;
    for (const mixxx::CueInfo& cueInfo : std::as_const(cueInfos)) {
        if (sequenceNumber > 0) {
            // Insert a blank line between records
            out << m_lineEnding;
        }
        sequenceNumber++;
        out << sequenceNumber << m_lineEnding;
        out << formatTimeCode(*cueInfo.getStartPositionMillis())
            << kTimeRangeSeparator
            << formatTimeCode(cueInfo.getEndPositionMillis().value_or(*cueInfo.getStartPositionMillis()))
            << m_lineEnding;

        QString label = cueInfo.getLabel();
        if (!m_type) {
            QString typeText;
            switch (cueInfo.getType()) {
            case mixxx::CueType::Invalid:
                typeText = QStringLiteral("Invalid");
                break;
            case mixxx::CueType::HotCue:
                typeText = QStringLiteral("Hotcue");
                break;
            case mixxx::CueType::MainCue:
                typeText = QStringLiteral("Main");
                break;
            case mixxx::CueType::Beat:
                typeText = QStringLiteral("Beat");
                break;
            case mixxx::CueType::Loop:
                typeText = QStringLiteral("Loop");
                break;
            case mixxx::CueType::Jump:
                typeText = QStringLiteral("Jump");
                break;
            case mixxx::CueType::Intro:
                typeText = QStringLiteral("Intro");
                break;
            case mixxx::CueType::Outro:
                typeText = QStringLiteral("Outro");
                break;
            case mixxx::CueType::N60dBSound:
                typeText = QStringLiteral("N60dBSound");
                break;
            default:
                typeText = QStringLiteral("Invalid");
                break;
            }

            QString prefix = QStringLiteral("<");
            prefix += typeText;
            if (cueInfo.getHotCueIndex()) {
                prefix += QStringLiteral(" ");
                prefix += QString::number(*cueInfo.getHotCueIndex());
            }
            prefix += QStringLiteral(">");
            label = prefix + label;
        }

        const QStringList lines = label.split(kLineBreakRegex);
        for (const QString& line : std::as_const(lines)) {
            if (line.isEmpty()) {
                out << kBlankLineSentinel << m_lineEnding;
            } else {
                out << line << m_lineEnding;
            }
        }
    }
}
