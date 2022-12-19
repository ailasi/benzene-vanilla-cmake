#include "config.h"
#include "Misc.hpp"
#include "RandomEngine.hpp"
#include "CommonProgram.hpp"
#include "SwapCheck.hpp"

using namespace benzene;


int main(int argc, char** argv)
{
    MiscUtil::FindProgramDir(argc, argv);

    CommonProgram com;
    com.Shutdown();

    CommonProgram program;
    BenzeneEnvironment::Get().RegisterProgram(program);
    program.Initialize(argc, argv);
    try
    {
        RandomEngine gh(program.BoardSize());
        std::string config = program.ConfigFileToExecute();
        if (config != "")
            gh.ExecuteFile(config);
        GtpInputStream gin(std::cin);
        GtpOutputStream gout(std::cout);
        gh.MainLoop(gin, gout);

        program.Shutdown();
    }
    catch (const GtpFailure& f)
    {
        std::cerr << f.Response() << std::endl;
        return 1;
    }
    return 0;
}
