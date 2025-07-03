#include "wintun_adapter.h"

WintunAdapter::WintunAdapter()
    : m_wintunDll(NULL),
      m_createAdapter(NULL),
      m_closeAdapter(NULL),
      m_openAdapter(NULL),
      m_getAdapterLuid(NULL),
      m_getDriverVersion(NULL),
      m_startSession(NULL),
      m_endSession(NULL),
      m_getReadWaitEvent(NULL),
      m_receivePacket(NULL),
      m_releaseReceivePacket(NULL),
      m_allocateSendPacket(NULL),
      m_sendPacket(NULL),
      m_adapter(NULL),
      m_session(NULL),
      m_initialized(false) {
}

WintunAdapter::~WintunAdapter() {
    CloseSession();
    
    if (m_adapter != NULL && m_closeAdapter != NULL) {
        m_closeAdapter(m_adapter);
        m_adapter = NULL;
    }
    
    if (m_wintunDll != NULL) {
        FreeLibrary(m_wintunDll);
        m_wintunDll = NULL;
    }
}

bool WintunAdapter::LoadWintunDLL() {
    // Load the Wintun DLL
    m_wintunDll = LoadLibraryExW(L"wintun.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    
    if (m_wintunDll == NULL) {
        std::cerr << "Failed to load wintun.dll. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Get function pointers
    m_createAdapter = (WINTUN_CREATE_ADAPTER_FUNC*)GetProcAddress(m_wintunDll, "WintunCreateAdapter");
    m_closeAdapter = (WINTUN_CLOSE_ADAPTER_FUNC*)GetProcAddress(m_wintunDll, "WintunCloseAdapter");
    m_openAdapter = (WINTUN_OPEN_ADAPTER_FUNC*)GetProcAddress(m_wintunDll, "WintunOpenAdapter");
    m_getAdapterLuid = (WINTUN_GET_ADAPTER_LUID_FUNC*)GetProcAddress(m_wintunDll, "WintunGetAdapterLUID");
    m_getDriverVersion = (WINTUN_GET_RUNNING_DRIVER_VERSION_FUNC*)GetProcAddress(m_wintunDll, "WintunGetRunningDriverVersion");
    m_startSession = (WINTUN_START_SESSION_FUNC*)GetProcAddress(m_wintunDll, "WintunStartSession");
    m_endSession = (WINTUN_END_SESSION_FUNC*)GetProcAddress(m_wintunDll, "WintunEndSession");
    m_getReadWaitEvent = (WINTUN_GET_READ_WAIT_EVENT_FUNC*)GetProcAddress(m_wintunDll, "WintunGetReadWaitEvent");
    m_receivePacket = (WINTUN_RECEIVE_PACKET_FUNC*)GetProcAddress(m_wintunDll, "WintunReceivePacket");
    m_releaseReceivePacket = (WINTUN_RELEASE_RECEIVE_PACKET_FUNC*)GetProcAddress(m_wintunDll, "WintunReleaseReceivePacket");
    m_allocateSendPacket = (WINTUN_ALLOCATE_SEND_PACKET_FUNC*)GetProcAddress(m_wintunDll, "WintunAllocateSendPacket");
    m_sendPacket = (WINTUN_SEND_PACKET_FUNC*)GetProcAddress(m_wintunDll, "WintunSendPacket");
    
    // Check if all function pointers were successfully loaded
    if (!m_createAdapter || !m_closeAdapter || !m_openAdapter || !m_getAdapterLuid ||
        !m_getDriverVersion || !m_startSession || !m_endSession || !m_getReadWaitEvent ||
        !m_receivePacket || !m_releaseReceivePacket || !m_allocateSendPacket || !m_sendPacket) {
        std::cerr << "Failed to load Wintun functions. Error: " << GetLastError() << std::endl;
        FreeLibrary(m_wintunDll);
        m_wintunDll = NULL;
        return false;
    }
    
    return true;
}

bool WintunAdapter::Initialize(const std::string& adapterName, const std::string& tunnelType) {
    m_adapterName = adapterName;
    m_tunnelType = tunnelType;
    
    // Load the Wintun DLL
    if (!LoadWintunDLL()) {
        return false;
    }
    
    // Convert strings to wide strings for Windows API
    int adapterNameSize = MultiByteToWideChar(CP_UTF8, 0, adapterName.c_str(), -1, NULL, 0);
    int tunnelTypeSize = MultiByteToWideChar(CP_UTF8, 0, tunnelType.c_str(), -1, NULL, 0);
    
    if (adapterNameSize <= 0 || tunnelTypeSize <= 0) {
        std::cerr << "Failed to convert strings to wide strings." << std::endl;
        return false;
    }
    
    wchar_t* wideAdapterName = new wchar_t[adapterNameSize];
    wchar_t* wideTunnelType = new wchar_t[tunnelTypeSize];
    
    MultiByteToWideChar(CP_UTF8, 0, adapterName.c_str(), -1, wideAdapterName, adapterNameSize);
    MultiByteToWideChar(CP_UTF8, 0, tunnelType.c_str(), -1, wideTunnelType, tunnelTypeSize);
    
    // Create the adapter
    m_adapter = m_createAdapter(wideAdapterName, wideTunnelType, NULL);
    
    // Clean up wide strings
    delete[] wideAdapterName;
    delete[] wideTunnelType;
    
    if (m_adapter == NULL) {
        std::cerr << "Failed to create Wintun adapter. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

bool WintunAdapter::StartSession() {
    if (!m_initialized || m_adapter == NULL) {
        std::cerr << "Adapter not initialized." << std::endl;
        return false;
    }
    
    // Start a session
    m_session = m_startSession(m_adapter, 0x400000); // 4 MiB ring capacity
    
    if (m_session == NULL) {
        std::cerr << "Failed to start Wintun session. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    return true;
}

void WintunAdapter::CloseSession() {
    if (m_session != NULL && m_endSession != NULL) {
        m_endSession(m_session);
        m_session = NULL;
    }
}

bool WintunAdapter::SendPacket(const BYTE* packet, DWORD packetSize) {
    if (!m_initialized || m_session == NULL) {
        std::cerr << "Session not initialized." << std::endl;
        return false;
    }
    
    // Allocate memory for the packet
    BYTE* sendPacket = m_allocateSendPacket(m_session, packetSize);
    
    if (sendPacket == NULL) {
        std::cerr << "Failed to allocate send packet. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Copy the packet data
    memcpy(sendPacket, packet, packetSize);
    
    // Send the packet
    m_sendPacket(m_session, sendPacket);
    
    return true;
}

BYTE* WintunAdapter::ReceivePacket(DWORD* packetSize) {
    if (!m_initialized || m_session == NULL) {
        std::cerr << "Session not initialized." << std::endl;
        return nullptr;
    }
    
    // Receive a packet
    return m_receivePacket(m_session, packetSize);
}

void WintunAdapter::ReleaseReceivePacket(BYTE* packet) {
    if (m_initialized && m_session != NULL && packet != NULL) {
        m_releaseReceivePacket(m_session, packet);
    }
}