#ifndef IU_CORE_UTILS_SINGLETON_H
#define IU_CORE_UTILS_SINGLETON_H

#include "CoreTypes.h"

template<class T> class Singleton
{
	public:
		Singleton<T>() {

		}
		virtual ~Singleton<T>() {

		}
		static T* instance() {
			if ( !instance_ ) {
				instance_ = new T();
				atexit(&cleanUp);
			}
			return instance_;
		}

	private:
		static void cleanUp() { 
			delete instance_; 
			instance_ = 0; 
		}

		DISALLOW_COPY_AND_ASSIGN(Singleton<T>);
		static T* instance_;
};

template<class T> T*  Singleton<T>::instance_ = 0;

#endif