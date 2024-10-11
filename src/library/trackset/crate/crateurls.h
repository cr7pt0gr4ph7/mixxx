#pragma once

#include <QUrl>
#include <QUrlQuery>

#include "library/trackset/crate/cratefolderid.h"
#include "library/trackset/crate/crateid.h"
#include "library/trackset/crate/crateorfolderid.h"

class CrateURLs {
  private:
    static const QString kCrateUrl;
    static const QString kFolderUrl;

    static QUrl urlFromTemplate(const QString& urlTemplate, DbId id) {
        if (!id.isValid()) {
            return QUrl();
        }
        return QUrl(QStringLiteral("%1?id=%2").arg(urlTemplate, id.toString()));
    }

    template<typename T>
    static T idFromUrl(const QString& urlTemplate, const QUrl& url) {
        if (url.isEmpty()) {
            return T();
        }
        const QString urlString = url.adjusted(QUrl::RemoveFragment | QUrl::RemoveQuery).toString();
        if (urlString != urlTemplate) {
            return T();
        }
        const QUrlQuery query(url);
        const QString id = query.queryItemValue("id");
        if (id.isEmpty()) {
            return T();
        }
        // If id is a valid number, it will be automatically parsed while
        // coercing the QVariant to int inside the DbId constructor.
        return T(QVariant(id));
    }

  public:
    // Returns the URL representing the specified, valid crateId,
    // or an empty QUrl if crateId is invalid.
    static QUrl toUrl(CrateId crateId) {
        return urlFromTemplate(kCrateUrl, crateId);
    }

    // Returns the CrateId represented by the specified QUrl,
    // or an invalid CrateId if the URL does not represent a crate.
    static CrateId parseCrateUrl(const QUrl& url) {
        return idFromUrl<CrateId>(kCrateUrl, url);
    }

    // Returns the URL representing the specified, valid folderId,
    // or an empty QUrl if folderId is invalid.
    static QUrl toUrl(CrateFolderId folderId) {
        return urlFromTemplate(kFolderUrl, folderId);
    }

    // Returns the CrateFolderId represented by the specified QUrl,
    // or an invalid CrateFolderId if the URL does not represent a crate folder.
    static CrateFolderId parseFolderUrl(const QUrl& url) {
        return idFromUrl<CrateFolderId>(kFolderUrl, url);
    }

    // Returns the URL representing the specified, valid id,
    // or an empty QUrl if crateOrFolderId is invalid.
    static QUrl toUrl(CrateOrFolderId crateOrFolderId) {
        if (crateOrFolderId.isCrate()) {
            return toUrl(crateOrFolderId.toCrateId());
        }
        if (crateOrFolderId.isFolder()) {
            return toUrl(crateOrFolderId.toFolderId());
        }
        return QUrl();
    }

    // Returns the CrateOrFolderId represented by the specified QUrl,
    // or an invalid CrateOrFolderId if the URL does not represent
    // a crate or crate folder.
    static CrateOrFolderId parseCrateOrFolderUrl(const QUrl& url) {
        CrateId crateId = parseCrateUrl(url);
        if (crateId.isValid()) {
            return crateId;
        }
        CrateFolderId folderId = parseFolderUrl(url);
        if (folderId.isValid()) {
            return folderId;
        }
        return CrateOrFolderId();
    }

    // Parses the list of URLs, and returns the corresponding list of crate/folder ids.
    // Urls that do not represent crate or crate folder references are ignored.
    static QList<CrateOrFolderId> parseCrateOrFolderUrls(const QList<QUrl>& urls) {
        QList<CrateOrFolderId> ids;
        for (const QUrl& url : urls) {
            CrateOrFolderId id = parseCrateOrFolderUrl(url);
            if (id.isValid()) {
                ids.append(id);
            }
        }
        return ids;
    }
};

const QString CrateURLs::kCrateUrl = QStringLiteral("mixxx://library/crates");
const QString CrateURLs::kFolderUrl = QStringLiteral("mixxx://library/cratefolders");
