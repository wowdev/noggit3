#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <map>
#include <algorithm>

// base class for manager objects

class ManagedItem 
{
	int refcount;
public:
	std::string name;
	ManagedItem( const std::string& n ) : refcount( 0 )
	{
		name = n;
		std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
	}

	virtual ~ManagedItem( ) { }

	void addref( )
	{
		++refcount;
	}

	bool delref( )
	{
		return --refcount == 0;
	}
	
};


template <class IDTYPE>
class Manager
{
public:
	std::map<std::string, IDTYPE> names;
	std::map<IDTYPE, ManagedItem*> items;

	Manager( ) { }

	virtual IDTYPE add( const std::string& name ) = 0;

	virtual void del( IDTYPE id )
	{
		if( items[id]->delref( ) )
		{
			doDelete( id );
			
			std::string name = items[id]->name;
			if( names.find( name ) != names.end( ) )
				names.erase( names.find( name ) );
			items.erase( items.find( id ) );
		}
	}

	void delbyname( const std::string& name )
	{
		if( has( name ) )
			del( get( name ) );
	}

	virtual void doDelete( IDTYPE id ) { }

	bool has( const std::string& name )
	{
		return( names.find( name ) != names.end( ) );
	}

	IDTYPE get( const std::string& name )
	{
		return names[name];
	}

protected:
	void do_add( const std::string& name, IDTYPE id, ManagedItem* item )
	{
		names[name] = id;
		item->addref( );
		items[id] = item;
	}
};

class SimpleManager : public Manager<int>
{
	int baseid;
public:
	SimpleManager( ) : baseid( 0 ) { }

protected:
	int nextID( )
	{
		return baseid++;
	}
};

#endif

