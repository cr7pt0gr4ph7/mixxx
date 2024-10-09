#pragma once

#include "library/trackset/crate/cratefolderid.h"
#include "library/trackset/crate/crateid.h"
#include "util/db/dbid.h"

class CrateOrFolderId {
  public:
    enum ItemType {
        Invalid = 0,
        Crate,
        Folder
    };

    constexpr CrateOrFolderId()
            : m_isFolder(false) {
    }

    CrateOrFolderId(CrateId crateId)
            : m_isFolder(false), m_id(crateId) {
    }

    CrateOrFolderId(CrateFolderId folderId)
            : m_isFolder(true), m_id(folderId) {
    }

    explicit CrateOrFolderId(bool isFolder, DbId id)
            : m_isFolder(isFolder), m_id(id) {
    }

    explicit CrateOrFolderId(const QVariant& variant) {
        // TODO(cr7pt0gr4ph7): Implement a more efficient QVariant encoding for CrateOrFolderId
        if (variant.isValid()) {
            auto hash = variant.toHash();
            m_isFolder = hash[QStringLiteral("isFolder")].toBool();
            m_id = DbId(hash[QStringLiteral("id")]);
        } else {
            m_isFolder = false;
            m_id = DbId();
        }
    }

  public:
    bool isValid() const {
        return m_id.isValid();
    }

    bool isCrate() const {
        return !m_isFolder;
    }

    bool isFolder() const {
        return m_isFolder;
    }

    ItemType itemType() const {
        if (isCrate()) {
            return ItemType::Crate;
        } else if (isFolder()) {
            return ItemType::Folder;
        } else {
            return ItemType::Invalid;
        }
    }

    DbId id() const {
        return m_id;
    }

    CrateId toCrateId() const {
        if (!isCrate()) {
            return CrateId();
        }
        return CrateId(m_id.toVariantOrNull());
    }

    CrateFolderId toFolderId() const {
        if (!isFolder()) {
            return CrateFolderId();
        }
        return CrateFolderId(m_id.toVariantOrNull());
    }

    // This function should be used for value binding in DB queries
    // with bindValue().
    QVariant toVariant() const {
        // TODO: Not fully correct
        QHash<QString, QVariant> hash;
        hash[QStringLiteral("isFolder")] = QVariant(m_isFolder);
        hash[QStringLiteral("id")] = m_id.toVariantOrNull();
        return QVariant(hash);
    }

    QString toString() const {
        return (m_isFolder ? QStringLiteral("Folder(")
                           : QStringLiteral("Crate(")) +
                m_id.toString() + QStringLiteral(")");
    }

    friend bool operator==(const CrateOrFolderId& lhs, const CrateOrFolderId& rhs) {
        return lhs.m_isFolder == rhs.m_isFolder && lhs.m_id == rhs.m_id;
    }

    friend bool operator!=(const CrateOrFolderId& lhs, const CrateOrFolderId& rhs) {
        return lhs.m_isFolder != rhs.m_isFolder || lhs.m_id != rhs.m_id;
    }

    friend QDebug operator<<(QDebug debug, const CrateOrFolderId& id) {
        return debug << id.toString();
    }

  private:
    bool m_isFolder;
    DbId m_id;
};

Q_DECLARE_TYPEINFO(CrateOrFolderId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CrateOrFolderId)
