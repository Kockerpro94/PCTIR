#pragma once

#include "PCTester.h"
#include <windows.h>
#include <wincrypt.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class PCTester::Impl {
public:
    Impl();
    ~Impl();
    
    void run_full_diagnostics();
    void generate_html_report(const std::string& filename) const;
    
private:
    SystemInfo sys_info;
    TestResults test_results;
    
    void collect_system_info();
    void cpu_benchmark();
    void ram_test();
    void disk_test();
    void network_test();
    void gpu_benchmark();
    void monitor_temperatures(std::atomic<bool>& stop_monitoring);
    
    std::string wstring_to_string(const std::wstring& wstr) const;
    
    double get_cpu_temperature();
    double get_gpu_temperature();
};
