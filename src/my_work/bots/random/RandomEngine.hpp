#ifndef RANDOMENGINE_HPP
#define RANDOMENGINE_HPP

#include "CommonHtpEngine.hpp"

_BEGIN_BENZENE_NAMESPACE_

class RandomEngine : public CommonHtpEngine {
    public:
        RandomEngine(int boardsize);

        ~RandomEngine();

    private:
        HexPoint GenMove(HexColor color, bool useGameClock);
    };

_END_BENZENE_NAMESPACE_

#endif //RANDOMENGINE_HPP
