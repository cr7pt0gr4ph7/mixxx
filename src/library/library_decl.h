#pragma once

class Library;

enum class LibraryRemovalType {
    KeepTracks,
    HideTracks,
    PurgeTracks
};

enum class FocusWidget {
    None,
    Searchbar,
    Sidebar,
    TracksTable, // or a feature root view (WLibraryTextBrowser)
    ContextMenu, // See LibraryControl::getFocusedWidget() for detailed
    Dialog,      // descriptions
    Unknown,     // Unknown skin widget or unknown window
    Count        // Used for setting the number of PushButton states of
                 // m_pFocusedWidget in librarycontrol.cpp
};

namespace LibraryViewName {
const QString Analysis = QStringLiteral("Analysis");
const QString AutoDj = QStringLiteral("Auto DJ");
const QString FileBrowser = QStringLiteral("BROWSEHOME");
const QString Playlists = QStringLiteral("PLAYLISTHOME");
const QString Rekordbox = QStringLiteral("REKORDBOXHOME");
const QString Recording = QStringLiteral("Recording");
const QString Serato = QStringLiteral("SERATOHOME");
const QString Tracks = QStringLiteral("WTrackTableView");
}; // namespace LibraryViewName
