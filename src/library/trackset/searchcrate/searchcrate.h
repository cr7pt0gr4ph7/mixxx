#pragma once

#include "library/trackset/searchcrate/searchcrateid.h"
#include "util/db/dbnestedentity.h"

class SearchCrate : public DbNestedEntity<SearchCrateId> {
  public:
    explicit SearchCrate(SearchCrateId id = SearchCrateId())
            : DbNestedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~SearchCrate() override = default;

    QString getQuery() const {
        return m_query;
    }
    void setQuery(QString query) {
        m_query = query;
    }

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
    QString m_query;
    bool m_locked;
    bool m_autoDjSource;
};
