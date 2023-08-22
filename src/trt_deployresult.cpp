#include "trt_deployresult.h"
#include "config.h"

namespace gf
{

void FightPPTSMResults::Get(const std::string &idx_name, std::vector<float> &res)
{
	if (std::find(Config::OUTPUT_NAMES.begin(), Config::OUTPUT_NAMES.end(),
				  idx_name) != Config::OUTPUT_NAMES.end()) {
		res = m_res[idx_name];
	}
}

void FightPPTSMResults::Set(const std::pair<std::string, std::vector<float>> &data)
{
	if (std::find(Config::OUTPUT_NAMES.begin(), Config::OUTPUT_NAMES.end(),
				  data.first) != Config::OUTPUT_NAMES.end()) {
		m_res[data.first] = data.second;
	}
}

void FightPPTSMResults::Clear()
{
	m_res.clear();
}
}

