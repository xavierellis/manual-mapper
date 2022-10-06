#pragma once
#ifndef MAPPER_API //defined by mapper_internal if compiling the implementation
#define MAPPER_API __declspec(dllimport) __stdcall
#endif

#include <cstdint>
#include <span>
#include <string_view>
#include <Windows.h>

namespace mapper
{

	struct MapperHandle
	{
		uintptr_t base{};
		size_t size{};
	};

	enum class eMapperError
	{
		Unknown = 0,
		Success,

	};

	enum eMapperFlags : uint64_t
	{
		eMapperFlags_None = 1 << 0,
		eMapperFlags_ErasePeHeaders = 1 << 1,
		eMapperFlags_SupportExceptions = 1 << 2,
		eMapperFlags_LinkLdr = 1 << 3,


		eMapperFlags_Default = eMapperFlags_None
	};

	struct Mapper
	{
		eMapperError MAPPER_API map_ex(HANDLE process, std::string_view path, MapperHandle& out_handle, eMapperFlags flags = eMapperFlags_Default);

		eMapperError MAPPER_API map_ex(HANDLE process, std::span<std::byte> dll, MapperHandle& out_handle, eMapperFlags flags = eMapperFlags_Default);

		eMapperError MAPPER_API unmap_ex(HANDLE process, MapperHandle handle);

		eMapperError MAPPER_API map(std::string_view path, MapperHandle& out_handle, eMapperFlags flags = eMapperFlags_Default);

		eMapperError MAPPER_API map(std::span<std::byte> dll, MapperHandle& out_handle, eMapperFlags flags = eMapperFlags_Default);

		eMapperError MAPPER_API unmap(MapperHandle handle);

	};


}
