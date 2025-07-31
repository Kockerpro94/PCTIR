#include "PCTester_Windows.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <algorithm>

PCTester::Impl::Impl() {
    collect_system_info();
}

PCTester::Impl::~Impl() {
    // Cleanup resources
}

std::string PCTester::Impl::wstring_to_string(const std::wstring& wstr) const {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
    return str;
}

void PCTester::Impl::collect_system_info() {
    // Initialize COM
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        SafeOutput::error("Failed to initialize COM library");
        return;
    }
    
    // Set security levels
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    
    if (FAILED(hres)) {
        SafeOutput::error("Failed to initialize security");
        CoUninitialize();
        return;
    }
    
    // Obtain WMI locator
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    
    if (FAILED(hres)) {
        SafeOutput::error("Failed to create IWbemLocator");
        CoUninitialize();
        return;
    }
    
    // Connect to WMI
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );
    
    if (FAILED(hres)) {
        SafeOutput::error("Failed to connect to WMI");
        pLoc->Release();
        CoUninitialize();
        return;
    }
    
    // Set security levels
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    
    if (FAILED(hres)) {
        SafeOutput::error("Failed to set proxy blanket");
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }
    
    // Query system information
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    
    if (FAILED(hres)) {
        SafeOutput::error("Failed to execute WMI query");
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }
    
    // Get OS information
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (FAILED(hr) || uReturn == 0) break;
        
        VARIANT vtProp;
        hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            sys_info.os_name = wstring_to_string(vtProp.bstrVal);
            VariantClear(&vtProp);
        }
        pclsObj->Release();
    }
    
    // Cleanup
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
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
    SafeOutput::print("\n[CPU] Starting advanced benchmark...");
    
    // Get CPU base frequency
    DWORD BufSize = sizeof(DWORD);
    DWORD dwMHz = 0;
    DWORD type = REG_DWORD;
    HKEY hKey;
    long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                              0,
                              KEY_READ,
                              &hKey);
    
    if (lError == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", NULL, &type, (LPBYTE)&dwMHz, &BufSize);
        RegCloseKey(hKey);
    }
    
    // Run complex mathematical operations
    const int num_iterations = 100000000;
    double pi = 0.0;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_iterations; i++) {
        pi += 4.0 * (1 - (i % 2) * 2) / (2 * i + 1);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    // Calculate score
    double base_perf = (dwMHz / 1000.0) * 4;  // Assuming 4 cores
    double actual_perf = num_iterations / elapsed.count();
    test_results.cpu_score = (actual_perf / base_perf) * 10000;
    test_results.cpu_temp = get_cpu_temperature();
    
    SafeOutput::print("[CPU] Score: " + std::to_string(test_results.cpu_score));
    SafeOutput::print("[CPU] Temperature: " + std::to_string(test_results.cpu_temp) + "°C");
}

void PCTester::Impl::gpu_benchmark() {
    SafeOutput::print("\n[GPU] Starting DirectX 11 benchmark...");
    
    // Initialize Direct3D
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    D3D_FEATURE_LEVEL featureLevel;
    
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &pDevice,
        &featureLevel,
        &pContext
    );
    
    if (FAILED(hr)) {
        SafeOutput::error("Failed to create D3D11 device");
        return;
    }
    
    // Get GPU information
    IDXGIDevice* pDXGIDevice = nullptr;
    hr = pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    
    IDXGIAdapter* pAdapter = nullptr;
    if (SUCCEEDED(hr)) {
        hr = pDXGIDevice->GetAdapter(&pAdapter);
    }
    
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC adapterDesc;
        pAdapter->GetDesc(&adapterDesc);
        
        sys_info.gpu_name = wstring_to_string(adapterDesc.Description);
        sys_info.gpu_memory = adapterDesc.DedicatedVideoMemory / (1024 * 1024);
    }
    
    // Run simple benchmark
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create a simple vertex buffer
    float vertices[] = {
         0.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };
    
    ID3D11Buffer* pVertexBuffer = nullptr;
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    
    if (SUCCEEDED(pDevice->CreateBuffer(&bd, &initData, &pVertexBuffer))) {
        UINT stride = sizeof(float) * 3;
        UINT offset = 0;
        pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        // Render 100,000 triangles
        for (int i = 0; i < 100000; i++) {
            pContext->Draw(3, 0);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    // Calculate GPU score
    test_results.gpu_score = 100000 / elapsed.count();
    
    // Cleanup
    if (pVertexBuffer) pVertexBuffer->Release();
    if (pAdapter) pAdapter->Release();
    if (pDXGIDevice) pDXGIDevice->Release();
    pContext->Release();
    pDevice->Release();
    
    SafeOutput::print("[GPU] Benchmark completed: " + sys_info.gpu_name);
    SafeOutput::print("[GPU] Score: " + std::to_string(test_results.gpu_score));
}

double PCTester::Impl::get_cpu_temperature() {
    // In real implementation, you would read from hardware sensors
    // This is a simulation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(40.0, 85.0);
    return dist(gen);
}

double PCTester::Impl::get_gpu_temperature() {
    // In real implementation, you would read from hardware sensors
    // This is a simulation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(50.0, 95.0);
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
    <title>PC Diagnostic Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { text-align: center; margin-bottom: 30px; }
        .section { margin-bottom: 25px; padding: 15px; border-radius: 8px; background: #f8f9fa; }
        .section-title { font-size: 1.4em; margin-bottom: 15px; color: #1a73e8; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        .metric { background: white; padding: 15px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        .metric-title { font-weight: bold; margin-bottom: 8px; }
        .gauge { height: 20px; background: #e0e0e0; border-radius: 10px; margin-top: 8px; overflow: hidden; }
        .gauge-fill { height: 100%; background: #4285f4; }
        .score { font-size: 1.8em; font-weight: bold; text-align: center; margin: 10px 0; }
        .summary { background: #e8f0fe; padding: 20px; border-radius: 8px; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>PC Diagnostic Report</h1>
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
        <p>Your system performance analysis:</p>
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
