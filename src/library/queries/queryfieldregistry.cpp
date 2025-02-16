#include "library/queries/queryfieldregistry.h"

#include "library/dao/trackschema.h"
#include "util/assert.h"

QList<QueryFieldInfo> QueryFieldRegistry::s_defaultFields = {
        QueryFieldInfo(
                QStringLiteral("simple_query"),
                QueryFieldType::Text,
                {}),
        QueryFieldInfo(
                QStringLiteral("artist"),
                QueryFieldType::Text,
                { LIBRARYTABLE_ARTIST, LIBRARYTABLE_ALBUMARTIST }),
        QueryFieldInfo(
                QStringLiteral("album_artist"),
                QueryFieldType::Text,
                { LIBRARYTABLE_ALBUMARTIST }),
        QueryFieldInfo(
                QStringLiteral("album"),
                QueryFieldType::Text,
                { LIBRARYTABLE_ALBUM }),
        QueryFieldInfo(
                QStringLiteral("title"),
                QueryFieldType::Text,
                { LIBRARYTABLE_TITLE }),
        QueryFieldInfo(
                QStringLiteral("genre"),
                QueryFieldType::Text,
                { LIBRARYTABLE_GENRE }),
        QueryFieldInfo(
                QStringLiteral("composer"),
                QueryFieldType::Text,
                { LIBRARYTABLE_COMPOSER }),
        QueryFieldInfo(
                QStringLiteral("grouping"),
                QueryFieldType::Text,
                { LIBRARYTABLE_GROUPING }),
        QueryFieldInfo(
                QStringLiteral("comment"),
                QueryFieldType::Text,
                { LIBRARYTABLE_COMMENT }),
        QueryFieldInfo(
                QStringLiteral("type"), // aka. filetype
                QueryFieldType::Text,
                { LIBRARYTABLE_FILETYPE }),
        QueryFieldInfo(
                QStringLiteral("location"),
                QueryFieldType::Text,
                QList(/* Not yet implemented */)),
        QueryFieldInfo(
                QStringLiteral("year"),
                QueryFieldType::Year,
                { LIBRARYTABLE_YEAR }),
        QueryFieldInfo(
                QStringLiteral("duration"),
                QueryFieldType::Duration,
                { LIBRARYTABLE_DURATION }),
        QueryFieldInfo(
                QStringLiteral("key"),
                QueryFieldType::ChromaticKey,
                { LIBRARYTABLE_KEY }),
        QueryFieldInfo(
                QStringLiteral("key_id"),
                // FIXME(cr7pt0gr4ph7): Check type of key_id
                QueryFieldType::Text,
                { LIBRARYTABLE_KEY_ID }),
        QueryFieldInfo(
                QStringLiteral("id"),
                // FIXME(cr7pt0gr4ph7): Check type of id
                QueryFieldType::Text,
                { LIBRARYTABLE_ID }),
        QueryFieldInfo(
                QStringLiteral("bpm"),
                QueryFieldType::Bpm,
                { LIBRARYTABLE_BPM }),
        QueryFieldInfo(
                QStringLiteral("bitrate"),
                QueryFieldType::Number,
                { LIBRARYTABLE_BITRATE }),
        QueryFieldInfo(
                QStringLiteral("datetime_added"),
                QueryFieldType::Date,
                { LIBRARYTABLE_DATETIMEADDED }),
        QueryFieldInfo(
                QStringLiteral("last_played_at"),
                QueryFieldType::Date,
                { LIBRARYTABLE_LAST_PLAYED_AT }),
        QueryFieldInfo(
                QStringLiteral("played"),
                QueryFieldType::Number,
                { LIBRARYTABLE_PLAYED }),
        QueryFieldInfo(
                QStringLiteral("timesplayed"),
                QueryFieldType::Number,
                { LIBRARYTABLE_TIMESPLAYED }),
        QueryFieldInfo(
                QStringLiteral("rating"),
                QueryFieldType::Number,
                { LIBRARYTABLE_RATING }),
        QueryFieldInfo(
                QStringLiteral("track"), /// aka. tracknumber
                QueryFieldType::Number,
                { LIBRARYTABLE_TRACKNUMBER }),
        QueryFieldInfo(
                QStringLiteral("track_in"), // aka. track:
                QueryFieldType::Track,
                QList(/* Requires special handling */)),
        QueryFieldInfo(
                QStringLiteral("playlist"),
                QueryFieldType::Playlist,
                QList(/* Requires special handling */)),
        QueryFieldInfo(
                QStringLiteral("crate"),
                QueryFieldType::Crate,
                QList(/* Requires special handling */)),
        QueryFieldInfo(
                QStringLiteral("history"),
                QueryFieldType::History,
                QList(/* Requires special handling */)),
};

 QueryFieldCategory(const QString& name, const QString& localizedName, const QList<QueryFieldInfo>& fields)
   : m_name(name),
     m_localizedName(localizedName),
     m_fields(fields) {
 }

QueryFieldRegistry::QueryFieldRegistry()
  : m_defaultFieldsAdded(false) {
}

QueryFieldRegistry::QueryFieldRegistry(const QList<QString>& fields)
  : m_defaultFieldsAdded(false) {
    addFields(fields);
}

QueryFieldInfo QueryFieldRegistry::get(const QString& name) const {
    auto lookupResult = m_nameToField.constFind(name);
    if (lookupResult == m_nameToField.constEnd()) {
        return QueryFieldInfo(name, QueryFieldType::Invalid, QList());
    }
    return lookupResult.value();
}

QList<QString> QueryFieldRegistry::allFields() const {
    return m_allFields;
}

QList<QueryFieldCategory> QueryFieldRegistry::allFieldsByCategory() const {
    // TODO(cr7pt0gr4ph7): Group the defined fields into the following categories:
    // - Ungrouped: Simple search in all main track fields
    // - "Track Fields": Album, Artist, Title, Album Artist
    // - "Additional Track Fields": Rating, Genre, Grouping, Comment, Year, ...
    // - "File Info": Bitrate, Filetype, (Duration?), Location
    // - "Library Metadata" Date Added, Times Played, Last Played At, ...
    // - "Custom": Custom fields, if these are ever implemented
    return { QueryFieldCategory("all", "All Fields", allFields()) };
}

void QueryFieldRegistry::addFields(const QList<QueryFieldInfo>& fields) {
    for (const auto& field : fields) {
        if (field.name().isEmpty()) {
            qWarning() << "QueryFieldRegistry: Skipping field with empty name.";
            continue;
        }
        if (field.type().kind() == QueryFieldType::Invalid) {
            qWarning() << "QueryFieldRegistry: Skipping field with invalid type";
            continue;
        }
        if (m_nameToField.contains(field.name())) {
            qWarning() << "QueryFieldRegistry: Skipping field with duplicate name.";
            continue;
        }
        m_nameToField.insert(field.name(), field);
        m_allFields.append(field);
    }
}

void QueryFieldRegistry::addDefaultFields() {
    VERIFY_OR_DEBUG_ASSERT(!m_defaultFieldsAdded) {
        qWarning() << "QueryFieldRegistry::addDefaultFields() has already "
                      "been called on this instance.";
        return;
    }
    addFields(s_defaultFields);
    m_defaultFieldsAdded = true;
}
