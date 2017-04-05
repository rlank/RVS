#pragma once


#include <string>
#include <map>

using namespace std;

namespace RVS
{
namespace Disturbance
{

	class DisturbAction
	{
	public:
		DisturbAction(int actionYear, string actionType, string actionSubType, map<string, double> params);
		virtual ~DisturbAction();

		inline const int getActionYear() { return actionYear; }
		inline const string getActionType() { return actionType; }
		inline const string getActionSubType() { return actionSubType; }
		inline const map<string, double> getParameters() { return params; }

	private:
		int actionYear;
		string actionType;
		string actionSubType;

		map<string, double> params;
	};
}
}