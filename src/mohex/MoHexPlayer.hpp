//----------------------------------------------------------------------------
/** @file MoHexPlayer.hpp
 */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYER_HPP
#define MOHEXPLAYER_HPP

#include "BenzenePlayer.hpp"
#include "HexUctSearch.hpp"
#include "HexUctPolicy.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Player using UCT to generate moves. */
class MoHexPlayer : public BenzenePlayer
{
public:

    /** Constructor. */
    MoHexPlayer();

    /** Destructor. */
    virtual ~MoHexPlayer();

    /** Returns "mohex". */
    std::string Name() const;

    /** Returns the search. */
    HexUctSearch& Search();

    /** Returns the search. */
    const HexUctSearch& Search() const;

    /** Returns the shared policy. */
    HexUctSharedPolicy& SharedPolicy();

    /** Returns the shared policy. */
    const HexUctSharedPolicy& SharedPolicy() const;

    /** Copy settings from other player. */
    void CopySettingsFrom(const MoHexPlayer& other);

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    bool BackupIceInfo() const;

    void SetBackupIceInfo(bool enable);

    /** Max number of games to play. */
    int MaxGames() const;

    /** See MaxGames() */
    void SetMaxGames(int games);

    /** Maximum time to spend on search (in seconds). */
    double MaxTime() const;

    /** See MaxTime() */
    void SetMaxTime(double time);

    /** Use time control to determine how much time to use per move. */
    bool UseTimeManagement() const;

    /** See UseTimeManagement() */
    void SetUseTimeManagement(bool flag);

    /** Search is initialized using the subttree of the last search
        tree rooted at the current position. */
    bool ReuseSubtree() const;
    
    /** See ReuseSubtree() */
    void SetReuseSubtree(bool reuse);

    /** Searches while waiting for a command. */
    bool Ponder() const;

    /** See Ponder() */
    void SetPonder(bool flag);

    /** Searches 1ply for easy wins before the search. */
    bool PerformPreSearch() const;

    /** See PerformPreSearch() */
    void SetPerformPreSearch(bool flag);

    // @}

protected:

    HexUctSharedPolicy m_shared_policy;
    
    HexUctSearch m_search;
   
    bool m_backup_ice_info;

    /** See MaxGames() */
    int m_max_games;

    /** See MaxTime() */
    double m_max_time;

    /** See UseTimeManagement() */
    bool m_useTimeManagement;

    /** See ReuseSubtree() */
    bool m_reuse_subtree;

    /** See Ponder() */
    bool m_ponder;

    bool m_performPreSearch;
    
    /** Generates a move in the given gamestate using uct. */
    HexPoint Search(const HexState& state, const Game& game,
                    HexBoard& brd, const bitset_t& consider,
                    double maxTime, double& score);

    HexPoint LastMoveFromHistory(const MoveSequence& history);

    bool PerformPreSearch(HexBoard& brd, HexColor color, bitset_t& consider, 
                          float maxTime, PointSequence& winningSequence);

    void PrintParameters(HexColor color, double remaining);
    
    SgUctTree* TryReuseSubtree(const HexUctSharedData& oldData,
                               HexUctSharedData& newData);

    void CopyKnowledgeData(const SgUctTree& tree, const SgUctNode& node,
                           HexColor color, MoveSequence& sequence,
                           const HexUctSharedData& oldData,
                           HexUctSharedData& newData) const;
};

inline std::string MoHexPlayer::Name() const
{
    return "mohex";
}

inline HexUctSearch& MoHexPlayer::Search()
{
    return m_search;
}

inline const HexUctSearch& MoHexPlayer::Search() const
{
    return m_search;
}

inline HexUctSharedPolicy& MoHexPlayer::SharedPolicy()
{
    return m_shared_policy;
}

inline const HexUctSharedPolicy& MoHexPlayer::SharedPolicy() const
{
    return m_shared_policy;
}

inline bool MoHexPlayer::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void MoHexPlayer::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

inline int MoHexPlayer::MaxGames() const
{
    return m_max_games;
}

inline void MoHexPlayer::SetMaxGames(int games)
{
    m_max_games = games;
}

inline double MoHexPlayer::MaxTime() const
{
    return m_max_time;
}

inline void MoHexPlayer::SetMaxTime(double time)
{
    m_max_time = time;
}

inline bool MoHexPlayer::UseTimeManagement() const
{
    return m_useTimeManagement;
}

inline void MoHexPlayer::SetUseTimeManagement(bool flag)
{
    m_useTimeManagement = flag;
}

inline bool MoHexPlayer::ReuseSubtree() const
{
    return m_reuse_subtree;
}

inline void MoHexPlayer::SetReuseSubtree(bool reuse)
{
    m_reuse_subtree = reuse;
}

inline bool MoHexPlayer::Ponder() const
{
    return m_ponder;
}

inline void MoHexPlayer::SetPonder(bool flag)
{
    m_ponder = flag;
}

inline bool MoHexPlayer::PerformPreSearch() const
{
    return m_performPreSearch;
}

inline void MoHexPlayer::SetPerformPreSearch(bool flag)
{
    m_performPreSearch = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPLAYER_HPP
