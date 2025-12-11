#pragma once
#include "engine_api.hpp"

typedef void CURL;

class ENGINE_API Engine final : public IEngineAPI
{

public:
	static Engine& get();

	void initialize();
	void run();

private:
	void _finish();
};
