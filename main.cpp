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

    public: void SetPointer(HANDLE ProcessHandle, DWORD BaseAddress){
        Pointer = GetPointerAddress(ProcessHandle, BaseAddress, Address, Offsets);
    }

    public: void SetPointerRelative(DWORD RelAddr, int offset){
        Pointer = RelAddr + offset;
    }

};


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

void inputFloat(const string& message, float& destination){       
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
    cout << "6. Change Character (warning! breaks game sometimes)" << endl;
    cout << "7. Set Health" << endl;
    cout << "q. Quit" << endl;
}


DWORD pId;
HANDLE Process;
MODULEENTRY32 Module;
DWORD BaseAddress;

Memory yPos("Y Value", 0x009E41BC, {0x48, 0x970, 0x14,0x14,0x14,0x10,0x74}, Process, BaseAddress);
Memory xPos("X Value", 0x0, {0x0}, Process, BaseAddress);
Memory zPos("Z Value", 0x0, {0x0}, Process, BaseAddress);
Memory playerChar("Character", 0xD7C9C0, {0x5118}, Process, 0);
Memory playerHealth("Health", 0xDE41BC, {0x14C, 0x13BC}, Process, 0);
Memory playerInventory("Inventory", 0xD7C9C0, {0}, Process, 0);
Memory ammo("Ammo", 0x0097C938, {0x168, 0x4, 0xFFC}, Process, BaseAddress);


atomic<bool> godmode = false;
atomic<bool> infammo = false;


void enableGodMode(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer,SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten){
    while(true){
        if (godmode){
            //cout << "godmode enabled" << endl;
            int health = 1000;
            playerHealth.SetPointer(Process, BaseAddress);
            WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);

        } 
    }

}

void enableInfAmmo(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer,SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten){
    while(true){
        if (infammo){
            //cout << "godmode enabled" << endl;
            int health = 1000;
            ammo.SetPointer(Process, BaseAddress);
            WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);

        }
    }
}

int main()
{
    
    pId = GetProcessIdByName("bhd.exe");
    Process  = OpenProcess(PROCESS_ALL_ACCESS, 0, pId);
    Module = GetModuleFromProcess(pId);
    BaseAddress = (DWORD)Module.modBaseAddr;
    
    string input;

    int godhealth = 10000;
    int infAmmo = 10000;
    thread godThread(enableGodMode, Process, (LPVOID)playerHealth.Pointer, &godhealth, sizeof(godhealth), nullptr );
    thread ammoThread(enableInfAmmo, Process, (LPVOID)ammo.Pointer, &infAmmo, sizeof(infAmmo), nullptr );

    bool running = true;


    //WriteProcessMemory(Process, (LPVOID)CHEAT.Pointer, &data, sizeof(data), nullptr);
    while(running){
        system("cls");
        printMenu();
        string selection;
        cin >> selection;

        if (selection == "1"){
            godmode = !godmode;
            if(!godmode){
                playerHealth.SetPointer(Process, BaseAddress);
                int maxhealth = ReadProcessMemory(Process, (LPVOID)(playerHealth.Pointer + 4), &godhealth, sizeof(godhealth), nullptr );
                WriteProcessMemory(Process, (LPVOID)playerHealth.Pointer, &maxhealth, sizeof(maxhealth), nullptr );
            }
        }
        else if (selection == "2"){
            float x,y,z;
            inputFloat("enter X position", x);
            inputFloat("enter Y position", y);
            inputFloat("enter Z position", z);

            yPos.SetPointer(Process, BaseAddress);
            xPos.SetPointerRelative(yPos.Pointer, -4);
            zPos.SetPointerRelative(yPos.Pointer, 4);

            WriteProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            WriteProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            WriteProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);
        }
        else if (selection == "3"){
            float x,y,z;
            float dx,dy,dz;

            yPos.SetPointer(Process, BaseAddress);
            xPos.SetPointerRelative(yPos.Pointer, -4);
            zPos.SetPointerRelative(yPos.Pointer, 4);

            ReadProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            ReadProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            ReadProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);

            inputFloat("translate X by", dx);
            inputFloat("translate Y by", dy);
            inputFloat("translate Z by", dz);

            x += dx;
            y += dy;
            z += dz;

            WriteProcessMemory(Process, (LPVOID)xPos.Pointer, &x, sizeof(x), nullptr);
            WriteProcessMemory(Process, (LPVOID)yPos.Pointer, &y, sizeof(y), nullptr);
            WriteProcessMemory(Process, (LPVOID)zPos.Pointer, &z, sizeof(z), nullptr);


        }
        else if (selection == "4"){
            infammo = !infammo;
        }
        else if (selection == "5"){
            //0x38 is slot 0
            int invSlot, itemID, amount;

            do{
                inputInteger("What Inventory Slot Do You Want to Alter? (0-7)", invSlot);
            }
            while (invSlot < 0 || invSlot > 7);
            

            do{
                inputInteger("Enter the ID of the item you want", itemID);
            } while( itemID < 0 || itemID > 132);

            do{
                inputInteger("How many do you want", amount);
            }   while(amount < 0 || amount > 1000);

            playerInventory.SetPointer(Process, BaseAddress);
            WriteProcessMemory(Process, (LPVOID)(playerInventory.Pointer + 0x38 + (invSlot * 8)), &itemID, sizeof(itemID), nullptr);
            WriteProcessMemory(Process, (LPVOID)(playerInventory.Pointer + 0x3C + (invSlot * 8)), &amount, sizeof(amount), nullptr);

        }

        else if (selection == "6"){
            
            int charId;

            do{
                inputInteger("Enter Character ID # (0-2)", charId);
            } while (charId < 0 || charId > 2);

            playerChar.SetPointer(Process, BaseAddress);
            WriteProcessMemory(Process, (LPVOID)playerChar.Pointer, &charId, sizeof(charId), nullptr);
        }
        
        else if (selection == "7"){
            int health;
            inputInteger("input health", health);
            playerHealth.SetPointer(Process, BaseAddress);
            WriteProcessMemory(Process, (LPVOID)playerHealth.Pointer, &health, sizeof(health), nullptr);

        }
        else if (selection == "q"){
             running = false;
        }

    }


    return 0;
}