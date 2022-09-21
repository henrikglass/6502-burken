#ifdef TEST

#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

int main()
{
    std::cout << "Hello World!" << std::endl;

    json data = json::parse();
}

#endif
