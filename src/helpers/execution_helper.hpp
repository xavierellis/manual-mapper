#pragma once
#include "process_helper.hpp"
namespace mapper
{
	/*
	 * Class responsible for altering control flow of the remote process.
	 * E.g. hijack a thread, create a thread
	 */
	class execution_helper : process_helper
	{

	public:

		execution_helper(HANDLE process) noexcept : process_helper(process)
		{}

		template<typename Fn>
		int32_t execute_in_new_thread(Fn&& func) noexcept
		{


		}

	};
}