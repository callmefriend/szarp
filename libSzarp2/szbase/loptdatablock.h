#ifndef __LOPTDATABLOCK_H__
#define __LOPTDATABLOCK_H__

/* 
  SZARP: SCADA software 
  

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "config.h"

#include <boost/shared_ptr.hpp>

#include "luadatablock.h"

LuaDatablock *create_lua_data_block(szb_buffer_t *b, TParam* p, int y, int m);

#ifdef LUA_PARAM_OPTIMISE

namespace LuaExec {

class ExecutionEngine;

typedef double Val;

struct ParamRef {
	szb_buffer_t *m_buffer;
	TParam* m_param;
	size_t m_param_index;
	ExecutionEngine* m_exec_engine;
public:
	void SetExecutionEngine(ExecutionEngine *exec_engine) { m_exec_engine = exec_engine; }
	Val Value(const double &time, const double& period);
};

class Var {
	size_t m_var_no;
	ExecutionEngine* m_ee;
public:
	Var(size_t var_no) : m_var_no(var_no) {}
	Val& operator()();
	Val& operator=(const Val& val);
	void SetExecutionEngine(ExecutionEngine *ee) { m_ee = ee; }
};


class Expression {
public:
	virtual Val Value() = 0;
};

class Statement {
public:
	virtual void Execute() = 0;
};

typedef boost::shared_ptr<Expression> PExpression;
typedef boost::shared_ptr<Statement> PStatement;

class StatementList : public Statement {
	std::vector<PStatement> m_statements;
public:
	void AddStatement(PStatement statement);
	virtual void Execute();
};

class Param {
public:
	bool m_optimized;
	std::vector<Var> m_vars;
	std::vector<ParamRef> m_par_refs;
	std::map<szb_buffer_t*, time_t> m_last_update_times;
	PStatement m_statement;
};

};

class LuaOptDatablock : public LuaDatablock 
{
	void CalculateValues(LuaExec::ExecutionEngine *ee, int end_probe);
public:
	LuaExec::Param *exec_param;
	LuaOptDatablock(szb_buffer_t * b, TParam * p, int y, int m);
	virtual void Refresh();
};

#endif

#endif
