#include "widget/wlibrarysidebar.h"

#include <QApplication>
#include <QHeaderView>
#include <QUrl>
#include <QtDebug>

#include "library/sidebarmodel.h"
#include "moc_wlibrarysidebar.cpp"
#include "util/defs.h"
#include "util/dnd.h"

namespace {

constexpr int expand_time = 250;
constexpr int collapse_time = 750;

} // namespace

WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this),
          m_longHover(this,
                  300,
                  100,
                  20) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
    //Set some properties
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    //Drag and drop setup
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragDropOverwriteMode(true);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setAutoScroll(true);
    setAutoExpandDelay(expand_time);
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

/// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent * event) {
    // event->source() will be NULL if something is dropped
    // from a different application. This knowledge is used
    // inside the LibraryFeature implementations.
    auto* sidebarModel = qobject_cast<SidebarModel*>(model());
    if (sidebarModel) {
        sidebarModel->setSourceOfCurrentDragDropEvent(event->source());
    }
    QTreeView::dragEnterEvent(event);
    if (event->isAccepted()) {
        event->acceptProposedAction();
    }
    if (sidebarModel) {
        sidebarModel->setSourceOfCurrentDragDropEvent(nullptr);
    }
}

/// Drag leave event, happens when the dragged item leaves the track sources view
/// or when the drag is aborted through Escape or other means.
void WLibrarySidebar::dragLeaveEvent(QDragLeaveEvent* event) {
    m_autoExpandIndex = QModelIndex();
    m_longHover.clearState();
    QTreeView::dragLeaveEvent(event);
}

/// Drag move event, happens when a dragged item hovers over the track sources view...
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent * event) {
    auto* sidebarModel = qobject_cast<SidebarModel*>(model());
    if (sidebarModel) {
        sidebarModel->setSourceOfCurrentDragDropEvent(event->source());
    }

    auto pos = event->position().toPoint();
    auto index = indexAt(pos);
    m_longHover.hoveringOnItem(index, pos);

    if (m_autoExpandIndex != index) {
        if (m_activationTimer.isValid()) {
            qDebug() << "Last timer" << m_activationTimer.elapsed() << m_autoExpandIndex;
        }
        m_activationTimer.start();
        m_autoExpandIndex = index;
        if (isExpanded(index)) {
            setAutoExpandDelay(collapse_time);
        } else {
            setAutoExpandDelay(expand_time);
        }
        // QTreeView::dragMoveEvent just restarts the autoExpand timer
        // and then calls QAbstractItemView::dragMoveEvent
        QTreeView::dragMoveEvent(event);
    } else {
        // Skip resetting the autoExpand timer (see above)
        // because we are still hovering over the same item
        QAbstractItemView::dragMoveEvent(event);
    }

    if (event->isAccepted()) {
        event->acceptProposedAction();
    }
    if (sidebarModel) {
        sidebarModel->setSourceOfCurrentDragDropEvent(nullptr);
    }
}

void WLibrarySidebar::timerEvent(QTimerEvent *event) {
    if (m_longHover.timerEvent(event)) {
        return;
    }
    QTreeView::timerEvent(event);
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view
void WLibrarySidebar::dropEvent(QDropEvent* event) {
    m_autoExpandIndex = QModelIndex();

    auto* sidebarModel = qobject_cast<SidebarModel*>(model());
    if (sidebarModel) {
        // event->source() will be NULL if something is dropped
        // from a different application. This knowledge is used
        // inside the LibraryFeature implementations.
        sidebarModel->setSourceOfCurrentDragDropEvent(event->source());
    }

    auto pos = event->position().toPoint();
    auto index = indexAt(pos);
    auto probableTarget = m_longHover.tryGuessIntendedTarget(index, pos);
    qDebug() << "Dropping after" << m_activationTimer.elapsed() << index << probableTarget.item;
    m_activationTimer.invalidate();

    if (probableTarget.item != index) {
        // Use the target item that the user likely intended to hit,
        // instead of the one that is currently under the mouse cursor
        QDropEvent syntheticEvent(
                probableTarget.position,
                event->possibleActions(),
                event->mimeData(),
                event->buttons(),
                event->modifiers(),
                event->type());

        // Copy mutable state from original event
        syntheticEvent.setAccepted(event->isAccepted());
        syntheticEvent.setDropAction(event->dropAction());

        // Execute the original handling logic,
        // but with the synthetic event instead
        QTreeView::dropEvent(&syntheticEvent);

        // Mirror modifications back to the original event
        event->setAccepted(syntheticEvent.isAccepted());
        event->setDropAction(syntheticEvent.dropAction());
    } else {
        QTreeView::dropEvent(event);
    }

    if (event->isAccepted()) {
        event->acceptProposedAction();
    }
    if (sidebarModel) {
        sidebarModel->setSourceOfCurrentDragDropEvent(nullptr);
    }
    m_longHover.clearState();
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

void WLibrarySidebar::setChildIndexExpanded(const QModelIndex& index, bool expand) {
    qDebug() << "WLibrarySidebar::setChildIndexExpanded" << index << expand;
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return;
    }
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        qDebug() << " >> model() is not SidebarModel";
        return;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        qDebug() << " >> index can't be translated";
        return;
    }
    setExpanded(translated, expand);
}

bool WLibrarySidebar::isChildIndexExpanded(const QModelIndex& index) {
    // qDebug() << "WLibrarySidebar::isChildIndexExpanded" << index;
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
    return isExpanded(translated);
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
    QModelIndex selIndex = selectedIndex();
    if (sidebarModel && selIndex.isValid() && event->matches(QKeySequence::Paste)) {
        sidebarModel->paste(selIndex);
        return;
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
    case kRenameSidebarItemShortcutKey:              // F2
    case kRenameSidebarItemAlternativeShortcutKey: { // R
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
