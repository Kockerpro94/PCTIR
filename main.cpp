#include "PCTester.h"
#include <iostream>

int main() {
    PCTester tester;
    SafeOutput::print("=== Advanced PCTester v3.0 ===");
    SafeOutput::print("Starting comprehensive hardware diagnostics...");
    
    try {
        tester.run_full_diagnostics();
        tester.generate_html_report("diagnostic_report.html");
        SafeOutput::print("\nDiagnostics completed successfully!");
    } catch (const std::exception& e) {
        SafeOutput::print("\nERROR: " + std::string(e.what()));
    }
    
    SafeOutput::print("Press Enter to exit...");
    std::cin.get();
    return 0;
}
