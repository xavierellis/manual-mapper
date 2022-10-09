#pragma once
namespace mapper
{

	//wraps a non-owning pointer to the start of a PE file
	class [[nodiscard]] pe_helper
	{
	public:
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


		[[nodiscard]] PIMAGE_SECTION_HEADER sections_begin() const noexcept
		{
			return IMAGE_FIRST_SECTION(nt_header());
		}

		[[nodiscard]] constexpr PIMAGE_SECTION_HEADER sections_end() const noexcept
		{
			return sections_begin() + file_header()->NumberOfSections;
		}

		template <typename T>
		T RVAtoVA(auto RVA) const noexcept
		{
			return reinterpret_cast<T>(reinterpret_cast<std::uintptr_t>(m_file) + static_cast<std::uintptr_t>(RVA));
		}

	protected:
		void* m_file;



	};


}