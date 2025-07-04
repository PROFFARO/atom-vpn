#include <iostream>
#include <windows.h>
#include <thread>
#include <atomic>
#include <csignal>
#include "windivert_helper.h"
#include "wintun_adapter.h"

// Global flag for shutdown
std::atomic<bool> g_running(true);

// Signal handler for Ctrl+C
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down..." << std::endl;
        g_running = false;
    }
}

int main() {
    std::cout << "Atom VPN - Starting up..." << std::endl;

    // Set up signal handler for shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Check for administrator privileges
    if (!WinDivertHelper::IsRunningAsAdmin()) {
        std::cerr << "Error: This application requires administrator privileges." << std::endl;
        std::cerr << "Please restart the application as Administrator." << std::endl;
        return 1;
    }
    
    std::cout << "Running with administrator privileges." << std::endl;
    
    // Initialize Wintun adapter
    WintunAdapter wintunAdapter;
    if (!wintunAdapter.Initialize("AtomVPN", "AtomVPN")) {
        std::cerr << "Failed to initialize Wintun adapter. Exiting." << std::endl;
        return 1;
    }
    
    std::cout << "Wintun adapter initialized successfully." << std::endl;
    
    // Start Wintun session
    if (!wintunAdapter.StartSession()) {
        std::cerr << "Failed to start Wintun session. Exiting." << std::endl;
        return 1;
    }
    
    std::cout << "Wintun session started successfully." << std::endl;
    
    // Initialize WinDivert with a basic filter
    // This filter captures all outbound IP packets except those going to the VPN server
    // In a real VPN, you would need to exclude your VPN server's IP
    std::string filter = "outbound and ip";
    HANDLE divertHandle = WinDivertHelper::Initialize(
        filter,
        WINDIVERT_LAYER_NETWORK,  // Capture at the network layer
        0,                        // Default priority
        0                         // No flags
    );
    
    if (divertHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to initialize WinDivert. Exiting." << std::endl;
        wintunAdapter.CloseSession();
        return 1;
    }
    
    std::cout << "WinDivert initialized successfully." << std::endl;
    std::cout << "Filter: " << filter << std::endl;
    
    // Packet buffers
    const UINT maxPacketSize = 0xFFFF;
    BYTE packet[maxPacketSize];
    UINT packetLen = 0;
    WINDIVERT_ADDRESS addr;
    
    std::cout << "Atom VPN is running. Press Ctrl+C to stop." << std::endl;
    
    // Main packet processing loop
    while (g_running) {
        // Set a timeout for WinDivertRecv to allow checking g_running periodically
        UINT readLen = 0;
        if (WinDivertHelper::SetReceiveTimeout(divertHandle, 1000)) {
            // Receive a packet from the network
            if (WinDivertHelper::ReceivePacket(divertHandle, packet, maxPacketSize, &addr, &readLen)) {
                // Process the packet (in a real VPN, you would encrypt it here)
                std::cout << "Captured packet: " << readLen << " bytes" << std::endl;
                
                // Send the packet to the Wintun adapter
                if (!wintunAdapter.SendPacket(packet, readLen)) {
                    std::cerr << "Failed to send packet to Wintun adapter." << std::endl;
                }
            }
        }
        
        // Receive packets from the Wintun adapter
        DWORD tunnelPacketSize = 0;
        BYTE* tunnelPacket = wintunAdapter.ReceivePacket(&tunnelPacketSize);
        
        if (tunnelPacket != nullptr) {
            // Process the packet (in a real VPN, you would decrypt it here)
            std::cout << "Received packet from tunnel: " << tunnelPacketSize << " bytes" << std::endl;
            
            // Send the packet back to the network
            UINT writeLen = 0;
            if (!WinDivertHelper::SendPacket(divertHandle, tunnelPacket, tunnelPacketSize, &addr, &writeLen)) {
                std::cerr << "Failed to send packet to network." << std::endl;
            }
            
            // Release the packet
            wintunAdapter.ReleaseReceivePacket(tunnelPacket);
        }
    }
    
    // Cleanup
    WinDivertHelper::Close(divertHandle);
    wintunAdapter.CloseSession();
    std::cout << "Atom VPN - Shutdown complete." << std::endl;
    
    return 0;
}