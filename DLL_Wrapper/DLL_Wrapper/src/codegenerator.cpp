#include "stdafx.h"
#include "codegenerator.h"


std::string CodeGenerator::_generateParamatersDef(const std::vector<Configuration::InterceptFunction::Param>& params)
{
	std::stringstream		ss;

	for (auto itr = params.begin(); itr != params.end(); ++itr)
	{
		ss << itr->type << " " << itr->name;
		if (itr + 1 != params.end())
		{
			ss << ", ";
		}
	}
	return ss.str();
}

std::string CodeGenerator::_generateParamatersCall(const std::vector<Configuration::InterceptFunction::Param>& params)
{
	std::stringstream		ss;

	for (auto itr = params.begin(); itr != params.end(); ++itr)
	{
		ss << itr->name;
		if (itr + 1 != params.end())
		{
			ss << ", ";
		}
	}
	return ss.str();
}

CodeGenerator::CodeGenerator(const Configuration::Configuration & config)
	: _config(config), _forwards_fname(FORWARDS_FNAME),
	_intercepts_fname(INTERCEPTS_FNAME), _call_table_fname(CALL_TABLE_FNAME)
{
}

CodeGenerator::~CodeGenerator()
{
}

bool CodeGenerator::generate(const PEHeader::Exports & exports)
{
	unsigned int		function_index;
	const std::vector<Configuration::InterceptFunction>	intercept_functions =
		_config.getInterceptFunctions().getFunctions();

	function_index = 0;
	if (!generateForwardsHeader(exports))
		return false;
	for (auto itr = intercept_functions.begin(); itr != intercept_functions.end(); ++itr)
	{
		if (!generateInterceptedFunction(*itr, function_index))
			return false;
		if (!generateBridge(*itr, function_index))
			return false;
		++function_index;
	}
	if (!generateCExtCallTable(function_index))
		return false;
	if (!generateInterceptsHeader(function_index))
		return false;
	return true;
}

bool CodeGenerator::generateForwardsHeader(const PEHeader::Exports & exports)
{
	FILE		*f_forwards;
	std::string	file_path;
	
	file_path = _config.getOutputDir() + "/" + _forwards_fname;
	f_forwards = fopen(file_path.c_str(), "w");
	if (f_forwards == NULL)
	{
		std::cerr << "Could not open or create file: " << file_path << std::endl;
		return false;
	}

	fprintf(f_forwards, "//This file has been generated by DLL_Wrapper");
	fprintf(f_forwards, "#pragma once\n\n");
	std::stringstream	ss = exports.toVsLinkerExport(_config.getTargetRename().c_str(),
		_config.getInterceptFunctions());
	fprintf(f_forwards, ss.str().c_str());
	fclose(f_forwards);

	return true;
}

bool CodeGenerator::generateInterceptsHeader(unsigned int call_table_size)
{
	FILE												*f_intercepts;
	std::string											file_path;
	const std::vector<Configuration::InterceptFunction>	intercept_functions = 
		_config.getInterceptFunctions().getFunctions();
	std::string											paramaters;
	std::string											bridge_name;

	file_path = _config.getOutputDir() + "/" + _intercepts_fname;
	f_intercepts = fopen(file_path.c_str(), "w");
	if (f_intercepts == NULL)
	{
		std::cerr << "Could not open or create file: " << file_path << std::endl;
		return false;
	}

	fprintf(f_intercepts, intercepts_h_header, call_table_size);

	for (auto itr = intercept_functions.begin(); itr != intercept_functions.end(); ++itr)
	{
		paramaters = _generateParamatersDef(itr->_paramaters);
		bridge_name = itr->_name + "_bridge";
		fprintf(f_intercepts, intercepts_h_function,
			itr->_return_type.c_str(), itr->_name.c_str(), paramaters.c_str(),
			itr->_return_type.c_str(), bridge_name.c_str(), paramaters.c_str());
	}
	fclose(f_intercepts);
	return true;
}

bool CodeGenerator::generateCExtCallTable(unsigned int call_table_size)
{
	FILE			*f_call_table;
	std::string		file_path;

	file_path = _config.getOutputDir() + "/" + _call_table_fname;
	f_call_table = fopen(file_path.c_str(), "w");
	if (f_call_table == NULL)
	{
		std::cerr << "Could not open or create file: " << file_path << std::endl;
		return false;
	}
	fprintf(f_call_table, c_ext_intercepts_global, call_table_size);
	fclose(f_call_table);
	return true;
}

bool CodeGenerator::generateInterceptedFunction(const Configuration::InterceptFunction & function, unsigned int function_index)
{
	FILE				*f_function_cpp;
	std::string			file_path = _config.getOutputDir() + "/" + function._name + ".cpp";
	std::string			paramaters = _generateParamatersDef(function._paramaters);
	std::string			param_call = _generateParamatersCall(function._paramaters);
	std::string			bridge_name = function._name + "_bridge";

	f_function_cpp = fopen(file_path.c_str(), "w");
	if (f_function_cpp == NULL)
	{
		std::cerr << "Could not open or create file: " << file_path << std::endl;
		return false;
	}

	fprintf(f_function_cpp, intercepted_function,
		function._return_type.c_str(), function._name.c_str(), paramaters.c_str(),
		_config.getTargetRename().c_str(),
		function._name.c_str(), function._name.c_str(), function._name.c_str(),
		function_index, bridge_name.c_str(), param_call.c_str()
	);

	fclose(f_function_cpp);
	return true;
}

bool CodeGenerator::generateBridge(const Configuration::InterceptFunction & function, unsigned int function_index)
{
	FILE			*f_bridge_asm;
	std::string		bridge_name = function._name + "_bridge";
	std::string		file_path = _config.getOutputDir() + "/" + bridge_name + ".asm";

	f_bridge_asm = fopen(file_path.c_str(), "w");
	if (f_bridge_asm == NULL)
	{
		std::cerr << "Could not open or create file: " << file_path << std::endl;
		return false;
	}

	fprintf(f_bridge_asm, asm_bridge,
		bridge_name.c_str(), function_index, bridge_name.c_str(),
		bridge_name.c_str(), bridge_name.c_str(), bridge_name.c_str(),
		bridge_name.c_str(), bridge_name.c_str()
		);
	
	fclose(f_bridge_asm);
	return true;
}
