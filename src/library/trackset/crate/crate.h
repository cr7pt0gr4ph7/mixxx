#pragma once

#include "library/trackset/crate/crateid.h"
#include "util/db/dbnesteddentity.h"

class Crate : public DbNestedEntity<CrateId> {
  public:
    explicit Crate(CrateId id = CrateId())
            : DbNestedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~Crate() override = default;

    bool isLocked() const {
        return m_locked;
    }
    void setLocked(bool locked = true) {
        m_locked = locked;
    }

    bool isAutoDjSource() const {
        return m_autoDjSource;
    }
    void setAutoDjSource(bool autoDjSource = true) {
        m_autoDjSource = autoDjSource;
    }

  private:
    bool m_locked;
    bool m_autoDjSource;
};
