/// ********************************************************** ///
/// Name: DataTable.h                                          ///
/// Desc:         ///
/// Base Class(es): none                                       ///
/// Requires:                             ///
/// ********************************************************** ///

#pragma once

#ifndef DATATABLE_H
#define DATATABLE_H

namespace RVS
{
namespace DataManagement
{
	class DataTable
	{
	public:
		inline DataTable() {}
		virtual inline ~DataTable(void) {}


		inline int getColumns() { return columns; }
		inline int getRows() { return rows; }
		inline char* getColumnNames() { return columnNames; }

	private:
		int columns;
		int rows;
		
		char* columnNames;
		int* columnTypes;
		int* colimnSizes;

		
	};
}
}

#endif