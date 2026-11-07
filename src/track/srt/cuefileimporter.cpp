#include "track/srt/cuefileimporter.h"

#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QTime>

#include "track/srt/srt_defs.h"
#include "util/duration.h"

namespace {

// Regex pattern string for matching time codes of the form:
// 00:02:16,612
const QString kTimeCodePattern =
        QStringLiteral("(\\d{2}):(\\d{2}:\\d{2},\\d{3}))");

/// Regex for matching time range strings of the following form:
/// 00:02:16,612 --> 00:02:19,376
const QRegularExpression kTimeRangeRegex(
        QStringLiteral("^") +
        kTimeCodePattern +
        kTimeRangeSeparator +
        kTimeCodePattern +
        QStringLiteral("$"));

const QRegularExpression kTypeRegex(
        QStringLiteral("^<([A-Za-z](?:[A-Za-z ]*[A-Za-z])?(?: ([0-9]+))?>(.*)$"));

constexpr double kInvalidMilliseconds =
        std::numeric_limits<double>::quiet_NaN();

double parseTimeCodeAsMillis(const QString& daysText, const QString& timeCode, bool* pOk = nullptr) {
    bool ok;
    double result = kInvalidMilliseconds;
    double days = static_cast<double>(QLocale::c().toUInt(daysText, &ok));
    if (ok) {
        QTime time = QTime::fromString(timeCode, QStringLiteral("hh:mm:ss,zzz"));
        if (time.isValid()) {
            result = days * mixxx::Duration::kMillisPerSecond * mixxx::Duration::kSecondsPerDay +
                    static_cast<double>(time.msecsSinceStartOfDay());
        } else {
            ok = false;
        }
    }
    if (pOk) {
        *pOk = ok;
    }
    return result;
}

} // namespace

mixxx::SrtCueFileImporter::SrtCueFileImporter(std::optional<mixxx::CueType> type)
        : m_type(type) {
}

QList<mixxx::CueInfo> mixxx::SrtCueFileImporter::importCueInfos(
        const QString& filePath,
        const QString& fileType,
        const mixxx::audio::SignalInfo& signalInfo) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QList<mixxx::CueInfo>();

    QTextStream in(&file);
    return importCueInfos(in);
}

QList<mixxx::CueInfo> mixxx::SrtCueFileImporter::importCueInfos(
        QTextStream& in) const {
    bool success = true;
    QString line = in.readLine();
    QList<mixxx::CueInfo> result;
    while (true) {
        // Skip empty lines between records
        while (!line.isNull() && line.isEmpty()) {
            line = in.readLine();
        }
        if (line.isNull()) {
            // We have reached the end of the file
            success = true;
            break;
        }

        // Read the sequence number
        // TODO: Validate that sequence numbers are monotonically increasing and continuous?
        bool ok;
        uint sequenceNumber = QLocale::c().toUInt(line, &ok);
        if (!ok) {
            // Failed to parse the sequence number as an unsigned integer
            success = false;
            break;
        }

        // Read the time code range
        line = in.readLine();
        if (line.isNull()) {
            // We have reached the end of the file at an unexpected location
            success = false;
            break;
        }
        const auto match = kTimeRangeRegex.match(line);
        if (!match.hasMatch()) {
            // Failed to parse the time code range
            success = false;
            break;
        }

        double startTime = parseTimeCodeAsMillis(
                match.captured(1),
                match.captured(2),
                &ok);
        if (!ok) {
            // Failed to parse the first time code
            success = false;
            break;
        }

        double endTime = parseTimeCodeAsMillis(
                match.captured(3),
                match.captured(4),
                &ok);
        if (!ok) {
            // Failed to parse the second time code
            success = false;
            break;
        }

        // Read zero or more text lines for the label, until the first blank line
        // or the end of the file is found.
        QString label;
        line = in.readLine();
        bool insertNewline = false;
        while (!line.isNull() && !line.isEmpty()) {
            if (insertNewline) {
                label += QStringLiteral("\n");
                insertNewline = false;
            }
            if (line == kBlankLineSentinel) {
                label += QStringLiteral("\n");
                insertNewline = false;
            } else {
                label += line;
                insertNewline = true;
            }
            line = in.readLine();
        }

        mixxx::CueType type;
        uint number = sequenceNumber;
        if (m_type) {
            type = *m_type;
        } else {
            const auto match = kTypeRegex.match(label);
            if (match.hasMatch()) {
                QString cueTypeText = match.captured(1);
                QString cueNumberText = match.captured(2);
                label = match.captured(3);

                if (cueTypeText == QStringLiteral("Invalid")) {
                    type = mixxx::CueType::Invalid;
                } else if (cueTypeText == QStringLiteral("Hotcue")) {
                    type = mixxx::CueType::HotCue;
                } else if (cueTypeText == QStringLiteral("Main")) {
                    type = mixxx::CueType::MainCue;
                } else if (cueTypeText == QStringLiteral("Beat")) {
                    type = mixxx::CueType::Beat;
                } else if (cueTypeText == QStringLiteral("Loop")) {
                    type = mixxx::CueType::Loop;
                } else if (cueTypeText == QStringLiteral("Jump")) {
                    type = mixxx::CueType::Jump;
                } else if (cueTypeText == QStringLiteral("Intro")) {
                    type = mixxx::CueType::Intro;
                } else if (cueTypeText == QStringLiteral("Outro")) {
                    type = mixxx::CueType::Outro;
                } else if (cueTypeText == QStringLiteral("N60dBSound")) {
                    type = mixxx::CueType::N60dBSound;
                } else {
                    type = mixxx::CueType::Invalid;
                }

                if (!cueNumberText.isEmpty()) {
                    uint cueNumber = QLocale::c().toUInt(cueNumberText, &ok);
                    if (!ok) {
                        // Failed to parse cue number
                        success = false;
                        break;
                    }
                    number = cueNumber;
                }
            }
        }

        result.append(mixxx::CueInfo(
                type,
                std::make_optional(startTime),
                startTime == endTime ? std::nullopt : std::make_optional(endTime),
                type == mixxx::CueType::HotCue ? std::make_optional<int>(number - 1) : std::nullopt,
                label,
                std::nullopt));
    }
    return result;
}
