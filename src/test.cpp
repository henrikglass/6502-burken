#ifdef TEST

#include "nlohmann/json.hpp"
#include "m6502.h"
#include "memory.h"

#include <iostream>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

template <typename T>
bool run_test(const T &it)
{
    Memory mem;
    Cpu cpu(mem);

    // read test data
    std::string test_name = (*it)["name"];
    auto initial_state    = (*it)["initial"];
    auto final_state      = (*it)["final"];

    // set initial cpu & memory state
    cpu.PC  = initial_state["pc"];
    cpu.SR  = initial_state["p"]; // odd naming but ok
    cpu.ACC = initial_state["a"];
    cpu.X   = initial_state["x"];
    cpu.Y   = initial_state["y"];
    cpu.SP  = initial_state["s"];
    auto init_mem_state = initial_state["ram"];
    for (auto mem_it = init_mem_state.begin(); mem_it != init_mem_state.end(); mem_it++) {
        u16 addr = (*mem_it)[0];
        u8  val  = (*mem_it)[1];
        mem[addr] = val;
    }
    
    bool dbg_halt = mem[cpu.PC] == 0x40;

    //`n_cycles` == 0 means that op-code is illegal. We don't care about illegal opcodes.
    if (instruction_table[mem[cpu.PC]].n_cycles == 0)
        return true;

    // we don't care about decimal mode, for now.
    if (cpu.SR & 0b10000)
        return true;

    // execute instruction
    cpu.fetch_execute_next();

    // compare with final cpu state
    bool pass = true; 
    pass = pass && (cpu.PC  == final_state["pc"]);
    pass = pass && (cpu.SR  == final_state["p"]);
    pass = pass && (cpu.ACC == final_state["a"]);
    pass = pass && (cpu.X   == final_state["x"]);
    pass = pass && (cpu.Y   == final_state["y"]);
    pass = pass && (cpu.SP  == final_state["s"]);
    auto final_mem_state = final_state["ram"];
    for (auto mem_it = final_mem_state.begin(); mem_it != final_mem_state.end(); mem_it++) {
        u16 addr = (*mem_it)[0];
        u8  val  = (*mem_it)[1];
        pass = pass && (mem[addr] == val);
    }

    // TODO compare # of cycles

    if (dbg_halt && !pass && !false) {
        std::cout << "\n\ntest name: " << test_name << std::endl;

        std::cout << "\n\ninitial state:" << std::endl;
        std::cout << "\tpc:  0x" << std::hex << (int)initial_state["pc"] << std::dec<< std::endl;
        std::cout << "\tsr:  0x" << std::hex << (int)initial_state["p"]  << std::dec<< std::endl;
        std::cout << "\tacc: 0x" << std::hex << (int)initial_state["a"]  << std::dec<< std::endl;
        std::cout << "\tx:   0x" << std::hex << (int)initial_state["x"]  << std::dec<< std::endl;
        std::cout << "\ty:   0x" << std::hex << (int)initial_state["y"]  << std::dec<< std::endl;
        std::cout << "\tsp:  0x" << std::hex << (int)initial_state["s"]  << std::dec<< std::endl;
        for (auto mem_it = init_mem_state.begin(); mem_it != init_mem_state.end(); mem_it++) {
            u16 addr = (*mem_it)[0];
            u8  val  = (*mem_it)[1];
            std::cout << std::hex << "\tmem[0x" << addr << "] = 0x" << (int)val << std::dec << std::endl;
        }
        
        std::cout << "\n\nfinal state:" << std::endl;
        std::cout << "\tpc:  0x" << std::hex << (int)final_state["pc"] << std::dec<< std::endl;
        std::cout << "\tsr:  0x" << std::hex << (int)final_state["p"]  << std::dec<< std::endl;
        std::cout << "\tacc: 0x" << std::hex << (int)final_state["a"]  << std::dec<< std::endl;
        std::cout << "\tx:   0x" << std::hex << (int)final_state["x"]  << std::dec<< std::endl;
        std::cout << "\ty:   0x" << std::hex << (int)final_state["y"]  << std::dec<< std::endl;
        std::cout << "\tsp:  0x" << std::hex << (int)final_state["s"]  << std::dec<< std::endl;
        for (auto mem_it = final_mem_state.begin(); mem_it != final_mem_state.end(); mem_it++) {
            u16 addr = (*mem_it)[0];
            u8  val  = (*mem_it)[1];
            std::cout << std::hex << "\tmem[0x" << addr << "] = 0x" << (int)val << std::dec << std::endl;
        }
        
        std::cout << "\n\nactual state:" << std::endl;
        std::cout << "\tpc:  0x" << std::hex << (int)cpu.PC  << std::dec << std::endl;
        std::cout << "\tsr:  0x" << std::hex << (int)cpu.SR  << std::dec << std::endl;
        std::cout << "\tacc: 0x" << std::hex << (int)cpu.ACC << std::dec << std::endl;
        std::cout << "\tx:   0x" << std::hex << (int)cpu.X   << std::dec << std::endl;
        std::cout << "\ty:   0x" << std::hex << (int)cpu.Y   << std::dec << std::endl;
        std::cout << "\tsp:  0x" << std::hex << (int)cpu.SP  << std::dec << std::endl;
        for (auto mem_it = final_mem_state.begin(); mem_it != final_mem_state.end(); mem_it++) {
            u16 addr = (*mem_it)[0];
            std::cout << std::hex << "\tmem[0x" << addr << "] = 0x" << (int)mem[addr] << std::dec << std::endl;
        }

        exit(0);
    }

    return pass;
}

int main()
{
    std::cout << "Running tests:" << std::endl;

    int n_instr_total = 0;
    int n_instr_passed = 0;
    for (const auto &entry : std::filesystem::directory_iterator("tests")) {
        std::ifstream f(entry.path());
        json data = json::parse(f);

        std::cout << "Running tests in `" << entry.path() << "`";
        int n_tests_passed = 0;
        int n_tests_total = 0;
        for (auto it = data.begin(); it != data.end(); it++) {
            //std::cout << n_tests_total << std::endl;
            if (run_test(it))
                n_tests_passed++;
            n_tests_total++;
        }
        std::cout << ": " << n_tests_passed << "/" << n_tests_total;
        std::cout << ((n_tests_passed == n_tests_total) ? 
                        "\t[\033[1;32m✓\033[0m]" : 
                        "\t[\033[1;31m×\033[0m]") << std::endl;

        if (n_tests_passed == n_tests_total)
            n_instr_passed++;
        n_instr_total++;
    }
    
    printf("\n\n\t%d/%d instructions passed.\n\n", n_instr_passed, n_instr_total);
   
}

#endif
