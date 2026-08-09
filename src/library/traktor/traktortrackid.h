#pragma once

#include "util/db/dbid.h"

class TraktorTrackId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(TraktorTrackId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TraktorTrackId)
