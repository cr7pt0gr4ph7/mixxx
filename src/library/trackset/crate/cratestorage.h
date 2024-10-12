#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/crate/crateorfolderid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class Crate;
class CrateFolder;
class CrateFolderSummary;
class CrateSummary;

class CrateFolderQueryFields {
  public:
    CrateFolderQueryFields() {
    }
    explicit CrateFolderQueryFields(const FwdSqlQuery& query);
    virtual ~CrateFolderQueryFields() = default;

    CrateFolderId getId(const FwdSqlQuery& query) const {
        return CrateFolderId(query.fieldValue(m_iId));
    }
    QString getName(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iName).toString();
    }
    CrateFolderId getParentId(const FwdSqlQuery& query) const {
        return CrateFolderId(query.fieldValue(m_iParentId));
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            CrateFolder* pFolder) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iParentId;
};

class CrateFolderSelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateFolderSelectResult(CrateFolderSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateFolderSelectResult() override = default;

    bool populateNext(CrateFolder* pFolder) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pFolder);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateFolderSelectResult() = default;
    explicit CrateFolderSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateFolderQueryFields m_queryFields;
};

class CrateFolderSummaryQueryFields : public CrateFolderQueryFields {
  public:
    CrateFolderSummaryQueryFields() = default;
    explicit CrateFolderSummaryQueryFields(const FwdSqlQuery& query);
    ~CrateFolderSummaryQueryFields() override = default;

    QString getFullPath(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iFullPath).toString();
    }
    QList<CrateFolderId> getAncestorIds(const FwdSqlQuery& query) const;

    void populateFromQuery(
            const FwdSqlQuery& query,
            CrateFolderSummary* pFolderSummary) const;

  private:
    DbFieldIndex m_iFullPath;
    DbFieldIndex m_iAncestorIds;
};

class CrateFolderSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateFolderSummarySelectResult(CrateFolderSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateFolderSummarySelectResult() override = default;

    bool populateNext(CrateFolderSummary* pFolderSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pFolderSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateFolderSummarySelectResult() = default;
    explicit CrateFolderSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateFolderSummaryQueryFields m_queryFields;
};

class CrateQueryFields {
  public:
    CrateQueryFields() {
    }
    explicit CrateQueryFields(const FwdSqlQuery& query);
    virtual ~CrateQueryFields() = default;

    CrateId getId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iId));
    }
    QString getName(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iName).toString();
    }
    CrateFolderId getFolderId(const FwdSqlQuery& query) const {
        return CrateFolderId(query.fieldValue(m_iFolderId));
    }
    bool isLocked(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iLocked);
    }
    bool isAutoDjSource(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iAutoDjSource);
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            Crate* pCrate) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iFolderId;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class CrateSelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateSelectResult(CrateSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSelectResult() override = default;

    bool populateNext(Crate* pCrate) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrate);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateSelectResult() = default;
    explicit CrateSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateQueryFields m_queryFields;
};

class CrateSummaryQueryFields : public CrateQueryFields {
  public:
    CrateSummaryQueryFields() = default;
    explicit CrateSummaryQueryFields(const FwdSqlQuery& query);
    ~CrateSummaryQueryFields() override = default;

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

    void populateFromQuery(
            const FwdSqlQuery& query,
            CrateSummary* pCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
    DbFieldIndex m_iFullPath;
    DbFieldIndex m_iFolderPath;
};

class CrateSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateSummarySelectResult(CrateSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSummarySelectResult() override = default;

    bool populateNext(CrateSummary* pCrateSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrateSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateSummarySelectResult() = default;
    explicit CrateSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateSummaryQueryFields m_queryFields;
};

class CrateTrackQueryFields {
  public:
    CrateTrackQueryFields() = default;
    explicit CrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~CrateTrackQueryFields() = default;

    CrateId crateId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iCrateId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iCrateId;
    DbFieldIndex m_iTrackId;
};

class TrackQueryFields {
  public:
    TrackQueryFields() = default;
    explicit TrackQueryFields(const FwdSqlQuery& query);
    virtual ~TrackQueryFields() = default;

    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iTrackId;
};

class CrateTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateTrackSelectResult(CrateTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateTrackSelectResult() override = default;

    CrateId crateId() const {
        return m_queryFields.crateId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class CrateStorage;
    CrateTrackSelectResult() = default;
    explicit CrateTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateTrackQueryFields m_queryFields;
};

class TrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    TrackSelectResult(TrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~TrackSelectResult() override = default;

    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class CrateStorage;
    TrackSelectResult() = default;
    explicit TrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    TrackQueryFields m_queryFields;
};

class CrateStorage : public virtual /*implements*/ SqlStorage {
  public:
    CrateStorage() = default;
    ~CrateStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Crate write operations (transactional, non-const)
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

    bool onInsertingFolder(
            const CrateFolder& folder,
            CrateFolderId* pFolderId = nullptr);

    bool onUpdatingFolder(
            const CrateFolder& folder);

    bool onDeletingFolder(
            CrateFolderId folderId);

    bool onInsertingCrate(
            const Crate& crate,
            CrateId* pCrateId = nullptr);

    bool onUpdatingCrate(
            const Crate& crate);

    bool onDeletingCrate(
            CrateId crateId);

    bool onAddingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Crate read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countFolders() const;

    // Omit the pFolder parameter for checking if the corresponding folder exists.
    bool readFolderById(
            CrateFolderId id,
            CrateFolder* pFolder = nullptr) const;
    bool readFolderByName(
            CrateFolderId parent,
            const QString& name,
            CrateFolder* pFolder = nullptr) const;

    // The following list results are ordered by crate/folder name:
    //  - case-insensitive
    //  - locale-aware
    CrateFolderSelectResult selectFolders() const; // all folders
    CrateFolderSelectResult selectFoldersByIds(    // subset of folders
            const QString& subselectForFolderIds,
            SqlSubselectMode subselectMode) const;

    uint countCrates() const;

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateById(
            CrateId id,
            Crate* pCrate = nullptr) const;
    bool readCrateByName(
            CrateFolderId folder,
            const QString& name,
            Crate* pCrate = nullptr) const;

    // The following list results are ordered by crate name:
    //  - case-insensitive
    //  - locale-aware
    CrateSelectResult selectCrates() const; // all crates
    CrateSelectResult selectCratesByIds(    // subset of crates
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
    CrateSelectResult selectAutoDjCrates(bool autoDjSource = true) const;

    // Crate content, i.e. the crate's tracks referenced by id
    uint countCrateTracks(CrateId crateId) const;

    // Format a subselect query for the tracks contained in crate.
    static QString formatSubselectQueryForCrateTrackIds(
            CrateId crateId); // no db access

    QString formatQueryForTrackIdsByCrateNameLike(
            const QString& crateNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithCrate(); // no db access
    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    CrateTrackSelectResult selectCrateTracksSorted(
            CrateId crateId) const;
    CrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;
    CrateSummarySelectResult selectCratesWithTrackCount(
            const QList<TrackId>& trackIds) const;
    CrateTrackSelectResult selectTracksSortedByCrateNameLike(
            const QString& crateNameLike) const;
    TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of crate ids for crates that contain any of the
    // provided track ids.
    QSet<CrateId> collectCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // CrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Returns whether folderA is an ancestor folder of folderB.
    bool isAncestor(CrateFolderId folderA, CrateFolderId folderB) const;

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    CrateFolderSummarySelectResult selectFolderSummaries() const; // all crates

    // Omit the pFolder parameter for checking if the corresponding folder exists.
    bool readFolderSummaryById(CrateFolderId id,
            CrateFolderSummary* pFolderSummary = nullptr) const;

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    CrateSummarySelectResult selectCrateSummaries() const; // all crates

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateSummaryById(CrateId id, CrateSummary* pCrateSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
