#pragma once

#include "util/db/dbid.h"

class PlaylistId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;

    /// Compatibility constructor.
    /// Remove once migration to PlaylistId is completed.
    PlaylistId(const int value)
            : DbId(QVariant(value)) {
    }

    /// Compatibility conversion function.
    /// Remove once migration to PlaylistId is completed.
    operator int() const {
        return toVariant().toInt();
    }

    /// Compatibility operators.
    /// Remove once migration to PlaylistId is completed.
    bool operator==(const PlaylistId& lhs, const int rhs) {
        return lhs == PlaylistId(rhs);
    }

    bool operator!=(const PlaylistId& lhs, const int rhs) {
        return lhs != PlaylistId(rhs);
    }

    bool operator==(const int lhs, const PlaylistId& rhs) {
        return PlaylistId(lhs) == rhs;
    }

    bool operator!=(const int lhs, const PlaylistId& rhs) {
        return PlaylistId(lhs) != rhs;
    }
};

Q_DECLARE_TYPEINFO(PlaylistId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(PlaylistId)
