#pragma once

#include <iostream>
#include <string>
#include <windows.h>
#include <windivert.h>

class WinDivertHelper {
public:
    // Initialize WinDivert with the specified filter
    static HANDLE Initialize(const std::string& filter, WINDIVERT_LAYER layer = WINDIVERT_LAYER_NETWORK, 
                            INT16 priority = 0, UINT64 flags = 0) {
        // Open a WinDivert handle
        HANDLE handle = WinDivertOpen(filter.c_str(), layer, priority, flags);
        
        if (handle == INVALID_HANDLE_VALUE) {
            DWORD lastError = GetLastError();
            std::cerr << "Error opening WinDivert handle: " << lastError << std::endl;
            
            switch (lastError) {
                case ERROR_INVALID_PARAMETER:
                    std::cerr << "Invalid parameter. Check your filter syntax." << std::endl;
                    break;
                case ERROR_FILE_NOT_FOUND:
                    std::cerr << "WinDivert driver not found. Make sure it's installed correctly." << std::endl;
                    break;
                case ERROR_ACCESS_DENIED:
                    std::cerr << "Access denied. Make sure you're running as Administrator." << std::endl;
                    break;
                default:
                    std::cerr << "Unknown error." << std::endl;
                    break;
            }
            return INVALID_HANDLE_VALUE;
        }
        
        return handle;
    }
    
    // Check if the application is running with administrator privileges
    static bool IsRunningAsAdmin() {
        BOOL isAdmin = FALSE;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        
        // Initialize SID for the Administrators group
        if (AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup)) {
            
            // Check if the current process token is a member of the Administrators group
            if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
                isAdmin = FALSE;
            }
            
            FreeSid(AdministratorsGroup);
        }
        
        return isAdmin;
    }
    
    // Receive a packet from WinDivert
    static bool ReceivePacket(HANDLE handle, PVOID packet, UINT packetLen, PWINDIVERT_ADDRESS addr, UINT* readLen) {
        return WinDivertRecv(handle, packet, packetLen, readLen, addr);
    }
    
    // Send a packet through WinDivert
    static bool SendPacket(HANDLE handle, PVOID packet, UINT packetLen, PWINDIVERT_ADDRESS addr, UINT* writeLen) {
        return WinDivertSend(handle, packet, packetLen, writeLen, addr);
    }
    
    // Close the WinDivert handle
    static void Close(HANDLE handle) {
        if (handle != INVALID_HANDLE_VALUE) {
            WinDivertClose(handle);
        }
    }
    
    // Set a WinDivert parameter
    static bool SetParam(HANDLE handle, WINDIVERT_PARAM param, UINT64 value) {
        return WinDivertSetParam(handle, param, value);
    }
    
    // Get a WinDivert parameter
    static bool GetParam(HANDLE handle, WINDIVERT_PARAM param, UINT64* pValue) {
        return WinDivertGetParam(handle, param, pValue);
    }
    
    // Set the receive timeout (in milliseconds)
    static bool SetReceiveTimeout(HANDLE handle, UINT64 timeoutMs) {
        // WINDIVERT_PARAM_QUEUE_TIME controls how long packets stay in the queue
        // This effectively sets a timeout for WinDivertRecv
        return SetParam(handle, WINDIVERT_PARAM_QUEUE_TIME, timeoutMs);
    }
};