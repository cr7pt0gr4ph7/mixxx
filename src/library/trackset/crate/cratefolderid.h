#pragma once

#include "util/db/dbid.h"

class CrateFolderId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(CrateFolderId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CrateFolderId)
