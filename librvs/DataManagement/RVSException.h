/// ********************************************************** ///
/// Name: .h                                                ///
/// Desc:        ///
/// ********************************************************** ///

#pragma once

#ifndef RVSEXCEPTION_H
#define RVSEXCEPTION_H

#include <iostream>
#include <exception>

namespace RVS
{
namespace DataManagement
{
	// 
	class DataTypeMismatchException : public std::exception
	{
	public:
		//DataTypeMismatchException(void) = delete;
		inline DataTypeMismatchException(const char* sql) { this->sql = sql; }

		virtual inline const char* what() const throw()
		{
			return "Attempted to retrieve data of different type.";
		}
	private:
		const char* sql;
	};

	// 
	class DataNotFoundException : public std::exception
	{
	public:
		//DataNotFoundException(void);
		inline DataNotFoundException(const char* sql) { this->sql = sql; }

		virtual inline const char* what() const throw()
		{
			return "Requested data not found in database.";
		}

	private:
		const char* sql;

	};
}
}

#endif
