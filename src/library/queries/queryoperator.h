#pragma once

#include <QList>
#include <QString>

/// Represents a query operator within a search query.
///
/// Not all query field types support all operators,
/// and the number of arguments supported varies
/// between operators.
enum class QueryOperator {
    /// The user has not yet chosen a valid query operator.
    Invalid = 0,

    /// The target field must be equal to the provided argument.
    /// Depending on the type of the target field, this can mean
    /// textual or numerical equivalence.
    ///
    /// Requires a single argument.
    Equals,

    /// The target field must contain the provided argument as a substring.
    ///
    /// Requires a single argument.
    Contains,

    /// The target field must be a string that starts with the provided argument.
    ///
    /// Requires a single argument.
    StartsWith,

    /// The target field must be a string that ends with the provided argument.
    ///
    /// Requires a single argument.
    EndsWith,

    /// The target field must be equal (as defined by QueryOperator::Equals)
    /// to one of the provided arguments.
    ///
    /// Requires zero or more arguments.
    In,

    /// The target field must have a date value that is less than
    /// the provided argument (which must also be parsable as a date).
    ///
    /// Requires a single argument that is parseable as a date.
    Before,

    /// The target field must have a (numerical or date) value that is
    /// less than the provided argument.
    ///
    /// Requires a single argument that is parseable as a number or a date.
    LessThan,

    /// The target field must have a (numerical or date) value that is
    /// less than or equal to the provided argument.
    ///
    /// Requires a single argument that is parseable as a number or a date.
    LessThanOrEqual,

    /// The target field must have a date value that is less than
    /// the provided argument (which must also be parsable as a date).
    ///
    /// Requires a single argument that is parseable as a date.
    After,

    /// The target field must have a (numerical or date) value that is
    /// greater than the provided argument.
    ///
    /// Requires a single argument that is parseable as a number or a date.
    GreaterThan,

    /// The target field must have a (numerical or date) value that is
    /// greater than or equal to the provided argument.
    ///
    /// Requires a single argument that is parseable as a number or a date.
    GreaterThanOrEqual,

    /// The target field must have a (numerical or date) value that is
    /// between arguments[0] and arguments[1], inclusive.
    ///
    /// Requires two arguments.
    Between,

    /// The target field must be a chromatic key that is compatible
    /// with the key specified in the provided argument.
    ///
    /// Requires a single argument that is parseable as a chromatic key.
    Compatible,

    /// The target field must be empty (or null).
    ///
    /// No arguments are allowed.
    IsEmpty,

    // TODO(cr7pt0gr4ph7): Document (or potentially remove) Is operator
    Is,

    // TODO(cr7pt0gr4ph7): Document (or potentially remove) IsMemberOf operator
    IsMemberOf,
};

/// Describes the types of query operators available within search queries.
///
/// NOTE(cr7pt0gr4ph7): For simplicity, you should be able to pass
/// QueryOperatorInfo around as a value. Consumers do not (and should not) have
/// to care about the underlying object identities. For reduced memory consumption
/// we might explore implicit sharing using internal pointers.
class QueryOperatorInfo {
  public:
    /// Gets the detailed metadata for the specified query operator type.
    static const QueryOperatorInfo get(QueryOperator kind);

    /// Returns a list of all available query operators.
    static const QList<QueryOperatorInfo> allOperators();

    QueryOperator kind() const {
        return m_type;
    }

    /// Gets the name for this query operator.
    QString name(bool inverted) const {
        return m_name;
    }

    /// Gets the localized name for this query operator.
    QString localizedName(bool inverted) const {
        // FIXME(cr7pt0gr4ph7): Translate the name using tr("...")
        return m_name;
    }

    /// Gets the symbol representing this query operator.
    QString symbol() const {
        // FIXME(cr7pt0gr4ph): The Between operator has different syntax for different data types
        return m_symbol;
    }

    /// Returns whether the specified number of arguments is valid for this operator.
    bool isValidNumberOfArguments(int number) const;

  private:
    /// Represents the number of arguments accepted by a query operator.
    enum class Args {
        Invalid = 0,
        Zero,
        One,
        Two,
        ZeroOrMore,
        OneOrMore
    };

    /// External consumers should use get() and allTypes() to obtain
    /// the metadata for a given field type.
    QueryOperatorInfo(const QueryFieldType kind,
            const QString& name,
            const QString& invertedName,
            const QString& symbol,
            const Args numArgs,
            const QList<QueryOperator>& supportedOperators);

    static QList<QueryOperatorInfo> s_allOperators;
    QueryOperator m_type;
    QString m_name;
    QString m_invertedName;
    QString m_symbol;
    Args m_numArgs;
};
