#pragma once

#include <QRegularExpression>
#include <QString>

#include "track/customfields.h"

class CustomFieldsParser {
  public:
    virtual CustomFieldValues parse(const QString& text) const = 0;
    virtual QString serializeInto(const QString& oldText, const CustomFieldValues& data) const = 0;
    virtual QString serialize(const CustomFieldValues& data) const = 0;
};

class CustomFieldsTextParser : public CustomFieldsParser {
  public:
    CustomFieldsTextParser();
    CustomFieldValues parse(const QString& text) const override;
    QString serializeInto(const QString& oldText, const CustomFieldValues& data) const override;
    QString serialize(const CustomFieldValues& data) const override;

  private:
    QRegularExpression m_regex;
};

class CustomFieldsJsonParser : public CustomFieldsParser {
  public:
    CustomFieldValues parse(const QString& text) const override;
    QString serializeInto(const QString& oldText, const CustomFieldValues& data) const override;
    QString serialize(const CustomFieldValues& data) const override;
};
