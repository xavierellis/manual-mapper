#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winternl.h>
#include <concepts>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <assert.h>
#include <iterator>
#include <algorithm>
#include <iostream>

namespace mapper
{
#ifdef _WIN64
	static constexpr inline bool x64 = true;
#else
	static constexpr inline bool x64 = false;
#endif
}