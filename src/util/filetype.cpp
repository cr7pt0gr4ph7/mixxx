#include "util/filetype.h"

#include <QObject>
#include <QUrl>

#include "util/assert.h"
#include "util/fileinfo.h"

// static
mixxx::FileType mixxx::FileType::AllFiles() {
    return mixxx::FileType(
            QObject::tr("All Files"),
            QStringLiteral("*"),
            QStringLiteral("*"));
}

mixxx::FileType::FileType(const QString& title, const QString& fileTypeAndSuffix)
        : m_title(title),
          m_fileType(fileTypeAndSuffix) {
    m_fileSuffixes.append(fileTypeAndSuffix);
}

mixxx::FileType::FileType(const QString& title, const QString& fileType, const QString& fileSuffix)
        : m_title(title),
          m_fileType(fileType) {
    m_fileSuffixes.append(fileSuffix);
}

mixxx::FileType::FileType(const QString& title, const QString& fileType, const QStringList& fileSuffixes)
        : m_title(title),
          m_fileType(fileType),
          m_fileSuffixes(fileSuffixes) {
}

bool mixxx::FileType::isValid() const {
    return !m_title.isEmpty() && !m_fileType.isEmpty() && !m_fileSuffixes.isEmpty();
}

QStringList mixxx::FileType::getFileNamePatterns() const {
    QStringList result;
    result.reserve(m_fileSuffixes.size());
    for (const QString& fileSuffix : std::as_const(m_fileSuffixes)) {
        VERIFY_OR_DEBUG_ASSERT(!fileSuffix.isEmpty()) {
            continue;
        }
        result.append(QStringLiteral("*.") + fileSuffix);
    }
    return result;
}

QString mixxx::FileType::toQFileDialogPattern() const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return QString();
    }
    QString result;
    result.reserve(m_title.size() + 3 + m_fileSuffixes.size() * (3 + 3) - 1);
    result += m_title;
    result += " (";
    bool isFirst = true;
    for (const QString& fileSuffix : std::as_const(m_fileSuffixes)) {
        VERIFY_OR_DEBUG_ASSERT(!fileSuffix.isEmpty()) {
            continue;
        }
        if (isFirst) {
            isFirst = false;
        } else {
            result += QStringLiteral(" ");
        }
        result += fileSuffix;
    }
    return result;
}

bool mixxx::FileType::isUrlSupported(const QUrl& url) const {
    return isFileSupported(mixxx::FileInfo::fromQUrl(url));
}

bool mixxx::FileType::isFileSupported(const mixxx::FileInfo& fileInfo) const {
    return isFileNameSupported(fileInfo.fileName());
}

bool mixxx::FileType::isFileNameSupported(const QString& fileName) const {
    DEBUG_ASSERT(!fileName.isEmpty());
    for (const QString& fileSuffix : std::as_const(m_fileSuffixes)) {
        if (fileSuffix == QStringLiteral("*")) {
            return true;
        } else if (fileName.endsWith(QStringLiteral(".") + fileSuffix)) {
            // TODO: Case sensitivity/case insensitivity?
            return true;
        }
    }
    return false;
}

bool mixxx::FileType::isFileTypeSupported(const QString& fileType) const {
    VERIFY_OR_DEBUG_ASSERT(!m_fileType.isEmpty()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(!fileType.isEmpty()) {
        return false;
    }
    return m_fileType == fileType;
}

bool mixxx::FileType::isFileSuffixSupported(const QString& fileSuffix) const {
    DEBUG_ASSERT(!fileSuffix.isEmpty());
    // TODO: Case sensitivity/case insensitivity?
    return m_fileSuffixes.contains(fileSuffix);
}

QString mixxx::FileTypeSet::toQFileDialogPatterns() const {
    QString result;
    bool isFirst = true;
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        if (isFirst) {
            isFirst = false;
        } else {
            result += QStringLiteral(";;");
        }
        result += supportedType.toQFileDialogPattern();
    }
    return result;
}

QStringList mixxx::FileTypeSet::getSupportedFileTypes() const {
    QStringList result;
    result.reserve(size());
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        result.append(supportedType.getFileType());
    }
    return result;
}

/// Get the list of supported file extensions (e.g. "srt", flag", "aif", "aiff")
///
/// A single file type may map to multiple file suffixes, e.g.
/// "aiff" to "aif" or "aiff".
QStringList mixxx::FileTypeSet::getSupportedFileSuffixes() const {
    QStringList result;
    result.reserve(size());
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        result.append(supportedType.getFileSuffixes());
    }
    return result;
}

/// Get the list of supported file extensions for the specified file type.
///
/// A single file type may map to multiple file suffixes, e.g.
/// "aiff" to "aif" or "aiff".
QStringList mixxx::FileTypeSet::getFileSuffixesForFileType(const QString& fileType) const {
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        if (supportedType.isFileTypeSupported(fileType)) {
            return supportedType.getFileSuffixes();
        }
    }
    return QStringList();
}

bool mixxx::FileTypeSet::isUrlSupported(const QUrl& url) const {
    return isFileSupported(mixxx::FileInfo::fromQUrl(url));
}

bool mixxx::FileTypeSet::isFileSupported(const mixxx::FileInfo& fileInfo) const {
    return isFileNameSupported(fileInfo.fileName());
}

bool mixxx::FileTypeSet::isFileNameSupported(const QString& fileName) const {
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        if (supportedType.isFileNameSupported(fileName)) {
            return true;
        }
    }
    return false;
}

bool mixxx::FileTypeSet::isFileTypeSupported(const QString& fileType) const {
    VERIFY_OR_DEBUG_ASSERT(!fileType.isEmpty()) {
        return false;
    }
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        if (supportedType.isFileTypeSupported(fileType)) {
            return true;
        }
    }
    return false;
}

bool mixxx::FileTypeSet::isFileSuffixSupported(const QString& fileSuffix) const {
    VERIFY_OR_DEBUG_ASSERT(!fileSuffix.isEmpty()) {
        return false;
    }
    for (const FileType& supportedType : *this) {
        DEBUG_ASSERT(supportedType.isValid());
        if (supportedType.isFileSuffixSupported(fileSuffix)) {
            return true;
        }
    }
    return false;
}
