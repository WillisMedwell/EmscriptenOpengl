#pragma once

#include "Libraries.hpp"

#define ENABLE_GLAZE_ENTITY_SERIALIASATION

#ifdef ENABLE_GLAZE_ENTITY_SERIALIASATION
#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>
#endif

template <typename... Components>
class SparseFlexEcs {
	using ComponentsTuple = std::tuple<Components...>;
	using EntityId = uint64_t;
	using VaryingComponent = std::variant<Components...>;

	using IndexIntoComposition = uint64_t;
public:
	using CompositionLayout = std::bitset<sizeof...(Components)>;
private:
	using IsActive = bool;
	using EntityLookupInfo = std::tuple<IndexIntoComposition, CompositionLayout, IsActive>;
	using InactiveEntitiesCount = size_t;

	using Composition = std::tuple<
		InactiveEntitiesCount,
		std::vector<std::optional<EntityId>>,
		std::array<std::vector<VaryingComponent>, sizeof...(Components)>>;

	size_t m_inactive_entities_count = 0;
	std::vector<EntityLookupInfo> m_entities_lookup_info = {};
	std::unordered_map<CompositionLayout, Composition> m_compositions = {};

	constexpr static CompositionLayout empty_composition_layout = {};

public:
	struct Entity {
		SparseFlexEcs* ecs;
		std::optional<EntityId> id;
	};

private:
	template <typename T>
	auto addTypeToCompositionLayout(CompositionLayout layout)
	{
		if constexpr (!std::is_constructible_v<VaryingComponent, T>)
		{
			static_assert(std::is_constructible_v<VaryingComponent, T>, "Bad Type");
			return -1;
		}
		else
		{
			auto i = VaryingComponent{ T {} }.index();
			layout.set(i);
			return layout;
		}
	}

	auto reserveEntryInComposition(CompositionLayout layout, Composition& composition, EntityId entity_id)
	{
		auto& [inactive_count, ids, components] = composition;

		bool can_replace_inactive = inactive_count > 0;
		size_t entry_index = -1;
		if (can_replace_inactive) {
			--inactive_count;
			auto iter = std::ranges::find_if(ids, [](const auto& maybe_id) { return !maybe_id; });
			assert(iter != ids.end());
			entry_index = std::distance(ids.begin(), iter);
			*iter = entity_id;
		}
		else {
			entry_index = ids.size();
			for (size_t i = 0; i < layout.size(); ++i) {
				if (layout.test(i)) {
					components[i].emplace_back();
				}
			}
			ids.emplace_back(entity_id);
		}
		assert(entity_id < m_entities_lookup_info.size());
		m_entities_lookup_info[entity_id] = EntityLookupInfo{ entry_index, layout, true };
		return entry_index;
	}

public:
	auto createEntity()
	{
		Entity entity;
		entity.ecs = this;
		if (m_inactive_entities_count > 0) {
			auto iter = std::ranges::find_if(
				m_entities_lookup_info,
				[&](const EntityLookupInfo& entity_lookup_info) {
					return !std::get<IsActive>(entity_lookup_info);
				});
			assert(iter != m_entities_lookup_info.end());
			entity.id = std::distance(m_entities_lookup_info.begin(), iter);
			;
			--m_inactive_entities_count;
		}
		else {
			entity.id = m_entities_lookup_info.size();
			m_entities_lookup_info.emplace_back();
		}
		auto entry_index = reserveEntryInComposition(empty_composition_layout, m_compositions[empty_composition_layout], entity.id.value());

		// check m_entities_lookup_info
		assert(std::get<0>(m_entities_lookup_info[entity.id.value()]) == entry_index);
		assert(std::get<1>(m_entities_lookup_info[entity.id.value()]) == empty_composition_layout);
		assert(std::get<2>(m_entities_lookup_info[entity.id.value()]) == true);

		// check m_compositions
		assert(
			std::get<InactiveEntitiesCount>(m_compositions[empty_composition_layout]) == std::ranges::count(std::get<1>(m_compositions[empty_composition_layout]), std::optional<EntityId> { std::nullopt }));
		assert(std::get<std::vector<std::optional<EntityId>>>(m_compositions[empty_composition_layout])[entry_index].value() == entity.id);

		return entity;
	}

	auto destroyEntity(Entity& entity)
	{
		auto& [entry_index, layout, is_active] = m_entities_lookup_info[entity.id.value()];
		if (!is_active) {
			entity.id = std::nullopt;
		}

		auto& [inactive_count, ids, components] = m_compositions[layout];

		assert(ids[entry_index].value() == entity.id.value());

		ids[entry_index] = std::nullopt;
		++inactive_count;
		++m_inactive_entities_count;
		is_active = false;
	}

	template <typename T, typename... Args>
	auto addComponentToEntity(Entity& entity, Args&&... args)
	{
		const auto [old_entry_index, old_layout, old_is_active] = m_entities_lookup_info[entity.id.value()];
		auto& [old_inactive_count, old_ids, old_components] = m_compositions[old_layout];

		assert(old_ids[old_entry_index].value() == entity.id.value());
		old_ids[old_entry_index] = std::nullopt;
		++old_inactive_count;

		CompositionLayout new_layout = addTypeToCompositionLayout<T>(old_layout);
		Composition& new_composition = m_compositions[new_layout];

		size_t new_entry_index = reserveEntryInComposition(new_layout, new_composition, entity.id.value());

		auto& [new_inactive_count, new_ids, new_components] = new_composition;

		// copy old components, into new components
		for (size_t i = 0; i < old_layout.size(); ++i) {
			if (old_layout.test(i)) {
				new_components[i][new_entry_index] = std::move(old_components[i][old_entry_index]);
			}
		}

		if constexpr (!std::is_constructible_v<VaryingComponent, T>) {
			static_assert(std::is_constructible_v<VaryingComponent, T>, "Bad Component Type. Probably trying to add component not declared in instace of ECS.");
		}
		else if constexpr (sizeof...(Args) == 1) {
			auto i = VaryingComponent{ T {} }.index();
			using Arg = std::tuple_element_t<0, typename std::tuple<Args...>>;
			if constexpr (std::is_same_v<Arg, T>) {
				new_components[i][new_entry_index] = T{ std::forward<Args>(args)... };
			}
			else {
				new_components[i][new_entry_index] = T{ static_cast<T>(args)... };
			}
		}
		else {
			auto i = VaryingComponent{ T {} }.index();
			new_components[i][new_entry_index] = T{ std::forward<Args>(args)... };
		}


		assert(std::get<0>(m_entities_lookup_info[entity.id.value()]) == new_entry_index);
		assert(std::get<1>(m_entities_lookup_info[entity.id.value()]) == new_layout);
		assert(std::get<2>(m_entities_lookup_info[entity.id.value()]) == true);
	}

	template <typename... T>
	class ComponentIterator {
		using CompositionIterator = std::unordered_map<CompositionLayout, Composition>::iterator;

	public:
		SparseFlexEcs* ecs;
		Composition* composition_curr;
		CompositionIterator composition_iter_curr;
		CompositionIterator composition_iter_end;
		size_t i = 0;
		size_t i_max = 0;


		template <typename C>
		constexpr static auto getTypeIndex()
		{
			CompositionLayout layout;
			auto i = VaryingComponent{ C {} }.index();
			return i;
		}


		template <typename C>
		constexpr static auto getTypeMask()
		{
			CompositionLayout layout;
			auto i = VaryingComponent{ C {} }.index();
			layout.set(i);
			return layout;
		}

		constexpr static auto getLayoutMask()
		{
			CompositionLayout layout = (getTypeMask<T>() | ...);
			return layout;
		}

#if BUILD_TARGET == NATIVE_BUILD
		constexpr static CompositionLayout required_layout_mask = getLayoutMask();
#else
		const CompositionLayout required_layout_mask = getLayoutMask();
#endif
		void findGoodStart()
		{
			auto matchingLayout = [&]() {
				auto& [layout, composition] = *composition_iter_curr;
				return (layout & required_layout_mask) == required_layout_mask;
				};

			while (composition_iter_curr != composition_iter_end && !matchingLayout()) {
				++composition_iter_curr;
			}

			if (composition_iter_curr == composition_iter_end) {
				return;
			}

			auto& [layout, composition] = *composition_iter_curr;
			auto& [inactive_count, ids, components] = composition;

			composition_curr = &composition;

			i = 0;
			i_max = ids.size();

			bool is_all_inactive = inactive_count == ids.size();

			if (is_all_inactive) {
				++composition_iter_curr;
				findGoodStart();
			}
			else {
				auto isActive = [](const auto& e) { return e.has_value(); };
				auto iter = std::ranges::find_if(ids, isActive);
				i = std::distance(ids.begin(), iter);
			}
		}

		auto operator++() -> ComponentIterator<T...>&
		{

			auto& [_, ids, components] = *composition_curr;

			for (++i; i < i_max; ++i) {
				if (ids[i].has_value()) {
					return *this;
				}
			}

			if (composition_iter_curr != composition_iter_end) {
				++composition_iter_curr;
				findGoodStart();
			}
			return *this;
		}

		auto operator!=(const ComponentIterator& other)
		{
			return this->composition_iter_curr != other.composition_iter_curr;
		}
		auto operator*() -> std::tuple<T&...>
		{
			auto& [_, ids, components] = *composition_curr;
			assert(((std::holds_alternative<T>(components[getTypeIndex<T>()][i])) && ...));
			return std::tuple<T&...>(std::get<T>(components[getTypeIndex<T>()][i])...);
		}
	};

	template <typename... T>
	struct ComponentRange {
		SparseFlexEcs* ecs;
		auto begin()
		{
			ComponentIterator<T...> iter;
			iter.ecs = ecs;
			iter.composition_iter_curr = ecs->m_compositions.begin();
			iter.composition_iter_end = ecs->m_compositions.end();
			iter.findGoodStart();
			return iter;
		}
		auto end()
		{
			ComponentIterator<T...> iter;
			iter.composition_iter_curr = ecs->m_compositions.end();
			return iter;
		}
	};

	template <typename... T>
	auto forAnyWith()
	{
		return ComponentRange<T...> { this };
	}

private:
	template <typename C>
	consteval static auto getTypeIndex()
	{
		CompositionLayout layout;
		auto i = VaryingComponent{ C {} }.index();
		return i;
	}

public:
	template <typename... T>
	auto getComponents(Entity& entity) -> std::tuple<T&...>
	{
		assert(entity.id.has_value());
		const auto& [entry_index, layout, is_active] = m_entities_lookup_info[entity.id.value()];
		assert(is_active);
		auto& [_, ids, components] = m_compositions[layout];

		assert(((std::holds_alternative<T>(components[getTypeIndex<T>()][entry_index])) && ...));
		return std::tuple<T&...>(std::get<T>(components[getTypeIndex<T>()][entry_index])...);
	}

	auto shrinkToFit()
	{
		// remove inactive entities from compositions
		auto cleanComposition = [&](Composition& composition) {
			auto& [old_inactive_count, old_ids, old_components] = composition;

			size_t active_count = old_ids.size() - old_inactive_count;

			if (!old_inactive_count) {
				return;
			}

			Composition clean_composition;
			auto& [new_inactive_count, new_ids, new_components] = clean_composition;
			new_inactive_count = 0;
			new_ids.reserve(active_count);
			for (size_t i = 0; i < new_components.size(); i++) {
				if (old_components[i].size()) {
					new_components[i].reserve(active_count);
				}
			}

			size_t visited = 0;
			size_t i = 0;
			for (; visited < active_count && i < old_ids.size(); ++i) {
				if (old_ids[i] != std::nullopt) {
					++visited;
					new_ids.emplace_back(old_ids[i]);
					for (size_t c = 0; c < new_components.size(); ++c) {
						if (old_components[c].size()) {
							new_components[c].emplace_back(std::move(old_components[c][i]));
						}
					}
					std::get<IndexIntoComposition>(m_entities_lookup_info[new_ids.back().value()]) = new_ids.size() - 1;
				}
			}
			composition = std::move(clean_composition);
			};

		std::ranges::for_each(m_compositions | std::views::values, cleanComposition);

	}


	//struct JsonEntity {
	//	std::tuple<std::optional<Components>...> opt_components;

	//	/*struct glaze {
	//		using T = JsonEntity;
	//		static constexpr auto value = glz::object(
	//			"Components", &T::opt_components
	//			);
	//	};*/

	//	GLZ_LOCAL_META(JsonEntity, opt_components);
	//};

};






