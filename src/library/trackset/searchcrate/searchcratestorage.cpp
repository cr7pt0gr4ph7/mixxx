#include "library/trackset/crate/cratestorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/searchcrate/searchcrate.h"
#include "library/trackset/searchcrate/searchcrateschema.h"
#include "library/trackset/searchcrate/searchcratesummary.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("SearchCrateStorage");

const QString SEARCHCRATE_SUMMARY_VIEW = "search_crate_summary";
const QString SEARCHCRATE_TRACKS_VIEW = "search_crate_tracks";

const QString SEARCHCRATESUMMARY_TRACK_COUNT = "track_count";
const QString SEARCHCRATESUMMARY_TRACK_DURATION = "track_duration";
const QString SEARCHCRATESUMMARY_FULL_PATH = "full_path";
const QString SEARCHCRATESUMMARY_FOLDER_PATH = "folder_path";
const QString SEARCHCRATESUMMARY_ANCESTOR_IDS = "ancestor_ids";

const QString kSubCrateSeparator(" / ");
const QChar kSqlListSeparator(',');

const QString kCrateFullPathTableExpression =
        QStringLiteral(
                "WITH RECURSIVE full_path_recursive(id, path, ancestors) AS "
                "( "
                "SELECT %2, %3, '' FROM %1 WHERE %4 IS NULL "
                "UNION ALL "
                "SELECT %1.%2, "
                "   full_path_recursive.path||'%5'||%1.%3, "
                "   full_path_recursive.ancestors||'%6'||full_path_recursive.id "
                "FROM %1 "
                "JOIN full_path_recursive ON %1.%4=full_path_recursive.id "
                ")")
                .arg(
                        SEARCHCRATE_TABLE,
                        SEARCHCRATETABLE_ID,
                        SEARCHCRATETABLE_NAME,
                        SEARCHCRATETABLE_PARENTID,
                        kSubCrateSeparator,
                        kSqlListSeparator);

const QString kCrateFullPathJoin =
        QStringLiteral("LEFT JOIN full_path_recursive ON %1.%2=full_path_recursive.id")
                .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_ID);

const QString kCrateTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_ID, SEARCHCRATE_TRACKS_VIEW, SEARCHCRATETRACKSTABLE_CRATEID);

const QString kLibraryTracksJoin = kCrateTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SEARCHCRATE_TRACKS_VIEW, SEARCHsCRATETRACKSTABLE_TRACKID, LIBRARY_TABLE, LIBRARYTABLE_ID);

const QString kCrateSummaryViewSelect =
        QStringLiteral(
                "%12 "
                "SELECT %1.*, "
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5, "
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6, "
                "fpr_self.path AS %7, "
                "fpr_parent.path AS %8, "
                "fpr_self.ancestors AS %9 "
                "FROM %1 "
                "LEFT JOIN full_path_recursive AS fpr_self ON %1.%10=fpr_self.id "
                "LEFT JOIN full_path_recursive AS fpr_parent ON %1.%11=fpr_parent.id ")
                .arg(
                        SEARCHCRATE_TABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        SEARCHCRATESUMMARY_TRACK_COUNT,
                        SEARCHCRATESUMMARY_TRACK_DURATION,
                        SEARCHCRATESUMMARY_FULL_PATH,
                        SEARCHCRATESUMMARY_FOLDER_PATH,
                        SEARCHCRATESUMMARY_ANCESTOR_IDS,
                        SEARCHCRATETABLE_ID,
                        SEARCHCRATETABLE_PARENTID,
                        kCrateFullPathTableExpression);

const QString kCrateSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        SEARCHCRATE_SUMMARY_VIEW,
                        kCrateSummaryViewSelect,
                        kLibraryTracksJoin,
                        SEARCHCRATE_TABLE,
                        SEARCHCRATETABLE_ID);

class SearchCrateQueryBinder final {
  public:
    explicit SearchCrateQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }

    void bindId(const QString& placeholder, const SearchCrate& crate) const {
        m_query.bindValue(placeholder, crate.getId());
    }
    void bindName(const QString& placeholder, const SearchCrate& crate) const {
        m_query.bindValue(placeholder, crate.getName());
    }
    void bindParentId(const QString& placeholder, const SearchCrate& crate) const {
        m_query.bindValue(placeholder, crate.getParentId().toVariantOrNull());
    }
    void bindLocked(const QString& placeholder, const SearchCrate& crate) const {
        m_query.bindValue(placeholder, QVariant(crate.isLocked()));
    }
    void bindAutoDjSource(const QString& placeholder, const SearchCrate& crate) const {
        m_query.bindValue(placeholder, QVariant(crate.isAutoDjSource()));
    }

  protected:
    FwdSqlQuery& m_query;
};

// It is not possible to bind multiple values as a list to a query.
// The list of track ids has to be transformed into a single list
// string before it can be used in an SQL query.
QString joinSqlStringList(const QList<TrackId>& trackIds) {
    QString joinedTrackIds;
    // Reserve memory up front to prevent reallocation. Here we
    // assume that all track ids fit into 6 decimal digits and
    // add 1 character for the list separator.
    joinedTrackIds.reserve((6 + 1) * trackIds.size());
    for (const auto& trackId : trackIds) {
        if (!joinedTrackIds.isEmpty()) {
            joinedTrackIds += kSqlListSeparator;
        }
        joinedTrackIds += trackId.toString();
    }
    return joinedTrackIds;
}

} // anonymous namespace

SearchCrateQueryFields::SearchCrateQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(SEARCHCRATETABLE_ID)),
          m_iName(query.fieldIndex(SEARCHCRATETABLE_NAME)),
          m_iParentId(query.fieldIndex(SEARCHCRATETABLE_PARENTID)),
          m_iLocked(query.fieldIndex(SEARCHCRATETABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(SEARCHCRATETABLE_AUTODJ_SOURCE)) {
}

void SearchCrateQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SearchCrate* pCrate) const {
    pCrate->setId(getId(query));
    pCrate->setName(getName(query));
    pCrate->setParentId(getParentId(query));
    pCrate->setLocked(isLocked(query));
    pCrate->setAutoDjSource(isAutoDjSource(query));
}

SearchCrateTrackQueryFields::SearchCrateTrackQueryFields(const FwdSqlQuery& query)
        : m_iCrateId(query.fieldIndex(SEARCHCRATETRACKSTABLE_CRATEID)),
          m_iTrackId(query.fieldIndex(SEARCHCRATETRACKSTABLE_TRACKID)) {
}

TrackQueryFields::TrackQueryFields(const FwdSqlQuery& query)
        : m_iTrackId(query.fieldIndex(SEARCHCRATETRACKSTABLE_TRACKID)) {
}

SearchCrateSummaryQueryFields::SearchCrateSummaryQueryFields(const FwdSqlQuery& query)
        : SearchCrateQueryFields(query),
          m_iTrackCount(query.fieldIndex(SEARCHCRATESUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(SEARCHCRATESUMMARY_TRACK_DURATION)),
          m_iFullPath(query.fieldIndex(SEARCHCRATESUMMARY_FULL_PATH)),
          m_iFolderPath(query.fieldIndex(SEARCHCRATESUMMARY_FOLDER_PATH)),
          m_iAncestorIds(query.fieldIndex(SEARCHCRATESUMMARY_ANCESTOR_IDS)) {
}


QList<SearchCrateId> SearchCrateSummaryQueryFields::getAncestorIds(const FwdSqlQuery& query) const {
    QList<SearchCrateId> result;
    for (const QString& id : query.fieldValue(m_iAncestorIds)
                              .toString()
                              .split(kSqlListSeparator, Qt::KeepEmptyParts)) {
        if (id.isEmpty()) {
            result.append(SearchCrateId());
        } else {
            // If id is a valid number, it will be automatically parsed while
            // coercing the QVariant to int inside the DbId constructor.
            SearchCrateId parsedId = SearchCrateId(id);
            DEBUG_ASSERT(parsedId.isValid());
            result.append(parsedId);
        }
    }
    return result;
}

void SearchCrateSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SearchCrateSummary* pCrateSummary) const {
    SearchCrateQueryFields::populateFromQuery(query, pCrateSummary);
    pCrateSummary->setTrackCount(getTrackCount(query));
    pCrateSummary->setTrackDuration(getTrackDuration(query));
    pCrateSummary->setFullPath(getFullPath(query));
    pCrateSummary->setFolderPath(getFolderPath(query));
    pCrateSummary->setAncestorIds(getAncestorIds(query));
}

void SearchCrateStorage::repairDatabase(const QSqlDatabase& database) {
    // NOTE(uklotzde): No transactions
    // All queries are independent so there is no need to enclose some
    // or all of them in a transaction. Grouping into transactions would
    // improve the overall performance at the cost of increased resource
    // utilization. Since performance is not an issue for a maintenance
    // operation the decision was not to use any transactions.

    // NOTE(uklotzde): Nested scopes
    // Each of the following queries is enclosed in a nested scope.
    // When leaving this scope all resources allocated while executing
    // the query are released implicitly and before executing the next
    // query.

    // Smart crates
    {
        // Delete crates with empty names
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "crates with empty names";
        }
    }
    {
        // Fix invalid values in the "locked" column
        FwdSqlQuery query(database,
                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_LOCKED));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed boolean values in table" << SEARCHCRATE_TABLE
                    << "column" << SEARCHCRATETABLE_LOCKED
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Fix invalid values in the "autodj_source" column
        FwdSqlQuery query(database,
                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_AUTODJ_SOURCE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed boolean values in table" << SEARCHCRATE_TABLE
                    << "column" << SEARCHCRATETABLE_AUTODJ_SOURCE
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Fix invalid -1/NULL values in the "parent_id" column
        FwdSqlQuery query(database,
                QStringLiteral("UPDATE %1 SET %2=NULL WHERE %2<0")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_PARENTID));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed NULL values in table" << SEARCHCRATE_TABLE
                    << "column" << SEARCHCRATETABLE_PARENTID
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Attach subcrates with a non-existent parent to the root folder instead
        FwdSqlQuery query(database,
                QStringLiteral(
                        "UPDATE %1 SET %3=NULL "
                        "FROM ( "
                        "SELECT self_crate.%2 FROM %1 AS self_crate "
                        "LEFT JOIN crates AS parent_crate on parent_crate.%2=self_crate.%3 "
                        "WHERE self_crate.%3 IS NOT NULL AND parent_crate.%2 IS NULL "
                        ") AS no_parent "
                        "WHERE %1.%2=no_parent.%3")
                        .arg(
                                SEARCHCRATE_TABLE,
                                SEARCHCRATETABLE_ID,
                                SEARCHCRATETABLE_PARENTID));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed broken references in table" << SEARCHCRATE_TABLE
                    << "column" << SEARCHCRATETABLE_PARENTID
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Break cycles where a subcrate is its own ancestor by attaching
        // all crates that are part of any cycle to the root folder instead
        FwdSqlQuery query(database,
                QStringLiteral(
                        "UPDATE %1 SET %3=NULL "
                        "FROM ( "
                        "%4 "
                        "SELECT self_crate.%2 FROM %1 AS self_crate "
                        "LEFT JOIN full_path_recursive ON self_crate.%2=full_path_recursive.id "
                        "WHERE full_path_recursive.ancestors IS NULL "
                        ") AS no_parent "
                        "WHERE %1.%2=no_parent.%3")
                        .arg(
                                SEARCHCRATE_TABLE,
                                SEARCHCRATETABLE_ID,
                                SEARCHCRATETABLE_PARENTID,
                                kCrateFullPathTableExpression));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed cyclic references in table" << SEARCHCRATE_TABLE
                    << "column" << SEARCHCRATETABLE_PARENTID
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
}

void SearchCrateStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void SearchCrateStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void SearchCrateStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kCrateSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for crate summaries!";
    }
}

uint SearchCrateStorage::countCrates() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(SEARCHCRATE_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool SearchCrateStorage::readCrateById(SearchCrateId id, SearchCrate* pCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SearchCrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning() << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Search crate not found by id:" << id;
        }
    }
    return false;
}

bool SearchCrateStorage::readCrateByName(
        SearchCrateId parentId, const QString& name, SearchCrate* pCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name "
                           "AND (CASE WHEN :parent IS NULL THEN %3 IS NULL ELSE %3=:parent END)")
                    .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_NAME, SEARCHCRATETABLE_PARENTID));
    query.bindValue(":name", name);
    query.bindValue(":parent", parentId.toVariantOrNull());
    if (query.execPrepared()) {
        SearchCrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning() << "Ambiguous crate name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "SearchCrate not found by name:" << name;
            }
        }
    }
    return false;
}

SearchCrateSelectResult SearchCrateStorage::selectCrates() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_NAME)));

    if (query.execPrepared()) {
        return SearchCrateSelectResult(std::move(query));
    } else {
        return SearchCrateSelectResult();
    }
}

SearchCrateSelectResult SearchCrateStorage::selectCratesByIds(
        const QString& subselectForCrateIds,
        SqlSubselectMode subselectMode) const {
    QString subselectPrefix;
    switch (subselectMode) {
    case SQL_SUBSELECT_IN:
        if (subselectForCrateIds.isEmpty()) {
            // edge case: no crates
            return SearchCrateSelectResult();
        }
        subselectPrefix = "IN";
        break;
    case SQL_SUBSELECT_NOT_IN:
        if (subselectForCrateIds.isEmpty()) {
            // edge case: all crates
            return selectCrates();
        }
        subselectPrefix = "NOT IN";
        break;
    }
    DEBUG_ASSERT(!subselectPrefix.isEmpty());
    DEBUG_ASSERT(!subselectForCrateIds.isEmpty());

    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 "
                                   "WHERE %2 %3 (%4) "
                                   "ORDER BY %5")
                            .arg(SEARCHCRATE_TABLE,
                                    SEARCHCRATETABLE_ID,
                                    subselectPrefix,
                                    subselectForCrateIds,
                                    SEARCHCRATETABLE_NAME)));

    if (query.execPrepared()) {
        return SearchCrateSelectResult(std::move(query));
    } else {
        return SearchCrateSelectResult();
    }
}

SearchCrateSummarySelectResult SearchCrateStorage::selectAutoDjCrates(bool autoDjSource) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral(
                            "SELECT * FROM %1 WHERE %2=:autoDjSource "
                            "ORDER BY %3")
                            .arg(
                                    SEARCHCRATE_SUMMARY_VIEW,
                                    SEARCHCRATETABLE_AUTODJ_SOURCE,
                                    SEARCHCRATESUMMARY_FULL_PATH)));
    query.bindValue(":autoDjSource", QVariant(autoDjSource));
    if (query.execPrepared()) {
        return SearchCrateSummarySelectResult(std::move(query));
    } else {
        return SearchCrateSummarySelectResult();
    }
}

bool SearchCrateStorage::isAncestor(SearchCrateId crateA, SearchCrateId crateB) const {
    // Note: An "invalid"/NULL crateA id is not actually invalid
    //       for this function, but instead represents the root folder.
    SearchCrateSummary crateSummary;
    readCrateSummaryById(crateB, &crateSummary);
    return crateSummary.isDescendantOf(crateA);
}

SearchCrateSummarySelectResult SearchCrateStorage::selectCrateSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SEARCHCRATE_SUMMARY_VIEW, SEARCHCRATETABLE_NAME)));
    if (query.execPrepared()) {
        return SearchCrateSummarySelectResult(std::move(query));
    } else {
        return SearchCrateSummarySelectResult();
    }
}

bool SearchCrateStorage::readCrateSummaryById(
        SearchCrateId id, SearchCrateSummary* pCrateSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SEARCHCRATE_SUMMARY_VIEW, SEARCHCRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SearchCrateSummarySelectResult crateSummaries(std::move(query));
        if ((pCrateSummary != nullptr)
                        ? crateSummaries.populateNext(pCrateSummary)
                        : crateSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crateSummaries.next()) {
                kLogger.warning() << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "SearchCrate summary not found by id:" << id;
        }
    }
    return false;
}

uint SearchCrateStorage::countCrateTracks(SearchCrateId crateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:crateId")
                    .arg(SEARCHCRATE_TRACKS_TABLE, SEARCHCRATETRACKSTABLE_CRATEID));
    query.bindValue(":crateId", crateId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

//static
QString SearchCrateStorage::formatSubselectQueryForCrateTrackIds(SearchCrateId crateId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATE_TRACKS_TABLE,
                    SEARCHCRATETRACKSTABLE_CRATEID,
                    crateId.toString());
}

QString SearchCrateStorage::formatQueryForTrackIdsByCrateNameLike(
        const QString& crateNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedCrateNameLike = escaper.escapeString(
            kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATE_TRACKS_TABLE,
                    SEARCHCRATE_TABLE,
                    SEARCHCRATETRACKSTABLE_CRATEID,
                    SEARCHCRATETABLE_ID,
                    SEARCHCRATETABLE_NAME,
                    escapedCrateNameLike);
}

//static
QString SearchCrateStorage::formatQueryForTrackIdsWithCrate() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATE_TRACKS_TABLE,
                    SEARCHCRATE_TABLE,
                    SEARCHCRATETRACKSTABLE_CRATEID,
                    SEARCHCRATETABLE_ID);
}

SearchCrateTrackSelectResult SearchCrateStorage::selectCrateTracksSorted(
        SearchCrateId crateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:crateId ORDER BY %3")
                    .arg(SEARCHCRATE_TRACKS_TABLE,
                            SEARCHCRATETRACKSTABLE_CRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    query.bindValue(":crateId", crateId);
    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

SearchCrateTrackSelectResult SearchCrateStorage::selectTrackCratesSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(SEARCHCRATE_TRACKS_TABLE,
                            SEARCHCRATETRACKSTABLE_TRACKID,
                            SEARCHCRATETRACKSTABLE_CRATEID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

SearchCrateSummarySelectResult SearchCrateStorage::selectCratesWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT *, "
                                   "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                                   "%1.%5 in (%10)) AS %6, "
                                   "0 as %7 FROM %2 "
                                   "ORDER BY %8 ASC NULLS LAST, %9")
                            .arg(
                                    SEARCHCRATE_TRACKS_TABLE,
                                    SEARCHCRATE_SUMMARY_VIEW,
                                    SEARCHCRATETABLE_ID,
                                    SEARCHCRATETRACKSTABLE_CRATEID,
                                    SEARCHCRATETRACKSTABLE_TRACKID,
                                    SEARCHCRATESUMMARY_TRACK_COUNT,
                                    SEARCHCRATESUMMARY_TRACK_DURATION,
                                    SEARCHCRATESUMMARY_FULL_PATH,
                                    SEARCHCRATETABLE_NAME,
                                    joinSqlStringList(trackIds))));

    if (query.execPrepared()) {
        return SearchCrateSummarySelectResult(std::move(query));
    } else {
        return SearchCrateSummarySelectResult();
    }
}

SearchCrateTrackSelectResult SearchCrateStorage::selectTracksSortedByCrateNameLike(
        const QString& crateNameLike) const {
    // TODO: Do SQL LIKE wildcards in crateNameLike need to be escaped?
    // Previously we used SqlLikeWildcardEscaper in the past for this
    // purpose. This utility class has become obsolete but could be
    // restored from the 2.3 branch if ever needed again.
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :crateNameLike "
                           "ORDER BY %1")
                    .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                            SEARCHCRATETRACKSTABLE_CRATEID,
                            SEARCHCRATE_TRACKS_TABLE,
                            SEARCHCRATE_TABLE,
                            SEARCHCRATETABLE_ID,
                            SEARCHCRATETRACKSTABLE_CRATEID,
                            SEARCHCRATETABLE_NAME));
    query.bindValue(":crateNameLike",
            QVariant(kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

TrackSelectResult SearchCrateStorage::selectAllTracksSorted() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT DISTINCT %1 FROM %2 ORDER BY %1")
                    .arg(SEARCHCRATETRACKSTABLE_TRACKID, SEARCHCRATE_TRACKS_TABLE));
    if (query.execPrepared()) {
        return TrackSelectResult(std::move(query));
    } else {
        return TrackSelectResult();
    }
}

QSet<SearchCrateId> SearchCrateStorage::collectCrateIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<SearchCrateId> trackCrates;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by crate id
        // here. But since the corresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        SearchCrateTrackSelectResult crateTracks(selectTrackCratesSorted(trackId));
        while (crateTracks.next()) {
            DEBUG_ASSERT(crateTracks.trackId() == trackId);
            trackCrates.insert(crateTracks.crateId());
        }
    }
    return trackCrates;
}

bool SearchCrateStorage::onInsertingCrate(
        const SearchCrate& crate,
        SearchCrateId* pCrateId) {
    VERIFY_OR_DEBUG_ASSERT(!crate.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert crate with a valid id:" << crate.getId();
        return false;
    }
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT INTO %1 (%2,%3,%4,%5) "
                    "VALUES (:name,:parent,:locked,:autoDjSource)")
                    .arg(
                            SEARCHCRATE_TABLE,
                            SEARCHCRATETABLE_NAME,
                            SEARCHCRATETABLE_PARENTID,
                            SEARCHCRATETABLE_LOCKED,
                            SEARCHCRATETABLE_AUTODJ_SOURCE));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SearchCrateQueryBinder queryBinder(query);
    queryBinder.bindName(":name", crate);
    queryBinder.bindParentId(":parent", crate);
    queryBinder.bindLocked(":locked", crate);
    queryBinder.bindAutoDjSource(":autoDjSource", crate);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pCrateId != nullptr) {
            *pCrateId = SearchCrateId(query.lastInsertId());
            DEBUG_ASSERT(pCrateId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool SearchCrateStorage::onUpdatingCrate(
        const SearchCrate& crate) {
    VERIFY_OR_DEBUG_ASSERT(crate.getId().isValid()) {
        kLogger.warning()
                << "Cannot update crate without a valid id";
        return false;
    }
    // Ensure that we do not create a cycle where a crate becomes its own ancestor.
    VERIFY_OR_DEBUG_ASSERT(crate.getId() != crate.getParentId() &&
            !isAncestor(crate.getId(), crate.getParentId())) {
        kLogger.warning() << "Cannot update parent crate of crate" << crate.getId()
                          << "to" << crate.getParentId()
                          << "because that would create a cycle";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:parent,%4=:locked,%5=:autoDjSource "
                    "WHERE %6=:id")
                    .arg(
                            SEARCHCRATE_TABLE,
                            SEARCHCRATETABLE_NAME,
                            SEARCHCRATETABLE_PARENTID,
                            SEARCHCRATETABLE_LOCKED,
                            SEARCHCRATETABLE_AUTODJ_SOURCE,
                            SEARCHCRATETABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SearchCrateQueryBinder queryBinder(query);
    queryBinder.bindId(":id", crate);
    queryBinder.bindName(":name", crate);
    queryBinder.bindParentId(":parent", crate);
    queryBinder.bindLocked(":locked", crate);
    queryBinder.bindAutoDjSource(":autoDjSource", crate);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple crates with the same id" << crate.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent crate with id" << crate.getId();
        return false;
    }
}

bool SearchCrateStorage::onDeletingCrate(
        SearchCrateId crateId) {
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        kLogger.warning()
                << "Cannot delete crate without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SEARCHCRATE_TRACKS_TABLE, SEARCHCRATETRACKSTABLE_CRATEID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty crate with id"
                        << crateId;
            }
        }
    }
    {
        // TODO(cr7pt0gr4ph7): Delete child crates instead of orphaning them
        FwdSqlQuery query(m_database,
                QStringLiteral("UPDATE %1 SET %2=NULL WHERE %2=:id")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_PARENTID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting crate without child crates with id"
                        << crateId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SEARCHCRATE_TABLE, SEARCHCRATETABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple crates with the same id" << crateId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent crate with id" << crateId;
            return false;
        }
    }
}

bool SearchCrateStorage::onAddingCrateTracks(
        SearchCrateId crateId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:crateId,:trackId)")
                    .arg(
                            SEARCHCRATE_TRACKS_TABLE,
                            SEARCHCRATETRACKSTABLE_CRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in crate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to crate" << crateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SearchCrateStorage::onRemovingCrateTracks(
        SearchCrateId crateId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:crateId AND %3=:trackId")
                    .arg(
                            SEARCHCRATE_TRACKS_TABLE,
                            SEARCHCRATETRACKSTABLE_CRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in crate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from crate" << crateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SearchCrateStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from crates one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(SEARCHCRATE_TRACKS_TABLE, SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
    }
    return true;
}
