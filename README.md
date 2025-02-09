# twain-helper

- Virtial Scanners - https://sourceforge.net/projects/twain-samples/files/TWAIN%202%20Sample%20Application/
- DSM - https://sourceforge.net/projects/twain-dsm/?source=dlp

# Java Scanner Access TWAIN

This repository provides a comprehensive solution for integrating TWAIN-based scanner access into a Java Swing application. It supports both 32-bit and 64-bit TWAIN drivers by dynamically choosing the correct scanning mechanism at runtime. Depending on the driver’s bitness, the solution either uses direct native integration (via JNI/JNA) with the 64-bit TWAINDSM.dll or spawns an external native helper executable (twain-helper.exe) to handle 32-bit drivers.

---

## Table of Contents

- [Features](#features)
- [Architecture Overview](#architecture-overview)
- [High-Level Design](#high-level-design)
- [Low-Level Design](#low-level-design)

---

## Features

- **Dual Architecture Support**  
  - **Direct Integration (64-bit):** Uses JNI/JNA to directly interface with the 64-bit TWAINDSM.dll.
  - **Helper-Based Integration (32-bit):** Invokes a 32-bit native helper (`twain-helper.exe`) using inter-process communication (IPC) via `ProcessBuilder`.

- **Java Swing User Interface**  
  - A modern Swing-based UI that allows users to list available scanners, select a device, and initiate scanning.

- **Modular and Extensible Design**  
  - The scanning operations are abstracted behind a common `ScannerHandler` interface.
  - A factory (`ScannerHandlerFactory`) determines and returns the proper implementation (direct or helper-based) at runtime.

- **Native TWAIN Helper**  
  - A C++ executable that loads the appropriate TWAINDSM.dll version (32-bit or 64-bit), enumerates scanner sources, and performs scanning operations.
  - Provides a simple command-line interface with commands such as `list` (to enumerate scanners) and `scan <scannerName> <outputPath>` (to perform scanning).

---

## Architecture Overview

The solution consists of two main parts: the Java Swing application and the native TWAIN helper.

```plaintext
+-----------------------------------------------------------------------------------------+
|                                  Java Swing Application                                 |
|  +-----------------------------------------------------------------------------------+  |
|  |                              Scanner UI (Swing JFrame)                              |  |
|  |  - Displays available scanners                                                   |  |
|  |  - Provides controls for initiating scans                                        |  |
|  +-----------------------------------------------------------------------------------+  |
|                                                                                         |
|  +---------------------------+         +------------------------------------------+  |
|  |   ScannerHandlerFactory   |-------->|      ScannerHandler Interface            |  |
|  |  - Detects scanner driver |         |  - listScanners()                        |  |
|  |    bitness & system info  |         |  - scan(ScannerDevice, ScanParams)       |  |
|  +---------------------------+         +------------------------------------------+  |
|            ^                                       ^               ^                |
|            |                                       |               |                |
|  +----------------------+         +----------------------+   +----------------------+  |
|  | DirectScannerHandler |         | HelperScannerHandler |   |  (Other Implementations)| |
|  |  (64-bit JNI/JNA)    |         |   (32-bit via IPC)   |   |                        | |
|  +----------------------+         +----------------------+   +----------------------+  |
|            |                                       |                              |
|            V                                       V                              |
|  +-----------------------+             +-----------------------------+             |
|  | Native TWAINDSM DLL   |             | TWAIN Helper (twain-helper) |             |
|  |   (64-bit Version)    |             |   - Built as 32-bit binary  |             |
|  |                       |             |   - Loads TWAINDSM.dll (32) |             |
|  +-----------------------+             |   - Enumerates scanners     |             |
|                                        |   - Performs scanning       |             |
|                                        |   - Returns results via IPC  |             |
|                                        +-----------------------------+             |
+-----------------------------------------------------------------------------------------+
```

## High-Level Design

### Java Swing Application

- **User Interface:**  
  A Swing-based UI (e.g., `ScannerUI.java`) that displays available scanner devices, allows selection, and initiates scanning.

- **ScannerHandler Interface:**  
  Defines the operations required for scanning (e.g., `listScanners()` and `scan(ScannerDevice, ScanParameters)`).

- **Factory Pattern:**  
  A `ScannerHandlerFactory` determines which handler to use based on the scanner's driver bitness (or system configuration).

### Scanner Handler Implementations

- **DirectScannerHandler:**  
  Implements the interface using JNI/JNA to directly interact with the 64-bit `TWAINDSM.dll`.

- **HelperScannerHandler:**  
  Implements the interface by invoking the external 32-bit native helper (`twain-helper.exe`) through IPC.

### Native TWAIN Helper

- **Purpose:**  
  A native C++ application that loads `TWAINDSM.dll`, enumerates available scanners, and performs scan operations.

- **Interface:**  
  Provides a command-line interface:  
  - `list`: Lists available scanners.  
  - `scan <scannerName> <outputPath>`: Initiates a scan and outputs the result (e.g., a file path or base64 string).

### TWAINDSM.dll

- **Two versions are required:**  
  - **32-bit TWAINDSM.dll:** Used by the 32-bit helper.  
  - **64-bit TWAINDSM.dll:** Used by the 64-bit integration via JNI/JNA.


## Low-Level Design

### Native TWAIN Helper (C++ Component)

- **Loading TWAINDSM.dll:**  
  Uses `LoadLibrary` to dynamically load the correct `TWAINDSM.dll` and retrieves the `DSM_Entry` function pointer via `GetProcAddress`.

- **Enumerating Scanner Sources:**  
  Uses the TWAIN DSM (Data Source Manager) API with messages `MSG_GETFIRST` and `MSG_GETNEXT` to list available scanner sources.

- **Scanning Operation:**  
  Simulates (or fully implements) scanning by enabling the appropriate scanner, transferring image data, and saving it to a specified output path.

- **Command-Line Parsing:**  
  The `main` function interprets command-line arguments to execute commands like `list` or `scan`.

### Java Side

- **ScannerHandler Interface & Implementations:**  
  The Java code abstracts scanner operations through the `ScannerHandler` interface. Two implementations are provided:
  - **DirectScannerHandler:**  
    For direct 64-bit scanning via native calls.
  - **HelperScannerHandler:**  
    For 32-bit scanning via an external process (invoking `twain-helper.exe`).

- **Factory:**  
  The `ScannerHandlerFactory` inspects system configuration or scanner properties and returns the appropriate handler.

- **Swing UI Integration:**  
  The UI uses the handler to list available scanners and perform scanning without needing to know whether the underlying implementation is direct or helper-based.
