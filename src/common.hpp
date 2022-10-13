#pragma once

#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <concepts>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <iostream>

#include "jitasm.h"

namespace mapper
{
#ifdef _WIN64
	static constexpr inline bool x64 = true;
#else
	static constexpr inline bool x64 = false;
#endif
}