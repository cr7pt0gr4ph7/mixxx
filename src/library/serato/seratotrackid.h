#pragma once

#include "util/db/dbid.h"

class SeratoTrackId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(SeratoTrackId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SeratoTrackId)
