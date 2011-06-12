#ifndef __SELECTION_H__
#define __SELECTION_H__

enum eSelectionEntryTypes
{
	eEntry_Fake,
	eEntry_Model,
	eEntry_WMO,
	eEntry_MapChunk
};

#include <string>
#include <vector>

// Instead of includes.
class ModelInstance;
class WMOInstance;
class MapChunk;

/**
 ** nameEntry 
 ** 
 ** This is used for selectable objects.
 **
 **/

class nameEntry
{
private:
	std::string	Name;
public:
	int	type;
	union
	{
		ModelInstance	*model;
		WMOInstance		*wmo;
		MapChunk		*mapchunk;
	} data;

	explicit nameEntry( ModelInstance *model );
	explicit nameEntry( WMOInstance *wmo );
	explicit nameEntry( MapChunk *chunk );
	nameEntry();
	
	const std::string& returnName();
};

/**
 ** nameEntryManager
 ** 
 ** This is used for managing those selectable objects.
 **
 **/
 
class nameEntryManager
{
private:
	unsigned int NextName;
	std::vector<nameEntry*> items;
public:
	nameEntryManager();
	
	unsigned int add( ModelInstance *mod );
	unsigned int add( WMOInstance *wmo );
	unsigned int add( MapChunk *chunk );
	
	void del( unsigned int Ref );
	
	nameEntry *findEntry( unsigned int ref ) const;
};

#endif
