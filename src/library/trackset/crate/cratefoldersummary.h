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

  private:
    QString m_fullPath;
};
