#include "minmax.hpp"

using namespace std;

int main()
{
	Connect4AI* ai = new Connect4AI(7);
	ai->game();
	return 0;
}
