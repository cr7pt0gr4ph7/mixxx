#pragma once

#include "util/db/dbid.h"

class SeratoPlaylistTrackId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(SeratoPlaylistTrackId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SeratoPlaylistTrackId)
