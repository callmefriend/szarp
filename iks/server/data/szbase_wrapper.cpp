#include "szbase_wrapper.h"

#include <conversion.h>

bool SzbaseWrapper::initialized = false;
boost::filesystem::path SzbaseWrapper::szarp_dir;

bool SzbaseWrapper::init( const std::string& _szarp_dir )
{
	if( initialized )
		return boost::filesystem::path(_szarp_dir) == szarp_dir;

	szarp_dir = _szarp_dir;

	IPKContainer::Init( szarp_dir.wstring(), szarp_dir.wstring(), L"pl_PL", new NullMutex());
	Szbase::Init( szarp_dir.wstring() , NULL );

	initialized = true;

	return true;
}

SzbaseWrapper::SzbaseWrapper( const std::string& base )
	: base_name(base)
{
	std::wstring wbp( base_name.begin() , base_name.end() );
	IPKContainer::GetObject()->GetConfig( wbp );
}

double SzbaseWrapper::get_avg( const std::string& param , time_t time , ProbeType type ) const
	throw( szbase_init_error, szbase_get_value_error )
{
	if( !SzbaseWrapper::is_initialized() )
		throw szbase_init_error("Szbase not initialized");

	std::string bp = base_name + ":" + param;

	std::basic_string<unsigned char> ubp( bp.begin() , bp.end() );

	std::wstring wbp = SC::U2S( ubp );

	bool is_fixed, ok;
	std::wstring error;
	double val = Szbase::GetObject()->GetValue(
			wbp ,
			time , type , 0 ,
			&is_fixed , ok , error );

	if( !ok )
		throw szbase_get_value_error("Cannot get value from param " + param + ": " + SC::S2A(error) );

	return val;
}

double SzbaseWrapper:: get_avg( const std::string& param , time_t start , time_t end ) const
	throw( szbase_init_error, szbase_get_value_error )
{
	if( !SzbaseWrapper::is_initialized() )
		throw szbase_init_error("Szbase not initialized");

	int len = end - start;

	std::string bp = base_name + ":" + param;
	std::wstring wbp( bp.begin() , bp.end() );

	bool is_fixed, ok;
	std::wstring error;
	double val = Szbase::GetObject()->GetValue(
			wbp ,
			start , PT_CUSTOM , len ,
			&is_fixed , ok , error );

	if( !ok )
		throw szbase_get_value_error("Cannot get value from param " + param + ": " + SC::S2A(error) );

	return val;
}

time_t SzbaseWrapper::next( time_t t , SzbaseWrapper::ProbeType pt )
{
	return szb_move_time( t , 1 , pt , 0 );
}

