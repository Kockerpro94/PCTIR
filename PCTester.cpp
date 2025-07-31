#include "PCTester.h"
#include <iostream>
#include <mutex>
#include <string>

// Thread-safe console output implementation
class SafeOutputImpl {
public:
    void print(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::cout << msg << std::endl;
    }
    
    void error(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::cerr << "ERROR: " << msg << std::endl;
    }
    
private:
    std::mutex mtx_;
};

void SafeOutput::print(const std::string& msg) {
    static SafeOutputImpl instance;
    instance.print(msg);
}

void SafeOutput::error(const std::string& msg) {
    static SafeOutputImpl instance;
    instance.error(msg);
}

// Platform-specific implementation
#if defined(_WIN32)
    #include "PCTester_Windows.h"
#elif defined(__linux__)
    #include "PCTester_Linux.h"
#elif defined(__APPLE__)
    #include "PCTester_MacOS.h"
#else
    #error "Unsupported platform"
#endif

PCTester::PCTester() : pimpl(std::make_unique<Impl>()) {}
PCTester::~PCTester() = default;
void PCTester::run_full_diagnostics() { pimpl->run_full_diagnostics(); }
void PCTester::generate_html_report(const std::string& filename) const { 
    pimpl->generate_html_report(filename); 
}   
