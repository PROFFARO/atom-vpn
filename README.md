# Atom VPN

A C++ VPN application for Windows using WinDivert and Wintun, inspired by ProtonVPN.

## Overview

Atom VPN is a lightweight VPN client for Windows that uses:

- **WinDivert**: For packet capture, filtering, and manipulation
- **Wintun**: For creating a virtual network adapter (TUN device)

## Features (Planned)

- Packet interception and tunneling
- Traffic encryption
- Split tunneling capability
- DNS leak prevention
- Simple user interface

## Requirements

- Windows 10/11
- Administrator privileges (required for WinDivert and Wintun)
- Visual Studio 2019 or later

## Project Structure

```
├── src/                  # Source files
│   ├── core/             # Core VPN functionality
│   │   ├── divert/       # WinDivert integration
│   │   ├── tunnel/       # Wintun integration
│   │   └── network/      # Network utilities
│   ├── ui/               # User interface
│   └── main.cpp          # Entry point
├── include/              # Header files
├── lib/                  # External libraries
│   ├── windivert/        # WinDivert library
│   └── wintun/           # Wintun library
├── build/                # Build output
└── docs/                 # Documentation
```

## Building

1. Clone the repository
2. Open the solution in Visual Studio
3. Build the solution

## Usage

The application requires administrator privileges to run.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgements

- [WinDivert](https://reqrypt.org/windivert.html) - Windows Packet Divert
- [Wintun](https://www.wintun.net/) - TUN driver for Windows
- [ProtonVPN](https://protonvpn.com/) - Inspiration for this project