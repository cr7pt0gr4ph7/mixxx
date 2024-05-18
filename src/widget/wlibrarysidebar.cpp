#include "widget/wlibrarysidebar.h"

#include <QHeaderView>
#include <QUrl>
#include <QtDebug>

#include "library/sidebarmodel.h"
#include "moc_wlibrarysidebar.cpp"
#include "util/defs.h"
#include "util/dnd.h"

constexpr int expand_time = 250;

static constexpr int LongHoverActivationTimeMs = 2000;
static constexpr int LongHoverExpirationTimeMs = 100;
static constexpr int LongHoverMaxDistanceManhattan = 10;

WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
    //Set some properties
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    //Drag and drop setup
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setAutoScroll(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void WLibrarySidebar::contextMenuEvent(QContextMenuEvent *event) {
    //if (event->state() & Qt::RightButton) { //Dis shiz don werk on windowze
    QModelIndex clickedIndex = indexAt(event->pos());
    if (!clickedIndex.isValid()) {
        return;
    }
    // Use this instead of setCurrentIndex() to keep current selection
    selectionModel()->setCurrentIndex(clickedIndex, QItemSelectionModel::NoUpdate);
    event->accept();
    emit rightClicked(event->globalPos(), clickedIndex);
    //}
}

// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent * event) {
    qDebug() << "WLibrarySidebar::dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls()) {
        // We don't have a way to ask the LibraryFeatures whether to accept a
        // drag so for now we accept all drags. Since almost every
        // LibraryFeature accepts all files in the drop and accepts playlist
        // drops we default to those flags to DragAndDropHelper.
        QList<mixxx::FileInfo> fileInfos = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), false, true);
        if (!fileInfos.isEmpty()) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
    //QTreeView::dragEnterEvent(event);
}

// Drag move event, happens when a dragged item hovers over the track sources view...
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent * event) {
    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    // Start a timer to auto-expand sections the user hovers on.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->position().toPoint();
#else
    QPoint pos = event->pos();
#endif
    QModelIndex index = indexAt(pos);
    if (m_hoverIndex != index) {
        m_expandTimer.stop();
        m_hoverIndex = index;
        m_expandTimer.start(expand_time, this);
    }

    // --------------------------------------------------------------------
    //
    // The mouse cursor has moved to a new item, either
    // intentionally or unintentionally.
    //
    // The latter may happen e.g. with laptop touchpads when a
    // double-tap-and-hold gesture was used to initiate the drag:
    //
    // Releasing the finger from the trackpad always causes an
    // unintentional movement of at least a few pixels, often causing
    // the mouse cursor to move away from the intended drop target
    // when the latter is very small.
    //
    // --------------------------------------------------------------------
    //
    // We try to detect & repair such unintentional movements using
    // the following heuristic to detect the user's intent:
    //
    // 1) The cursor was held over a drop target for at least
    //    ActivationTimeMs while the DnD was in progress, and:
    //
    // 2) At the time of dropAccept, the cursor is over a different
    //    item, and:
    //
    // 3) The time since last hovering over the 'intended' drop
    //    target from step (1) is less than ExpirationTimeMs.
    //
    //    ExpirationTimeMs should be just enough to cover the time
    //    between beginning to release and fully releasing the
    //    finger from the touchpad.
    //
    // 4) The distance between the current cursor position and the last
    //    position that was over the 'intended' drop target is less
    //    than MaxDistancePx.
    //
    //    Large mouse movements are very likely intentional, so this
    //    is a precaution to avoid interfering with the user.
    //

    // Mouse has moved to a new item:
    // - Store the last position where the mouse cursor was over the previous item,
    //   as well as the previous item itself.
    // - Start the expiration timer on the previous item.
    //   The heuristic only kicks in when the item is dropped before
    //   the expiration timer expires.
    // - Start the activation timer on the new item.
    //   It will become the "new" intended item when the activation
    //   timer expires before the cursor leaves the item.
    if (m_longHoverNew.modelIndex != index) {
        // The new "old" / the old "new" item is the "intended" item
        // because its activation timer elapsed. Start the expiration timer
        if (m_longHoverNew.isReady) {
            m_longHoverOld.modelIndex = m_longHoverNew.modelIndex;
            m_longHoverOld.position = m_longHoverNew.position;
            m_longHoverOld.isValid = m_longHoverNew.isValid;
            m_longHoverOld.isReady = m_longHoverNew.isReady;
        } else {
            m_longHoverOld.modelIndex = QModelIndex();
            m_longHoverOld.position = QPoint();
            m_longHoverOld.isValid = false;
            m_longHoverOld.isReady = false;
        }

        // Wait for the new item to become active
        m_longHoverNew.modelIndex = index;
        m_longHoverNew.position = pos;
        m_longHoverNew.isValid = true;
        m_longHoverNew.isReady = false; // only becomes true once activationTimer expires
        m_longHoverNew.activationTimer.start(LongHoverActivationTimeMs, this);

        if (m_longHoverOld.isValid && m_longHoverOld.isReady) {
            m_longHoverOld.expirationTimer.start(LongHoverExpirationTimeMs, this);
        }
    } else {
        // Continuously update the "last position"
        m_longHoverNew.position = pos;
    }

    // This has to be here instead of after, otherwise all drags will be
    // rejected -- rryan 3/2011
    QTreeView::dragMoveEvent(event);
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        // Drag and drop within this widget
        if ((event->source() == this)
                && (event->possibleActions() & Qt::MoveAction)) {
            // Do nothing.
            event->ignore();
        } else {
            SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
            bool accepted = true;
            if (sidebarModel) {
                accepted = false;
                for (const QUrl& url : urls) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    QPoint pos = event->position().toPoint();
#else
                    QPoint pos = event->pos();
#endif
                    QModelIndex destIndex = indexAt(pos);
                    if (sidebarModel->dragMoveAccept(destIndex, url)) {
                        // We only need one URL to be valid for us
                        // to accept the whole drag...
                        // consider we have a long list of valid files, checking all will
                        // take a lot of time that stales Mixxx and this makes the drop feature useless
                        // Eg. you may have tried to drag two MP3's and an EXE, the drop is accepted here,
                        // but the EXE is sorted out later after dropping
                        accepted = true;
                        break;
                    }
                }
            }
            if (accepted) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        }
    } else {
        event->ignore();
    }
}

void WLibrarySidebar::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_expandTimer.timerId()) {
        QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
        if (viewport()->rect().contains(pos)) {
            QModelIndex index = indexAt(pos);
            if (m_hoverIndex == index) {
                setExpanded(index, !isExpanded(index));
            }
        }
        m_expandTimer.stop();
        return;
    }

    if (event->timerId() == m_longHoverOld.expirationTimer.timerId()) {
        // ExpirationTimeMs elapsed before a dropEvent could take place.
        // "old" will not be the "intended" item anymore
        m_longHoverOld.expirationTimer.stop();
        m_longHoverOld.isValid = false;
        m_longHoverOld.modelIndex = QModelIndex();
        m_longHoverOld.position = QPoint();
        return;
    }

    if (event->timerId() == m_longHoverNew.activationTimer.timerId()) {
        // "new" becomes the "intended" item
        m_longHoverNew.activationTimer.stop();
        m_longHoverNew.isReady = m_longHoverNew.isValid;

        // No need to remember the "old" "intended" item
        m_longHoverOld.expirationTimer.stop();
        m_longHoverOld.modelIndex = QModelIndex();
        m_longHoverOld.position = QPoint();
        m_longHoverOld.isValid = false;
        m_longHoverOld.isReady = false;
        return;
    }
    QTreeView::timerEvent(event);
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view
void WLibrarySidebar::dropEvent(QDropEvent * event) {
    if (event->mimeData()->hasUrls()) {
        // Drag and drop within this widget
        if ((event->source() == this)
                && (event->possibleActions() & Qt::MoveAction)) {
            // Do nothing.
            event->ignore();
        } else {
            //Reset the selected items (if you had anything highlighted, it clears it)
            //this->selectionModel()->clear();
            //Drag-and-drop from an external application or the track table widget
            //eg. dragging a track from Windows Explorer onto the sidebar
            SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
            if (sidebarModel) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QPoint pos = event->position().toPoint();
#else
                QPoint pos = event->pos();
#endif

                QModelIndex possibleDestIndex = indexAt(pos);
                QModelIndex destIndex = possibleDestIndex;

                // Heuristic activated, use replacement item instead
                if (m_longHoverNew.isReady &&
                        (pos - m_longHoverNew.position).manhattanLength() <
                                LongHoverMaxDistanceManhattan) {
                    destIndex = m_longHoverNew.modelIndex;

                    // Forget everything.
                    m_longHoverOld.expirationTimer.stop();
                    m_longHoverNew.activationTimer.stop();

                    m_longHoverOld.modelIndex = QModelIndex();
                    m_longHoverOld.position = QPoint();
                    m_longHoverOld.isValid = false;
                    m_longHoverOld.isReady = false;

                    m_longHoverNew.modelIndex = QModelIndex();
                    m_longHoverNew.position = QPoint();
                    m_longHoverNew.isValid = false;
                    m_longHoverNew.isReady = false;
                }

                // event->source() will return NULL if something is dropped from
                // a different application
                const QList<QUrl> urls = event->mimeData()->urls();
                if (sidebarModel->dropAccept(destIndex, urls, event->source())) {
                    event->acceptProposedAction();
                } else {
                    event->ignore();
                }
            }
        }
        //emit trackDropped(name);
        //repaintEverything();
    } else {
        event->ignore();
    }

    // Forget everything.
    m_longHoverOld.expirationTimer.stop();
    m_longHoverNew.activationTimer.stop();

    m_longHoverOld.modelIndex = QModelIndex();
    m_longHoverOld.position = QPoint();
    m_longHoverOld.isValid = false;
    m_longHoverOld.isReady = false;

    m_longHoverNew.modelIndex = QModelIndex();
    m_longHoverNew.position = QPoint();
    m_longHoverNew.isValid = false;
    m_longHoverNew.isReady = false;
}

void WLibrarySidebar::renameSelectedItem() {
    // Rename crate or playlist (internal, external, history)
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return;
    }
    emit renameItem(selIndex);
    return;
}

void WLibrarySidebar::toggleSelectedItem() {
    QModelIndex index = selectedIndex();
    if (index.isValid()) {
        // Activate the item so its content shows in the main library.
        emit clicked(index);
        // Expand or collapse the item as necessary.
        setExpanded(index, !isExpanded(index));
    }
}

bool WLibrarySidebar::isLeafNodeSelected() {
    QModelIndex index = selectedIndex();
    if (index.isValid()) {
        if(!index.model()->hasChildren(index)) {
            return true;
        }
        const SidebarModel* sidebarModel = qobject_cast<const SidebarModel*>(index.model());
        if (sidebarModel) {
            return sidebarModel->hasTrackTable(index);
        }
    }
    return false;
}

bool WLibrarySidebar::isChildIndexSelected(const QModelIndex& index) {
    // qDebug() << "WLibrarySidebar::isChildIndexSelected" << index;
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return false;
    }
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        // qDebug() << " >> model() is not SidebarModel";
        return false;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        // qDebug() << " >> index can't be translated";
        return false;
    }
    return translated == selIndex;
}

bool WLibrarySidebar::isFeatureRootIndexSelected(LibraryFeature* pFeature) {
    // qDebug() << "WLibrarySidebar::isFeatureRootIndexSelected";
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return false;
    }
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        return false;
    }
    const QModelIndex rootIndex = sidebarModel->getFeatureRootIndex(pFeature);
    return rootIndex == selIndex;
}

/// Invoked by actual keypresses (requires widget focus) and emulated keypresses
/// sent by LibraryControl
void WLibrarySidebar::keyPressEvent(QKeyEvent* event) {
    // TODO(XXX) Should first keyEvent ensure previous item has focus? I.e. if the selected
    // item is not focused, require second press to perform the desired action.

    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (sidebarModel && !selectedIndices.isEmpty()) {
        QModelIndex index = selectedIndices.at(0);
        if (event->matches(QKeySequence::Delete) || event->key() == Qt::Key_Backspace) {
            sidebarModel->clear(index);
            return;
        }
        if (event->matches(QKeySequence::Paste)) {
            sidebarModel->paste(index);
            return;
        }
    }

    focusSelectedIndex();

    switch (event->key()) {
    case Qt::Key_Return:
        toggleSelectedItem();
        return;
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    case Qt::Key_End:
    case Qt::Key_Home: {
        // Let the tree view move up and down for us.
        QTreeView::keyPressEvent(event);
        // After the selection changed force-activate (click) the newly selected
        // item to save us from having to push "Enter".
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        // Ensure the new selection is visible even if it was already selected/
        // focused, like when the topmost item was selected but out of sight and
        // we pressed Up, Home or PageUp.
        scrollTo(selIndex);
        emit pressed(selIndex);
        return;
    }
    case Qt::Key_Right: {
        if (event->modifiers() & Qt::ControlModifier) {
            emit setLibraryFocus(FocusWidget::TracksTable);
        } else {
            QTreeView::keyPressEvent(event);
        }
        return;
    }
    case Qt::Key_Left: {
        // If an expanded item is selected let QTreeView collapse it
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        // collapse knot
        if (isExpanded(selIndex)) {
            QTreeView::keyPressEvent(event);
            return;
        }
        // Else jump to its parent and activate it
        QModelIndex parentIndex = selIndex.parent();
        if (parentIndex.isValid()) {
            selectIndex(parentIndex);
            emit pressed(parentIndex);
        }
        return;
    }
    case Qt::Key_Escape:
        // Focus tracks table
        emit setLibraryFocus(FocusWidget::TracksTable);
        return;
    case kRenameSidebarItemShortcutKey: { // F2
        renameSelectedItem();
        return;
    }
    case kHideRemoveShortcutKey: { // Del (macOS: Cmd+Backspace)
        // Delete crate or playlist (internal, external, history)
        if (event->modifiers() != kHideRemoveShortcutModifier) {
            return;
        }
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        emit deleteItem(selIndex);
        return;
    }
    default:
        QTreeView::keyPressEvent(event);
    }
}

void WLibrarySidebar::mousePressEvent(QMouseEvent* event) {
    // handle right click only in contextMenuEvent() to not select the clicked index
    if (event->buttons().testFlag(Qt::RightButton)) {
        return;
    }
    QTreeView::mousePressEvent(event);
}

void WLibrarySidebar::focusInEvent(QFocusEvent* event) {
    // Clear the current index, i.e. remove the focus indicator
    selectionModel()->clearCurrentIndex();
    QTreeView::focusInEvent(event);
}

void WLibrarySidebar::selectIndex(const QModelIndex& index) {
    //qDebug() << "WLibrarySidebar::selectIndex" << index;
    if (!index.isValid()) {
        return;
    }
    auto* pModel = new QItemSelectionModel(model());
    pModel->select(index, QItemSelectionModel::Select);
    if (selectionModel()) {
        selectionModel()->deleteLater();
    }
    if (index.parent().isValid()) {
        expand(index.parent());
    }
    setSelectionModel(pModel);
    setCurrentIndex(index);
    scrollTo(index);
}

/// Selects a child index from a feature and ensures visibility
void WLibrarySidebar::selectChildIndex(const QModelIndex& index, bool selectItem) {
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        qDebug() << "model() is not SidebarModel";
        return;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        return;
    }

    if (selectItem) {
        auto* pModel = new QItemSelectionModel(sidebarModel);
        pModel->select(translated, QItemSelectionModel::Select);
        if (selectionModel()) {
            selectionModel()->deleteLater();
        }
        setSelectionModel(pModel);
        setCurrentIndex(translated);
    }

    QModelIndex parentIndex = translated.parent();
    while (parentIndex.isValid()) {
        expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
    scrollTo(translated, EnsureVisible);
}

QModelIndex WLibrarySidebar::selectedIndex() {
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (selectedIndices.isEmpty()) {
        return QModelIndex();
    }
    QModelIndex selIndex = selectedIndices.first();
    DEBUG_ASSERT(selIndex.isValid());
    return selIndex;
}

/// Refocus the selected item after right-click
void WLibrarySidebar::focusSelectedIndex() {
    // After the context menu was activated (and closed, with or without clicking
    // an action), the currentIndex is the right-clicked item.
    // If if the currentIndex is not selected, make the selection the currentIndex
    QModelIndex selIndex = selectedIndex();
    if (selIndex.isValid() && selIndex != selectionModel()->currentIndex()) {
        setCurrentIndex(selIndex);
    }
}

bool WLibrarySidebar::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QTreeView::event(pEvent);
}

void WLibrarySidebar::slotSetFont(const QFont& font) {
    setFont(font);
    // Resize the feature icons to be a bit taller than the label's capital
    int iconSize = static_cast<int>(QFontMetrics(font).height() * 0.8);
    setIconSize(QSize(iconSize, iconSize));
}
