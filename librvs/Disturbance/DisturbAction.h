/// ********************************************************** ///
/// Name: DisturbAction.h                                      ///
/// Desc: Value type representing a single disturbance action. ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef DISTURBACTION_H
#define DISTURBACTION_H

#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace RVS
{
namespace Disturbance
{
	class DisturbAction
	{
	public:
		DisturbAction(
			const int actionYear,
			std::string actionType,
			std::string actionSubType,
			std::map<std::string, double> params)
			: actionYear(actionYear),
			  actionType(std::move(actionType)),
			  actionSubType(std::move(actionSubType)),
			  params(std::move(params))
		{
		}

		virtual ~DisturbAction() = default;

		DisturbAction(const DisturbAction&) = default;
		DisturbAction& operator=(const DisturbAction&) = default;
		DisturbAction(DisturbAction&&) noexcept = default;
		DisturbAction& operator=(DisturbAction&&) noexcept = default;

		[[nodiscard]] constexpr int getActionYear() const noexcept { return actionYear; }
		[[nodiscard]] const std::string& getActionType() const noexcept { return actionType; }
		[[nodiscard]] const std::string& getActionSubType() const noexcept { return actionSubType; }
		[[nodiscard]] const std::map<std::string, double>& getParameters() const noexcept { return params; }

	private:
		int actionYear = 0;
		std::string actionType;
		std::string actionSubType;
		std::map<std::string, double> params;
	};
}
}

#endif // DISTURBACTION_H
