#include "AsyncLoader.h"
#include "AsyncObject.h"

void AsyncLoader::process()
{
	while(true)
	{
		AsyncObject* object = nextObjectToLoad();
		if(object)
		{
			object->finishLoading();
		}
		else
		{
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
	}
}

AsyncObject* AsyncLoader::nextObjectToLoad()
{
	boost::mutex::scoped_lock lock(m_loadingMutex);
	std::list<AsyncObject*>::iterator it;
	do
	{
		it = m_objects.begin();
		if(it != m_objects.end() && (*it)->finishedLoading())
		{
			m_objects.erase(it);
		}
		else
		{
			break;
		}
	}
	while(it != m_objects.end());
	if(it == m_objects.end())
	{
		return NULL;
	}
	else
	{
		return *it;
	}
}

void AsyncLoader::addObject(AsyncObject* _pObject)
{
	m_objects.push_back(_pObject);
}

void AsyncLoader::start(int _numThreads)
{
	for(int i = 0; i < _numThreads; ++i)
	{
		m_threads.add_thread(new boost::thread(&AsyncLoader::process, this));
	}
}

void AsyncLoader::join()
{
	m_threads.join_all();
}
