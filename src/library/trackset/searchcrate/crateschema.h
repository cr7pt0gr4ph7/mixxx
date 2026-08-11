#pragma once

#include <QString>

#define SEARCHCRATE_TABLE "crates"

const QString SEARCHCRATETABLE_ID = QStringLiteral("id");
const QString SEARCHCRATETABLE_NAME = QStringLiteral("name");
const QString SEARCHCRATETABLE_QUERY = QStringLiteral("query");
const QString SEARCHCRATETABLE_LOCKED = QStringLiteral("locked");
const QString SEARCHCRATETABLE_PARENTID = QStringLiteral("parent_id");

// TODO(XXX): Fix AutoDJ database design.
// Crates should have no dependency on AutoDJ stuff. Which
// crates are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString SEARCHCRATETABLE_AUTODJ_SOURCE = QStringLiteral("autodj_source");

const QString SEARCHCRATETRACKSTABLE_CRATEID = QStringLiteral("crate_id");
const QString SEARCHCRATETRACKSTABLE_TRACKID = QStringLiteral("track_id");
