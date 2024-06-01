#include "track/customfields.h"

#include "util/keyvalueiterable.h"

QDebug operator<<(QDebug dbg, CustomFieldValues arg) {
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "CustomFieldValues {";
    for (auto [key, value] : asKeyValueIterable(arg.m_namesToValues)) {
        dbg << " " << key << ": " << value;
    }
    dbg << " }";
    return dbg;
}
