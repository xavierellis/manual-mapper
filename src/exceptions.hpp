#pragma once
#include "common.hpp"
#include "helpers/pe_helper.hpp"
#include "helpers/memory_helper.hpp"
namespace mapper
{

	struct exception_shellcode_generator : jitasm::Frontend
	{
		exception_shellcode_generator() = delete;

		exception_shellcode_generator(pe_helper pe, memory_helper mem, std::byte* ptr) noexcept
		: pe(pe), memory(mem), remote_image(ptr)
		{}

		void InternalMain() override
		{
			if constexpr (!x64) {}
			else
			{
				
			}
		}


		pe_helper pe;
		memory_helper memory;
		std::byte* remote_image;
	};

}