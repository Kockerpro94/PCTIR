#pragma once

#include "PCTester.h"
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <cmath>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <random>

class PCTester::Impl {
public:
    Impl();
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
    
    double get_cpu_temperature();
    double get_gpu_temperature();
    double get_cpu_usage();
};
