#pragma once

#include <QObject>
#include <QPoint>
#include <QTimerEvent>
#include <QtDebug>

#include "util/assert.h"

/// @brief Filter mouse movements to improve the usability of touchpads.
///
/// WLongHoverTracker is used to filter out certain erratic
/// mouse movements caused by touchpad physics. It is basically
/// just a lowpass filter with some additional restrictions.
///
/// # Rationale
///
/// During a drag-and-drop operation, the mouse cursor can
/// move between items either intentionally or unintentionally.
///
/// The latter may happen e.g. with laptop touchpads when a
/// double-tap-and-hold gesture was used to initiate the drag:
///
/// Releasing the finger from the trackpad always causes an
/// unintentional movement of at least a few pixels, often causing
/// the mouse cursor to move away from the intended drop target
/// when the latter is very small.
///
/// # Algorithm
///
/// We try to detect & filter out such unintentional movements
/// using the following heuristic to detect the user's intent:
///
/// 1. The cursor was held over a drop target for at least
///    m_activationTimeMs while the D'n'D was in progress, and:
///
/// 2. At the time of dropAccept, the cursor is over a different
///    item, and:
///
/// 3. The time since last hovering over the 'intended' drop
///    target from step (1) is less than m_expirationTimeMs.
///
///    ExpirationTimeMs should be just enough to cover the time
///    between beginning to release and fully releasing the
///    finger from the touchpad.
///
/// 4. The distance between the current cursor position and the last
///    position that was over the 'intended' drop target is less
///    than m_maxDistance.
///
///    Large mouse movements are very likely intentional,
///    so we have added this rule to avoid false positives.
template<typename ItemRef>
class WLongHoverTracker {
  public:
    WLongHoverTracker(QObject* pParent)
            : WLongHoverTracker(pParent, 2000, 100, 10) {
    }

    WLongHoverTracker(QObject* pParent,
            int activationTimeMs,
            int expirationTimeMs,
            int maxDistance)
            : m_pParent(pParent),
              m_activationTimeMs(activationTimeMs),
              m_expirationTimeMs(expirationTimeMs),
              m_maxDistance(maxDistance) {
        DEBUG_ASSERT(pParent);

        VERIFY_OR_DEBUG_ASSERT(expirationTimeMs < activationTimeMs) {
            qWarning() << "Expiration time" << expirationTimeMs
                       << "should be smaller than activation time" << activationTimeMs;
        }

        VERIFY_OR_DEBUG_ASSERT(maxDistance >= 0) {
            qWarning() << "Maximum distance" << maxDistance << "should not be negative";
        }
    }

    /// Notify the tracker that the cursor is hovering over an item.
    void hoveringOnItem(const ItemRef& itemRef, QPoint mousePosition);

    /// Forward the timer events to this method.
    bool timerEvent(QTimerEvent* event);

    struct TargetInfo {
        ItemRef item;
        QPoint position;
    };

    /// Use heuristics to determine the item that the user probably
    /// wanted to interact with, even when it differs from the item
    /// that is currently under the mouse cursor.
    TargetInfo tryGuessIntendedTarget(
            const ItemRef& currentItem, QPoint currentMousePosition);

    /// Reset the state of the WLongHoverTracker. Call this method when
    /// the drag-and-drop operation has been finished or canceled.
    void clearState();

    class Item {
      public:
        Item()
                : itemRef(), position(), hasValue(false) {
        }

        /// A reference that identifies this item within the backing model
        ItemRef itemRef;

        /// The last mouse position that intersected with the item
        QPoint position;

        /// Whether itemRef and position contain values
        bool hasValue;

        inline void set(const ItemRef& itemRef, QPoint position) {
            this->itemRef = itemRef;
            this->position = position;
            hasValue = true;
        }

        inline void clear() {
            hasValue = false;
            itemRef = ItemRef();
            position = QPoint();
        }
    };

    Item m_intendedItem;
    Item m_currentItem;

    QObject* m_pParent;
    QBasicTimer m_activationTimer;
    QBasicTimer m_expirationTimer;

    int m_activationTimeMs;
    int m_expirationTimeMs;
    int m_maxDistance;
};

template<typename ItemRef>
inline void WLongHoverTracker<ItemRef>::hoveringOnItem(const ItemRef& itemRef, QPoint position) {
    if (m_currentItem.hasValue && m_currentItem.itemRef == itemRef) {
        // Are we still hovering over the same item as before?
        // If yes, just update the last known mouse position,
        // and wait for the activationTimer to make the currentItem
        // also the intendedItem.
        m_currentItem.position = position;
    } else if (m_intendedItem.itemRef == itemRef) {
        // Have we moved back to the previous "intended item" before it had
        // a chance to expire? => Stop the expiration timer in that case.
        m_expirationTimer.stop();
        m_activationTimer.stop();
        m_currentItem.clear();
        m_intendedItem.position = position;
    } else {
        // Otherwise, we have moved to a different item.
        //
        // => Restart the expiration timer that will clear
        //    the old m_intendedItem when it elapses.
        //
        // => Restart the activation timer so that itemRef
        //    can become the new m_intendedItem once it elapses.
        m_currentItem.set(itemRef, position);
        if (m_intendedItem.hasValue) {
            m_expirationTimer.start(m_expirationTimeMs, m_pParent);
        }
        m_activationTimer.start(m_activationTimeMs, m_pParent);
    }
}

template<typename ItemRef>
inline bool WLongHoverTracker<ItemRef>::timerEvent(QTimerEvent* event) {
    if (event->timerId() == m_expirationTimer.timerId()) {
        qDebug() << "Expiration timer elapsed" << m_expirationTimeMs << "ms";
        // m_expirationTimeMs elapsed before a dropEvent could take place
        // => Forget about the previous m_intendedItem
        m_expirationTimer.stop();
        m_intendedItem.clear();
        return true;
    }

    if (event->timerId() == m_activationTimer.timerId()) {
        qDebug() << "Activation timer elapsed after" << m_activationTimeMs << "ms";
        // The cursor has hovered over the m_currentItem long
        // enough for it to become the new m_intendedItem
        // (i.e. longer than m_activationTimeMs)
        m_activationTimer.stop();
        m_expirationTimer.stop();
        m_intendedItem = m_currentItem;
        m_currentItem.clear();
        return true;
    }

    return false;
}

template<typename ItemRef>
inline typename WLongHoverTracker<ItemRef>::TargetInfo WLongHoverTracker<ItemRef>::tryGuessIntendedTarget(
        const ItemRef& currentItem, QPoint currentMousePosition) {
    if (!m_intendedItem.hasValue) {
        // m_expirationTimeMs has already elapsed, so we predict
        // that the currentItem is the one the user intended.
        qDebug() << "No intended item";
        return {currentItem, currentMousePosition};
    }

    QPoint posDiff = currentMousePosition - m_intendedItem.position;
    if (posDiff.manhattanLength() > m_maxDistance) {
        // m_expirationTimeMs has not yet elapsed, but the cursor
        // has moved far enough to indicate that the cursor movement
        // wasn't done by accident.
        qDebug() << "Distance " << posDiff.manhattanLength() << posDiff
                 << "was larger than" << m_maxDistance;
        return {currentItem, currentMousePosition};
    }

    if (m_intendedItem.itemRef == currentItem) {
        qDebug() << "Already on intended item";
    } else {
        qDebug() << "Substituting intended item";
    }

    // Both the time and distance thresholds have been met, so we
    // consider the mouse movement to be accidental, and replace the
    // currentItem with the item that was probably intended by the user.
    return {m_intendedItem.itemRef, m_intendedItem.position};
}

template<typename ItemRef>
inline void WLongHoverTracker<ItemRef>::clearState() {
    m_expirationTimer.stop();
    m_activationTimer.stop();
    m_intendedItem.clear();
    m_currentItem.clear();
}
