#pragma once

#include "util/db/dbid.h"

class SeratoPlaylistId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(SeratoPlaylistId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SeratoPlaylistId)
