#include <iostream>
#include <string>
#include <cstdio>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <Psapi.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

int ListProcesses(){

    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 Entry;
    Entry.dwSize = sizeof(PROCESSENTRY32);

        if(Process32First(Snapshot, &Entry) == TRUE){
            while(Process32Next(Snapshot, &Entry) == TRUE){
                cout << Entry.szExeFile << endl;
            }
        }
        CloseHandle(Snapshot);
    return 1;
} 

DWORD GetProcessIdByName(string s){
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    const char *name = s.c_str();
    PROCESSENTRY32 Entry;
    Entry.dwSize = sizeof(PROCESSENTRY32);
        if(Process32First(Snapshot, &Entry) == TRUE){
            while(Process32Next(Snapshot, &Entry) == TRUE){
                if( stricmp((char*)Entry.szExeFile, name) == 0){
                    cout << "Found Process: " << s << endl;
                    cout << "pId: " << Entry.th32ProcessID << endl;
                    CloseHandle(Snapshot);
                    return Entry.th32ProcessID;
                }
                
            }
        }
        return NULL;
}

//NEED TO REMAKE THIS FUNCTION MYSELF TO UNDERSTAND IT
DWORD GetPointerAddress(HANDLE phandle, DWORD gameBaseAddr, DWORD address, vector<DWORD> offsets)
{
 
    DWORD offset_null = NULL;
    ReadProcessMemory(phandle, (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
    DWORD pointeraddress = offset_null; // the address we need
    for (int i = 0; i < offsets.size() - 1; i++) // we dont want to change the last offset value so we do -1
    {
        ReadProcessMemory(phandle, (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
    }
    return pointeraddress += offsets.at(offsets.size() - 1); // adding the last offset
}

MODULEENTRY32 GetModuleFromProcess(DWORD pId){

    cout << "Pid is still: " << pId << endl;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pId);

    if(Snapshot == INVALID_HANDLE_VALUE) cout << "Bad handle!" << endl;

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);

    if(Module32First(Snapshot, &entry) == TRUE){
        cout << entry.szModule << " Base: " << (DWORD)entry.modBaseAddr << endl;
        cout << entry.modBaseSize << endl;
    }
    else cout << "No Modules" << endl;


    return entry;
}

class Memory{
    public: 
    string Name;
    DWORD Address;
    vector<DWORD> Offsets;
    DWORD Pointer;
    

    public: Memory(string Name, DWORD address, vector<DWORD> offsets){
        this->Address = address;
        this->Name = Name;
        this->Offsets = offsets;
    }

    public: void SetPointer(HANDLE ProcessHandle, DWORD BaseAddress){
        Pointer = GetPointerAddress(ProcessHandle, BaseAddress, Address, Offsets);
    }

    public: void SetPointerRelative(DWORD RelAddr, int offset){
        Pointer = RelAddr + offset;
    }
    public: void StaticPointer(HANDLE ProcessHandle){
        Pointer = GetPointerAddress(ProcessHandle, Address, 0x0, Offsets);
    }

};

int main()
{
    string s;
    //ListProcesses();
    //cin >> s;
    s = "bhd.exe";

    DWORD pId = GetProcessIdByName(s);
    HANDLE Process  = OpenProcess(PROCESS_ALL_ACCESS, 0, pId);

    MODULEENTRY32 Module = GetModuleFromProcess(pId);


    DWORD BaseAddress = (DWORD)Module.modBaseAddr;
    
    SIZE_T numBytes = 0;

    int data = 0;
    string input;

    Memory yPos("Y Value", 0x009E41BC, {0x48, 0x970, 0x14,0x14,0x14,0x10,0x74});
    Memory xPos("X Value", 0x0, {0x0});
    Memory playerChar("Character", 0xD7C9C0, {0x5118});
    playerChar.StaticPointer(Process);

    
    

    cout << "Pointer Addr:  " << (LPCVOID)playerChar.Pointer << endl;

    yPos.SetPointer(Process,BaseAddress);
    xPos.SetPointerRelative(yPos.Pointer, -4);

    while (true) {

        


        ReadProcessMemory(Process, (LPCVOID)playerChar.Pointer, &data, sizeof(data), nullptr);

        cout << "Player Char Value: " << data << endl;
        cin >> input;
        data = stof(input);
        if(data == -1){
            break;
        }

        WriteProcessMemory(Process, (LPVOID)playerChar.Pointer, &data, sizeof(data), &numBytes);
        


    /*
        
        cout << "Data: " << data << endl;
        cout << "Please Input New Y Value: ";
        cin >> data;
        */
        
    }

    

    

    return 0;
}