#include "app.hpp"
#include "../common/paths.hpp"
int main(int argc, char* argv[]) {
    try { library::run(coursework::dataPath(argc, argv)); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
