#include "hook.hpp"

HookPtr hitman_blood_money::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    const auto& base_ptr = base_ptrs.at(0);
    const auto game_stats_ptr = base_ptr + 0x5B2538;
    return install_hook(
        handle,
        {
            Source{
                base_ptr + 0x0668C0,
                // push esi
                // mov esi,[esp+08]
                // mov eax,esi
                // push edi
                {0x56, 0x8B, 0x74, 0x24, 0x08, 0x8B, 0xC6, 0x57},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Fill{3, 0x90}, Label{100}},
            },
            Source{
                base_ptr + 0x2AF230,
                // mov eax,fs:[0]
                {0x64, 0xA1, 0, 0, 0, 0},
                {Jump{Code{0xE9}, Label{210}}, Code{0x90}, Label{200}},
            },
        },
        {
            // scene name, convert slashes, lower case
            Label{110},
            Code{0x56},                    // push esi
            Code{0x57},                    // push edi
            Code{0x51},                    // push ecx
            Code{0x8B, 0x74, 0x24, 0x10},  // mov esi,[esp+10]
            Code{0xBF},                    // mov edi,Label{150}
            Ptr{Label{150}},               // ...
            Code{0xB9, 0x40, 0, 0, 0},     // mov ecx,40
            Code{0xFC},                    // cld
            Code{0xAC},                    // lodsb
            Code{0x3C, 0x5C},              // cmp al,'\'
            Code{0x75, 0x04},              // jne short 4
            Code{0xB0, 0x2F},              // mov al,'/'
            Code{0xEB, 0x0A},              // jmp A
            Code{0x3C, 0x41},              // cmp al,'A'
            Code{0x7C, 0x06},              // jl short 6
            Code{0x3C, 0x5A},              // cmp al,'Z'
            Code{0x7F, 0x02},              // jg short 2
            Code{0x0C, 0x20},              // or al,20
            Code{0xAA},                    // stosb
            Code{0x84, 0xC0},              // test al, al
            Code{0x74, 0x03},              // jz 3
            Code{0x49},                    // dec ecx
            Code{0x75, 0xE5},              // jnz -27
            Code{0x8B, 0x74, 0x24, 0x10},  // mov esi,[esp+10]
            Code{0x8B, 0xC6},              // mov eax,esi
            Code{0x59},                    // pop ecx
            Jump{Code{0xE9}, Label{100}},  // jmp Label{100}

            Align{4, 0xCC},
            Label{150},  // scene name
            Fill{0x40},

            Align{4, 0xCC},
            // count witnesses
            Label{210},
            Code{0x53},                          // push ebx
            Code{0x51},                          // push ecx
            Code{0x52},                          // push edx
            Code{0x55},                          // push ebp
            Code{0x56},                          // push esi
            Code{0x57},                          // push edi
            Code{0x8B, 0xF1},                    // mov esi,ecx
            Code{0x83, 0xEC, 0x0C},              // sub esp,C
            Code{0x31, 0xDB},                    // xor ebx,ebx
            Code{0x8B, 0xEB},                    // mov ebp,ebx
            Code{0x8D, 0xBE, 0x74, 0x04, 0, 0},  // lea edi,[esi+474]
            Code{0x39, 0xDF},                    // cmp edi,ebx
            JumpShort{Code{0x74}, Label{230}},   // jz Label{230}
            Code{0x8B, 0x17},                    // mov edx,[edi]
            Code{0x54},                          // push esp
            Code{0x8B, 0xCF},                    // mov ecx,edi
            Code{0xFF, 0x52, 0x50},              // call dword ptr [edx+50]
            Label{220},
            Code{0x8B, 0x17},                   // mov edx,[edi]
            Code{0x54},                         // push esp
            Code{0x8B, 0xCF},                   // mov ecx,edi
            Code{0xFF, 0x52, 0x60},             // call dword ptr [edx+60]
            Code{0x39, 0x1C, 0x24},             // cmp [esp],ebx
            JumpShort{Code{0x74}, Label{230}},  // jz Label{230}
            Code{0x50},                         // push eax
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0xE5110}
            },                       // call HitmanBloodMoney.exe+E5110
            Code{0x83, 0xC4, 0x04},  // add esp,4
            Code{0x8B, 0x10},        // mov edx,[eax]
            Code{0x8B, 0xC8},        // mov ecx,eax
            Code{0xFF, 0x92, 0xDC, 0x03, 0, 0},  // call dword ptr [edx+3DC]
            Code{0x84, 0xC0},                    // test al,al
            JumpShort{Code{0x75}, Label{220}},   // jz Label{220}
            Code{0x45},                          // inc ebp
            JumpShort{Code{0xEB}, Label{220}},   // jmp Label{220}
            Label{230},
            Code{0x89, 0x2D},              // mov [game_stats.witnesses],ebp
            Ptr{game_stats_ptr + 0x003C},  // ...
            Code{0x83, 0xC4, 0x08},        // add esp,8
            Code{0x8B, 0x05},  //  mov eax,[HitmanBloodMoney.exe + 41F83C]
            Ptr{base_ptr + 0x41F83C},
            Code{0x8B, 0x8E, 0xC4, 0x02, 0, 0},  // mov ecx,[esi+2C4]
            Code{0x39, 0xD9},                    // cmp ecx,ebx
            Code{0x8D, 0xA8, 0x58, 0x0A, 0, 0},  // lea ebp,[eax+A58]
            Code{0x89, 0x1C, 0x24},              // mov [esp],ebx
            JumpShort{Code{0x74}, Label{240}},   // je Label{240}
            Code{0x55},                          // push ebp
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0x256A10}
            },  // call HitmanBloodMoney.exe+256A10
            Code{0x8B, 0x8E, 0xC4, 0x02, 0, 0},  // mov ecx,[esi+2C4]
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0x256A60}
            },                       // call HitmanBloodMoney.exe+256A60
            Code{0x89, 0x04, 0x24},  // mov [esp],eax
            Label{240},
            Code{0x8B, 0x8E, 0xC8, 0x02, 0, 0},  // mov ecx,[esi+2C8]
            Code{0x39, 0xD9},                    // cmp ecx,ebx
            JumpShort{Code{0x74}, Label{250}},   // je Label{250}
            Code{0x55},                          // push ebp
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0x256A10}
            },  // call HitmanBloodMoney.exe+256A10
            Code{0x8B, 0x8E, 0xC8, 0x02, 0, 0},  // mov ecx,[esi+2C8]
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0x256A60}
            },                       // call HitmanBloodMoney.exe+256A60
            Code{0x01, 0x04, 0x24},  // add [esp],eax
            Label{250},
            Code{0x8B, 0xCE},  // mov ecx,esi
            Jump{
                Code{0xE8}, Ptr{base_ptr + 0x2AE000}
            },  // call HitmanBloodMoney.exe+2AE000
            Code{0x8B, 0x8E, 0xB4, 0x01, 0, 0},  // mov ecx,[esi+1B4]
            Code{0x2B, 0x0C, 0x24},              // sub ecx,[esp]
            Code{0x29, 0xC1},                    // sub ecx,eax
            Code{0x89, 0x0D},  // mov [HitmanBloodMoney.exe+5B262C],ecx
            Ptr{game_stats_ptr + 0x00F4},  // ..
            Code{0x83, 0xC4, 0x04},        // add esp,4
            Code{0x5F},                    // pop edi
            Code{0x5E},                    // pop esi
            Code{0x5D},                    // pop ebp
            Code{0x5A},                    // pop edx
            Code{0x59},                    // pop ecx
            Code{0x5B},                    // pop ebx
            Code{0x64, 0xA1, 0, 0, 0, 0},  // mov eax,fs:[0]
            Jump{Code{0xE9}, Label{200}},
        }
    );
}