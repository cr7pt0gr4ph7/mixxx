#pragma once

#include "util/db/dbid.h"

class TraktorPlaylistId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(TraktorPlaylistId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TraktorPlaylistId)
