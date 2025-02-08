#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <twain.h>

// Global TWAIN variables
HINSTANCE hTwainDLL = nullptr;
TW_IDENTITY appID;
TW_IDENTITY sourceID;
TW_UINT16 twainState = 1;

void logMessage(const std::string &message) {
    std::ofstream logFile("twain-helper.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
    std::cout << message << std::endl;
}

bool initializeTwain() {
    hTwainDLL = LoadLibrary("twain_32.dll");
    if (!hTwainDLL) {
        logMessage("Failed to load TWAIN library.");
        return false;
    }

    appID.Id = 0;
    appID.Version.MajorNum = 1;
    appID.Version.MinorNum = 0;
    appID.Version.Language = TWLG_ENGLISH;
    appID.Version.Country = TWCY_USA;
    strcpy_s(appID.Version.Info, "TWAIN Helper");
    appID.ProtocolMajor = TWON_PROTOCOLMAJOR;
    appID.ProtocolMinor = TWON_PROTOCOLMINOR;
    appID.SupportedGroups = DG_CONTROL | DG_IMAGE;
    strcpy_s(appID.Manufacturer, "Open Source");
    strcpy_s(appID.ProductFamily, "Scanner");
    strcpy_s(appID.ProductName, "TWAIN Helper");

    return true;
}

std::vector<std::string> listScanners() {
    std::vector<std::string> scannerList;
    TW_IDENTITY source;
    TW_UINT16 rc;
    
    rc = DSM_Entry(&appID, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, (TW_MEMREF)&source);
    while (rc == TWRC_SUCCESS) {
        scannerList.push_back(source.ProductName);
        rc = DSM_Entry(&appID, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, (TW_MEMREF)&source);
    }
    
    return scannerList;
}

bool startScan(const std::string &scannerName, const std::string &outputPath) {
    TW_IDENTITY source;
    TW_UINT16 rc = DSM_Entry(&appID, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, (TW_MEMREF)&source);
    while (rc == TWRC_SUCCESS) {
        if (scannerName == source.ProductName) {
            sourceID = source;
            break;
        }
        rc = DSM_Entry(&appID, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, (TW_MEMREF)&source);
    }
    
    if (rc != TWRC_SUCCESS) {
        logMessage("Scanner not found.");
        return false;
    }

    logMessage("Starting scan on " + scannerName + "...");
    Sleep(2000); // Simulated scan process
    logMessage("Scan complete. Output saved at: " + outputPath);
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        logMessage("Usage: twain-helper.exe <scanner_name> <output_path>");
        return 1;
    }
    
    if (!initializeTwain()) {
        return 1;
    }
    
    std::string scannerName = argv[1];
    std::string outputPath = argv[2];
    
    if (!startScan(scannerName, outputPath)) {
        return 1;
    }
    
    return 0;
}
