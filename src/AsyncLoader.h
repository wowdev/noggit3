#ifndef ASYNCLOADER_H
#define ASYNCLOADER_H

#include <boost/thread.hpp>
#include <list>

class AsyncObject;

class AsyncLoader
{
public:
	AsyncLoader() { }
	
	void process();
	
	AsyncObject* nextObjectToLoad();
	
	void addObject(AsyncObject* _pObject);
	void removeObject(AsyncObject* _pObject);
	
	void start(int _numThreads = 1);
	void stop();
	
	void join();
private:
	std::list<AsyncObject*> m_objects;
	boost::thread_group m_threads;
	boost::mutex m_loadingMutex;
};

#endif //ASYNCLOADER_H
