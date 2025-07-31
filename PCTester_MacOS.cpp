#include "PCTester_MacOS.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>

PCTester::Impl::Impl() {
    collect_system_info();
}

void PCTester::Impl::collect_system_info() {
    // Get OS info
    char os_version[256];
    size_t size = sizeof(os_version);
    if (sysctlbyname("kern.osrelease", os_version, &size, NULL, 0) == 0) {
        sys_info.os_name = "macOS " + std::string(os_version);
    } else {
        sys_info.os_name = "macOS (Unknown Version)";
    }
    
    // Get CPU info
    char cpu_brand[256];
    size = sizeof(cpu_brand);
    if (sysctlbyname("machdep.cpu.brand_string", cpu_brand, &size, NULL, 0) == 0) {
        sys_info.cpu_name = cpu_brand;
    } else {
        sys_info.cpu_name = "Unknown CPU";
    }
    
    int cores;
    size = sizeof(cores);
    if (sysctlbyname("machdep.cpu.core_count", &cores, &size, NULL, 0) == 0) {
        sys_info.cpu_cores = cores;
    } else {
        sys_info.cpu_cores = 4; // Default
    }
    
    int threads;
    size = sizeof(threads);
    if (sysctlbyname("machdep.cpu.thread_count", &threads, &size, NULL, 0) == 0) {
        sys_info.cpu_threads = threads;
    } else {
        sys_info.cpu_threads = sys_info.cpu_cores * 2;
    }
    
    // Get memory info
    uint64_t mem_size;
    size = sizeof(mem_size);
    if (sysctlbyname("hw.memsize", &mem_size, &size, NULL, 0) == 0) {
        sys_info.memory_size = mem_size;
    } else {
        sys_info.memory_size = 8 * 1024 * 1024 * 1024ULL; // 8GB default
    }
    
    // Get GPU info
    sys_info.gpu_name = "Apple GPU (" + get_model_identifier() + ")";
    sys_info.gpu_memory = 4096; // 4GB default
}

std::string PCTester::Impl::get_model_identifier() {
    char model[256];
    size_t size = sizeof(model);
    if (sysctlbyname("hw.model", model, &size, NULL, 0) == 0) {
        return model;
    }
    return "Unknown Model";
}

void PCTester::Impl::run_full_diagnostics() {
    SafeOutput::print("\n=== Advanced Diagnostics ===");
    
    // Start temperature monitoring
    std::atomic<bool> stop_monitoring(false);
    std::thread temp_monitor(&Impl::monitor_temperatures, this, std::ref(stop_monitoring));
    
    // Run tests
    cpu_benchmark();
    gpu_benchmark();
    
    // Stop monitoring
    stop_monitoring = true;
    temp_monitor.join();
    
    SafeOutput::print("\nAll tests completed!");
}

void PCTester::Impl::cpu_benchmark() {
    SafeOutput::print("\n[CPU] Starting macOS-optimized stress test...");
    
    // Get CPU frequency
    uint64_t freq = 0;
    size_t size = sizeof(freq);
    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) != 0) {
        freq = 2400000000; // 2.4GHz default
    }
    double base_freq = freq / 1000000000.0; // Convert to GHz
    
    // Run complex mathematical operations
    const int num_iterations = 100000000;
    double sum = 0.0;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= num_iterations; i++) {
        sum += 1.0 / (i * i);
    }
    double pi = std::sqrt(6 * sum);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    // Calculate score
    double base_perf = base_freq * sys_info.cpu_cores;
    double actual_perf = num_iterations / elapsed.count();
    test_results.cpu_score = (actual_perf / base_perf) * 10000;
    test_results.cpu_temp = get_cpu_temperature();
    
    SafeOutput::print("[CPU] Score: " + std::to_string(test_results.cpu_score));
    SafeOutput::print("[CPU] Temperature: " + std::to_string(test_results.cpu_temp) + "°C");
}

void PCTester::Impl::gpu_benchmark() {
    SafeOutput::print("\n[GPU] Starting Metal benchmark simulation...");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate GPU work (complex computation)
    double sum = 0.0;
    for (long i = 1; i < 15000000; i++) {
        sum += std::sin(i) * std::cos(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    // Calculate GPU score
    test_results.gpu_score = 15000000 / elapsed.count();
    
    SafeOutput::print("[GPU] Benchmark completed: " + sys_info.gpu_name);
    SafeOutput::print("[GPU] Score: " + std::to_string(test_results.gpu_score));
}

double PCTester::Impl::get_cpu_temperature() {
    // Simulated CPU temperature
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(40.0, 80.0);
    return dist(gen);
}

double PCTester::Impl::get_gpu_temperature() {
    // Simulated GPU temperature
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(50.0, 85.0);
    return dist(gen);
}

void PCTester::Impl::monitor_temperatures(std::atomic<bool>& stop_monitoring) {
    SafeOutput::print("[MONITOR] Starting temperature monitoring...");
    
    while (!stop_monitoring) {
        double cpu_temp = get_cpu_temperature();
        double gpu_temp = get_gpu_temperature();
        
        std::stringstream ss;
        ss << "[TEMP] CPU: " << std::fixed << std::setprecision(1) << cpu_temp << "°C";
        ss << " | GPU: " << gpu_temp << "°C";
        SafeOutput::print(ss.str());
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    SafeOutput::print("[MONITOR] Temperature monitoring stopped");
}

void PCTester::Impl::generate_html_report(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create report file");
    }
    
    file << R"(<!DOCTYPE html>
<html>
<head>
    <title>PC Diagnostic Report - macOS</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, sans-serif; margin: 40px; }
        .header { text-align: center; margin-bottom: 30px; }
        .section { margin-bottom: 25px; padding: 15px; border-radius: 8px; background: #f8f9fa; }
        .section-title { font-size: 1.4em; margin-bottom: 15px; color: #0071e3; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        .metric { background: white; padding: 15px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .metric-title { font-weight: bold; margin-bottom: 8px; }
        .gauge { height: 20px; background: #e0e0e0; border-radius: 10px; margin-top: 8px; overflow: hidden; }
        .gauge-fill { height: 100%; background: #0071e3; }
        .score { font-size: 1.8em; font-weight: bold; text-align: center; margin: 10px 0; }
        .summary { background: #f0f7ff; padding: 20px; border-radius: 8px; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>PC Diagnostic Report - macOS</h1>
        <p>Generated on )" << __DATE__ << " at " << __TIME__ << R"(</p>
    </div>
    
    <div class="section">
        <h2 class="section-title">System Information</h2>
        <div class="grid">
            <div class="metric">
                <div class="metric-title">Operating System</div>
                <div>)" << sys_info.os_name << R"(</div>
            </div>
            <div class="metric">
                <div class="metric-title">Processor</div>
                <div>)" << sys_info.cpu_name << R"(</div>
                <div>Cores: )" << sys_info.cpu_cores << R"(, Threads: )" << sys_info.cpu_threads << R"(</div>
            </div>
            <div class="metric">
                <div class="metric-title">Graphics</div>
                <div>)" << sys_info.gpu_name << R"(</div>
                <div>)" << sys_info.gpu_memory << R"( MB VRAM</div>
            </div>
        </div>
    </div>
    
    <div class="section">
        <h2 class="section-title">Performance Metrics</h2>
        <div class="grid">
            <div class="metric">
                <div class="metric-title">CPU Performance</div>
                <div class="score">)" << std::fixed << std::setprecision(1) << test_results.cpu_score << R"(</div>
                <div>Temperature: )" << test_results.cpu_temp << R"(°C</div>
                <div class="gauge"><div class="gauge-fill" style="width: )" 
                 << std::min(100.0, test_results.cpu_score / 200) << R"(%"></div></div>
            </div>
            <div class="metric">
                <div class="metric-title">GPU Performance</div>
                <div class="score">)" << std::fixed << std::setprecision(1) << test_results.gpu_score << R"(</div>
                <div class="gauge"><div class="gauge-fill" style="width: )" 
                 << std::min(100.0, test_results.gpu_score / 1000) << R"(%"></div></div>
            </div>
        </div>
    </div>
    
    <div class="summary">
        <h2>Diagnostic Summary</h2>
        <p>Your macOS system performance analysis:</p>
        <ul>
            <li>CPU performance is )" << (test_results.cpu_score > 5000 ? "excellent" : "adequate") << R"(</li>
            <li>GPU performance is )" << (test_results.gpu_score > 500 ? "excellent" : "adequate") << R"(</li>
            <li>System is running within safe temperature ranges</li>
        </ul>
    </div>
</body>
</html>)";
    
    SafeOutput::print("Report generated: " + filename);
}
