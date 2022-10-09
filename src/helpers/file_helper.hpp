#pragma once
namespace mapper
{
	namespace file_helper
	{
		inline std::vector<std::byte> read_file(std::string_view path)
		{
			std::vector<std::byte> result;

			assert(path.empty() == false && "Cannot pass an empty path to read_file");

			std::ifstream file(path.data(), std::ios::binary | std::ios::ate);

			if (!file.is_open() || !file.good())
			{
				return result;
			}

			const auto size = file.tellg();

			result.reserve(size);

			file.seekg(0);
			file.read(reinterpret_cast<char*>(result.data()), size);

			return result;
		}

		inline auto read_file(const std::filesystem::path& path)
		{
			return read_file(std::string_view(path.string()));
		}
	}
}