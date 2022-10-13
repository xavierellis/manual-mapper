#include "common.hpp"
#include "mapper_internal.hpp"
#include "helpers/file_helper.hpp"
#include "helpers/pe_helper.hpp"
#include "helpers/memory_helper.hpp"
#include "helpers/process_helper.hpp"
#include "exceptions.hpp"
namespace mapper
{

	[[nodiscard]] static bool fix_page_protection(pe_helper pe, memory_helper mem, std::byte* remote_image) noexcept
	{
		bool result = true;
		for (auto section = pe.sections_begin(); section != pe.sections_end() && section->Misc.VirtualSize > 0; ++section)
		{
			std::uint32_t protection = PAGE_NOACCESS;

			if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
			{
				protection = (section->Characteristics & IMAGE_SCN_MEM_WRITE) ? PAGE_EXECUTE_READWRITE :
				((section->Characteristics & IMAGE_SCN_MEM_READ) ? PAGE_EXECUTE_READ : PAGE_EXECUTE);
			}
			else if (section->Characteristics & IMAGE_SCN_MEM_WRITE) //write but not execute
			{
				protection = PAGE_READWRITE;
			}
			else if (section->Characteristics & IMAGE_SCN_MEM_READ) //read but not write and not execute
			{
				protection = PAGE_READONLY;
			}

			if (!mem.protect(remote_image + section->VirtualAddress, section->Misc.VirtualSize, protection, nullptr))
			{
				result = false;
			}
		}

		return result;
	}

	/*
	 * 1. Check if image supports relocations & required relocation
	 * 2. Make a full local copy of the mapped dll
	 * 3. Apply relocations to the copy using the base of the real mapped image
	 * 4. Copy all bytes after the pe headers back to mapped image, relocations applied
	 */
	[[nodiscard]] static bool apply_relocations(pe_helper pe, memory_helper mem, std::byte* remote_image) noexcept
	{
		const auto delta = reinterpret_cast<std::intptr_t>(remote_image) - static_cast<std::intptr_t>(pe.image_base());

		if (!pe.is_dynamic_base() ||delta == 0) //image loaded at desired base
		{
			return true;
		}

		//make copy

		std::vector copy(static_cast<std::byte*>(pe.data()), static_cast<std::byte*>(pe.data()) + pe.size());
	
		const auto base_relocs = pe.get_directory<pe_helper::RelocData*>(IMAGE_DIRECTORY_ENTRY_BASERELOC);

		//no relocation data in the image
		if (base_relocs == nullptr)
		{
			return true;
		}

		for (auto reloc = base_relocs; reloc; reloc = reinterpret_cast<pe_helper::RelocData*>(reinterpret_cast<
			     std::uintptr_t>(reloc) + reloc->BlockSize))
		{
			const auto count = (reloc->BlockSize - 8) >> 1;

			for (std::uint32_t i = 0; i < count; i++)
			{
				const auto offset = reloc->Item->Offset % 4096;

				switch (reloc->Item[i].Type)
				{
				case IMAGE_REL_BASED_ABSOLUTE: //no relocation needed
					continue;

				case IMAGE_REL_BASED_HIGHLOW:
				case IMAGE_REL_BASED_DIR64:
					{
						std::uintptr_t rva = offset + reloc->PageRVA;

						if constexpr (x64)
						{
							const auto fixed = *reinterpret_cast<std::uint64_t*>(copy.data() + rva) + delta;
							*reinterpret_cast<std::uint64_t*>(copy.data() + rva) = fixed;
						}
						else
						{
							const auto fixed = *reinterpret_cast<std::uint32_t*>(copy.data() + rva) + delta;
							*reinterpret_cast<std::uint32_t*>(copy.data() + rva) = fixed;
						}

						break;
					}

				default: //unsupported relocation -> failure 
					return false;
				}
			}
		}

		return mem.write(remote_image + 0x1000, copy.size() - 0x1000, copy.data() + 0x1000);
	}


	[[nodiscard]] static bool install_exception_handlers(pe_helper pe, memory_helper mem, std::byte* remote_image) noexcept
	{
		if constexpr (!x64)
		{
			//TODO implement on 32 bit
		}

		exception_shellcode_generator generator(pe, mem, remote_image);

		const auto local_code = generator.GetCode();

		if (local_code == nullptr)
		{
			return false;
		}

	}

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

		for (auto section = pe.sections_begin(); 
			section != pe.sections_end() && section->SizeOfRawData > 0; 
			++section)
		{
			if (!mem.write(remote_image + section->VirtualAddress, section->SizeOfRawData,
			               pe.RVAtoVA<std::byte*>(section->PointerToRawData)))
			{
				mem.deallocate(remote_image, pe.size());
				return eMapperError::Failed_Writing_Remote_Memory;
			}
		}

		if (apply_relocations(pe, mem, remote_image) == false)
		{
			mem.deallocate(remote_image, pe.size());
			return eMapperError::Failed_Handling_Relocations;
		}

		if ((flags & eMapperFlags_SupportExceptions) && install_exception_handlers(pe, mem, remote_image) == false)
		{
			mem.deallocate(remote_image, pe.size());
			return eMapperError::Failed_Installing_Exception_Handlers;
		}


		/*
		 *
		 * Still need to handle imports!!!!
		 *
		 *
		 *
		 *
		 */



		if (fix_page_protection(pe, mem, remote_image) == false) //this is the final stage...
		{
			return eMapperError::Failed_Fixing_Protection;
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