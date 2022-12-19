#include "SgSystem.h"

#include "BitsetIterator.hpp"
#include "RandomEngine.hpp"
#include "BoardUtil.hpp"


using namespace benzene;

RandomEngine::RandomEngine(int boardsize) : CommonHtpEngine(boardsize) {}

RandomEngine::~RandomEngine() {}

/** Generates a random move. */
HexPoint RandomEngine::GenMove(HexColor color, bool useGameClock)
{
    SG_UNUSED(color);
    SG_UNUSED(useGameClock);
    BoardUtil::RandomEmptyCell(m_game.Board());
}
