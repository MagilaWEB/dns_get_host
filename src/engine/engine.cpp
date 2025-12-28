#include "engine.h"
#include "version.hpp"
#include "curl/curl.h"

Engine& Engine::get()
{
#if __clang__
	[[clang::no_destroy]]
#endif
	static Engine instance;
	return instance;
}

void Engine::initialize()
{
	// Set UTF-8
	SetConsoleCP(65'001);
	SetConsoleOutputCP(65'001);
	Debug::initLogFile();

	Debug::ok("initialize");
}

void Engine::run()
{
	auto patch_domain = Core::get().userPath() / "domains";
	if (!std::filesystem::is_directory(patch_domain))
		std::filesystem::create_directory(patch_domain);

	auto patch_dns_host = Core::get().userPath() / "hosts";
	if (!std::filesystem::is_directory(patch_dns_host))
		std::filesystem::create_directory(patch_dns_host);

	std::vector<std::string> all_file{};

	while (true)
	{
		all_file.clear();
		for (auto& entry : std::filesystem::directory_iterator(patch_domain))
			all_file.push_back(entry.path().stem().string());

		if (!all_file.empty())
		{
			auto index = InputConsole::selectFromList(all_file);

			std::string file_name{ all_file[index] };

			File domain{};
			domain.open(patch_domain / file_name, ".list", true);

			File dns_host{};
			dns_host.open(patch_dns_host / (file_name + "_dns"), ".list", true);
			dns_host.clear();

			CriticalSection lock{};

			domain.forLine(
				[&dns_host, &lock](std::string str)
				{
					Core::get().addTaskParallel(
						[&dns_host, &lock, str]
						{
							auto lines = Core::get().exec("nslookup " + str);

							constexpr pcstr iter[]{ "Address:", "Addresses:" };

							auto find_it = std::find_if(
								lines.begin(),
								lines.end(),
								[](auto t) { return std::regex_match(t, std::regex{ "Name:.*(?:.*|\\n)" }); }
							);

							if (find_it == lines.end())
								find_it = lines.begin();
							else
								find_it++;

							bool start_addresses{ false };
							bool start_file_write{ false };

							for (; find_it != lines.end(); find_it++)
							{
								auto&			  line = *find_it;
								static std::regex invalid_ipv6{ ".*::" };

								bool		test{ false };
								std::string regex_test{};
								if (!start_addresses)
								{
									for (auto& it : iter)
									{
										regex_test = std::string{ it } + "  ";

										if ((test = std::regex_match(line, std::regex{ std::string{ it } + ".*(?:.*|\\n)" })))
											break;
									}
								}

								if (test && !start_addresses)
								{
									start_addresses = true;

									size_t pos	   = line.find_first_not_of(regex_test);
									auto   address = line.substr(pos, line.length());
									utils::trim(address);

									if (std::regex_match(address, invalid_ipv6))
										continue;

									std::string string = utils::format("%s %s", address.c_str(), str.c_str());

									CRITICAL_SECTION_RAII(lock);
									dns_host.writeText(string);
									start_file_write = true;
									Debug::info("%s", string.c_str());
									continue;
								}

								if (std::regex_match(line, std::regex{ "Aliases:.*(?:.*|\\n)" }) && start_addresses)
									start_addresses = false;

								if (start_addresses)
								{
									utils::trim(line);

									if (line.empty())
										continue;

									if (std::regex_match(line, invalid_ipv6))
										continue;

									std::string string = utils::format("%s %s", line.c_str(), str.c_str());

									CRITICAL_SECTION_RAII(lock);
									dns_host.writeText(string);
									start_file_write = true;
									Debug::info("%s", string.c_str());
								}
							}

							if (!start_file_write)
								Debug::warning("not find IP domain [%s]", str.c_str());
						}
					);

					return false;
				}
			);

			Core::get().waitTaskParallel();

			dns_host.close();
		}

		InputConsole::textAsk("exit");
		if (InputConsole::selectFromList({ "Yes", "No" }) == 0)
			break;
	}

	_finish();
}

void Engine::_finish()
{
	Debug::ok("finish");
	Debug::log.close();
}
