/* This file is part of Raul.
 * Copyright (C) 2007 Dave Robillard <http://drobilla.net>
 * 
 * Raul is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * Raul is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <raul/Table.hpp>

namespace Raul {

#ifndef NDEBUG
template <typename K, typename T>
bool
Table<K,T>::is_sorted() const
{
	if (size() <= 1)
		return true;
	
	K prev_key = _entries[0].first;

	for (size_t i=1; i < size(); ++i)
		if (_entries[i].first < prev_key)
			return false;
		else
			prev_key = _entries[i].first;
	
	return true;
}
#endif


/** Binary search (O(log(n))) */
template <typename K, typename T>
typename Table<K,T>::iterator
Table<K,T>::find(const K& key)
{
	if (size() == 0)
		return end();

	size_t lower = 0;
	size_t upper = size() - 1;
	size_t i;
	
	while (upper >= lower) {
		i = lower + ((upper - lower) / 2);
		const Entry& elem = _entries[i];

		if (elem.first == key)
			return iterator(*this, i);
		else if (i < size()-1 && elem.first < key)
			lower = i + 1;
		else if (i > 0)
			upper = i - 1;
		else
			break;
	}
	
	return end();
}


template <typename K, typename T>
void
Table<K,T>::insert(const K& key, const T& value)
{
	if (size() == 0 || size() == 1 && key > _entries[0].first) {
		_entries.push_back(make_pair(key, value));
		return;
	} else if (size() == 1) {
		_entries.push_back(_entries[0]);
		_entries[0] = make_pair(key, value);
		return;
	}

	size_t lower = 0;
	size_t upper = size() - 1;
	size_t i;
	
	// Find the earliest element > key
	while (upper >= lower) {
		i = lower + ((upper - lower) / 2);
		assert(i >= lower);
		assert(i <= upper);
		assert(_entries[lower].first <= _entries[i].first);
		assert(_entries[i].first <= _entries[upper].first);

		assert(i < size());
		Entry& elem = _entries[i];

		if (elem.first == key) {
			break;
		} else if (elem.first > key) {
			if (i == 0 || _entries[i-1].first < key)
				break;
			upper = i - 1;
		} else {
			lower = i + 1;
		}
	}

	// Lil' off by one touchup :)
	if (i < size() && _entries[i].first <= key)
		++i;
	
	_entries.resize(size() + 1);

	// Shift everything beyond i right
	for (size_t j = size()-1; j > i; --j)
		_entries[j] = _entries[j-1];
	
	_entries[i] = make_pair(key, value);

	assert(is_sorted());
}


template <typename K, typename T>
void
Table<K,T>::erase(const K& key)
{
	erase(find(key));
}


template <typename K, typename T>
void
Table<K,T>::erase(iterator i)
{
	if (i == end())
		return;

	const size_t index = i._index;

	// Shift left
	for (size_t j=index; j < size()-1; ++j)
		_entries[j] = _entries[j+1];

	_entries.pop_back();

#ifndef NDEBUG
	assert(is_sorted());
#endif
}


/** Erase a range of elements from \a first to \a last, including first but
 * not including last.
 */
template <typename K, typename T>
void
Table<K,T>::erase(iterator first, iterator last)
{
	const size_t first_index = first._index;
	const size_t last_index = last._index;

	erase(first_index, last_index);
}


/** Erase a range of elements from \a first_index to \a last_index, including
 * first_index but not including last_index.
 */
template <typename K, typename T>
void
Table<K,T>::erase(size_t first_index, size_t last_index)
{
	const size_t num_removed = last_index - first_index;

	// Shift left
	for (size_t j=first_index; j < size() - num_removed; ++j)
		_entries[j] = _entries[j + num_removed];

	_entries.resize(size() - num_removed);

#ifndef NDEBUG
	assert(is_sorted());
#endif
}


} // namespace Raul

