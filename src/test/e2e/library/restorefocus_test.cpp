
void setup() {
    addLibraryTrack("...", "...", "...");
    addLibraryTrack("...");
    addLibraryTrack("...");
    selectLibraryView();
}

void multiSelectByMouse() {
    setSelectionByMouse(...);
    addToSelectionByMouse(...)
}

void verify() {

    trackListIs(trackOne, trackTwo, trackThree, trackFour, trackFive, trackSix);
}

void testme() {
    trackListIs_ShouldBe(trackOne, trackTwo, trackThree, trackFour, trackFive, trackSix);

    multiSelectByMouse(trackThree, trackFive);
    currentIndexIs(trackFive);
    removeSel
}
