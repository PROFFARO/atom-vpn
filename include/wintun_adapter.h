#pragma once

#include <iostream>
#include <string>
#include <windows.h>
#include <wintun.h>

class WintunAdapter {
public:
    WintunAdapter();
    ~WintunAdapter();

    // Initialize the Wintun adapter
    bool Initialize(const std::string& adapterName, const std::string& tunnelType);
    
    // Start a session for sending/receiving packets
    bool StartSession();
    
    // Close the session and cleanup
    void CloseSession();
    
    // Send a packet through the tunnel
    bool SendPacket(const BYTE* packet, DWORD packetSize);
    
    // Receive a packet from the tunnel (blocking)
    // Returns nullptr if no packet is available or error
    // Caller must call ReleaseReceivePacket when done with the packet
    BYTE* ReceivePacket(DWORD* packetSize);
    
    // Release a packet received from ReceivePacket
    void ReleaseReceivePacket(BYTE* packet);

private:
    // Load the Wintun DLL and get function pointers
    bool LoadWintunDLL();

    HMODULE m_wintunDll;
    WINTUN_CREATE_ADAPTER_FUNC* m_createAdapter;
    WINTUN_CLOSE_ADAPTER_FUNC* m_closeAdapter;
    WINTUN_OPEN_ADAPTER_FUNC* m_openAdapter;
    WINTUN_GET_ADAPTER_LUID_FUNC* m_getAdapterLuid;
    WINTUN_GET_RUNNING_DRIVER_VERSION_FUNC* m_getDriverVersion;
    WINTUN_START_SESSION_FUNC* m_startSession;
    WINTUN_END_SESSION_FUNC* m_endSession;
    WINTUN_GET_READ_WAIT_EVENT_FUNC* m_getReadWaitEvent;
    WINTUN_RECEIVE_PACKET_FUNC* m_receivePacket;
    WINTUN_RELEASE_RECEIVE_PACKET_FUNC* m_releaseReceivePacket;
    WINTUN_ALLOCATE_SEND_PACKET_FUNC* m_allocateSendPacket;
    WINTUN_SEND_PACKET_FUNC* m_sendPacket;

    WINTUN_ADAPTER_HANDLE m_adapter;
    WINTUN_SESSION_HANDLE m_session;
    std::string m_adapterName;
    std::string m_tunnelType;
    bool m_initialized;
};