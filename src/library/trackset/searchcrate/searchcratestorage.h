#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/crate/cratestorage.h
#include "library/trackset/searchcrate/searchcrateid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class SearchCrate;
class SearchCrateSummary;

class SearchCrateQueryFields {
  public:
    SearchCrateQueryFields() {
    }
    explicit SearchCrateQueryFields(const FwdSqlQuery& query);
    virtual ~SearchCrateQueryFields() = default;

    SearchCrateId getId(const FwdSqlQuery& query) const {
        return SearchCrateId(query.fieldValue(m_iId));
    }
    QString getName(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iName).toString();
    }
    SearchCrateId getParentId(const FwdSqlQuery& query) const {
        return SearchCrateId(query.fieldValue(m_iParentId));
    }
    bool isLocked(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iLocked);
    }
    bool isAutoDjSource(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iAutoDjSource);
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            SearchCrate* pCrate) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iParentId;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class SearchCrateSelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateSelectResult(SearchCrateSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateSelectResult() override = default;

    bool populateNext(SearchCrate* pCrate) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrate);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class SearchCrateStorage;
    SearchCrateSelectResult() = default;
    explicit SearchCrateSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateQueryFields m_queryFields;
};

class SearchCrateSummaryQueryFields : public SearchCrateQueryFields {
  public:
    SearchCrateSummaryQueryFields() = default;
    explicit SearchCrateSummaryQueryFields(const FwdSqlQuery& query);
    ~SearchCrateSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // crate is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // crate is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }
    QString getFullPath(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iFullPath).toString();
    }
    QString getFolderPath(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iFolderPath).toString();
    }
    QList<SearchCrateId> getAncestorIds(const FwdSqlQuery& query) const;

    void populateFromQuery(
            const FwdSqlQuery& query,
            SearchCrateSummary* pCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
    DbFieldIndex m_iFullPath;
    DbFieldIndex m_iFolderPath;
    DbFieldIndex m_iAncestorIds;
};

class SearchCrateSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateSummarySelectResult(SearchCrateSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateSummarySelectResult() override = default;

    bool populateNext(SearchCrateSummary* pCrateSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrateSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class SearchCrateStorage;
    SearchCrateSummarySelectResult() = default;
    explicit SearchCrateSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateSummaryQueryFields m_queryFields;
};

class SearchCrateTrackQueryFields {
  public:
    SearchCrateTrackQueryFields() = default;
    explicit SearchCrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~SearchCrateTrackQueryFields() = default;

    SearchCrateId crateId(const FwdSqlQuery& query) const {
        return SearchCrateId(query.fieldValue(m_iCrateId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iCrateId;
    DbFieldIndex m_iTrackId;
};

class SearchCrateTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateTrackSelectResult(SearchCrateTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateTrackSelectResult() override = default;

    SearchCrateId crateId() const {
        return m_queryFields.crateId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class SearchCrateStorage;
    SearchCrateTrackSelectResult() = default;
    explicit SearchCrateTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateTrackQueryFields m_queryFields;
};

class SearchCrateStorage : public virtual /*implements*/ SqlStorage {
  public:
    SearchCrateStorage() = default;
    ~SearchCrateStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Search Crate write operations (transactional, non-const)
    // Only invoked by TrackCollection!
    //
    // Naming conventions:
    //  on<present participle>...()
    //    - Invoked within active transaction
    //    - May fail
    //    - Performs only database modifications that are either committed
    //      or implicitly reverted on rollback
    //  after<present participle>...()
    //    - Invoked after preceding transaction has been committed (see above)
    //    - Must not fail
    //    - Typical use case: Update internal caches and compute change set
    //      for notifications
    /////////////////////////////////////////////////////////////////////////

    bool onInsertingCrate(
            const SearchCrate& crate,
            SearchCrateId* pCrateId = nullptr);

    bool onUpdatingCrate(
            const SearchCrate& crate);

    bool onDeletingCrate(
            SearchCrateId crateId);

    /////////////////////////////////////////////////////////////////////////
    // Search Crate read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countCrates() const;

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateById(
            SearchCrateId id,
            SearchCrate* pCrate = nullptr) const;
    bool readCrateByName(
            SearchCrateId parent,
            const QString& name,
            SearchCrate* pCrate = nullptr) const;

    // The following list results are ordered by crate name:
    //  - case-insensitive
    //  - locale-aware
    SearchCrateSelectResult selectCrates() const; // all crates
    SearchCrateSelectResult selectCratesByIds(    // subset of crates
            const QString& subselectForCrateIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // crateschema.h). AutoDJ should use the function selectCratesByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    SearchCrateSummarySelectResult selectAutoDjCrates(bool autoDjSource = true) const;

    // SearchCrate content, i.e. the crate's tracks referenced by id
    uint countCrateTracks(SearchCrateId crateId) const;

    // Format a subselect query for the tracks contained in crate.
    static QString formatSubselectQueryForCrateTrackIds(
            SearchCrateId crateId); // no db access

    QString formatQueryForTrackIdsByCrateNameLike(
            const QString& crateNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithCrate(); // no db access
    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    SearchCrateTrackSelectResult selectCrateTracksSorted(
            SearchCrateId crateId) const;
    SearchCrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;
    SearchCrateSummarySelectResult selectCratesWithTrackCount(
            const QList<TrackId>& trackIds) const;
    SearchCrateTrackSelectResult selectTracksSortedByCrateNameLike(
            const QString& crateNameLike) const;
    TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of crate ids for crates that contain any of the
    // provided track ids.
    QSet<SearchCrateId> collectCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // SearchCrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Returns whether crateA is an ancestor folder of crateB.
    bool isAncestor(SearchCrateId crateA, SearchCrateId crateB) const;

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    SearchCrateSummarySelectResult selectCrateSummaries() const; // all crates

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateSummaryById(SearchCrateId id, SearchCrateSummary* pCrateSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
