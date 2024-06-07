#include "track/customfieldsparser.h"

namespace {

const QString kCapturingGroupName = QStringLiteral("name");
const QString kCapturingGroupValue = QStringLiteral("value");

} // namespace

CustomFieldsTextParser::CustomFieldsTextParser()
        : m_regex(
                  "^(?<name>[a-zA-Z0-9_]+): ?(?<value>.+)$",
                  QRegularExpression::CaseInsensitiveOption |
                          QRegularExpression::MultilineOption) {
}

CustomFieldValues CustomFieldsTextParser::parse(const QString& text) const {
    CustomFieldValues result;
    auto allMatches = m_regex.globalMatch(text);
    while (allMatches.hasNext()) {
        auto match = allMatches.next();
        result.setField(
                CustomFieldName(match.captured(kCapturingGroupName)),
                match.captured(kCapturingGroupValue));
    }
    return result;
}

QString CustomFieldsTextParser::serialize(const CustomFieldValues& data) const {
    return QString();
}

QString CustomFieldsTextParser::serializeInto(
        const QString& oldText, const CustomFieldValues& data) const {
    return oldText;
}
