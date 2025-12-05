#pragma once

#include <functional>
#include <memory>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Entity.hpp>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace of::domain {

/**
 * \brief Internal type-erased interface for component storage.
 */
struct IComponentStorage {
    virtual ~IComponentStorage() = default;
    virtual void Remove(Entity e) = 0;
    virtual bool Has(Entity e) const = 0;
    virtual std::vector<Entity> GetEntities() const = 0;
};

/**
 * \brief Concrete storage for a specific component type.
 */
template <typename T>
struct ComponentStorage : public IComponentStorage {
    std::unordered_map<Entity, T> map;

    void Remove(Entity e) override {
        map.erase(e);
    }

    bool Has(Entity e) const override {
        return map.find(e) != map.end();
    }

    std::vector<Entity> GetEntities() const override {
        std::vector<Entity> res;
        res.reserve(map.size());
        for (auto& [e, _] : map)
            res.push_back(e);
        return res;
    }

    T* Get(Entity e) {
        auto it = map.find(e);
        return (it != map.end()) ? &it->second : nullptr;
    }

    const T* Get(Entity e) const {
        auto it = map.find(e);
        return (it != map.end()) ? &it->second : nullptr;
    }

    void Set(Entity e, const T& val) {
        map[e] = val;
    }
};

/**
 * \brief The central Entity-Component-System (ECS) registry.
 *
 * NOW INSTANCE-BASED (No more static storage).
 */
class Registry {
   public:
    using Callback = std::function<void(Entity)>;

    // --------- ENTITY ----------
    Entity CreateEntity() {
        return m_NextId++;
    }

    void DestroyEntity(Entity e) {
        for (auto& [type, storage] : m_Storages) {
            if (storage && storage->Has(e)) {
                storage->Remove(e);
            }
        }
    }

    // --------- COMPONENT CORE API ----------

    template <typename T>
    void AddComponent(Entity e, const T& comp) {
        GetStorage<T>().Set(e, comp);
        TriggerAdded<T>(e);
    }

    template <typename T>
    void RemoveComponent(Entity e) {
        GetStorage<T>().Remove(e);
        TriggerRemoved<T>(e);
    }

    template <typename T>
    T* GetComponent(Entity e) {
        return GetStorage<T>().Get(e);
    }

    template <typename T>
    const T* GetComponent(Entity e) const {
        // Use const_cast because GetStorage<T> modifies m_Storages if missing.
        // BUT wait: if missing, GetComponent should return nullptr, NOT create storage.
        // So for const access, we must check existence first.
        auto* storage = GetStorageRaw<T>();
        if (!storage)
            return nullptr;
        return storage->Get(e);
    }

    template <typename T>
    bool HasComponent(Entity e) const {
        auto* storage = GetStorageRaw<T>();
        if (!storage)
            return false;
        return storage->Has(e);
    }

    template <typename T>
    std::vector<Entity> GetEntitiesWith() const {
        auto* storage = GetStorageRaw<T>();
        if (!storage)
            return {};
        return storage->GetEntities();
    }

    // --------- ENTITIES ----------
    std::vector<Entity> Entities() const;

    // --------- EVENTS ----------
    template <typename T>
    void OnComponentAdded(Callback cb) {
        m_OnAdded[std::type_index(typeid(T))].push_back(cb);
    }

    template <typename T>
    void OnComponentRemoved(Callback cb) {
        m_OnRemoved[std::type_index(typeid(T))].push_back(cb);
    }

    template <typename T>
    void OnComponentModified(Callback cb) {
        m_OnModified[std::type_index(typeid(T))].push_back(cb);
    }

    // --------- DEBUG ----------
    void Dump() const;

   private:
    Entity m_NextId = 1;

    // Events
    std::unordered_map<std::type_index, std::vector<Callback>> m_OnAdded;
    std::unordered_map<std::type_index, std::vector<Callback>> m_OnRemoved;
    std::unordered_map<std::type_index, std::vector<Callback>> m_OnModified;

    // Storage (Instance based!)
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_Storages;

    // Helper to get/create storage
    template <typename T>
    ComponentStorage<T>& GetStorage() {
        std::type_index type = std::type_index(typeid(T));
        if (m_Storages.find(type) == m_Storages.end()) {
            m_Storages[type] = std::make_unique<ComponentStorage<T>>();
        }
        return *static_cast<ComponentStorage<T>*>(m_Storages[type].get());
    }

    // Helper to get storage (const safe, no creation)
    template <typename T>
    const ComponentStorage<T>* GetStorageRaw() const {
        std::type_index type = std::type_index(typeid(T));
        auto it = m_Storages.find(type);
        if (it == m_Storages.end())
            return nullptr;
        return static_cast<const ComponentStorage<T>*>(it->second.get());
    }

    // --------- EVENT HELPERS ----------
    template <typename T>
    void TriggerAdded(Entity e) {
        auto it = m_OnAdded.find(std::type_index(typeid(T)));
        if (it == m_OnAdded.end())
            return;
        for (auto& cb : it->second)
            cb(e);
    }

    template <typename T>
    void TriggerRemoved(Entity e) {
        auto it = m_OnRemoved.find(std::type_index(typeid(T)));
        if (it == m_OnRemoved.end())
            return;
        for (auto& cb : it->second)
            cb(e);
    }

    template <typename T>
    void TriggerModified(Entity e) {
        auto it = m_OnModified.find(std::type_index(typeid(T)));
        if (it == m_OnModified.end())
            return;
        for (auto& cb : it->second)
            cb(e);
    }
};

}  // namespace of::domain
