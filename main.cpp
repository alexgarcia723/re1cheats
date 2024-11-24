#include <iostream>
#include <limits>
#include <string>
#include <cstdio>
#include <Windows.h>
#undef max
#include <TlHelp32.h>
#include <vector>
#include <Psapi.h>
#include <thread>
#include <atomic>

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

/*
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
*/


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
    

    public: Memory(string Name, DWORD address, vector<DWORD> offsets, HANDLE ProcessHandle, DWORD BaseAddress){
        this->Address = address;
        this->Name = Name;
        this->Offsets = offsets;
        Pointer = GetPointerAddress(ProcessHandle, BaseAddress, Address, Offsets);
    }

    public: void SetPointerRelative(DWORD RelAddr, int offset){
        Pointer = RelAddr + offset;
    }

};

atomic<bool> godmode = false;


void enableGodMode(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer,SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten){
    while(true){
        if (godmode){
            //cout << "godmode enabled" << endl;
            int health = 1000;
            WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);

        }
    }

}


void inputInteger(const string& message, int& destination){       
    string error_message = "";
    while (true){
        system("cls");
        cout << error_message << message << endl;
        cin >> destination;
        if(cin.fail()){
            error_message = "Invalid input must be an integer\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else{
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
    }
}

void inputString(const string& message, string& destination){       
    string error_message = "";
    while (true){
        system("cls");
        cout << error_message << message << endl;
        cin >> destination;
        if(cin.fail()){
            error_message = "Invalid input must be a string\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else{
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
    }
}


void printMenu(){
    cout << "Resident Evil Cheat Menu" << endl;
    cout << "Select Cheats" << endl;
    cout << "1. Toggle Godmode" << endl;
    cout << "2. Change Player Position" << endl;
    cout << "3. Translate Player Position" << endl;
    cout << "4. Toggle Infinite Ammo" << endl;
    cout << "5. Add Item" << endl;
    cout << "6. Change Character" << endl;
    cout << "7. Set Health" << endl;
    cout << "q. Quit" << endl;
}

int main()
{
    
    DWORD pId = GetProcessIdByName("bhd.exe");
    HANDLE Process  = OpenProcess(PROCESS_ALL_ACCESS, 0, pId);
    MODULEENTRY32 Module = GetModuleFromProcess(pId);
    DWORD BaseAddress = (DWORD)Module.modBaseAddr;

    
    string input;

    Memory yPos("Y Value", 0x009E41BC, {0x48, 0x970, 0x14,0x14,0x14,0x10,0x74}, Process, BaseAddress);
    Memory xPos("X Value", 0x0, {0x0}, Process, BaseAddress);
    Memory zPos("Z Value", 0x0, {0x0}, Process, BaseAddress);
    xPos.SetPointerRelative(yPos.Pointer, -4);
    zPos.SetPointerRelative(yPos.Pointer, 4);
    Memory playerChar("Character", 0xD7C9C0, {0x5118}, Process, 0);
    Memory playerHealth("Health", 0xDE41BC, {0x14C, 0x13BC}, Process, 0);
    Memory playerInventory("Inventory", 0xD7C9C0, {0}, Process, 0);
    Memory ammo("Ammo", 0x0097C938, {0x168, 0x4, 0xFFC}, Process, BaseAddress);

    int godhealth = 10000;
    thread godThread(enableGodMode, Process, (LPVOID)playerHealth.Pointer, &godhealth, sizeof(godhealth), nullptr );

    bool running = true;


    //WriteProcessMemory(Process, (LPVOID)CHEAT.Pointer, &data, sizeof(data), nullptr);
    while(running){
        system("cls");
        printMenu();
        string selection;
        cin >> selection;

        if (selection == "1"){
            godmode = !godmode;
        }
        else if (selection == "2"){
            int x,y,z;
            inputInteger("enter X position", x);
            inputInteger("enter Y position", y);
            inputInteger("enter Z position", z);
            WriteProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            WriteProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            WriteProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);
        }
        else if (selection == "3"){
            int x,y,z;
            int dx,dy,dz;
            ReadProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            ReadProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            ReadProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);

            inputInteger("translate X by", dx);
            inputInteger("translate Y by", dy);
            inputInteger("translate Z by", dz);

            x += dx;
            y += dy;
            z += dz;

            WriteProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            WriteProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            WriteProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);


        }
        
        else if (selection == "6"){
            int health;
            inputInteger("input health", health);
            WriteProcessMemory(Process, (LPVOID)playerHealth.Pointer, &health, sizeof(health), nullptr);

        }
        else if (selection == "q"){
             running = false;
        }

    }


    

    

    return 0;
}