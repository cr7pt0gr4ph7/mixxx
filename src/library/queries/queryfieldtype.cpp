#include "library/queries/queryfieldtype.h"

QList<QueryFieldTypeInfo> QueryFieldTypeInfo::s_allTypes = {
        QueryFieldTypeInfo(QueryFieldType::Invalid, {}),
        QueryFieldTypeInfo(QueryFieldType::Text,
                "text",
                {
                        QueryOperator::Contains,
                        QueryOperator::StartsWith,
                        QueryOperator::EndsWith,
                        QueryOperator::IsEmpty,
                        QueryOperator::Equals,
                }),
        QueryFieldTypeInfo(QueryFieldType::Number,
                "numeric",
                {
                        QueryOperator::LessThanOrEqual,
                        QueryOperator::LessThan,
                        QueryOperator::GreaterThanOrEqual,
                        QueryOperator::GreaterThan,
                        QueryOperator::Equals,
                        QueryOperator::Between,
                }),
        QueryFieldTypeInfo(QueryFieldType::Bpm,
                "bpm",
                {
                        QueryOperator::LessThanOrEqual,
                        QueryOperator::LessThan,
                        QueryOperator::GreaterThanOrEqual,
                        QueryOperator::GreaterThan,
                        QueryOperator::Equals,
                        QueryOperator::Between,
                }),
        QueryFieldTypeInfo(QueryFieldType::Date,
                "date",
                {
                        // Could also be translated as LessThan
                        QueryOperator::Before,
                        // Could also be translated as GreaterThan
                        QueryOperator::After,
                        QueryOperator::Equals,
                        QueryOperator::Between,
                }),
        QueryFieldTypeInfo(QueryFieldType::Year,
                "year",
                {ea756e906f6eda1c7fd605ddfd8ab153a6c586c6
                        // Could also be translated as LessThan
                        QueryOperator::Before,
                        // Could also be translated as GreaterThan
                        QueryOperator::After,
                        QueryOperator::Equals,
                        QueryOperator::Between,
                }),
        QueryFieldTypeInfo(QueryFieldType::Duration,
                "duration",
                {
                        QueryOperator::LessThan,
                        QueryOperator::GreaterThan,
                        QueryOperator::Equals,
                        QueryOperator::Between,
                }),
        QueryFieldTypeInfo(QueryFieldType::ChromaticKey,
                "key",
                {
                        QueryOperator::Equals,
                        QueryOperator::Compatible,
                }),
        QueryFieldTypeInfo(QueryFieldType::Playlist,
                "playlist",
                {
                        QueryOperator::Is,
                        QueryOperator::Contains,
                }),
        QueryFieldTypeInfo(QueryFieldType::Track,
                "track",
                {
                        // Could also be translated as IsNotEmpty
                        QueryOperator::IsMemberOf,
                }),
        QueryFieldTypeInfo(QueryFieldType::Crate,
                "crate",
                {
                        QueryOperator::Is,
                        QueryOperator::Contains,
                }),
        QueryFieldTypeInfo(QueryFieldType::History,
                "history",
                {
                        QueryOperator::Is,
                        QueryOperator::Contains,
                }),
};

// static
QueryFieldTypeInfo QueryFieldTypeInfo::get(QueryFieldType kind) {
    return s_allTypes.value(static_cast<int>(kind));
}

// static
QList<QueryFieldTypeInfo> QueryFieldTypeInfo::allTypes() {
    return s_allTypes;
}

QueryFieldTypeInfo::QueryFieldTypeInfo(const QueryFieldType kind,
        const QString& name,
        const QList<QueryOperator>& supportedOperators)
        : m_kind(kind),
          m_name(name),
          m_supportedOperators(supportedOperators) {
}

bool QueryFieldTypeInfo::isSupported(QueryOperator op) const {
    return m_supportedOperators.contains(op);
}

bool QueryFieldTypeInfo::isSupported(const QueryOperatorInfo& op) const {
    return isSupported(op.kind());
}
