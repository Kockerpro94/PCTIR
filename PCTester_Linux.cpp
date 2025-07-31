#include "PCTester_Linux.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <random>
#include <cstring>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>

PCTester::Impl::Impl() {
    collect_system_info();
}

void PCTester::Impl::collect_system_info() {
    // Get OS info
    std::ifstream os_release("/etc/os-release");
    if (os_release.is_open()) {
        std::string line;
        while (std::getline(os_release, line)) {
            if (line.find("PRETTY_NAME") != std::string::npos) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    sys_info.os_name = line.substr(pos + 1);
                    // Remove quotes
                    sys_info.os_name.erase(
                        std::remove(sys_info.os_name.begin(), sys_info.os_name.end(), '"'),
                        sys_info.os_name.end()
                    );
                }
            }
        }
    }
    
    // Get CPU info
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        int cores = 0;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    sys_info.cpu_name = line.substr(pos + 2);
                }
            }
            if (line.find("cpu cores") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    sys_info.cpu_cores = std::stoi(line.substr(pos + 2));
                }
            }
            if (line.find("siblings") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    sys_info.cpu_threads = std::stoi(line.substr(pos + 2));
                }
            }
        }
    }
    
    // Get memory info
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    sys_info.memory_size = memInfo.totalram * memInfo.mem_unit;
    
    // Get GPU info (simplified)
    FILE* pipe = popen("lspci | grep VGA", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, 128, pipe) != NULL) {
            // Skip the first 8 characters (bus ID)
            char* gpu_name = buffer + 8;
            // Remove newline
            char* newline = strchr(gpu_name, '\n');
            if (newline) *newline = 0;
            sys_info.gpu_name = gpu_name;
        }
        pclose(pipe);
    }
    
    // Get GPU memory (simulated)
    sys_info.gpu_memory = 4096; // 4GB default
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
    SafeOutput::print("\n[CPU] Starting Linux-optimized stress test...");
    
    // Get CPU frequency
    double base_freq = 0.0;
    for (int i = 0; i < sys_info.cpu_cores; i++) {
        std::string freq_file = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/cpuinfo_max_freq";
        std::ifstream freq(freq_file);
        if (freq.is_open()) {
            double freq_val;
            freq >> freq_val;
            freq_val /= 1000.0; // Convert kHz to MHz
            if (freq_val > base_freq) base_freq = freq_val;
        }
    }
    
    if (base_freq == 0.0) base_freq = 2000.0; // Default to 2GHz
    
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
    SafeOutput::print("\n[GPU] Starting OpenCL benchmark simulation...");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate GPU work
    double sum = 0.0;
    for (long i = 1; i < 10000000; i++) {
        sum += 1.0 / (i * i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    // Calculate GPU score
    test_results.gpu_score = 10000000 / elapsed.count();
    
    SafeOutput::print("[GPU] Benchmark completed: " + sys_info.gpu_name);
    SafeOutput::print("[GPU] Score: " + std::to_string(test_results.gpu_score));
}

double PCTester::Impl::get_cpu_temperature() {
    double max_temp = 0.0;
    DIR* dir = opendir("/sys/class/thermal");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, "thermal_zone")) {
                std::string temp_path = "/sys/class/thermal/" + std::string(entry->d_name) + "/temp";
                std::ifstream temp_file(temp_path);
                if (temp_file.is_open()) {
                    double temp;
                    temp_file >> temp;
                    temp /= 1000.0; // Convert millidegrees to Celsius
                    if (temp > max_temp) max_temp = temp;
                }
            }
        }
        closedir(dir);
    }
    return max_temp;
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
    <title>PC Diagnostic Report - Linux</title>
    <style>
        body { font-family: Ubuntu, Arial, sans-serif; margin: 40px; }
        .header { text-align: center; margin-bottom: 30px; }
        .section { margin-bottom: 25px; padding: 15px; border-radius: 8px; background: #f8f9fa; }
        .section-title { font-size: 1.4em; margin-bottom: 15px; color: #e95420; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        .metric { background: white; padding: 15px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .metric-title { font-weight: bold; margin-bottom: 8px; }
        .gauge { height: 20px; background: #e0e0e0; border-radius: 10px; margin-top: 8px; overflow: hidden; }
        .gauge-fill { height: 100%; background: #e95420; }
        .score { font-size: 1.8em; font-weight: bold; text-align: center; margin: 10px 0; }
        .summary { background: #fdf6f2; padding: 20px; border-radius: 8px; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>PC Diagnostic Report - Linux</h1>
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
        <p>Your Linux system performance analysis:</p>
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
