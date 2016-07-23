#ifndef SOURCES_SCOUT_RESOURCEHOLDER_HPP_
#define SOURCES_SCOUT_RESOURCEHOLDER_HPP_

#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>

template <typename Resource, typename Identifier>
class ResourceHolder
{
  public:
    typedef std::unique_ptr<typename Resource> Ptr;
    typedef std::function<void()> PtrFunc;

    void load_later(Identifier id, const std::string& filename);

    template <typename Parameter>
    void load_later(Identifier id, const std::string& filename, const Parameter& secondParam);

    void load_now(Identifier id, const std::string& filename);

    template <typename Parameter>
    void load_now(Identifier id, const std::string& filename, const Parameter& secondParam);

    Resource& get(Identifier id);
    const Resource& get(Identifier id) const;

    bool hasResource(Identifier ID);
    
    size_t getPendingCount();
    void loadNextPending();

  private:
    std::map<Identifier, Ptr > resourceMap;
    std::list<PtrFunc> pendingResources;
    
    void insertResource(Identifier id, Ptr resource);
};

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load_later(Identifier id, const std::string& filename)
{
  if(!hasResource(id))
  {
    pendingResources.push_back([this,id,filename]()
      {
        load_now(id, filename);
      });
  }
}

template <typename Resource, typename Identifier>
template <typename Parameter>
void ResourceHolder<Resource, Identifier>::load_later(Identifier id, const std::string& filename, const Parameter& secondParam)
{
  if(!hasResource(id))
  {
    pendingResources.push_back([this,id,filename,secondParam]()
      {
        load_now(id, filename, secondParam);
      });
  }
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load_now(Identifier id, const std::string& filename)
{
  if(!hasResource(id))
  {
    Ptr resource(new Resource());
    if(!resource->loadFromFile(filename))
    {
      throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
    }

    insertResource(id, std::move(resource));
  }
}

template <typename Resource, typename Identifier>
template <typename Parameter>
void ResourceHolder<Resource, Identifier>::load_now(Identifier id, const std::string& filename, const Parameter& secondParam)
{
  if(!hasResource(id))
  {
    Ptr resource(new Resource());
    if(!resource->loadFromFile(filename, secondParam))
    {
      throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
    }

    insertResource(id, std::move(resource));
  }
}

template <typename Resource, typename Identifier>
Resource& ResourceHolder<Resource, Identifier>::get(Identifier id)
{
  auto found = resourceMap.find(id);
  assert(found != resourceMap.end());

  return *found->second;
}

template <typename Resource, typename Identifier>
const Resource& ResourceHolder<Resource, Identifier>::get(Identifier id) const
{
  auto found = resourceMap.find(id);
  assert(found != resourceMap.end());

  return *found->second;
}

template <typename Resource, typename Identifier>
bool ResourceHolder<Resource, Identifier>::hasResource(Identifier id)
{
  auto itr = resourceMap.find(id);
  return itr != resourceMap.end();
}

template <typename Resource, typename Identifier>
size_t ResourceHolder<Resource, Identifier>::getPendingCount()
{
  return pendingResources.size();
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::loadNextPending()
{
  auto loadFunction = pendingResources.front();
  if(loadFunction)
  {
    loadFunction();
    pendingResources.pop_front();
  }
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::insertResource(Identifier id, Ptr resource)
{
  auto inserted = resourceMap.insert(std::make_pair(id, std::move(resource)));
  assert(inserted.second);
}

#endif
