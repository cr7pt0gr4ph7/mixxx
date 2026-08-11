#pragma once

#include "library/trackset/searchcrate/searchcrateid.h"
#include "util/db/dbnamedentity.h"

class SearchCrate : public DbNamedEntity<SearchCrateId> {
  public:
    explicit SearchCrate(SearchCrateId id = SearchCrateId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~SearchCrate() override = default;

    SearchCrateId getParentId() const {
        return m_parentId;
    }
    void setParentId(SearchCrateId parentId) {
        m_parentId = parentId;
    }

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
    SearchCrateId m_parentId;
    QString m_query;
    bool m_locked;
    bool m_autoDjSource;
};
