#ifndef __CONNECTIONS_CONNECTION_H__
#define __CONNECTIONS_CONNECTION_H__

#include <functional>

#include "net/connection.h"

#include "utils/signals.h"

class Location;

typedef boost::signals2::signal<void (std::shared_ptr<Location>)> sig_location;
typedef sig_location::slot_type sig_location_slot;

class Location {
	friend class LocationsMgr;

public:
	typedef std::shared_ptr<Location> ptr;
	typedef std::shared_ptr<const Location> const_ptr;

	/** Build location with new connection */
	Location( const std::string& name , Connection* conn = NULL );

	virtual ~Location();

	const std::string& get_name()
	{	return name; }

	/** Swap connection object with another location */
	void swap_connection( Location& loc );

	virtual void request_location( Location::ptr loc )
	{	emit_request_location(loc); }

	slot_connection on_location_request( const sig_location_slot& slot ) const
	{	return emit_request_location.connect( slot ); }

protected:
	void die()
	{
		if( connection )
			connection->close();
		sig_conn.disconnect();
		connection = NULL;
	}

	virtual void parse_line( const std::string& line ) =0;

	void write_line( const std::string& line );

	mutable sig_location emit_request_location;

private:
	void init_connection();

	std::string name;
	Connection* connection;

	boost::signals2::scoped_connection sig_conn;
};

#endif /* end of include guard: __CONNECTIONS_CONNECTION_H__ */

