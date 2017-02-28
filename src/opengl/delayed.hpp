#pragma once

#include <unordered_set>

namespace opengl
{
  namespace delayed
  {
    // all objects being delayed inherit from this class
    class object
    {
    public:
      object()
        : _finished_upload(false)
      {}

      virtual void upload() = 0;

      inline bool finished_upload() const {
        return _finished_upload;
      }

    protected:
      bool _finished_upload;
    };

    // this is used to collect all objects _local_ to the draw entry point function (eg World::draw)
    // it is passed in to the draw function of a delayed::object
    // said object then check via uploader::can_draw if it can continue drawing
    // after all objects were drawn delayed::upload is called to upload a limited number of objects
    // im aware that it is not optimal for all objects to check if they can draw or not,
    // but i feel like it is the path of least resistance
    class uploader
    {
    public:
      inline bool can_draw(object* obj) {
        if(obj->finished_upload())
          return true;

        _delayed_objects.insert(obj);
        return false;
      }

      void upload(size_t count) {
        size_t counter (std::min(count, _delayed_objects.size()));

        for(auto obj : _delayed_objects) {
          if(!counter)
            return;

          obj->upload();

          counter--;
        }
      }

    private:
      std::unordered_set<object*> _delayed_objects;
    };
  }
}