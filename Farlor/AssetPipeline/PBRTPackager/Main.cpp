using namespace std;

#include <PBRTParser.h>

#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace Farlor;

int main(int argc, char** argv)
{
    assert(argc == 2);

    PBRTParser pbrtParser;
    pbrtParser.ParseScene(argv[1]);

    return 0;
}