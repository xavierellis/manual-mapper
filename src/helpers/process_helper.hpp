#pragma once
#include "memory_helper.hpp"
namespace mapper
{

	class process_helper
	{
	public:
		process_helper() = delete;

		process_helper(HANDLE process) noexcept : process(process)
		{
			read_info();
		}


		[[nodiscard]] std::vector<std::byte> copy_process() const noexcept
		{
			std::vector<std::byte> result(size);

			if (base_address == 0 || size == 0)
			{
				return result;
			}

			const memory_helper mem(process);

			//if the memory read fails, clear the buffer so it is always empty on error or non-empty on success
			if (!mem.read(base_address, size, result.data()))
			{
				result.clear();
			}

			return result;
		}

	private:

		void read_info()
		{
			WCHAR name[MAX_PATH];
			GetModuleFileNameExW(process, nullptr, name, std::size(name));

			if (const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(process));
				snapshot != INVALID_HANDLE_VALUE)
			{
				MODULEENTRY32W module;
				module.dwSize = sizeof(module);

				if (Module32FirstW(snapshot, &module))
				{
					do
					{
						if (wcscmp(module.szModule, name) == 0)
						{
							CloseHandle(snapshot);
							size = module.modBaseSize;
							base_address = reinterpret_cast<std::uintptr_t>(module.modBaseAddr);
						}

					} while (Module32NextW(snapshot, &module));
				}

				CloseHandle(snapshot);
			}
		}
		
		HANDLE process;
		std::uintptr_t base_address{ 0 };
		std::size_t size{ 0 };
		
	};

}