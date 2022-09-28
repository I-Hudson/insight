#include "Core/CommandLineArgs.h"
#include "Core/Logger.h"

#include "Core/StringUtils.h"

#include <fstream>
#include <sstream>

namespace Insight
{
	namespace Core
	{
		std::unordered_map<std::string, CommandLineValue> CommandLineArgs::m_args;
		CommandLineValue CommandLineArgs::s_default_command_line_value;

		CommandLineValue::CommandLineValue(std::string key, std::string value)
			: Key(std::move(key))
			, Value(std::move(value))
		{ }

		std::string CommandLineValue::GetString() const
		{
			return Value;
		}

		i32 CommandLineValue::GetI32() const
		{
			return std::stoi(Value);
		}

		u32 CommandLineValue::GetU32() const
		{
			return std::stoul(Value);
		}

		bool CommandLineValue::GetBool() const
		{
			// Bool should be defined as 0/1 not true and false as it works better in code.
			return std::stoi(Value);
		}

		void CommandLineArgs::ParseCommandLine(int const argc, char** const argv)
		{
			// Start at one as the first command line args is the path to the .exe
			for (int i = 0; i < argc; ++i)
			{
				std::string arg = argv[i];
				const u64 equal_index = arg.find('=');

				std::string key = equal_index != std::string::npos ? arg.substr(0, equal_index) : "";
				std::string value = equal_index != std::string::npos ? arg.substr(arg.find('=') + 1) : "";

				ToLowwer(key);
				ToLowwer(value);

				if (key.empty() || value.empty())
				{
					IS_CORE_WARN("[CommandLineArgs::ParseCommandLine] Key '{}' and/or value '{}' is empty.", key, value);
					continue;
				}

				if (m_args.find(key) != m_args.end())
				{
					IS_CORE_WARN("[CommandLineArgs::ParseCommandLine] Key '{}' already defined with value '{}'. Second definition ignored.", key, value);
					continue;
				}

				if (value == "true")
				{
					value = "1";
				}
				else if (value == "false")
				{
					value = "0";
				}

				m_args[key] = CommandLineValue(key, value);
			}
		}

		void CommandLineArgs::ParseCommandLine(const char* file_path)
		{
			std::vector<std::string> args;
			std::vector<char*> c_args;

			std::ifstream file;
			file.open(file_path);
			if (!file.is_open())
			{
				IS_CORE_WARN("[CommandLineArgs::ParseCommandLine] File '{} was unabled to be opened.'", file_path);
				return;
			}
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			std::string file_data = buffer.str();
			u32 offset = 0;
			while (offset < file_data.size())
			{
				const u32 next_space = file_data.find(' ', offset);
				const u32 next_new_line = file_data.find('\n', offset);
				const u32 new_offset = next_space < next_new_line ? next_space : next_new_line;

				std::string arg = file_data.substr(offset, new_offset - offset);
				if (!arg.empty())
				{
					args.push_back(std::move(arg));
				}
				offset = new_offset + 1;
			}

			for (size_t i = 0; i < args.size(); ++i)
			{
				c_args.push_back(&args[i].front());
			}

			ParseCommandLine(static_cast<int>(args.size()), c_args.data());
		}

		Ptr<CommandLineValue> CommandLineArgs::GetCommandLineValue(std::string key)
		{
			ToLowwer(key);
			if (auto itr = m_args.find(key); itr != m_args.end())
			{
				return &const_cast<CommandLineValue&>(itr->second);
			}
			return &s_default_command_line_value;
		}
	}
}