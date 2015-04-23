namespace JonsEngine
{
	#define IDMAP_INDEX_MASK(id) id & 0xFFFFFFFF
	#define IDMAP_VERSION_MASK(id) (id >> 16) & 0xFFFFFFFF


	//
	// IDMap::Item
	//
	template <typename T>
	template <typename... Arguments>
	IDMap<T>::Item::Item(uint16_t version, Arguments&&... args) : mVersion(version), mItem(std::forward<Arguments>(args)...)
	{
	}


	//
	// IDMap::Iterator
	//
	template <typename T>
	IDMap<T>::iterator::iterator(typename const std::vector<typename IDMap<T>::Item>::iterator& iter) : mIterator(iter)
	{
	}

	template <typename T>
	bool IDMap<T>::iterator::operator!=(const iterator& iter) const
	{
		return mIterator != iter.mIterator;
	}

	template <typename T>
	typename IDMap<T>::iterator& IDMap<T>::iterator::operator++()
	{
		++mIterator;

		return *this;
	}

	template <typename T>
	typename IDMap<T>::iterator IDMap<T>::iterator::operator++(int)
	{
		iterator old(++(*this));

		return old;
	}

	template <typename T>
	const T& IDMap<T>::iterator::operator*() const
	{
		return mIterator->mItem;
	}


	//
	// IDMap
	//
	template <typename T>
	template <typename... Arguments>
	typename IDMap<T>::ItemID IDMap<T>::AddItem(Arguments&&... args)
	{
		if (!mFreeIndices.empty())
		{
			const uint32_t freeIndex = mFreeIndices.back();
			mFreeIndices.pop_back();

			// destruct old item
			mItems[freeIndex].mItem.~T();

			Item* item = new(&mItems[freeIndex]) Item(mItems[freeIndex].mVersion + 1, std::forward<Arguments>(args)...);

			return (freeIndex | (static_cast<uint32_t>(item->mVersion) << 16));
		}
		else
		{
			mItems.emplace_back(1, std::forward<Arguments>(args)...);

			return ((mItems.size() - 1) | (static_cast<uint32_t>(1) << 16));
		}
	}

	template <typename T>
	void IDMap<T>::MarkAsFree(const ItemID id)
	{
		const uint16_t index = IDMAP_INDEX_MASK(id);
		const uint16_t version = IDMAP_VERSION_MASK(id);

		assert(mItems[index].mVersion == version);

		mFreeIndices.push_back(index);
	}

	template <typename T>
	T& IDMap<T>::GetItem(const ItemID id)
	{
		const uint16_t index = IDMAP_INDEX_MASK(id);
		const uint16_t version = IDMAP_VERSION_MASK(id);

		assert(mItems[index].mVersion == version);

		return mItems[index].mItem;
	}

	template <typename T>
	T* IDMap<T>::TryGetItem(const ItemID id) const
	{
		const uint16_t index = IDMAP_INDEX_MASK(id);
		const uint16_t version = IDMAP_VERSION_MASK(id);

		if (mItems[index].mVersion == version)
			return &mItems[index].mItem;
		else
			return nullptr;
	}

	template <typename T>
	void IDMap<T>::Clear()
	{
		mItems.clear();
		mFreeIndices.clear();
	}

	template <typename T>
	typename IDMap<T>::iterator IDMap<T>::begin()
	{
		return mItems.begin();
	}

	template <typename T>
	typename IDMap<T>::iterator IDMap<T>::end()
	{
		return mItems.end();
	}
}