#pragma once

#include <QList>
#include <QString>

#include "library/queries/queryfieldtype.h"

/// Describes a field that can be used in queries.
///
/// Values of this class are immutable.
///
/// NOTE(cr7pt0gr4ph7): For simplicity, you should be able to pass
/// QueryFieldInfo around as a value. Consumers do not (and should not) have to
/// care about the underlying object identities. For reduced memory consumption we
/// might explore implicit sharing using internal pointers.
class QueryFieldInfo {
  public:
    QueryFieldInfo(const QString& name,
            const QueryFieldType type,
            const QList<QString>& sqlColumns);

    /// Gets the identifier for this query field.
    QString name() const {
        return m_name;
    }

    /// Gets a localized display name for this query field.
    QString localizedName() const {
        // FIXME(cr7pt0gr4ph7): Translate the name using tr("...")
        return m_name;
    }

    /// Internal. Gets the SQL database columns that his query field is backed by.
    QList<QString> sqlColumns() const {
        return m_sqlColumns;
    }

    /// Gets the type of this query field (which can be used to
    /// to determine the query operators supported by this field).
    QueryFieldTypeInfo type() const;

    /// Gets the list of query operators supported by this field.
    const QList<QueryOperator> supportedOperators() const;

    /// Returns whether instance of this field support the specified query operator.
    bool isSupported(QueryOperator op) const;

    /// Returns whether instances of this field support the specified query operator.
    bool isSupported(const QueryOperatorInfo op) const;

  private:
    static QList<QueryFieldInfo> s_allFields;
    QString m_name;
    QueryFieldType m_type;
    QList<QString> m_sqlColumns;
};
