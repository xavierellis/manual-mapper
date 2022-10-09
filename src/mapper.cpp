#include "common.hpp"
#include "mapper_internal.hpp"
#include "helpers/file_helper.hpp"
#include "helpers/pe_helper.hpp"
#include "helpers/memory_helper.hpp"
namespace mapper
{

	eMapperError MAPPER_API Mapper::map_ex(HANDLE process, std::string_view path, MapperHandle& out_handle, eMapperFlags flags) noexcept
	{

		if (auto file = file_helper::read_file(path); !file.empty())
		{
			return Mapper::map_ex(process, std::span(file), out_handle, flags);
		}

		return eMapperError::Invalid_File_Path;
	}

	eMapperError MAPPER_API Mapper::map_ex(HANDLE process, std::span<std::byte> dll, MapperHandle& out_handle, eMapperFlags flags) noexcept
	{
		assert(dll.empty() == false && "Empty dll buffer passed to Mapper::max_ex");

		const pe_helper pe(dll.data());
		const memory_helper mem(process);

		if (!pe.is_valid_shallow())
		{
			return eMapperError::Invalid_Pe_Signature;
		}

		if (!pe.is_host_arch())
		{
			return eMapperError::Invalid_Pe_Architecture;
		}

		const auto remote_image = mem.allocate<std::byte*>(pe.size(), PAGE_EXECUTE_READWRITE);
		if (remote_image == nullptr)
		{
			return eMapperError::Allocation_Failed;
		}

		//write PE headers, always first 0x1000 bytes
		if (!mem.write(remote_image, 0x1000, pe.data()))
		{
			mem.deallocate(remote_image, pe.size());
			return eMapperError::Failed_Writing_Remote_Memory;
		}

		for (auto section = pe.sections_begin(); section != pe.sections_end(); ++section)
		{
			if (section->SizeOfRawData == 0)
				continue;

			if (!mem.write(remote_image + section->VirtualAddress, section->SizeOfRawData, pe.RVAtoVA<std::byte*>(section->PointerToRawData)))
			{
				mem.deallocate(remote_image, pe.size());
				return eMapperError::Failed_Writing_Remote_Memory;
			}
		}

		return eMapperError::Success;
	}

	eMapperError MAPPER_API Mapper::unmap_ex(HANDLE process, MapperHandle handle) noexcept
	{
		return eMapperError::Success;
	}

	eMapperError MAPPER_API Mapper::map(std::string_view path, MapperHandle& out_handle, eMapperFlags flags) noexcept
	{
		return map_ex(GetCurrentProcess(), path, out_handle, flags);
	}

	eMapperError MAPPER_API Mapper::map(std::span<std::byte> dll, MapperHandle& out_handle, eMapperFlags flags) noexcept
	{
		return map_ex(GetCurrentProcess(), dll, out_handle, flags);
	}

	eMapperError MAPPER_API Mapper::unmap(MapperHandle handle) noexcept
	{
		return unmap_ex(GetCurrentProcess(), handle);
	}

}