#pragma once

#include "library/trackset/crate/cratefolder.h"

// A crate folder with aggregated properties
class CrateFolderSummary : public CrateFolder {
  public:
    explicit CrateFolderSummary(CrateFolderId id = CrateFolderId())
            : CrateFolder(id) {
    }
    ~CrateFolderSummary() override = default;

    // The full path of this crate folder, formatted as
    // "Grandparent name / Parent name / Folder name".
    QString getFullPath() const {
        return m_fullPath;
    }
    void setFullPath(const QString& fullPath) {
        m_fullPath = fullPath;
    }

    // The list of ancestors of this crate folder (including the root folder).
    const QList<CrateFolderId>& getAncestorIds() const {
        return m_ancestorIds;
    }
    void setAncestorIds(const QList<CrateFolderId>& ancestorIds) {
        m_ancestorIds = ancestorIds;
    }
    bool isDescendantOf(CrateFolderId otherId) const {
        // Note: An "invalid"/NULL folderA id is not actually invalid
        //       for this function, but instead represents the root folder.
        //
        //       The root folder is explicitly contained in the list of
        //       ancestorIds, so no additional explicit handling is required.
        return m_ancestorIds.contains(otherId);
    }

  private:
    QString m_fullPath;
    QList<CrateFolderId> m_ancestorIds;
};
