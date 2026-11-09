#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "util/macros.h"

class QUrl;

namespace mixxx {

class FileInfo;

class FileType {
    // Properties
    MIXXX_DECL_PROPERTY(QString, title, Title)
    MIXXX_DECL_PROPERTY(QString, fileType, FileType)
    MIXXX_DECL_PROPERTY(QStringList, fileSuffixes, FileSuffixes)

  public:
    FileType(const QString& title, const QString& fileTypeAndSuffix);
    FileType(const QString& title, const QString& fileType, const QString& fileSuffix);
    FileType(const QString& title, const QString& fileType, const QStringList& fileSuffixes);

  public:
    bool isValid() const;

    QStringList getFileNamePatterns() const;
    QString toQFileDialogPattern() const;

    bool isUrlSupported(const QUrl& url) const;
    bool isFileSupported(const mixxx::FileInfo& fileInfo) const;
    bool isFileNameSupported(const QString& fileName) const;
    bool isFileTypeSupported(const QString& fileType) const;
    bool isFileSuffixSupported(const QString& fileSuffix) const;

  public:
    static FileType AllFiles();
};

class FileTypeSet : public QList<FileType> {
  public:
    using QList::QList;

  public:
    /// Get the list of supported file types and extensions
    /// in a form suitable for use as a QFileDialog filePattern.
    QString toQFileDialogPatterns() const;

    /// Get the list of supported file types (e.g. "srt", "flac", "aiff").
    ///
    /// A single file type may map to multiple file suffixes, e.g.
    /// "aiff" to "aif" or "aiff".
    QStringList getSupportedFileTypes() const;

    /// Get the list of supported file extensions (e.g. "srt", flag", "aif", "aiff")
    ///
    /// A single file type may map to multiple file suffixes, e.g.
    /// "aiff" to "aif" or "aiff".
    QStringList getSupportedFileSuffixes() const;

    /// Get the list of supported file extensions for the specified file type.
    ///
    /// A single file type may map to multiple file suffixes, e.g.
    /// "aiff" to "aif" or "aiff".
    QStringList getFileSuffixesForFileType(const QString& fileType) const;

    bool isUrlSupported(const QUrl& url) const;
    bool isFileSupported(const mixxx::FileInfo& fileInfo) const;
    bool isFileNameSupported(const QString& fileName) const;
    bool isFileTypeSupported(const QString& fileType) const;
    bool isFileSuffixSupported(const QString& fileSuffix) const;
};

} // namespace mixxx
