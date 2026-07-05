#include <iostream>

using std::cout;
using std::cin;
using std::stoi;

extern int *trench();

int main(int argc, const char *argv[])
{
    int stormfly = stoi(argv[1]);
    int trench_power = *trench();

    cout << "Power of stormfly is " << stormfly << '\n';
    cout << "Power of trench is " << trench_power << '\n';
    cout << "Winner is " << (stormfly >= trench_power ? "stormfly" : "trench") << '\n';

    return 0;
}