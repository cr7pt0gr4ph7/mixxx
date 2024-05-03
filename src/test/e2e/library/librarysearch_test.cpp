#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QString>
#include <QTest>
#include <QtDebug>

#include "test/mixxxuitest.h"

class LibrarySearchTest : public MixxxUITest {};

TEST_F(LibrarySearchTest, FocusSearchBoxOnCtrlF) {
    findAndFocusWidget("LibrarySearchBox");
    enterKeys()
    // assertTextEquals("");
    // enterText("Some search text");
    // assertTextEquals("Some search text");

    // library_selectView("Tracks");
    // library_focusWidget("Main");
}


// void setup() {
//     addLibraryTrack("...", "...", "...");
//     addLibraryTrack("...");
//     addLibraryTrack("...");
//     selectLibraryView();
// }

// void multiSelectByMouse() {
//     setSelectionByMouse(...);
//     addToSelectionByMouse(...)
// }

// void verify() {

//     trackListIs(trackOne, trackTwo, trackThree, trackFour, trackFive, trackSix);
// }

// void testme() {
//     trackListIs_ShouldBe(trackOne, trackTwo, trackThree, trackFour, trackFive, trackSix);

//     multiSelectByMouse(trackThree, trackFive);
//     currentIndexIs(trackFive);
//     removeSel
// }
