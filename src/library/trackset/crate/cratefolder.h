#pragma once

#include "library/trackset/crate/cratefolderid.h"
#include "util/db/dbnamedentity.h"

class CrateFolder : public DbNamedEntity<CrateFolderId> {
  public:
    explicit CrateFolder(CrateFolderId id = CrateFolderId())
            : DbNamedEntity(id) {
    }
    ~CrateFolder() override = default;

    CrateFolderId getParentId() const {
        return m_parentId;
    }
    void setParentId(CrateFolderId parentId) {
        m_parentId = parentId;
    }

  private:
    CrateFolderId m_parentId;
};
