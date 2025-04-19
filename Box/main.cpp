#include <memory>
#include <Windows.h>
#include "DXEngine/Core/EngineBase.h"


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    std::unique_ptr<EngineBase> engine = std::make_unique<EngineBase>();

    engine->Init(hInstance);

    while (true)
    {
        
    }
    
    return 0;
}