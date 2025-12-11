#pragma once

class CORE_API Core final
{
	FastLock						  _task_lock;
	FastLock						  _task_parallel_lock;
	std::deque<std::function<void()>> _task;
	std::deque<std::function<void()>> _task_parallel;

	std::atomic_bool _quit_task{ false };

	Core();
	~Core() = default;

	std::filesystem::path _current_path{};
	std::filesystem::path _bin_path{};
	std::filesystem::path _binaries_path{};
	std::filesystem::path _configs_path{};
	std::filesystem::path _user_path{};

public:
	Core(Core&&) = delete;

public:
	static Core& get();

	void initialize(const std::string& command_line);

	std::list<std::string> exec(std::string cmd);
	void parallel_run();
	void finish();

	std::filesystem::path currentPath() const;
	std::filesystem::path binPath() const;
	std::filesystem::path binariesPath() const;
	std::filesystem::path configsPath() const;
	std::filesystem::path userPath() const;

	void addTask(std::function<void()>&& callback);
	void waitTask();

	void addTaskParallel(std::function<void()>&& callback);
	void waitTaskParallel();

	std::deque<std::function<void()>>& getTask();
	FastLock&						   getTaskLock();
};
