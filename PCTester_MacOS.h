#pragma once
#include <iostream>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <thread>
#include <atomic>
#include <iomanip>

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
    void gpu_benchmark();
    void monitor_temperatures(std::atomic<bool>& stop_monitoring);
    
    double get_cpu_temperature();
    double get_gpu_temperature();
};

// Implementation
void PCTester::Impl::collect_system_info() {
    // Get OS info
    char os_version[256];
    size_t size = sizeof(os_version);
    if (sysctlbyname("kern.osrelease", os_version, &size, NULL, 0) == 0) {
        sys_info.os_name = "macOS " + std::string(os_version);
    }
    
    // Get CPU info
    char cpu_brand[256];
    size = sizeof(cpu_brand);
    sysctlbyname("machdep.cpu.brand_string", cpu_brand, &size, NULL, 0);
    sys_info.cpu_name = cpu_brand;
    
    int cores;
    size = sizeof(cores);
    sysctlbyname("machdep.cpu.core_count", &cores, &size, NULL, 0);
    sys_info.cpu_cores = cores;
    
    int threads;
    size = sizeof(threads);
    sysctlbyname("machdep.cpu.thread_count", &threads, &size, NULL, 0);
    sys_info.cpu_threads = threads;
    
    // Get memory info
    uint64_t mem_size;
    size = sizeof(mem_size);
    sysctlbyname("hw.memsize", &mem_size, &size, NULL, 0);
    sys_info.memory_size = mem_size;
    
    // Get GPU info
    // ... (using IOKit)
}

void PCTester::Impl::gpu_benchmark() {
    SafeOutput::print("\n[GPU] Starting Metal benchmark...");
    
    // Initialize Metal
    // ... (complex Metal initialization and benchmarking)
    
    SafeOutput::print("[GPU] Metal benchmark completed");
}
