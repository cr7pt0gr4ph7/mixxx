#include "library/queries/queryfield.h"

QueryFieldInfo(const QString& name, const QueryFieldType type, const QList<QString>& sqlColumns)
        : m_name(name),
          m_type(type),
          m_sqlColumns(sqlColumns) {
}

QueryFieldTypeInfo QueryFieldInfo::type() const {
    return QueryFieldTypeInfo::get(m_type);
}

const QList<QueryOperatorType> QueryFieldInfo::supportedOperators() const {
    return type().supportedOperators();
}

bool QueryFieldInfo::isSupported(QueryOperator op) const {
    return type().isSupported(op);
}

bool QueryFieldInfo::isSupported(const QueryOperatorInfo& op) const {
    return type().isSupported(op);
}
