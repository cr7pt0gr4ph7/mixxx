#include "library/trackset/crate/cratefeature.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <algorithm>
#include <vector>

#include "analyzer/analyzerscheduledtrack.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/parser.h"
#include "library/parsercsv.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratefeaturehelper.h"
#include "library/trackset/crate/cratefoldersummary.h"
#include "library/trackset/crate/cratesummary.h"
#include "library/treeitem.h"
#include "moc_cratefeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

QString formatLabel(
        const CrateSummary& crateSummary) {
    return QStringLiteral("%1 (%2) %3")
            .arg(
                    crateSummary.getName(),
                    QString::number(crateSummary.getTrackCount()),
                    crateSummary.getTrackDurationText());
}

const ConfigKey kConfigKeyLastImportExportCrateDirectoryKey(
        "[Library]", "LastImportExportCrateDirectory");

const CrateFolderId kRootFolderId = CrateFolderId();

} // anonymous namespace

using namespace mixxx::library::prefs;

CrateFeature::CrateFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, "CRATEHOME", QStringLiteral("crates")),
          m_lockedCrateIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_crateTableModel(this, pLibrary->trackCollectionManager()),
          m_folderMenuInitialized(false) {
    initActions();

    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    rebuildChildModel();

    connectLibrary(pLibrary);
    connectTrackCollection();
}

void CrateFeature::initActions() {
    m_pCreateCrateAction = make_parented<QAction>(tr("Create New Crate"), this);
    connect(m_pCreateCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotCreateCrate);

    m_pCreateFolderAction = make_parented<QAction>(tr("Create New Folder"), this);
    connect(m_pCreateFolderAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotCreateFolder);

    m_pAddToNewFolderAction = make_parented<QAction>(tr("Add to New Folder"), this);
    connect(m_pAddToNewFolderAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotAddToNewFolder);

    m_pRenameCrateAction = make_parented<QAction>(tr("Rename"), this);
    m_pRenameCrateAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotRenameItem);
    m_pDuplicateCrateAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotDuplicateCrate);
    m_pDeleteCrateAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteCrateAction->setShortcut(removeKeySequence);
    connect(m_pDeleteCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotDeleteItem);
    m_pLockCrateAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotToggleCrateLock);

    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(),
            &QAction::changed,
            this,
            &CrateFeature::slotAutoDjTrackSourceChanged);

    m_pAnalyzeCrateAction = make_parented<QAction>(tr("Analyze entire Crate"), this);
    connect(m_pAnalyzeCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotAnalyzeCrate);

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotImportPlaylist);
    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotCreateImportCrate);
    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Crate as Playlist"), this);
    connect(m_pExportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotExportTrackFiles);
#ifdef __ENGINEPRIME__
    m_pExportAllCratesAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    connect(m_pExportAllCratesAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::exportAllCrates);
    m_pExportCrateAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    connect(m_pExportCrateAction.get(),
            &QAction::triggered,
            this,
            [this]() {
                CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex).toCrateId();
                if (crateId.isValid()) {
                    emit exportCrate(crateId);
                }
            });
#endif
}

void CrateFeature::connectLibrary(Library* pLibrary) {
    connect(pLibrary,
            &Library::trackSelected,
            this,
            [this](const TrackPointer& pTrack) {
                const auto trackId = pTrack ? pTrack->getId() : TrackId{};
                slotTrackSelected(trackId);
            });
    connect(pLibrary,
            &Library::switchToView,
            this,
            &CrateFeature::slotResetSelectedTrack);
}

void CrateFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new, duplicated or imported playlist to new crate
            &TrackCollection::crateInserted,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::crateUpdated,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateDeleted,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // crate tracks hidden, unhidden or purged
            &TrackCollection::crateTracksChanged,
            this,
            &CrateFeature::slotCrateContentChanged);
    connect(m_pTrackCollection, // created new folder
            &TrackCollection::crateFolderInserted,
            this,
            &CrateFeature::slotCrateFolderTableChanged);
    connect(m_pTrackCollection, // renamed folder, changed the list of children of a folder
            &TrackCollection::crateFolderUpdated,
            this,
            &CrateFeature::slotCrateFolderTableChanged);
    connect(m_pTrackCollection, // deleted folder
            &TrackCollection::crateFolderDeleted,
            this,
            &CrateFeature::slotCrateFolderTableChanged);
    connect(m_pTrackCollection, // crate tracks hidden, unhidden or purged
            &TrackCollection::crateTracksChanged,
            this,
            &CrateFeature::slotCrateContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateSummaryChanged,
            this,
            &CrateFeature::slotUpdateCrateLabels);
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QString CrateFeature::formatRootViewHtml() const {
    QString cratesTitle = tr("Crates");
    QString cratesSummary =
            tr("Crates are a great way to help organize the music you want to "
               "DJ with.");
    QString cratesSummary2 =
            tr("Make a crate for your next gig, for your favorite electrohouse "
               "tracks, or for your most requested tracks.");
    QString cratesSummary3 =
            tr("Crates let you organize your music however you'd like!");

    QString html;
    QString createCrateLink = tr("Create New Crate");
    html.append(QStringLiteral("<h2>%1</h2>").arg(cratesTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary3));
    //Colorize links in lighter blue, instead of QT default dark blue.
    //Links are still different from regular text, but readable on dark/light backgrounds.
    //https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createCrateLink));
    return html;
}

TreeItem* CrateFeature::getTreeItemForFolder(
        CrateFolderId folderId) {
    if (!folderId.isValid()) {
        // The null value represents the root item
        return m_pSidebarModel->getRootItem();
    }
    auto i = m_idToFolder.find(folderId);
    if (i != m_idToFolder.end()) {
        return i.value();
    } else {
        // We need to store a non-owning reference in the hash table.
        // The code makes sure to create exactly one owning std::unique_ptr
        // per tree item later on.
        auto* pTreeItem = newTreeItemForFolder(folderId).release();
        m_idToFolder.insert(folderId, pTreeItem);
        return pTreeItem;
    }
}

std::unique_ptr<TreeItem> CrateFeature::newTreeItemForFolder(
        CrateFolderId folderId) {
    auto pTreeItem = TreeItem::newRoot(this);
    pTreeItem->setData(CrateOrFolderId(folderId).toVariant());
    pTreeItem->setForceExpandable(true);
    // Label will be set later (due to the way we handle the recursive folder structure)
    return pTreeItem;
}

void CrateFeature::updateTreeItemForFolder(
        TreeItem* pTreeItem, const CrateFolder& folder) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(CrateOrFolderId(folder.getId()).toVariant());
    } else {
        // The data (= CrateId) is immutable once it has been set
        DEBUG_ASSERT(CrateOrFolderId(pTreeItem->getData()) == folder.getId());
    }
    // Update mutable properties
    pTreeItem->setLabel(folder.getName());
}

std::unique_ptr<TreeItem> CrateFeature::newTreeItemForCrateSummary(
        const CrateSummary& crateSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForCrateSummary(pTreeItem.get(), crateSummary);
    if (crateSummary.getId().isValid()) {
        m_idToCrate.insert(crateSummary.getId(), pTreeItem.get());
    }
    return pTreeItem;
}

void CrateFeature::updateTreeItemForCrateSummary(
        TreeItem* pTreeItem, const CrateSummary& crateSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(CrateOrFolderId(crateSummary.getId()).toVariant());
    } else {
        // The data (= CrateId) is immutable once it has been set
        DEBUG_ASSERT(CrateOrFolderId(pTreeItem->getData()).toCrateId() == crateSummary.getId());
    }
    // Update mutable properties
    pTreeItem->setLabel(formatLabel(crateSummary));
    pTreeItem->setIcon(crateSummary.isLocked() ? m_lockedCrateIcon : QIcon());
}

bool CrateFeature::dropAcceptChild(
        const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    CrateOrFolderId destinationId(crateIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(destinationId.isValid() && destinationId.isCrate()) {
        return false;
    }
    CrateId crateId(destinationId.toCrateId());
    // If a track is dropped onto a crate's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    // pSource != nullptr it is a drop from inside Mixxx and indicates all
    // tracks already in the DB
    QList<TrackId> trackIds =
            m_pLibrary->trackCollectionManager()->resolveTrackIdsFromUrls(urls, !pSource);
    if (trackIds.isEmpty()) {
        return false;
    }

    m_pTrackCollection->addCrateTracks(crateId, trackIds);
    return true;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
    CrateOrFolderId crateOrFolderId(crateIdFromIndex(index));
    if (!crateOrFolderId.isValid()) {
        return false;
    }
    if (crateOrFolderId.isFolder()) {
        return false;
    } else {
        CrateId crateId(crateOrFolderId.toCrateId());
        Crate crate;
        if (!m_pTrackCollection->crates().readCrateById(crateId, &crate) ||
                crate.isLocked()) {
            return false;
        }
        return SoundSourceProxy::isUrlSupported(url) ||
                Parser::isPlaylistFilenameSupported(url.toLocalFile());
    }
}

void CrateFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    m_pCreateCrateAction->setShortcut(
            QKeySequence(keyboard->getKeyboardConfig()->getValue(
                    ConfigKey("[KeyboardShortcuts]", "LibraryMenu_NewCrate"),
                    tr("Ctrl+Shift+N"))));

    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &CrateFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void CrateFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* CrateFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void CrateFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

void CrateFeature::activateChild(const QModelIndex& index) {
    qDebug() << "   CrateFeature::activateChild()" << index;
    activateItemImpl(crateIdFromIndex(index), index);
}

bool CrateFeature::selectAndActivateItem(CrateOrFolderId itemId) {
    qDebug() << "CrateFeature::activateItem()" << itemId;
    VERIFY_OR_DEBUG_ASSERT(itemId.isValid()) {
        return false;
    }
    if (itemId.isCrate() &&
            !m_pTrackCollection->crates().readCrateSummaryById(
                    itemId.toCrateId())) {
        // this may happen if called by slotCrateTableChanged()
        // and the crate has just been deleted
        return false;
    }
    if (itemId.isFolder() && !m_pTrackCollection->crates().readFolderById(itemId.toFolderId())) {
        // this may happen if called by slotCrateTableChanged()
        // and the folder has just been deleted
        return false;
    }
    if (!activateItemImpl(itemId, indexFromCrateId(itemId))) {
        return false;
    }
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
    return true;
}

bool CrateFeature::activateItemImpl(CrateOrFolderId itemId, const QModelIndex& index) {
    VERIFY_OR_DEBUG_ASSERT(itemId.isValid()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingItem = CrateId();
    emit saveModelState();
    if (itemId.isCrate()) {
        m_crateTableModel.selectCrate(itemId.toCrateId());
        emit showTrackModel(&m_crateTableModel);
    } else {
        emit switchToView(m_rootViewName);
        emit disableSearch();
    }
    emit enableCoverArtDisplay(true);
    return true;
}

CrateFeature::ItemType CrateFeature::readItemById(
        CrateOrFolderId itemId, Crate* pCrate, CrateFolder* pFolder) const {
    if (itemId.isCrate()) {
        CrateId crateId = itemId.toCrateId();
        VERIFY_OR_DEBUG_ASSERT(
                m_pTrackCollection->crates().readCrateById(crateId, pCrate)) {
            qWarning() << "Failed to read selected crate with id" << crateId;
            return ItemType::Invalid;
        }
        return ItemType::Crate;
    } else if (itemId.isFolder()) {
        CrateFolderId folderId = itemId.toFolderId();
        VERIFY_OR_DEBUG_ASSERT(
                m_pTrackCollection->crates().readFolderById(folderId, pFolder)) {
            qWarning() << "Failed to read selected folder with id" << folderId;
            return ItemType::Invalid;
        }
        return ItemType::Folder;
    } else {
        DEBUG_ASSERT(!itemId.isValid() || itemId.isCrate() || itemId.isFolder());
        qWarning() << "Failed to determine type of selected item";
        return ItemType::Invalid;
    }
}

CrateFeature::ItemType CrateFeature::readLastRightClickedItem(
        Crate* pCrate, CrateFolder* pFolder) const {
    CrateOrFolderId selectionId(getLastRightClickedItem());
    VERIFY_OR_DEBUG_ASSERT(selectionId.isValid()) {
        qWarning() << "Failed to determine id of selected item";
        return ItemType::Invalid;
    }
    return readItemById(selectionId, pCrate, pFolder);
}

bool CrateFeature::readLastRightClickedCrate(Crate* pCrate) const {
    CrateOrFolderId selectionId(getLastRightClickedItem());
    CrateId crateId = selectionId.toCrateId();
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        qWarning() << "Failed to determine id of selected crate";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            m_pTrackCollection->crates().readCrateById(crateId, pCrate)) {
        qWarning() << "Failed to read selected crate with id" << crateId;
        return false;
    }
    return true;
}

CrateFolderId CrateFeature::getLastRightClickedParentFolder() const {
    Crate crate;
    CrateFolder folder;
    switch (readLastRightClickedItem(&crate, &folder)) {
    case ItemType::Crate: {
        // Use folder of selected crate as the parent
        // (may be CrateFolderId() if the crate sits directly
        //  under the "Crates" root item)
        return crate.getFolderId();
    }
    case ItemType::Folder: {
        // Use selected folder as the parent
        return folder.getId();
    }
    case ItemType::Invalid:
    default: {
        // Use "Crates" root item as parent
        return kRootFolderId;
    }
    }
}

bool CrateFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addAction(m_pCreateFolderAction.get());
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction.get());
#ifdef __ENGINEPRIME__
    menu.addSeparator();
    menu.addAction(m_pExportAllCratesAction.get());
#endif
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    CrateOrFolderId selectionId(crateIdFromIndex(index));
    if (!selectionId.isValid()) {
        return;
    }

    Crate selectedCrate;
    CrateFolder selectedFolder;
    CrateFolderId parentFolderId;
    auto itemType = readItemById(selectionId, &selectedCrate, &selectedFolder);
    switch (itemType) {
    case ItemType::Crate: {
        parentFolderId = selectedCrate.getFolderId();
        break;
    }
    case ItemType::Folder: {
        parentFolderId = selectedFolder.getParentId();
        break;
    }
    case ItemType::Invalid:
    default: {
        return;
    }
    }

    QMenu menu(m_pSidebarWidget);

    QMenu* moveToFolderMenu = new QMenu(&menu);
    moveToFolderMenu->setTitle(tr("Move to Folder"));
    moveToFolderMenu->setObjectName("FolderMenu");

    m_folderMenuInitialized = false;
    connect(moveToFolderMenu,
            &QMenu::aboutToShow,
            this,
            [this, moveToFolderMenu, selectionId, parentFolderId] {
                if (m_folderMenuInitialized) {
                    return;
                }
                moveToFolderMenu->clear();

                auto addMoveToFolderAction =
                        [this, moveToFolderMenu, selectionId, parentFolderId](
                                const CrateFolderSummary& folder) {
                            if (selectionId.isFolder() &&
                                    (selectionId.toFolderId() == folder.getId() ||
                                            folder.isDescendantOf(selectionId.toFolderId()))) {
                                // A folder cannot be moved into its own descendants,
                                // because that would create a cycle in the hierarchy.
                                return;
                            }

                            auto* pAction = moveToFolderMenu->addAction(folder.getFullPath());
                            pAction->setProperty("folderId", folder.getId().toVariantOrNull());
                            pAction->setEnabled(parentFolderId != folder.getId());
                            connect(pAction,
                                    &QAction::triggered,
                                    this,
                                    [this, folderId{folder.getId()}] {
                                        slotMoveToFolder(folderId);
                                    });
                        };

                CrateFolderSummary root;
                const QString rootTitle = tr("(Root)", "Move to Folder menu");
                root.setId(kRootFolderId);
                root.setName(rootTitle);
                root.setFullPath(rootTitle);
                addMoveToFolderAction(root);

                CrateFolderSummary folder;
                auto folders = m_pTrackCollection->crates().selectFolderSummaries();
                while (folders.populateNext(&folder)) {
                    addMoveToFolderAction(folder);
                }

                moveToFolderMenu->addSeparator();
                moveToFolderMenu->addAction(m_pAddToNewFolderAction);
                m_folderMenuInitialized = true;
            });

    if (selectionId.isCrate()) {
        m_pDeleteCrateAction->setEnabled(!selectedCrate.isLocked());
        m_pRenameCrateAction->setEnabled(!selectedCrate.isLocked());
        m_pAutoDjTrackSourceAction->setChecked(selectedCrate.isAutoDjSource());
        m_pLockCrateAction->setText(selectedCrate.isLocked() ? tr("Unlock") : tr("Lock"));

        menu.addAction(m_pCreateCrateAction.get());
        menu.addAction(m_pCreateFolderAction.get());
        menu.addSeparator();
        menu.addAction(m_pRenameCrateAction.get());
        menu.addAction(m_pDuplicateCrateAction.get());
        menu.addAction(m_pDeleteCrateAction.get());
        menu.addAction(m_pLockCrateAction.get());
        menu.addSeparator();
        menu.addAction(m_pAutoDjTrackSourceAction.get());
        menu.addSeparator();
        menu.addMenu(moveToFolderMenu);
        menu.addSeparator();
        menu.addAction(m_pAnalyzeCrateAction.get());
        menu.addSeparator();
        if (!selectedCrate.isLocked()) {
            menu.addAction(m_pImportPlaylistAction.get());
        }
        menu.addAction(m_pExportPlaylistAction.get());
        menu.addAction(m_pExportTrackFilesAction.get());
#ifdef __ENGINEPRIME__
        menu.addAction(m_pExportCrateAction.get());
#endif
    } else if (selectionId.isFolder()) {
        m_pDeleteCrateAction->setEnabled(true);
        m_pRenameCrateAction->setEnabled(true);

        menu.addAction(m_pCreateCrateAction.get());
        menu.addAction(m_pCreateFolderAction.get());
        menu.addSeparator();
        menu.addAction(m_pRenameCrateAction.get());
        menu.addAction(m_pDeleteCrateAction.get());
        menu.addSeparator();
        menu.addMenu(moveToFolderMenu);
    } else {
        return;
    }

    menu.exec(globalPos);
    m_folderMenuInitialized = false;
}

void CrateFeature::slotCreateCrateLink() {
    CrateFolderId selectedFolderId = kRootFolderId;
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        CrateOrFolderId itemId = crateIdFromIndex(m_lastClickedIndex);
        if (itemId.isFolder()) {
            selectedFolderId = itemId.toFolderId();
        }
    }
    createNewCrate(selectedFolderId, true);
}

void CrateFeature::slotCreateCrate() {
    createNewCrate(getLastRightClickedParentFolder(), true);
}

void CrateFeature::slotCreateFolder() {
    createNewFolder(getLastRightClickedParentFolder(), false);
}

void CrateFeature::createNewCrate(CrateFolderId parent, bool selectAfterCreation) {
    // Note: An "invalid"/NULL parent is not actually invalid
    //       for this function, but instead represents the root folder.
    CrateId crateId =
            CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyCrate(parent);

    if (selectAfterCreation && crateId.isValid()) {
        // expand Crates and scroll to new crate
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(crateId), false);
    }
}

void CrateFeature::createNewFolder(CrateFolderId parent, bool selectAfterCreation) {
    // Note: An "invalid"/NULL parent is not actually invalid
    //       for this function, but instead represents the root folder.
    CrateFolderId folderId =
            CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyFolder(parent);

    if (selectAfterCreation && folderId.isValid()) {
        // expand Crates and scroll to new folder
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(folderId), false);
    }
}

void CrateFeature::slotMoveToFolder(CrateFolderId destinationId) {
    // Note: An "invalid"/NULL destination is not actually invalid
    //       for this function, but instead represents the root folder.
    CrateOrFolderId itemToMove = getLastRightClickedItem();
    moveToFolder(destinationId, itemToMove, true);
}

bool CrateFeature::moveToFolder(CrateFolderId destinationId,
        CrateOrFolderId itemToMoveId,
        bool selectAfterMove) {
    // Note: An "invalid"/NULL destination is not actually invalid
    //       for this function, but instead represents the root folder.
    const bool moved = moveToFolder(destinationId, itemToMoveId);
    if (moved && selectAfterMove && itemToMoveId.isValid()) {
        // Scroll to new location of the selected crate/folder
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(itemToMoveId), false);
    }
    return moved;
}

bool CrateFeature::moveToFolder(CrateFolderId destinationId,
        const QList<CrateOrFolderId>& itemsToMove) {
    // Note: An "invalid"/NULL destination is not actually invalid
    //       for this function, but instead represents the root folder.
    bool success = false;
    for (CrateOrFolderId itemToMoveId : itemsToMove) {
        success |= moveToFolder(destinationId, itemToMoveId);
    }
    return success;
}

bool CrateFeature::moveToFolder(CrateFolderId destinationId, CrateOrFolderId itemToMoveId) {
    // Note: An "invalid"/NULL destination is not actually invalid
    //       for this function, but instead represents the root folder.
    Crate crate;
    CrateFolder folder;
    switch (readItemById(itemToMoveId, &crate, &folder)) {
    case ItemType::Crate: {
        crate.setFolderId(destinationId);
        return m_pTrackCollection->updateCrate(crate);
    }
    case ItemType::Folder: {
        folder.setParentId(destinationId);
        return m_pTrackCollection->updateCrateFolder(folder);
    }
    case ItemType::Invalid:
    default: {
        return false;
    }
    }
}

void CrateFeature::slotAddToNewFolder() {
    Crate crate;
    CrateFolder folder;
    auto itemType = readLastRightClickedItem(&crate, &folder);
    VERIFY_OR_DEBUG_ASSERT(itemType == ItemType::Crate || itemType == ItemType::Folder) {
        return;
    }

    // Note: The new folder is always created at the root
    //       when using "Add to New Folder". This differs
    //       from the behavior when using "Create New Folder".
    CrateFolderId newFolderId =
            CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyFolder(kRootFolderId);

    if (!newFolderId.isValid()) {
        return;
    }

    CrateOrFolderId idToSelect;
    if (itemType == ItemType::Crate) {
        idToSelect = crate.getId();
        crate.setFolderId(newFolderId);
        m_pTrackCollection->updateCrate(crate);
    } else if (itemType == ItemType::Folder) {
        idToSelect = folder.getId();
        folder.setParentId(newFolderId);
        m_pTrackCollection->updateCrateFolder(folder);
    } else {
        return;
    }

    // Scroll to new location of the selected crate/folder
    if (idToSelect.isValid()) {
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(idToSelect), false);
    }
}

void CrateFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteItem();
}

void CrateFeature::slotDeleteItem() {
    Crate crate;
    CrateFolder folder;
    switch (readLastRightClickedItem(&crate, &folder)) {
    case ItemType::Crate: {
        if (crate.isLocked()) {
            qWarning() << "Refusing to delete locked crate" << crate;
            return;
        }
        CrateId crateId = crate.getId();
        // Store sibling id to restore selection after crate was deleted
        // to avoid the scroll position being reset to Crate root item.
        m_prevSiblingItem = CrateId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingItemId(crateId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete crate <b>%1</b>?")
                        .arg(crate.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes && m_pTrackCollection->deleteCrate(crateId)) {
            qDebug() << "Deleted crate" << crate;
        }
        break;
    }
    case ItemType::Folder: {
        // if (m_pTrackCollection->crates().hasLockedCrates(crate.getId())) {
        //     qWarning() << "Refusing to delete folder" << folder << "containing locked crates";
        //     return;
        // }
        CrateFolderId folderId = folder.getId();
        // Store sibling id to restore selection after crate was deleted
        // to avoid the scroll position being reset to Crate root item.
        m_prevSiblingItem = CrateOrFolderId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingItemId(folderId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete the folder <b>%1</b>?")
                        .arg(crate.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes && m_pTrackCollection->deleteCrateFolder(folderId)) {
            qDebug() << "Deleted crate folder" << crate;
        }
        break;
    }
    case ItemType::Invalid:
    default: {
        qWarning() << "Failed to delete selected item";
        break;
    }
    }
}

void CrateFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameItem();
}

void CrateFeature::slotRenameItem() {
    Crate crate;
    CrateFolder folder;
    switch (readLastRightClickedItem(&crate, &folder)) {
    case ItemType::Crate: {
            const QString oldName = crate.getName();
            crate.resetName();
            for (;;) {
                bool ok = false;
                auto newName =
                        QInputDialog::getText(nullptr,
                                tr("Rename Crate"),
                                tr("Enter new name for crate:"),
                                QLineEdit::Normal,
                                oldName,
                                &ok)
                                .trimmed();
                if (!ok || newName.isEmpty()) {
                    return;
                }
                if (newName.isEmpty()) {
                    QMessageBox::warning(nullptr,
                            tr("Renaming Crate Failed"),
                            tr("A crate cannot have a blank name."));
                    continue;
                }
                if (m_pTrackCollection->crates().readCrateByName(crate.getFolderId(), newName)) {
                    QMessageBox::warning(nullptr,
                            tr("Renaming Crate Failed"),
                            tr("A crate by that name already exists."));
                    continue;
                }
                crate.setName(std::move(newName));
                DEBUG_ASSERT(crate.hasName());
                break;
            }

            if (!m_pTrackCollection->updateCrate(crate)) {
                qDebug() << "Failed to rename crate" << crate;
            }
            break;
    }
    case ItemType::Folder: {
            const QString oldName = folder.getName();
            folder.resetName();
            for (;;) {
                bool ok = false;
                auto newName =
                        QInputDialog::getText(nullptr,
                                tr("Rename Folder"),
                                tr("Enter new name for folder:"),
                                QLineEdit::Normal,
                                oldName,
                                &ok)
                                .trimmed();
                if (!ok || newName.isEmpty()) {
                    return;
                }
                if (newName.isEmpty()) {
                    QMessageBox::warning(nullptr,
                            tr("Renaming Folder Failed"),
                            tr("A folder cannot have a blank name."));
                    continue;
                }
                if (m_pTrackCollection->crates().readFolderByName(folder.getParentId(), newName)) {
                    QMessageBox::warning(nullptr,
                            tr("Renaming Folder Failed"),
                            tr("A folder with that name already exists."));
                    continue;
                }
                folder.setName(std::move(newName));
                DEBUG_ASSERT(folder.hasName());
                break;
            }

            if (!m_pTrackCollection->updateCrateFolder(folder)) {
                qDebug() << "Failed to rename folder" << folder;
            }
            break;
    }
    case ItemType::Invalid:
    default: {
            qDebug() << "Failed to rename selected item";
            break;
    }
    }
}

void CrateFeature::slotDuplicateCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        CrateId newCrateId =
                CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                        .duplicateCrate(crate);
        if (newCrateId.isValid()) {
            qDebug() << "Duplicate crate" << crate << ", new crate:" << newCrateId;
            return;
        }
    }
    qDebug() << "Failed to duplicate selected crate";
}

void CrateFeature::slotToggleCrateLock() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        crate.setLocked(!crate.isLocked());
        if (!m_pTrackCollection->updateCrate(crate)) {
            qDebug() << "Failed to toggle lock of crate" << crate;
        }
    } else {
        qDebug() << "Failed to toggle lock of selected crate";
    }
}

void CrateFeature::slotAutoDjTrackSourceChanged() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isAutoDjSource() != m_pAutoDjTrackSourceAction->isChecked()) {
            crate.setAutoDjSource(m_pAutoDjTrackSourceAction->isChecked());
            m_pTrackCollection->updateCrate(crate);
        }
    }
}

QModelIndex CrateFeature::rebuildChildModel(CrateOrFolderId selectedItemId) {
    qDebug() << "CrateFeature::rebuildChildModel()" << selectedItemId;

    m_lastRightClickedIndex = QModelIndex();
    QModelIndex selectedIndex = QModelIndex();

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return selectedIndex;
    }

    // Remove all existing tree items except for the root
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    // Create the nested folder structure
    m_idToCrate.clear();
    m_idToFolder.clear();

    CrateFolderSelectResult folders(
            m_pTrackCollection->crates().selectFolders());
    CrateFolder folder;
    while (folders.populateNext(&folder)) {
        auto* pThisItem = getTreeItemForFolder(folder.getId());
        auto* pParentItem = getTreeItemForFolder(folder.getParentId());
        updateTreeItemForFolder(pThisItem, folder);
        pParentItem->insertChild(pParentItem->childRows(), std::unique_ptr<TreeItem>(pThisItem));
        if (selectedItemId.isFolder() && selectedItemId.toFolderId() == folder.getId()) {
            // save index for selection
            selectedIndex = m_pSidebarModel->index(pThisItem);
        }
    }

    std::vector<std::unique_ptr<TreeItem>> modelRows;
    modelRows.reserve(m_pTrackCollection->crates().countCrates());

    CrateSummarySelectResult crateSummaries(
            m_pTrackCollection->crates().selectCrateSummaries());
    CrateSummary crateSummary;
    while (crateSummaries.populateNext(&crateSummary)) {
        auto pThisItem = newTreeItemForCrateSummary(crateSummary);
        auto pParentItem = getTreeItemForFolder(crateSummary.getFolderId());
        auto pThisItemPtr = pThisItem.get();
        pParentItem->insertChild(pParentItem->childRows(), std::move(pThisItem));

        if (selectedItemId.isCrate() && selectedItemId.toCrateId() == crateSummary.getId()) {
            // save index for selection
            selectedIndex = m_pSidebarModel->index(pThisItemPtr);
        }
    }

    // Update rendering of crates depending on the currently selected track
    slotTrackSelected(m_selectedTrackId);

    return selectedIndex;
}

void CrateFeature::updateChildModel(const QSet<CrateId>& updatedCrateIds) {
    const CrateStorage& crateStorage = m_pTrackCollection->crates();
    for (const CrateId& crateId : updatedCrateIds) {
        QModelIndex index = indexFromCrateId(crateId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        CrateSummary crateSummary;
        VERIFY_OR_DEBUG_ASSERT(
                crateStorage.readCrateSummaryById(crateId, &crateSummary)) {
            continue;
        }
        updateTreeItemForCrateSummary(
                m_pSidebarModel->getItem(index), crateSummary);
        m_pSidebarModel->triggerRepaint(index);
    }

    if (m_selectedTrackId.isValid()) {
        // Crates containing the currently selected track might
        // have been modified.
        slotTrackSelected(m_selectedTrackId);
    }
}

CrateOrFolderId CrateFeature::crateIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return CrateOrFolderId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return CrateOrFolderId();
    }
    return CrateOrFolderId(item->getData());
}

QModelIndex CrateFeature::indexFromCrateId(CrateOrFolderId itemId) const {
    VERIFY_OR_DEBUG_ASSERT(itemId.isValid()) {
        return QModelIndex();
    }
    if (itemId.isCrate()) {
        CrateId crateId = itemId.toCrateId();
        auto i = m_idToCrate.find(crateId);
        if (i == m_idToCrate.end()) {
            qDebug() << "Tree item for crate not found:" << crateId;
            return QModelIndex();
        }
        return m_pSidebarModel->index(i.value());
    } else if (itemId.isFolder()) {
        CrateFolderId folderId = itemId.toFolderId();
        auto i = m_idToFolder.find(folderId);
        if (i == m_idToFolder.end()) {
            qDebug() << "Tree item for folder not found:" << folderId;
            return QModelIndex();
        }
        return m_pSidebarModel->index(i.value());
    } else {
        return QModelIndex();
    }
}

void CrateFeature::slotImportPlaylist() {
    //qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }

    // Update the import/export crate directory
    QString fileDirectory(playlistFile);
    fileDirectory.truncate(playlistFile.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    CrateOrFolderId selectionId = crateIdFromIndex(m_lastRightClickedIndex);
    CrateId crateId = selectionId.toCrateId();
    Crate crate;
    if (crateId.isValid() && m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Importing playlist file" << playlistFile << "into crate"
                 << crateId << crate;
    } else {
        qDebug() << "Importing playlist file" << playlistFile << "into crate"
                 << crateId << crate << "failed!";
        return;
    }

    slotImportPlaylistFile(playlistFile, crateId);
    activateChild(m_lastRightClickedIndex);
}

void CrateFeature::slotImportPlaylistFile(const QString& playlistFile, CrateId crateId) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.
    QList<QString> locations = Parser().parse(playlistFile);
    if (locations.empty()) {
        return;
    }

    if (crateId == m_crateTableModel.selectedCrate()) {
        // Add tracks directly to the model
        m_crateTableModel.addTracks(QModelIndex(), locations);
    } else {
        // Create a temporary table model since the main one might have another
        // crate selected which is not the crate that received the right-click.
        std::unique_ptr<CrateTableModel> pCrateTableModel =
                std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
        pCrateTableModel->selectCrate(crateId);
        pCrateTableModel->select();
        pCrateTableModel->addTracks(QModelIndex(), locations);
    }
}

void CrateFeature::slotCreateImportCrate() {
    // Get file to read
    QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QString fileDirectory(playlistFiles.first());
    fileDirectory.truncate(playlistFiles.first().lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    // Set crate folder where the imported crates should be created
    CrateFolderId importedIntoFolder = getLastRightClickedParentFolder();

    CrateId lastCrateId;

    // For each selected file create a new crate
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);

        Crate crate;
        crate.setFolderId(importedIntoFolder);

        // Get a valid name
        const QString baseName = fileInfo.baseName();
        for (int i = 0;; ++i) {
            auto name = baseName;
            if (i > 0) {
                name += QStringLiteral(" %1").arg(i);
            }
            name = name.trimmed();
            if (!name.isEmpty()) {
                if (!m_pTrackCollection->crates().readCrateByName(importedIntoFolder, name)) {
                    // unused crate name found
                    crate.setName(std::move(name));
                    DEBUG_ASSERT(crate.hasName());
                    break; // terminate loop
                }
            }
        }

        if (!m_pTrackCollection->insertCrate(crate, &lastCrateId)) {
            QMessageBox::warning(nullptr,
                    tr("Crate Creation Failed"),
                    tr("An unknown error occurred while creating crate: ") +
                            crate.getName());
            return;
        }

        slotImportPlaylistFile(playlistFile, lastCrateId);
    }
    selectAndActivateItem(lastCrateId);
}

void CrateFeature::slotAnalyzeCrate() {
    if (m_lastRightClickedIndex.isValid()) {
        CrateOrFolderId selectionId = crateIdFromIndex(m_lastRightClickedIndex);
        if (selectionId.isCrate()) {
            CrateId crateId = selectionId.toCrateId();
            if (crateId.isValid()) {
                QList<AnalyzerScheduledTrack> tracks;
                tracks.reserve(
                        m_pTrackCollection->crates().countCrateTracks(crateId));
                {
                    CrateTrackSelectResult crateTracks(
                            m_pTrackCollection->crates().selectCrateTracksSorted(
                                    crateId));
                    while (crateTracks.next()) {
                        tracks.append(crateTracks.trackId());
                    }
                }
                emit analyzeTracks(tracks);
            }
        }
    }
}

void CrateFeature::slotExportPlaylist() {
    CrateOrFolderId selectionId = crateIdFromIndex(m_lastRightClickedIndex);
    if (!selectionId.isCrate()) {
        return;
    }
    CrateId crateId = selectionId.toCrateId();
    Crate crate;
    if (crateId.isValid() && m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Exporting crate" << crateId << crate;
    } else {
        qDebug() << "Failed to export crate" << crateId;
        return;
    }

    QString lastCrateDirectory = m_pConfig->getValue(
            kConfigKeyLastImportExportCrateDirectoryKey,
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for crate export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    const QString fileLocation = getFilePathWithVerifiedExtensionFromFileDialog(
            tr("Export Crate"),
            lastCrateDirectory.append("/").append(crate.getName()),
            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;PLS Playlist "
               "(*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
            tr("M3U Playlist (*.m3u)"));
    // Exit method if user cancelled the open dialog.
    if (fileLocation.isEmpty()) {
        return;
    }
    // Update the import/export crate directory
    QString fileDirectory(fileLocation);
    fileDirectory.truncate(fileLocation.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // check config if relative paths are desired
    bool useRelativePath =
            m_pConfig->getValue<bool>(
                    kUseRelativePathOnExportConfigKey);

    // Create list of files of the crate
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<CrateTableModel> pCrateTableModel =
            std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
    pCrateTableModel->selectCrate(crateId);
    pCrateTableModel->select();

    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(fileLocation, pCrateTableModel.get(), useRelativePath);
    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
        ParserCsv::writeReadableTextFile(fileLocation, pCrateTableModel.get(), false);
    } else {
        // populate a list of files of the crate
        QList<QString> playlistItems;
        int rows = pCrateTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pCrateTableModel->index(i, 0);
            playlistItems << pCrateTableModel->getTrackLocation(index);
        }
        exportPlaylistItemsIntoFile(
                fileLocation,
                playlistItems,
                useRelativePath);
    }
}

void CrateFeature::slotExportTrackFiles() {
    CrateOrFolderId itemId(crateIdFromIndex(m_lastRightClickedIndex));
    if (!itemId.isCrate()) {
        return;
    }
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<CrateTableModel> pCrateTableModel =
            std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
    pCrateTableModel->selectCrate(itemId.toCrateId());
    pCrateTableModel->select();

    int rows = pCrateTableModel->rowCount();
    TrackPointerList trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = m_crateTableModel.index(i, 0);
        trackpointers.push_back(m_crateTableModel.getTrack(index));
    }

    TrackExportWizard track_export(nullptr, m_pConfig, trackpointers);
    track_export.exportTracks();
}

void CrateFeature::storePrevSiblingItemId(CrateOrFolderId itemId) {
    QModelIndex actIndex = indexFromCrateId(itemId);
    m_prevSiblingItem = CrateId();
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (pTreeItem != m_pSidebarModel->getRootItem()) {
                m_prevSiblingItem = crateIdFromIndex(newIndex);
            }
        }
    }
}

void CrateFeature::slotCrateFolderTableChanged(CrateFolderId folderId) {
    slotCrateOrFolderTableChanged(folderId);
}

void CrateFeature::slotCrateTableChanged(CrateId crateId) {
    slotCrateOrFolderTableChanged(crateId);
}

void CrateFeature::slotCrateOrFolderTableChanged(CrateOrFolderId itemId) {
    Q_UNUSED(itemId);
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        // If the previously selected crate was loaded to the tracks table and
        // selected in the sidebar try to activate that or a sibling
        rebuildChildModel();
        if (!selectAndActivateItem(m_crateTableModel.selectedCrate())) {
            // probably last clicked crate was deleted, try to
            // select the stored sibling
            if (m_prevSiblingItem.isValid()) {
                selectAndActivateItem(m_prevSiblingItem);
            }
        }
    } else {
        // No valid selection to restore
        rebuildChildModel();
    }
}

void CrateFeature::slotCrateContentChanged(CrateId crateId) {
    QSet<CrateId> updatedCrateIds;
    updatedCrateIds.insert(crateId);
    updateChildModel(updatedCrateIds);
}

void CrateFeature::slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds) {
    updateChildModel(updatedCrateIds);
}

void CrateFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateCrateLink();
    } else {
        qDebug() << "Unknown crate link clicked" << link;
    }
}

void CrateFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    std::vector<CrateId> sortedTrackCrates;
    if (m_selectedTrackId.isValid()) {
        CrateTrackSelectResult trackCratesIter(
                m_pTrackCollection->crates().selectTrackCratesSorted(m_selectedTrackId));
        while (trackCratesIter.next()) {
            sortedTrackCrates.push_back(trackCratesIter.crateId());
        }
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem : m_idToCrate) {
        DEBUG_ASSERT(pTreeItem != nullptr);
        CrateOrFolderId itemId(pTreeItem->getData());
        VERIFY_OR_DEBUG_ASSERT(itemId.isCrate()) {
            continue;
        }
        bool crateContainsSelectedTrack =
                m_selectedTrackId.isValid() &&
                std::binary_search(
                        sortedTrackCrates.begin(),
                        sortedTrackCrates.end(),
                        itemId.toCrateId());
        pTreeItem->setBold(crateContainsSelectedTrack);
    }

    m_pSidebarModel->triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}
