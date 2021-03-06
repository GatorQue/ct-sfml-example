#ifndef SOURCES_SCOUT_RESOURCEHOLDER_HPP_
#define SOURCES_SCOUT_RESOURCEHOLDER_HPP_

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>

template <typename Resource, typename Identifier>
class ResourceHolder
{
  public:
    void load(Identifier id, const std::string& filename);

    template <typename Parameter>
    void load(Identifier id, const std::string& filename, const Parameter& secondParam);

    Resource& get(Identifier id);
    const Resource& get(Identifier id) const;

  private:
    std::map<Identifier, std::unique_ptr<Resource> > resourceMap;

    void insertResource(Identifier id, std::unique_ptr<Resource> resource);
};

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename)
{
  std::unique_ptr<Resource> resource(new Resource());
  if(!resource->loadFromFile(filename))
  {
    throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
  }

  insertResource(id, std::move(resource));
}

template <typename Resource, typename Identifier>
template <typename Parameter>
void ResourceHolder<Resource, Identifier>::load(Identifier id, const std::string& filename, const Parameter& secondParam)
{
  std::unique_ptr<Resource> resource(new Resource());
  if(!resource->loadFromFile(filename, secondParam))
  {
    throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
  }

  insertResource(id, std::move(resource));
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
void ResourceHolder<Resource, Identifier>::insertResource(Identifier id, std::unique_ptr<Resource> resource)
{
  auto inserted = resourceMap.insert(std::make_pair(id, std::move(resource)));
  assert(inserted.second);
}

#endif
