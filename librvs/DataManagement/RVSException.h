/// ********************************************************** ///
/// Name: RVSException.h                                       ///
/// Desc: Exception classes for RVS database operations        ///
/// Standard: C++20                                            ///
/// ********************************************************** ///

#pragma once

#ifndef RVSEXCEPTION_H
#define RVSEXCEPTION_H

#include <exception>
#include <string>
#include <string_view>

namespace RVS
{
namespace DataManagement
{
	class DataTypeMismatchException : public std::exception
	{
	public:
		explicit DataTypeMismatchException(std::string_view sql_query = "")
			: sql_(sql_query)
		{
		}

		[[nodiscard]] const char* what() const noexcept override
		{
			return "Attempted to retrieve data of different type.";
		}

		[[nodiscard]] const std::string& sql() const noexcept
		{
			return sql_;
		}

	private:
		std::string sql_;
	};

	class DataNotFoundException : public std::exception
	{
	public:
		explicit DataNotFoundException(std::string_view sql_query = "")
			: sql_(sql_query)
		{
		}

		[[nodiscard]] const char* what() const noexcept override
		{
			return "Requested data not found in database.";
		}

		[[nodiscard]] const std::string& sql() const noexcept
		{
			return sql_;
		}

	private:
		std::string sql_;
	};
}
}

#endif // RVSEXCEPTION_H
