# Clumsy - Network Packet Manipulation Tool

## Overview
Clumsy is a network packet manipulation tool that allows users to simulate various network conditions by intercepting and modifying network traffic in real-time. This version has been significantly enhanced with a modular UI system, improved logging, and enhanced functionality.

## Key Features

### Modular UI System
- Tab-based interface in the top-left corner similar to Windows tabs
- Side panels for automation, logging, and statistics that can be toggled on/off
- Panels open smoothly to the right of the main window
- Clean main window showing only core network manipulation modules

### Enhanced Logging System
- Real-time packet action logging (DROP, RST, MODIFY, etc.)
- Dedicated logging panel with live updates
- Configurable log formats (TEXT, CSV, JSON)

### Improved Statistics
- Comprehensive statistics display with all metrics on a single panel
- Real-time updates for all network manipulation actions
- Detailed tracking of packet modifications

### Network Manipulation Modules
1. **Drop** - Randomly drops packets
2. **Lag** - Delays packets by a specified time
3. **Duplicate** - Creates copies of packets
4. **Tamper** - Modifies packet contents
5. **Reset** - Sends TCP RST packets
6. **Throttle** - Limits packet throughput
7. **Out of Order** - Reorders packets
8. **Bandwidth** - Limits bandwidth
9. **Length Filter** - Filters packets by size with configurable actions
10. **TLS** - TLS/SSL packet manipulation

### Enhanced Length Filter Module
The length filter module has been completely reworked with a dropdown menu that allows users to select what action to take on packets that match the length filter criteria:
- **Pass Through** - Packets matching the filter are passed through without modification
- **Drop** - Packets matching the filter are dropped (discarded)
- **Lag** - Packets matching the filter are delayed using a configurable buffer
- **Duplicate** - Packets matching the filter are duplicated a configurable number of times

### Optimized UI Layout
- Compact layout for all module controls to prevent window widening
- Shortened labels and reduced input field sizes for better space utilization
- Efficient control arrangement without unused space after module names
- Consistent design across all modules with "In"/"Out" direction toggles
- **Natural Flow Layout**: Controls appear immediately after module names without unnecessary spacing
- **Dynamic Resizing**: Window adjusts properly when modules are enabled/disabled
- **Responsive Design**: Controls wrap to new lines when window is too narrow

## Technical Implementation

### Architecture
- Built with IUP 3.30 GUI framework for cross-platform interface
- Uses WinDivert 2.2.0 for network packet interception
- Written in C with Zig 0.10.1 build system targeting x86_64-windows-gnu
- Modular architecture with function pointers for easy extension

### Core Components
- **Main Application** - Central coordination and UI management
- **Divert Module** - Packet interception and injection using WinDivert
- **Module System** - Pluggable architecture for network manipulation functions
- **Logging System** - Real-time packet action logging and display
- **Statistics System** - Comprehensive metrics tracking
- **UI Components** - Interactive controls for all modules

### Build System
```bash
# Build with Zig (from project root)
zig build -Darch=x64 -Dconf=Release -Dsign=A
```

## Configuration
The application can be configured through:
1. **UI Controls** - Interactive sliders, checkboxes, and inputs for each module
2. **Parameterized Mode** - Command-line parameters for automated testing
3. **Config Files** - Persistent settings storage

## Usage
1. Run the application as Administrator (required for packet interception)
2. Configure the desired network manipulation modules
3. Click "Start" to begin packet interception
4. Monitor logs and statistics in real-time
5. Click "Stop" to end the session

## Documentation
- [TLS/SSL Functionality](TLS_SSL_FUNCTIONALITY.md)
- [Logging Improvements](LOGGING_IMPROVEMENTS.md)
- [Length Module Improvements](LENGTH_MODULE_IMPROVEMENTS.md)
- [UI Layout Optimization](UI_LAYOUT_OPTIMIZATION.md)
- [UI Improvements Summary](UI_IMPROVEMENTS_SUMMARY.md)
- [UI Flow Layout Improvements](UI_FLOW_LAYOUT_IMPROVEMENTS.md)
- [Implementation Summary](IMPLEMENTATION_SUMMARY.md)
- [Final Implementation Summary](FINAL_IMPLEMENTATION_SUMMARY.md)

## Requirements
- Windows Vista/2008 or later (x86/x64)
- Administrator privileges
- WinDivert driver (included)

## License
See [LICENSE](LICENSE) file for details.