#pragma once

#include <QList>
#include <QString>

#include "library/queries/queryoperator.h"

/// Represents the type of a queryable field, which
/// determines the search query operators that are available on it.
enum class QueryFieldType {
    Invalid = 0,
    Text,
    Number,
    Bpm,
    Date,
    Year,
    Duration,
    ChromaticKey,
    Playlist,
    Track,
    Crate,
    History
};

/// Describes the types of fields that can be used in queries.
///
/// Values of this class are immutable.
///
/// NOTE(cr7pt0gr4ph7): For simplicity, you should be able to pass
/// QueryFieldTypeInfo around as a value. Consumers do not (and should not) have
/// to care about the underlying object identities. For reduced memory consumption
/// we might explore implicit sharing using internal pointers.
class QueryFieldTypeInfo {
  public:
    /// Gets the detailed metadata for the specified query field type.
    static const QueryFieldTypeInfo get(QueryFieldType kind);

    /// Gets a list of all available query field types.
    static const QList<QueryFieldTypeInfo> allTypes();

    /// Gets the query field type represented by this instance.
    QueryFieldType kind() const {
        return m_kind;
    }

    /// Gets the name for this field type.
    QString name() const {
        return m_name;
    }

    /// Gets the localized name for this field type.
    QString localizedName() const {
        // FIXME(cr7pt0gr4ph7): Translate the name using tr("...")
        return m_name;
    }

    /// Gets the list of query operators supported by fields of this type.
    const QList<QueryOperator> supportedOperators() const {
        return m_supportedOperators;
    }

    /// Returns whether fields of this type support the specified query operator.
    bool isSupported(QueryOperator op) const;

    /// Returns whether fields of this type support the specified query operator.
    bool isSupported(const QueryOperatorInfo& op) const;

  private:
    /// External consumers should use get() and allTypes() to obtain
    /// the metadata for a given field type.
    QueryFieldTypeInfo(const QueryFieldType kind,
            const QString& name,
            const QList<QueryOperator>& supportedOperators);

    static QList<QueryFieldType> s_allTypes;
    QueryFieldType m_kind;
    QString m_name;
    QList<QueryOperator> m_supportedOperators;
};
