#pragma once

#include "track/cueinfo.h"

class CueUtils {
  public:
    static inline bool isHotCueType(mixxx::CueType type) {
        // NOTE: CueType::Loop is also used for storing the main loop.
        //       Use hotcueIndex != Cue::kNoHotCue to distinguish
        //       loop hotcues from the main loop.
        return (type == mixxx::CueType::HotCue ||
                type == mixxx::CueType::Loop ||
                type == mixxx::CueType::Jump);
    }

    static inline bool matchesKind(mixxx::CueType type, mixxx::CueKind kind) {
        return kind.testFlag(static_cast<mixxx::CueKindFlag>(1 << static_cast<int>(type)));
    }
};
