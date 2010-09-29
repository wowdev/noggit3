#include <sstream>

#include "selection.h"

#include "world.h" // gWorld
#include "MapChunk.h" // MapChunk
#include "WMOInstance.h" // WMOInstance
#include "ModelInstance.h" // ModelInstance

nameEntryManager SelectionNames;


/**
 ** nameEntry 
 ** 
 ** This is used for selectable objects.
 **
 **/

nameEntry::nameEntry( ModelInstance *model )
{
	type = eEntry_Model;
	data.model = model;
	std::stringstream temp; temp << "Object: " << model->d1 << " (M2)"; Name = temp.str();
}

nameEntry::nameEntry( WMOInstance *wmo )
{
	type = eEntry_WMO;
	data.wmo = wmo;
	std::stringstream temp; temp << "Object: " << wmo->mUniqueID << " (WMO)"; Name = temp.str();
}

nameEntry::nameEntry( MapChunk *chunk )
{
	type = eEntry_MapChunk;
	data.mapchunk = chunk;
	std::stringstream temp; temp << "Mapchunk: " << chunk->px << ", " << chunk->py; Name = temp.str();
}

nameEntry::nameEntry()
{
	type = eEntry_Fake;
	Name = "Fake";
}

const std::string& nameEntry::returnName()
{
	return Name;
}

/**
 ** nameEntryManager
 ** 
 ** This is used for managing those selectable objects.
 **
 **/
 
unsigned int nameEntryManager::add( ModelInstance *mod )
{
	items.push_back( new nameEntry( mod ) );
	return NextName++;
}
unsigned int nameEntryManager::add( WMOInstance *wmo )
{
	items.push_back( new nameEntry( wmo ) );
	return NextName++;
}
unsigned int nameEntryManager::add( MapChunk *chunk )
{
	items.push_back( new nameEntry( chunk ) );
	return NextName++;
}

nameEntry *nameEntryManager::findEntry( unsigned int ref ) const
{
	return items[ref];
}

nameEntryManager::nameEntryManager()
{
	items.push_back( new nameEntry() );
	NextName = 1;
}

void nameEntryManager::del( unsigned int Ref )
{
	if( items[Ref] )
	{
		if( gWorld->GetCurrentSelection() == items[Ref] )
			gWorld->ResetSelection();
		delete( items[Ref] );
		items[Ref] = 0;
	}
}
