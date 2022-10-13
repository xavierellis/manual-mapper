#pragma once
namespace mapper
{

	//wraps a non-owning pointer to the start of a PE file
	class [[nodiscard]] pe_helper
	{
	public:

		struct RelocData
		{
			ULONG PageRVA;
			ULONG BlockSize;

			struct
			{
				WORD Offset : 12;
				WORD Type : 4;
			}Item[1];
		};

		pe_helper() = delete;

		constexpr pe_helper(void* ptr) noexcept : m_file{ptr}
		{
			assert(ptr != nullptr);
		}

		[[nodiscard]] constexpr void* data() const noexcept
		{
			return m_file;
		}

		[[nodiscard]] constexpr PIMAGE_DOS_HEADER dos_header() const noexcept
		{
			return static_cast<PIMAGE_DOS_HEADER>(m_file);
		}

		[[nodiscard]] constexpr PIMAGE_NT_HEADERS nt_header() const noexcept
		{
			return RVAtoVA<PIMAGE_NT_HEADERS>(dos_header()->e_lfanew);
		}

		[[nodiscard]] constexpr PIMAGE_OPTIONAL_HEADER optional_header() const noexcept
		{
			return &nt_header()->OptionalHeader;
		}

		[[nodiscard]] constexpr PIMAGE_FILE_HEADER file_header() const noexcept
		{
			return &nt_header()->FileHeader;
		}

		[[nodiscard]] constexpr bool is_valid_shallow() const noexcept
		{
			return dos_header()->e_magic == 0x5A4D && dos_header()->e_lfanew != 0;
		}

		[[nodiscard]] constexpr bool is_host_arch() const noexcept
		{
			const auto arch = file_header()->Machine;
			return x64 ? arch == IMAGE_FILE_MACHINE_AMD64 : arch == IMAGE_FILE_MACHINE_I386;
		}

		[[nodiscard]] constexpr std::uint32_t size() const noexcept
		{
			return optional_header()->SizeOfImage;
		}

		[[nodiscard]] constexpr std::uintptr_t image_base() const noexcept
		{
			return optional_header()->ImageBase;
		}

		[[nodiscard]] constexpr bool is_dynamic_base() const noexcept
		{
			return optional_header()->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
		}

		[[nodiscard]] constexpr PIMAGE_DATA_DIRECTORY get_directory(std::uint32_t index) const noexcept
		{
			assert(std::size(optional_header()->DataDirectory) && "get_directory tried to read an invalid directory index!");
			return &optional_header()->DataDirectory[index];
		}

		template <typename Directory>
		requires std::is_pointer_v<Directory>
		[[nodiscard]] Directory get_directory(std::uint32_t index) const noexcept
		{
			const auto dir = get_directory(index);

			if (dir == nullptr || dir->VirtualAddress == 0)
			{
				return nullptr;
			}

			assert(dir->Size == sizeof(*Directory) && "incorrectly sized directory");

			return RVAtoVA<Directory>(dir->VirtualAddress);
		}

		[[nodiscard]] PIMAGE_SECTION_HEADER sections_begin() const noexcept
		{
			return IMAGE_FIRST_SECTION(nt_header());
		}

		[[nodiscard]] constexpr PIMAGE_SECTION_HEADER sections_end() const noexcept
		{
			return sections_begin() + file_header()->NumberOfSections;
		}

		template <typename T>
		requires std::is_pointer_v<T>
		[[nodiscard]] T RVAtoVA(auto RVA) const noexcept
		{
			return reinterpret_cast<T>(reinterpret_cast<std::uintptr_t>(m_file) + static_cast<std::uintptr_t>(RVA));
		}

	protected:
		void* m_file;
	};


}