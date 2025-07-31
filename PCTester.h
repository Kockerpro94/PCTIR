#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cstdint>

class SafeOutput {
public:
    static void print(const std::string& msg);
    static void error(const std::string& msg);
};

struct SystemInfo {
    std::string os_name;
    std::string cpu_name;
    size_t cpu_cores;
    size_t cpu_threads;
    uint64_t memory_size;
    std::string gpu_name;
    uint64_t gpu_memory;
    std::vector<std::string> disk_names;
};

struct TestResults {
    double cpu_score;
    double cpu_temp;
    double ram_score;
    double ram_usage;
    double disk_read;
    double disk_write;
    double network_latency;
    double network_bandwidth;
    double gpu_score;
};

class PCTester {
public:
    PCTester();
    ~PCTester();
    
    void run_full_diagnostics();
    void generate_html_report(const std::string& filename) const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
