#pragma once

class IEngineAPI
{
protected:
	explicit IEngineAPI()					 = default;
	IEngineAPI(const IEngineAPI&)			 = default;
	IEngineAPI(IEngineAPI&&)				 = default;
	IEngineAPI& operator=(const IEngineAPI&) = default;
	IEngineAPI& operator=(IEngineAPI&&)		 = default;
	virtual ~IEngineAPI()					 = default;

};
