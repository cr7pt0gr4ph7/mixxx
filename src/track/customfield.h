#pragma once

#include <QString>

#include "util/assert.h"

/// Represents the name of a custom track metadata field.
class CustomFieldName {
  public:
    static const QString kCustomColumnNamePrefix;

    CustomFieldName() {
    }

    CustomFieldName(const QString& name)
            : m_normalizedName(normalize(name)),
              m_columnName(columNameFromNormalizedName(m_normalizedName)) {
    }

    bool isValid() const {
        return !m_normalizedName.isEmpty();
    }

    QString normalizedName() const {
        return m_normalizedName;
    }

    QString columnName() const {
        return m_columnName;
    }

    static QString normalize(const QString& name) {
        return name.toLower();
    }

    static QString columNameFromNormalizedName(const QString& normalizedName) {
        DEBUG_ASSERT(normalize(normalizedName) == normalizedName);
        DEBUG_ASSERT(!normalizedName.startsWith(kCustomColumnNamePrefix));

        if (normalizedName.isEmpty()) {
            return QString();
        }

        return kCustomColumnNamePrefix + normalizedName;
    }

    static bool isCustomColumnName(const QString& columnName) {
        return columnName.startsWith(kCustomColumnNamePrefix);
    }

    static CustomFieldName fromColumnName(const QString& columnName) {
        if (isCustomColumnName(columnName)) {
            return columnName.sliced(kCustomColumnNamePrefix.size());
        }
        return CustomFieldName();
    }

  private:
    QString m_normalizedName;
    QString m_columnName;
};
