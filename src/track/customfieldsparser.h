#pragma once

#include <QString>

#include "track/customfields.h"

class CustomFieldsParser {
  public:
    virtual CustomFieldValues parse(const QString& text) const = 0;
    virtual QString serializeInto(const QString& oldText, const CustomFieldValues& data) const = 0;
    virtual QString serialize(const CustomFieldValues& data) const = 0;
};
