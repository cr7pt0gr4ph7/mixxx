#include "library/queries/queryoperator.h"

QList<QueryOperatorInfo> QueryOperatorInfo::s_allOperators = {
        QueryOperatorInfo(QueryOperator::Invalid, Args::Invalid, "", "", ""),
        QueryOperatorInfo(QueryOperator::Equals,
                Args::One,
                "equal to",
                "not equal to",
                "="),
        QueryOperatorInfo(QueryOperator::Contains,
                Args::One,
                "contains",
                "does not contain",
                "" /* Placeholder */),
        QueryOperatorInfo(QueryOperator::StartsWith,
                Args::One,
                "starts with",
                "does not start with",
                "" /* Placeholder */),
        QueryOperatorInfo(QueryOperator::EndsWith,
                Args::One,
                "ends with",
                "does not end with",
                "" /* Placeholder */),
        QueryOperatorInfo(QueryOperator::In,
                Args::ZeroOrMore,
                "in",
                "not in",
                "" /* Placeholder */),
        QueryOperatorInfo(
                QueryOperator::Before,
                Args::One,
                "before",
                "not before",
                "<"),
        QueryOperatorInfo(QueryOperator::LessThan,
                Args::One,
                "less than",
                "not less than",
                "<"),
        QueryOperatorInfo(QueryOperator::LessThanOrEqual,
                Args::One,
                "less than or equal",
                "not less than or equal",
                "<="),
        QueryOperatorInfo(
                QueryOperator::After,
                Args::One,
                "after",
                "not after",
                ">"),
        QueryOperatorInfo(QueryOperator::GreaterThan,
                Args::One,
                "greater than",
                "not greater than",
                ">"),
        QueryOperatorInfo(QueryOperator::GreaterThanOrEqual,
                Args::One,
                "greater than or equal",
                "not greater than or equal",
                ">="),
        QueryOperatorInfo(QueryOperator::Between,
                Args::Two,
                "",
                "",
                ".." /* Placeholder */),
        QueryOperatorInfo(QueryOperator::Compatible,
                Args::One,
                "is compatible with",
                "is not compatible with",
                "~"),
        QueryOperatorInfo(QueryOperator::IsEmpty,
                Args::Zero,
                "is empty",
                "is not empty",
                "-"),
        QueryOperatorInfo(QueryOperator::Is,
                Args::One,
                "is",
                "is not",
                "" /* Placeholder */),
        QueryOperatorInfo(QueryOperator::IsMemberOf,
                Args::One,
                "is member of",
                "is no member of",
                "" /* Placeholder */),
};

// static
static QueryOperatorInfo QueryOperatorInfo : get(QueryOperator kind) {
    return s_allOperators.value(static_cast<int>(kind));
}

// static
static QList<QueryOperatorInfo> QueryOperatorInfo::allOperators() {
    return s_allOperators;
}

QueryOperatorInfo::QueryOperatorInfo(const QueryFieldType kind,
        const Args numArgs,
        const QString& name,
        const QString& symbol)
        : m_kind(kind),
          m_numArgs(numArgs),
          m_name(name),
          m_symbol(symbol) {
}

bool QueryOperatorInfo::isValidNumberOfArguments(int number) const {
    switch (m_numArgs) {
    case Args::Invalid:
        return false;
    case Args::Zero:
        return number == 0;
    case Args::One:
        return number == 1;
    case Args::Two:
        return number == 2;
    case Args::ZeroOrMore:
        return number >= 0;
    case Args::OneOrMore:
        return number >= 1;
    default:
        return false;
    }
}
