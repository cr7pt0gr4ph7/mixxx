#pragma once

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/cratefolder.h"
#include "library/trackset/crate/crateorfolderid.h"
#include "library/trackset/crate/cratetablemodel.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class WLibrarySidebar;
class QAction;
class QPoint;
class CrateSummary;

class CrateFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    CrateFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    ~CrateFeature() override = default;

    QVariant title() override;

    bool navigateTo(const QUrl& url) override;

    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAccept(const QUrl& url) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateCrate();
    void slotCreateFolder();
    void deleteItem(const QModelIndex& index) override;
    void renameItem(const QModelIndex& index) override;

#ifdef __ENGINEPRIME__
  signals:
    void exportAllCrates();
    void exportCrate(CrateId crateId);
#endif

  private slots:
    void slotCreateCrateLink();
    void slotDeleteItem();
    void slotRenameItem();
    void slotDuplicateCrate();
    void slotAddToNewFolder();
    void slotMoveToFolder(CrateFolderId destinationId);
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString& playlistFile, CrateId crateId);
    void slotCreateImportCrate();
    void slotExportPlaylist();
    // Copy all of the tracks in a crate to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeCrate();
    void slotCrateOrFolderTableChanged(CrateOrFolderId itemId);
    void slotCrateFolderTableChanged(CrateFolderId folderId);
    void slotCrateTableChanged(CrateId crateId);
    void slotCrateContentChanged(CrateId crateId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();
    void slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds);

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectTrackCollection();

    // Navigation handling
    bool selectAndActivateItem(CrateOrFolderId itemId);
    bool activateItemImpl(CrateOrFolderId itemId, const QModelIndex& index);

    // TreeItem construction
    TreeItem* getTreeItemForFolder(CrateFolderId folderId);
    std::unique_ptr<TreeItem> newTreeItemForFolder(
            CrateFolderId folderId);
    void updateTreeItemForFolder(
            TreeItem* pTreeItem,
            const CrateFolder& folder) const;

    std::unique_ptr<TreeItem> newTreeItemForCrateSummary(
            const CrateSummary& crateSummary);
    void updateTreeItemForCrateSummary(
            TreeItem* pTreeItem,
            const CrateSummary& crateSummary) const;

    QModelIndex rebuildChildModel(CrateOrFolderId selectedCrateId = CrateOrFolderId());
    void updateChildModel(const QSet<CrateId>& updatedCrateIds);

    // TreeItem mapping
    CrateOrFolderId crateIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromCrateId(CrateOrFolderId crateId) const;

    bool isChildIndexSelectedInSidebar(const QModelIndex& index);

    typedef enum CrateOrFolderId::ItemType ItemType;

    ItemType readItemById(CrateOrFolderId itemId, Crate* pCrate, CrateFolder* pFolder) const;
    ItemType readLastRightClickedItem(Crate* pCrate, CrateFolder* pFolder) const;
    bool readLastRightClickedCrate(Crate* pCrate) const;
    CrateFolderId getLastRightClickedParentFolder() const;
    CrateOrFolderId getLastRightClickedItem() const {
        return crateIdFromIndex(m_lastRightClickedIndex);
    }

    // TreeItem actions
    void createNewCrate(CrateFolderId parent, bool selectAfterCreation);
    void createNewFolder(CrateFolderId parent, bool selectAfterCreation);
    bool moveToFolder(CrateFolderId destinationId, CrateOrFolderId itemToMoveId);
    bool moveToFolder(CrateFolderId destinationId,
            CrateOrFolderId itemToMoveId,
            bool selectAfterMove);
    bool moveToFolder(CrateFolderId destinationId, const QList<CrateOrFolderId>& itemsToMove);

    QString formatRootViewHtml() const;

    const QIcon m_lockedCrateIcon;

    TrackCollection* const m_pTrackCollection;

    CrateTableModel m_crateTableModel;

    QHash<CrateFolderId, TreeItem*> m_idToFolder;
    QHash<CrateId, TreeItem*> m_idToCrate;

    // Stores the id of a crate/folder in the sidebar that is adjacent to the crate/folder(itemId).
    void storePrevSiblingItemId(CrateOrFolderId itemId);
    // Can be used to restore a similar selection after the sidebar model was rebuilt.
    CrateOrFolderId m_prevSiblingItem;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    TrackId m_selectedTrackId;

    bool m_folderMenuInitialized;

    parented_ptr<QAction> m_pCreateCrateAction;
    parented_ptr<QAction> m_pCreateFolderAction;
    parented_ptr<QAction> m_pAddToNewFolderAction;
    parented_ptr<QAction> m_pDeleteCrateAction;
    parented_ptr<QAction> m_pRenameCrateAction;
    parented_ptr<QAction> m_pLockCrateAction;
    parented_ptr<QAction> m_pDuplicateCrateAction;
    parented_ptr<QAction> m_pAutoDjTrackSourceAction;
    parented_ptr<QAction> m_pImportPlaylistAction;
    parented_ptr<QAction> m_pCreateImportPlaylistAction;
    parented_ptr<QAction> m_pExportPlaylistAction;
    parented_ptr<QAction> m_pExportTrackFilesAction;
#ifdef __ENGINEPRIME__
    parented_ptr<QAction> m_pExportAllCratesAction;
    parented_ptr<QAction> m_pExportCrateAction;
#endif
    parented_ptr<QAction> m_pAnalyzeCrateAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
