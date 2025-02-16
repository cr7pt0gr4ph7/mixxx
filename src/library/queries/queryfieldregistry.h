#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <memory>

#include "library/queries/queryfield.h"

/// Represents a group of related query fields. This should only be used
/// for the purpose of presenting these logical groups within the UI.
///
/// Values of this class are immutable.
///
/// NOTE(cr7pt0gr4ph7): For simplicity, you should be able to pass
/// QueryFieldCategory around as a value. Consumers do not (and should not) have
/// to care about the underlying object identities. For reduced memory
/// consumption we might explore implicit sharing using internal pointers.
class QueryFieldCategory {
  public:
    QueryFieldCategory(const QString& name,
            const QString& localizedName,
            const QList<QueryFieldInfo>& fields);

    /// Gets the internal name of this group.
    QString name() const {
        // FIXME(cr7pt0gr4ph7): Translate the name using tr("...")
        return m_name;
    }

    /// Gets the localized display name of this group.
    QString localizedName() const {
        // FIXME(cr7pt0gr4ph7): Translate the name using tr("...")
        return m_localizedName;
    }

    /// Gets the fields that belong to this group.
    QList<QueryFieldInfo> fields() const {
        return m_fields;
    }

  private:
    QString m_name;
    QString m_localizedName;
    QList<QueryFieldInfo> m_fields;
}

/// Provides access to the list of fields that are available
/// for use in search queries.
class QueryFieldRegistry {
  public:
    QueryFieldRegistry();
    QueryFieldRegistry(const QList<QString>& fields);

    /// Gets the field with the specified name.
    ///
    /// Returns a QueryFieldInfo with QueryFieldType::Invalid when the specified
    /// name does not refer to a known field.
    QueryFieldInfo get(const QString& name) const;

    /// Returns a list of all available fields.
    QList<QueryFieldInfo> allFields() const;

    /// Returns a list of all fields grouped into logical groups.
    QList<QueryFieldCategory> allFieldsByGroup() const;

    /// Adds the default fields to this registry.
    ///
    /// Should only be called at most once on a given instance.
    ///
    /// Not thread-safe.
    void addDefaultFields();

  private:
    void addFields(const QList<QueryFieldInfo> fields);

    static QList<QueryFieldInfo> s_defaultFields;
    QList<QueryFieldInfo> m_allFields;
    QMap<QString, QueryFieldInfo> m_nameToField;
    bool m_defaultFieldsAdded;
};
