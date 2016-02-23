

#include "PlotFactory.h"
#include "SimBuilder.h"
#include "Simulator.h"

using namespace rvs;
using namespace std;


int main(int argc, char* argv[])
{
	Simulator sim = SimBuilder::create(50, "Normal");
	


	return 0;
}