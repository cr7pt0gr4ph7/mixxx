#pragma once

#include "util/db/dbnamedentity.h"

// Base class for database entities with a non-empty name that can be nested.
template<typename T> // where T is derived from DbId
class DbNestedEntity: public DbNamedEntity<T> {
  public:
    ~DbNamedEntity() override = default;

    T getParentId() const {
        return m_parentId;
    }
    void setParentId(T parentId) {
        m_parentId = parentId;
    }

  protected:
    DbNestedEntity() = default;
    explicit DbNestedEntity(T id)
        : DbNamedEntity<T>(std::forward<T>(id)) {
    }

  private:
    T m_parentId;
};

template<typename T>
QDebug operator<<(QDebug debug, const DbNestedEntity<T>& entity) {
    return debug << QString("%1 '%2'").arg(entity.getId().toString(), entity.getName());
}
