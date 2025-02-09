// twain-helper.cpp
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <twain.h>

// Define a function pointer type for DSM_Entry
typedef TW_UINT16 (WINAPI *DSM_EntryFunc)(pTW_IDENTITY, pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16, TW_MEMREF);

// Global pointer for the DSM_Entry function
DSM_EntryFunc DSM_EntryPtr = NULL;

// Log function: writes to console and a log file
void logMessage(const std::string &message) {
    std::ofstream logFile("twain-helper.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
    std::cout << message << std::endl;
}

// Load TWAINDSM.dll and retrieve DSM_Entry pointer
bool loadTwainDSM() {
    HMODULE hTwain = LoadLibrary(TEXT("TWAINDSM.dll"));
    if (!hTwain) {
        logMessage("Failed to load TWAINDSM.dll");
        return false;
    }
    DSM_EntryPtr = (DSM_EntryFunc)GetProcAddress(hTwain, "DSM_Entry");
    if (!DSM_EntryPtr) {
        logMessage("Failed to get DSM_Entry function pointer");
        return false;
    }
    return true;
}

// Enumerate available scanner sources using TWAIN
std::vector<std::string> listScanners() {
    std::vector<std::string> scanners;
    TW_IDENTITY appId;
    memset(&appId, 0, sizeof(appId));
    appId.Id = 0;
    appId.Version.MajorNum = 1;
    appId.Version.MinorNum = 0;
    appId.ProtocolMajor = TWON_PROTOCOLMAJOR;
    appId.ProtocolMinor = TWON_PROTOCOLMINOR;
    appId.SupportedGroups = DG_CONTROL | DG_IMAGE;
    strcpy_s(appId.Manufacturer, "OpenSource");
    strcpy_s(appId.ProductFamily, "TWAINHelper");
    strcpy_s(appId.ProductName, "TWAIN Helper");

    TW_IDENTITY source;
    memset(&source, 0, sizeof(source));

    // Open DSM
    TW_UINT16 rc = DSM_EntryPtr(&appId, NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, (TW_MEMREF)NULL);
    if(rc != TWRC_SUCCESS) {
        logMessage("Failed to open DSM");
        return scanners;
    }

    // Enumerate sources with MSG_GETFIRST and MSG_GETNEXT
    rc = DSM_EntryPtr(&appId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, (TW_MEMREF)&source);
    while(rc == TWRC_SUCCESS) {
        scanners.push_back(source.ProductName);
        rc = DSM_EntryPtr(&appId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, (TW_MEMREF)&source);
    }

    // Close DSM
    DSM_EntryPtr(&appId, NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, (TW_MEMREF)NULL);
    return scanners;
}

// Simulate scanning a document using the given scanner and output path.
// (In a full implementation, this would enable the source, acquire an image, and save it.)
bool scanDocument(const std::string &scannerName, const std::string &outputPath) {
    logMessage("Starting scan on scanner: " + scannerName);
    // Here you would select and open the TWAIN source matching scannerName.
    // For demonstration, we simulate a scan with a delay.
    Sleep(3000); // Simulate scanning delay
    logMessage("Scan complete. Output saved at: " + outputPath);
    return true;
}

// Main function: parses command-line arguments and executes commands.
int main(int argc, char* argv[]) {
    if(argc < 2) {
        logMessage("Usage: twain-helper.exe <command> [arguments]");
        logMessage("Commands:");
        logMessage("  list                           List available scanners");
        logMessage("  scan <scannerName> <outputPath>   Scan using specified scanner");
        return 1;
    }

    std::string command = argv[1];

    if(!loadTwainDSM()) {
        return 1;
    }

    if(command == "list") {
        std::vector<std::string> scanners = listScanners();
        if(scanners.empty()) {
            logMessage("No scanners found.");
        } else {
            logMessage("Available scanners:");
            for(const auto &s : scanners) {
                logMessage(" - " + s);
            }
        }
    } else if(command == "scan") {
        if(argc < 4) {
            logMessage("Usage: twain-helper.exe scan <scannerName> <outputPath>");
            return 1;
        }
        std::string scannerName = argv[2];
        std::string outputPath = argv[3];
        if(!scanDocument(scannerName, outputPath)) {
            logMessage("Scan operation failed.");
            return 1;
        }
    } else {
        logMessage("Unknown command: " + command);
        return 1;
    }
    return 0;
}
