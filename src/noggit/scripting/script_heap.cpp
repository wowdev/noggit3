// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_heap.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <vector>
#include <algorithm>

namespace noggit {
    namespace scripting {
        std::vector<void*> ptrs;

        void* script_malloc(size_t size) 
        {
            void* ptr = malloc(size);
            ptrs.push_back(ptr);
            return ptr;
        }

        void* script_calloc(size_t size)
        {
            void* ptr = calloc(1,size);
            ptrs.push_back(ptr);
            return ptr;
        }

        void script_free_all()
        {
            for(auto& ptr: ptrs)
            {
                free(ptr);
            }
            ptrs.clear();
        }

        void script_free(void* ptr)
        {
            std::vector<void*>::iterator pos = std::find(ptrs.begin(), ptrs.end(), ptr);
            if(pos == ptrs.end())
            {
                throw script_exception("Attempted to free invalid memory, this might cause a memory leak.");
            }
            ptrs.erase(pos);
            free(ptr);
        }
    }
}