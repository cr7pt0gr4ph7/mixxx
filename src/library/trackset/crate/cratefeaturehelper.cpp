#include "library/trackset/crate/cratefeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/cratefolder.h"
#include "library/trackset/crate/cratesummary.h"
#include "moc_cratefeaturehelper.cpp"

CrateFeatureHelper::CrateFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString CrateFeatureHelper::proposeNameForNewCrate(
        CrateFolderId folderId, const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->crates().readCrateByName(folderId, proposedName));
    // Found an unused crate name
    return proposedName;
}

QString CrateFeatureHelper::proposeNameForNewFolder(
        CrateFolderId parentId, const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->crates().readFolderByName(parentId, proposedName));
    // Found an unused crate name
    return proposedName;
}

CrateId CrateFeatureHelper::createEmptyCrate(CrateFolderId folderId) {
    const QString proposedCrateName =
            proposeNameForNewCrate(folderId, tr("New Crate"));
    Crate newCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return CrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->crates().readCrateByName(folderId, newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate by that name already exists."));
            continue;
        }
        newCrate.setFolderId(folderId);
        newCrate.setName(std::move(newName));
        DEBUG_ASSERT(newCrate.hasName());
        break;
    }

    CrateId newCrateId;
    if (m_pTrackCollection->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
    } else {
        DEBUG_ASSERT(!newCrateId.isValid());
        qWarning() << "Failed to create new crate"
                   << "->" << newCrate.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Crate Failed"),
                tr("An unknown error occurred while creating crate: ") + newCrate.getName());
    }
    return newCrateId;
}

CrateFolderId CrateFeatureHelper::createEmptyFolder(CrateFolderId parentId) {
    const QString proposedCrateName =
            proposeNameForNewFolder(parentId, tr("New Folder"));
    CrateFolder newFolder;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Folder"),
                        tr("Enter name for new folder:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return CrateFolderId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Folder Failed"),
                    tr("A folder cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->crates().readFolderByName(parentId, newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Folder Failed"),
                    tr("A folder by that name already exists."));
            continue;
        }
        newFolder.setParentId(parentId);
        newFolder.setName(std::move(newName));
        DEBUG_ASSERT(newFolder.hasName());
        break;
    }

    CrateFolderId newFolderId;
    if (m_pTrackCollection->insertCrateFolder(newFolder, &newFolderId)) {
        DEBUG_ASSERT(newFolderId.isValid());
        newFolder.setId(newFolderId);
        qDebug() << "Created new folder" << newFolder;
    } else {
        DEBUG_ASSERT(!newFolderId.isValid());
        qWarning() << "Failed to create new folder"
                   << "->" << newFolder.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Folder Failed"),
                tr("An unknown error occurred while creating folder: ") + newFolder.getName());
    }
    return newFolderId;
}

CrateId CrateFeatureHelper::duplicateCrate(const Crate& oldCrate) {
    const QString proposedCrateName =
            proposeNameForNewCrate(
                    oldCrate.getFolderId(),
                    QStringLiteral("%1 %2")
                            .arg(oldCrate.getName(), tr("copy", "//:")));
    Crate newCrate;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        proposedCrateName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return CrateId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->crates().readCrateByName(oldCrate.getFolderId(), newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Crate Failed"),
                    tr("A crate by that name already exists."));
            continue;
        }
        newCrate.setName(std::move(newName));
        DEBUG_ASSERT(newCrate.hasName());
        break;
    }

    CrateId newCrateId;
    if (m_pTrackCollection->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->crates().countCrateTracks(oldCrate.getId()));
        {
            CrateTrackSelectResult crateTracks(
                    m_pTrackCollection->crates().selectCrateTracksSorted(oldCrate.getId()));
            while (crateTracks.next()) {
                trackIds.append(crateTracks.trackId());
            }
        }
        if (m_pTrackCollection->addCrateTracks(newCrateId, trackIds)) {
            qDebug() << "Duplicated crate"
                     << oldCrate << "->" << newCrate;
        } else {
            qWarning() << "Failed to copy tracks from"
                       << oldCrate << "into" << newCrate;
        }
    } else {
        qWarning() << "Failed to duplicate crate"
                   << oldCrate << "->" << newCrate.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Crate Failed"),
                tr("An unknown error occurred while creating crate: ") + newCrate.getName());
    }
    return newCrateId;
}
