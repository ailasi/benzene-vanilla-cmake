//----------------------------------------------------------------------------
/** @file MoHexPlayer.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "SgTimer.h"
#include "SgUctTreeUtil.h"

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "VCSet.hpp"
#include "HexUctUtil.hpp"
#include "HexUctKnowledge.hpp"
#include "HexUctSearch.hpp"
#include "HexUctPolicy.hpp"
#include "MoHexPlayer.hpp"
#include "PlayerUtils.hpp"
#include "SequenceHash.hpp"
#include "Time.hpp"
#include "VCUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

void CommonPrefix(const MoveSequence& a, const MoveSequence& b, 
                  std::size_t& prefixLen)
{
    prefixLen = 0;
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i)
    {
        if (a[i] != b[i])
            break;
        ++prefixLen;
    }
}

}

//----------------------------------------------------------------------------

MoHexPlayer::MoHexPlayer()
    : BenzenePlayer(),
      m_shared_policy(),
      m_search(new HexThreadStateFactory(&m_shared_policy), 
               HexUctUtil::ComputeMaxNumMoves()),
      m_backup_ice_info(true),
      m_max_games(500000),
      m_max_time(10),
      m_reuse_subtree(false)
{
    LogFine() << "--- MoHexPlayer" << '\n';
}

MoHexPlayer::~MoHexPlayer()
{
}

void MoHexPlayer::CopySettingsFrom(const MoHexPlayer& other)
{
    SetBackupIceInfo(other.BackupIceInfo());
    Search().SetLockFree(other.Search().LockFree());
    Search().SetLiveGfx(other.Search().LiveGfx());
    Search().SetRave(other.Search().Rave());
    Search().SetBiasTermConstant(other.Search().BiasTermConstant());
    Search().SetExpandThreshold(other.Search().ExpandThreshold());
    Search().SetLiveGfxInterval(other.Search().LiveGfxInterval());
    SetMaxGames(other.MaxGames());
    SetMaxTime(other.MaxTime());
    Search().SetMaxNodes(other.Search().MaxNodes());
    Search().SetNumberThreads(other.Search().NumberThreads());
    Search().SetPlayoutUpdateRadius(other.Search().PlayoutUpdateRadius());
    Search().SetRaveWeightFinal(other.Search().RaveWeightFinal());
    Search().SetRaveWeightInitial(other.Search().RaveWeightInitial());
    Search().SetTreeUpdateRadius(other.Search().TreeUpdateRadius());
    Search().SetKnowledgeThreshold(other.Search().KnowledgeThreshold());
}

//----------------------------------------------------------------------------

HexPoint MoHexPlayer::search(HexBoard& brd, 
                             const Game& game_state,
			     HexColor color,
                             const bitset_t& given_to_consider,
                             double time_remaining,
                             double& score)
{
   
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(!brd.isGameOver());

    double start = Time::Get();
    PrintParameters(color, time_remaining);

    // Do presearch and abort if win found.
    SgTimer timer;
    bitset_t consider = given_to_consider;
    PointSequence winningSequence;
    if (PerformPreSearch(brd, color, consider, winningSequence))
    {
	LogInfo() << "Winning sequence found before UCT search!" << '\n'
		  << "Sequence: " << winningSequence[0] << '\n';
        score = IMMEDIATE_WIN;
	return winningSequence[0];
    }
    timer.Stop();
    double elapsed = timer.GetTime();
    time_remaining -= elapsed;
    LogInfo() << "Time for PreSearch: " << Time::Formatted(elapsed) << '\n';

    // Create the initial state data
    HexUctSharedData data;
    data.root_to_play = color;
    data.game_sequence = game_state.History();
    data.root_last_move_played = LastMoveFromHistory(game_state.History());
    data.root_stones = HexUctStoneData(brd);
    data.root_consider = consider;
    
    // Reuse the old subtree
    SgUctTree* initTree = 0;
    if (m_reuse_subtree)
    {
        HexUctSharedData oldData = m_search.SharedData();        
        initTree = TryReuseSubtree(oldData, data);
        if (!initTree)
            LogInfo() << "No subtree to reuse." << '\n';
    }
    m_search.SetSharedData(data);

    // Compute timelimit for this search. Use the minimum of the time
    // left in the game and the m_max_time parameter.
    double timelimit = std::min(time_remaining, m_max_time);
    if (timelimit < 0) 
    {
        LogWarning() << "***** timelimit < 0!! *****" << '\n';
        timelimit = m_max_time;
    }
    LogInfo() << "timelimit: " << timelimit << '\n';

    brd.ClearPatternCheckStats();
    int old_radius = brd.updateRadius();
    brd.setUpdateRadius(m_search.TreeUpdateRadius());

    // Do the search
    std::vector<SgMove> sequence;
    std::vector<SgMove> rootFilter;
    m_search.SetBoard(brd);
    score = m_search.Search(m_max_games, timelimit, sequence,
                            rootFilter, initTree, 0);

    brd.setUpdateRadius(old_radius);

    double end = Time::Get();

    // Output stats
    std::ostringstream os;
    os << '\n';
#if COLLECT_PATTERN_STATISTICS
    os << m_shared_policy.DumpStatistics() << '\n';
    os << brd.DumpPatternCheckStats() << '\n';
#endif
    os << "Elapsed Time   " << Time::Formatted(end - start) << '\n';
    m_search.WriteStatistics(os);
    os << "Score          " << score << "\n"
       << "Sequence      ";
    for (int i=0; i<(int)sequence.size(); i++) 
        os << " " << HexUctUtil::MoveString(sequence[i]);
    os << '\n';
    
    LogInfo() << os.str() << '\n';

#if 0
    if (m_save_games) {
        std::string filename = "uct-games.sgf";
        uct.SaveGames(filename);
        LogInfo() << "Games saved in '" << filename << "'." << '\n';
    }
#endif

    // Return move recommended by HexUctSearch
    if (sequence.size() > 0) 
        return static_cast<HexPoint>(sequence[0]);

    // It is possible that HexUctSearch only did 1 rollout (probably
    // because it ran out of time to do more); in this case, the move
    // sequence is empty and so we give a warning and return a random
    // move.
    LogWarning() << "**** HexUctSearch returned empty sequence!" << '\n'
		 << "**** Returning random move!" << '\n';
    return BoardUtils::RandomEmptyCell(brd);
}

/** Returns INVALID_POINT if history is empty, otherwise last move
    played to the board, ie, skips swap move.
*/
HexPoint MoHexPlayer::LastMoveFromHistory(const MoveSequence& history)
{
    HexPoint lastMove = INVALID_POINT;
    if (!history.empty()) 
    {
	lastMove = history.back().point();
	if (lastMove == SWAP_PIECES) 
        {
            HexAssert(history.size() == 2);
            lastMove = history.front().point();
	}
    }
    return lastMove;
}

/** Does a 1-ply search.

    For each move in the consider set, if the move is a win, returns
    true and the move. If the move is a loss, prune it out of the
    consider set if there are non-losing moves in the consider set.
    If all moves are losing, perform no pruning, search will resist.

    Returns true if there is a win, false otherwise. 

    @todo Is it true that MoHex will resist in the strongest way
    possible?
*/
bool MoHexPlayer::PerformPreSearch(HexBoard& brd, HexColor color, 
                                   bitset_t& consider, 
                                   PointSequence& winningSequence)
{
   
    bitset_t losing;
    HexColor other = !color;
    PointSequence seq;
    bool foundWin = false;
    for (BitsetIterator p(consider); p && !foundWin; ++p) 
    {
        brd.PlayMove(color, *p);
        seq.push_back(*p);

        // Found a winning move!
        if (PlayerUtils::IsLostGame(brd, other))
        {
            winningSequence = seq;
            foundWin = true;
        }	
        else if (PlayerUtils::IsWonGame(brd, other))
            losing.set(*p);

        seq.pop_back();
        brd.UndoMove();
    }

    // Abort if we found a one-move win
    if (foundWin)
        return true;

    // Backing up cannot cause this to happen, right? 
    HexAssert(!PlayerUtils::IsDeterminedState(brd, color));

    // Use the backed-up ice info to shrink the moves to consider
    if (m_backup_ice_info) 
    {
        bitset_t new_consider 
            = PlayerUtils::MovesToConsider(brd, color) & consider;

        if (new_consider.count() < consider.count()) 
        {
            consider = new_consider;       
            LogFine() << "$$$$$$ new moves to consider $$$$$$" 
                      << brd.printBitset(consider) << '\n';
        }
    }

    // Subtract any losing moves from the set we consider, unless all of them
    // are losing (in which case UCT search will find which one resists the
    // loss well).
    if (losing.any()) 
    {
	if (BitsetUtil::IsSubsetOf(consider, losing)) 
        {
	    LogInfo() << "************************************" << '\n'
                      << " All UCT root children are losing!!" << '\n'
                      << "************************************" << '\n';
	} 
        else 
        {
            LogFine() << "Removed losing moves: " 
		      << brd.printBitset(losing) << '\n';
	    consider = consider - losing;
	}
    }
    HexAssert(consider.any());
    LogInfo()<< "Moves to consider:" << '\n' 
             << brd.printBitset(consider) << '\n';
    return false;
}

void MoHexPlayer::PrintParameters(HexColor color, double remaining)
{
    LogInfo() << "--- MoHexPlayer::Search() ---" << '\n'
	      << "Color: " << color << '\n'
	      << "MaxGames: " << m_max_games << '\n'
	      << "MaxTime: " << m_max_time << '\n'
	      << "NumberThreads: " << m_search.NumberThreads() << '\n'
	      << "MaxNodes: " << m_search.MaxNodes()
	      << " (" << sizeof(SgUctNode)*m_search.MaxNodes() << " bytes)" 
	      << '\n'
	      << "TimeRemaining: " << remaining << '\n';
}

/** @bug CURRENTLY BROKEN!  
    
    Need to do a few things before subtrees can be reused:
    
    1) Deal with new root-state knowledge. SgUctSearch has a
    rootfilter that prunes moves in the root and is applied when it is
    passed an initial tree, so we can use that to apply more pruning
    to the new root node. Potential problem if new root knowledge adds
    moves that weren't present when knowledge was computed in the
    tree, but I don't think this would happen very often (or matter).
*/
SgUctTree* MoHexPlayer::TryReuseSubtree(const HexUctSharedData& oldData,
                                        HexUctSharedData& newData)
{
    // Must have knowledge on to reuse subtrees, since root has fillin
    // knowledge which affects the tree below.
    if (!m_search.KnowledgeThreshold())
    {
        LogInfo() << "ReuseSubtree: knowledge is off." << '\n';
        return 0;
    }

    LogSevere() << "\"param_mohex reuse_subtree\" is currently broken!" << '\n'
                << "Please see MoHexPlayer::TryReuseSubtree() in "
                << "the documentation." << '\n';

    const MoveSequence& oldSequence = oldData.game_sequence;
    const MoveSequence& newSequence = newData.game_sequence;

    // If no old knowledge for the current position, then we cannot
    // reuse the tree (since the root is given its knowledge and using
    // this knowledge would require pruning the trees under the root's
    // children). It is easier to just throw away the tree, since it
    // must be pretty small.
    {
        HexUctStoneData oldStateData;
        hash_t hash = SequenceHash::Hash(newSequence);
        if (!oldData.stones.get(hash, oldStateData))
        {
            LogInfo() << "ReuseSubtree: No knowledge for state in old tree."
                      << '\n';
            return 0;
        }

        // Check that the old knowledge is equal to the new knowledge
        // in the would-be root node.
        HexAssert(oldStateData == newData.root_stones);
    }

    LogInfo() << "Old: " << oldSequence << '\n';
    LogInfo() << "New: "<< newSequence << '\n';

    if (oldSequence.size() > newSequence.size())
    {
        LogInfo() << "ReuseSubtree: Backtracked to an earlier state." << '\n';
        return 0;
    }

    std::size_t prefixLen = 0;
    if (!oldSequence.empty())
    {
        CommonPrefix(oldSequence, newSequence, prefixLen);
        if (prefixLen == 0)
        {
            LogInfo() << "ReuseSubtree: No common prefix." << '\n';
            return 0;
        }
    }

    // Ensure alternating colors and extract suffix
    MoveSequence suffix;
    std::vector<SgMove> sequence;
    for (std::size_t i = prefixLen; i < newSequence.size(); ++i)
    {
        if (i && newSequence[i-1].color() == newSequence[i].color())
        {
            LogInfo() << "ReuseSubtree: Colors do not alternate." << '\n';
            return 0;
        }
        suffix.push_back(newSequence[i]);
        sequence.push_back(newSequence[i].point());
    }
    LogInfo() << "MovesPlayed: " << suffix << '\n';
    
    // @todo Return original tree if in same state as before?
    if (!suffix.size())
        return 0;

    // Extract the tree
    SgUctTree& tree = m_search.GetTempTree();
    SgUctTreeUtil::ExtractSubtree(m_search.Tree(), tree, sequence, true, 10.0);

    std::size_t newTreeNodes = tree.NuNodes();
    std::size_t oldTreeNodes = m_search.Tree().NuNodes();
    if (oldTreeNodes > 1 && newTreeNodes > 1)
    {
        // Fix root's children to be those in the consider set
        std::vector<SgMove> moves;
        for (BitsetIterator it(newData.root_consider); it; ++it)
            moves.push_back(static_cast<SgMove>(*it));
        tree.SetChildren(0, tree.Root(), moves);

//         for (SgUctChildIterator it(tree, tree.Root()); it; ++it)
//         {
//             LogInfo() << '(' << static_cast<HexPoint>((*it).Move())
//                       << ", " << (*it).MoveCount() << ')';
//         }
//         LogInfo() << '\n';

        float reuse = static_cast<float>(newTreeNodes) / oldTreeNodes;
        int reusePercent = static_cast<int>(100 * reuse);
        LogInfo() << "MoHexPlayer: Reusing " << newTreeNodes
                  << " nodes (" << reusePercent << "%)" << '\n';

        MoveSequence moveSequence = newSequence;
        CopyKnowledgeData(tree, tree.Root(), newData.root_to_play,
                          moveSequence, oldData, newData);
        float kReuse = static_cast<float>(newData.stones.count())
            / oldData.stones.count();
        int kReusePercent = static_cast<int>(100 * kReuse);
        LogInfo() << "MoHexPlayer: Reusing " 
                  << newData.stones.count() << " knowledge states ("
                  << kReusePercent << "%)" << '\n';
        return &tree;
    }
    return 0;
}

void MoHexPlayer::CopyKnowledgeData(const SgUctTree& tree,
                                    const SgUctNode& node,
                                    HexColor color, MoveSequence& sequence,
                                    const HexUctSharedData& oldData,
                                    HexUctSharedData& newData) const
{
    hash_t hash = SequenceHash::Hash(sequence);
    HexUctStoneData stones;
    if (!oldData.stones.get(hash, stones))
        return;
    newData.stones.put(hash, stones);
    if (!node.HasChildren())
        return;
    for (SgUctChildIterator it(tree, node); it; ++it)
    {
        sequence.push_back(Move(color, static_cast<HexPoint>((*it).Move())));
        CopyKnowledgeData(tree, *it, !color, sequence, oldData, newData);
        sequence.pop_back();
    }
}

//----------------------------------------------------------------------------
