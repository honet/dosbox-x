#include "c86ctlwrap.h"

#include <algorithm>
#include <functional>

// singleton
C86CTLWrap* C86CTLWrap::instance = 0;

HMODULE hmod = nullptr;
HRESULT(WINAPI* CreateInstance)(REFIID riid, void** ppi) = nullptr;
c86ctl::IRealChipBase* baseif = nullptr;

bool initialized = false;

C86CTLWrap* C86CTLWrap::GetInstance()
{
	if (!instance)
		instance = new C86CTLWrap();

	return instance;
}

C86CTLWrap::C86CTLWrap()
{
}

C86CTLWrap::~C86CTLWrap()
{
}

bool C86CTLWrap::Initialize()
{
    if (initialized) return true;
	
	hmod = ::LoadLibrary( TEXT("c86ctl.dll") );
	if (!hmod) return false;

	INT_PTR proc = (INT_PTR)GetProcAddress(hmod, "CreateInstance");
    if (proc == NULL) {
        ::FreeLibrary(hmod);
        hmod = nullptr;
        return false;
    }

    *((INT_PTR*)&CreateInstance) = proc;

    HRESULT hr = CreateInstance(c86ctl::IID_IRealChipBase, (void**)&baseif);
    if (FAILED(hr)) {
        ::FreeLibrary(hmod);
        hmod = nullptr;
        return false;
    }

    baseif->initialize();

    initialized = true;
    return TRUE;
}

void C86CTLWrap::Deinitialize()
{
    if (baseif) {
        baseif->deinitialize();
        baseif->Release();
    }

    ::FreeLibrary(hmod);
    hmod = nullptr;
    initialized = false;
}


bool C86CTLWrap::QueryChipIF(c86ctl::ChipType type, c86ctl::IRealChip3** chipif)
{
    if (!initialized) return nullptr;

    int nchips = baseif->getNumberOfChip();

    *chipif = nullptr;
    for (int i = 0; i < nchips; i++) {
        c86ctl::IRealChip3* cif = nullptr;
        HRESULT hr = baseif->getChipInterface(i, c86ctl::IID_IRealChip3, (void**)&cif);
        if (FAILED(hr)) continue;

        c86ctl::ChipType ctype;
        cif->getChipType(&ctype);
        if (ctype == type) {
            *chipif = cif;
            break;
        } else {
            cif->Release();
        }
    }

    return (chipif != nullptr);
}

bool C86CTLWrap::QueryChipIF(std::function<bool(c86ctl::ChipType)> func, c86ctl::IRealChip3** chipif)
{
    if (!initialized) return nullptr;

    int nchips = baseif->getNumberOfChip();
    *chipif = nullptr;
    for (int i = 0; i < nchips; i++) {
        c86ctl::IRealChip3* cif = nullptr;
        HRESULT hr = baseif->getChipInterface(i, c86ctl::IID_IRealChip3, (void**)&cif);
        if (FAILED(hr)) continue;

        c86ctl::ChipType ctype;
        cif->getChipType(&ctype);
        if (func(ctype)) {
            *chipif = cif;
            break;
        } else {
            cif->Release();
        }
    }

    return (chipif != nullptr);
}

