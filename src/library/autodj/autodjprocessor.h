#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "control/controlproxy.h"
#include "engine/channels/enginechannel.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/class.h"
#include "util/duration.h"

class ControlPushButton;
class TrackCollectionManager;
class PlayerManagerInterface;
class BaseTrackPlayer;
class PlaylistTableModel;
typedef QList<QModelIndex> QModelIndexList;

class TrackOrDeckAttributes : public QObject {
    Q_OBJECT
  public:
    virtual ~TrackOrDeckAttributes();

    virtual mixxx::audio::FramePos introStartPosition() const = 0;
    virtual mixxx::audio::FramePos introEndPosition() const = 0;
    virtual mixxx::audio::FramePos outroStartPosition() const = 0;
    virtual mixxx::audio::FramePos outroEndPosition() const = 0;
    virtual mixxx::audio::SampleRate sampleRate() const = 0;
    virtual mixxx::audio::FramePos trackEndPosition() const = 0;
    virtual double playPosition() const = 0;
    virtual double rateRatio() const = 0;

    virtual TrackPointer getLoadedTrack() const = 0;

    bool isEmpty() const {
        return !getLoadedTrack();
    }
};

class FadeableTrackOrDeckAttributes : public TrackOrDeckAttributes {
    Q_OBJECT
  public:
    FadeableTrackOrDeckAttributes();
    virtual ~FadeableTrackOrDeckAttributes();

    double startPos;     // Set in toDeck nature
    double fadeBeginPos; // set in fromDeck nature
    double fadeEndPos;   // set in fromDeck nature
    double fadeDurationSeconds;
    bool isFromDeck;
};

/// Exposes the attributes of a track from the Auto DJ queue
class TrackAttributes : public FadeableTrackOrDeckAttributes {
    Q_OBJECT
  public:
    TrackAttributes(TrackPointer pTrack);
    virtual ~TrackAttributes();

    virtual mixxx::audio::FramePos introStartPosition() const override;
    virtual mixxx::audio::FramePos introEndPosition() const override;
    virtual mixxx::audio::FramePos outroStartPosition() const override;
    virtual mixxx::audio::FramePos outroEndPosition() const override;
    virtual mixxx::audio::SampleRate sampleRate() const override;
    virtual mixxx::audio::FramePos trackEndPosition() const override;
    virtual double playPosition() const override;
    virtual double rateRatio() const override;

    TrackPointer getLoadedTrack() const override {
        return m_pTrack;
    }

  private:
    TrackPointer m_pTrack;
};

/// Exposes the attributes of the track loaded in a certain player deck
class DeckAttributes : public FadeableTrackOrDeckAttributes {
    Q_OBJECT
  public:
    DeckAttributes(int index,
            BaseTrackPlayer* pPlayer);
    virtual ~DeckAttributes();

    bool isLeft() const {
        return m_orientation.get() == static_cast<double>(EngineChannel::LEFT);
    }

    bool isRight() const {
        return m_orientation.get() == static_cast<double>(EngineChannel::RIGHT);
    }

    bool isPlaying() const {
        return m_play.toBool();
    }

    void stop() {
        m_play.set(0.0);
    }

    void play() {
        m_play.set(1.0);
    }

    double playPosition() const override {
        return m_playPos.get();
    }

    void setPlayPosition(double playpos) {
        m_playPos.set(playpos);
    }

    bool isRepeat() const {
        return m_repeat.toBool();
    }

    void setRepeat(bool enabled) {
        m_repeat.set(enabled ? 1.0 : 0.0);
    }

    mixxx::audio::FramePos introStartPosition() const override {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_introStartPos.get());
    }

    mixxx::audio::FramePos introEndPosition() const override {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_introEndPos.get());
    }

    mixxx::audio::FramePos outroStartPosition() const override {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_outroStartPos.get());
    }

    mixxx::audio::FramePos outroEndPosition() const override {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_outroEndPos.get());
    }

    mixxx::audio::SampleRate sampleRate() const override {
        return mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());
    }

    mixxx::audio::FramePos trackEndPosition() const override {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_trackSamples.get());
    }

    double rateRatio() const override {
        return m_rateRatio.get();
    }

    TrackPointer getLoadedTrack() const override;

  signals:
    void playChanged(DeckAttributes* pDeck, bool playing);
    void playPositionChanged(DeckAttributes* pDeck, double playPosition);
    void introStartPositionChanged(DeckAttributes* pDeck, double introStartPosition);
    void introEndPositionChanged(DeckAttributes* pDeck, double introEndPosition);
    void outroStartPositionChanged(DeckAttributes* pDeck, double outtroStartPosition);
    void outroEndPositionChanged(DeckAttributes* pDeck, double outroEndPosition);
    void trackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void loadingTrack(DeckAttributes* pDeck, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty(DeckAttributes* pDeck);
    void rateChanged(DeckAttributes* pDeck);

  private slots:
    void slotPlayPosChanged(double v);
    void slotPlayChanged(double v);
    void slotIntroStartPositionChanged(double v);
    void slotIntroEndPositionChanged(double v);
    void slotOutroStartPositionChanged(double v);
    void slotOutroEndPositionChanged(double v);
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotPlayerEmpty();
    void slotRateChanged(double v);

  public:
    int index;
    QString group;
    bool loading; // The data is inconsistent during loading a deck

  private:
    ControlProxy m_orientation;
    ControlProxy m_playPos;
    ControlProxy m_play;
    ControlProxy m_repeat;
    ControlProxy m_introStartPos;
    ControlProxy m_introEndPos;
    ControlProxy m_outroStartPos;
    ControlProxy m_outroEndPos;
    ControlProxy m_trackSamples;
    ControlProxy m_sampleRate;
    ControlProxy m_rateRatio;
    BaseTrackPlayer* m_pPlayer;
};

class AutoDJProcessor : public QObject {
    Q_OBJECT
  public:
    enum AutoDJState {
        ADJ_IDLE = 0,
        ADJ_LEFT_FADING,
        ADJ_RIGHT_FADING,
        ADJ_ENABLE_P1LOADED,
        ADJ_ENABLE_P1PLAYING,
        ADJ_DISABLED
    };

    enum AutoDJError {
        ADJ_OK = 0,
        ADJ_IS_INACTIVE,
        ADJ_QUEUE_EMPTY,
        ADJ_BOTH_DECKS_PLAYING,
        ADJ_DECKS_3_4_PLAYING,
        ADJ_NOT_TWO_DECKS
    };

    enum class TransitionMode {
        FullIntroOutro,
        FadeAtOutroStart,
        FixedFullTrack,
        FixedSkipSilence
    };

    AutoDJProcessor(QObject* pParent,
                    UserSettingsPointer pConfig,
                    PlayerManagerInterface* pPlayerManager,
                    TrackCollectionManager* pTrackCollectionManager,
                    int iAutoDJPlaylistId);
    virtual ~AutoDJProcessor();

    AutoDJState getState() const {
        return m_eState;
    }

    double getTransitionTime() const {
        return m_transitionTime;
    }

    TransitionMode getTransitionMode() const {
        return m_transitionMode;
    }

    PlaylistTableModel* getTableModel() const {
        return m_pAutoDJTableModel;
    }

    mixxx::Duration getRemainingTime() const {
        return m_timeRemaining;
    }

    int getRemainingTracks() const;

    bool nextTrackLoaded();

    void setTransitionTime(int seconds);

    void setTransitionMode(TransitionMode newMode);

    AutoDJError shufflePlaylist(const QModelIndexList& selectedIndices);
    AutoDJError skipNext();
    void fadeNow();
    AutoDJError toggleAutoDJ(bool enable);

  signals:
    void loadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void autoDJError(AutoDJProcessor::AutoDJError error);
    void remainingTimeChanged(int numTracks, mixxx::Duration duration);
    void transitionTimeChanged(int time);
    void randomTrackRequested(int tracksToAdd);

  private slots:
    void crossfaderChanged(double value);
    void playerPositionChanged(DeckAttributes* pDeck, double position);
    void playerPlayChanged(DeckAttributes* pDeck, bool playing);
    void playerIntroStartChanged(DeckAttributes* pDeck, double position);
    void playerIntroEndChanged(DeckAttributes* pDeck, double position);
    void playerOutroStartChanged(DeckAttributes* pDeck, double position);
    void playerOutroEndChanged(DeckAttributes* pDeck, double position);
    void playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void playerLoadingTrack(DeckAttributes* pDeck, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty(DeckAttributes* pDeck);
    void playerRateChanged(DeckAttributes* pDeck);
    void playlistFirstTrackChanged();

    void playlistTracksChanged();
    void tracksChanged(const QSet<TrackId>& tracks);
    void multipleTracksChanged();
    void updateRemainingTime();

    void controlEnableChangeRequest(double value);
    void controlFadeNow(double value);
    void controlShuffle(double value);
    void controlSkipNext(double value);
    void controlAddRandomTrack(double value);

  protected:
    // The following virtual signal wrappers are used for testing
    virtual void emitLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play) {
        emit loadTrackToPlayer(pTrack, group, play);
    }
    virtual void emitAutoDJStateChanged(AutoDJProcessor::AutoDJState state) {
        emit autoDJStateChanged(state);
    }

  private:
    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value);

    // Following functions return seconds computed from samples or -1 if
    // track in deck has invalid sample rate (<= 0)
    double getIntroStartSecond(const TrackOrDeckAttributes& track);
    double getIntroEndSecond(const TrackOrDeckAttributes& track);
    double getOutroStartSecond(const TrackOrDeckAttributes& track);
    double getOutroEndSecond(const TrackOrDeckAttributes& track);
    double getFirstSoundSecond(const TrackOrDeckAttributes& track);
    double getLastSoundSecond(const TrackOrDeckAttributes& track);
    double getEndSecond(const TrackOrDeckAttributes& track);
    double framePositionToSeconds(mixxx::audio::FramePos position,
            const TrackOrDeckAttributes& track);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue(const DeckAttributes& pDeck, bool play = false);
    void calculateTransition(
            DeckAttributes* pFromDeck,
            DeckAttributes* pToDeck,
            bool seekToStartPoint);
    void calculateTransitionImpl(
            FadeableTrackOrDeckAttributes& pFromDeck,
            FadeableTrackOrDeckAttributes& pToDeck,
            bool seekToStartPoint);
    void useFixedFadeTime(
            FadeableTrackOrDeckAttributes& fromTrack,
            FadeableTrackOrDeckAttributes& toTrack,
            double fromDeckSecond,
            double fadeEndSecond,
            double toDeckStartSecond);
    DeckAttributes* getLeftDeck();
    DeckAttributes* getRightDeck();
    DeckAttributes* getOtherDeck(const DeckAttributes* pThisDeck);
    DeckAttributes* getFromDeck();

    /// Calculates the total remaining duration of tracks in the AutoDJ playlist,
    /// excluding the track that is currently playing already.
    mixxx::Duration calculateRemainingTime();

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck);

    // Removes the provided track from the top of the AutoDJ queue if it is
    // present.
    bool removeTrackFromTopOfQueue(TrackPointer pTrack);
    void maybeFillRandomTracks();
    UserSettingsPointer m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    double m_transitionProgress;
    double m_transitionTime; // the desired value set by the user
    TransitionMode m_transitionMode;

    QList<DeckAttributes*> m_decks;

    ControlProxy* m_pCOCrossfader;
    ControlProxy* m_pCOCrossfaderReverse;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pAddRandomTrack;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    ControlObject* m_pTracksRemaining;
    ControlObject* m_pTimeRemaining;
    mixxx::Duration m_timeRemaining;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};
