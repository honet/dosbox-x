#pragma once
#include "c86ctl.h"

#include <functional>

class C86CTLWrap
{
public:
	static C86CTLWrap* GetInstance();
	bool Initialize();
	void Deinitialize();

    bool QueryChipIF(c86ctl::ChipType type, c86ctl::IRealChip3** chipif);
	bool QueryChipIF(std::function<bool(c86ctl::ChipType)> func, c86ctl::IRealChip3** chipif);

private:
	C86CTLWrap();
	~C86CTLWrap();
	
	static C86CTLWrap* instance;
};


