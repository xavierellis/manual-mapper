#pragma once
namespace mapper
{
	struct [[nodiscard]] memory_helper
	{
		memory_helper() = delete;

		memory_helper(HANDLE proc) noexcept : process{proc}
		{
		}

		[[nodiscard]] bool read(void* ptr, std::size_t size, void* buf) const noexcept
		{
			std::size_t bytes_read{};

			if (!ReadProcessMemory(process, ptr, buf, size, &bytes_read))
			{
				return false;
			}

			return bytes_read == size;
		}

		template <std::integral T>
		[[nodiscard]] auto read(T ptr, std::size_t size, void* buf) const noexcept
		{
			return read(reinterpret_cast<void*>(ptr), size, buf);
		}

		template <std::integral T, std::integral Y>
		[[nodiscard]] auto read(T ptr, std::size_t size, Y buf) const noexcept
		{
			return read(reinterpret_cast<void*>(ptr), size, reinterpret_cast<void*>(buf));
		}

		[[nodiscard]] bool write(void* ptr, std::size_t size, void* buf) const noexcept
		{
			std::size_t bytes_written{};

			if (!WriteProcessMemory(process, ptr, buf, size, &bytes_written))
			{
				return false;
			}

			return bytes_written == size;
		}

		template <std::integral T>
		[[nodiscard]] auto write(T ptr, std::size_t size, void* buf) const noexcept
		{
			return write(reinterpret_cast<void*>(ptr), size, buf);
		}

		template <std::integral T, std::integral Y>
		[[nodiscard]] auto write(T ptr, std::size_t size, Y buf) const noexcept
		{
			return write(reinterpret_cast<void*>(ptr), size, reinterpret_cast<void*>(buf));
		}


		[[nodiscard]] void* allocate(std::size_t size, std::uint32_t protection) const noexcept
		{
			return VirtualAllocEx(process, nullptr, size, MEM_COMMIT | MEM_RESERVE, protection);
		}

		template<typename T>
		[[nodiscard]] T allocate(std::size_t size, std::uint32_t protection) const noexcept
		{
			return reinterpret_cast<T>(allocate(size, protection));
		}

		bool deallocate(void* ptr, std::size_t size) const noexcept
		{
			return VirtualFreeEx(process, ptr, size, MEM_RELEASE);
		}

		bool protect(void* ptr, std::size_t size, std::uint32_t protection, std::uint32_t* old_protection) const noexcept
		{
			[[maybe_unused]] std::uint32_t stub{};
			return VirtualProtectEx(process, ptr, size, protection, reinterpret_cast<PDWORD>(old_protection ? old_protection : &stub));
		}

		HANDLE process{};
	};

	//non movable, non copyable RAII wrapper for remote memory protection
	struct scoped_protection
	{
		scoped_protection() = delete;
		scoped_protection(const scoped_protection&) = delete;
		scoped_protection(const scoped_protection&&) = delete;
		scoped_protection(scoped_protection&) = delete;
		scoped_protection(scoped_protection&&) = delete;
		scoped_protection& operator=(const scoped_protection&) = delete;

		scoped_protection(HANDLE proc, void* addr, std::size_t size, DWORD prot) noexcept
		: process{ proc }, address{addr}, size{size}
		{
			assert(addr != nullptr && "nullptr passed to scoped_protection object!");
			VirtualProtectEx(process, address, size, prot, &original);
		}

		~scoped_protection()
		{
			reset();
		}

		void reset()
		{
			VirtualProtectEx(process, address, size, original, &original);
		}

		HANDLE process;
		PVOID address = nullptr;
		SIZE_T size = 0;
		DWORD original = 0;
	};


}
