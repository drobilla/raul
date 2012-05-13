/* This file is part of Raul.
 * Copyright 2007-2011 David Robillard <http://drobilla.net>
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

#ifndef RAUL_LIST_HPP
#define RAUL_LIST_HPP

#include <cstddef>
#include <cassert>

#include "raul/AtomicInt.hpp"
#include "raul/AtomicPtr.hpp"
#include "raul/Deletable.hpp"
#include "raul/Noncopyable.hpp"

namespace Raul {

/** A realtime safe, (partially) thread safe doubly-linked list.
 *
 * Elements can be added safely while another thread is reading the list.
 * Like a typical ringbuffer, this is single-reader single-writer threadsafe
 * only.  See documentation for specific functions for specifics.
 * \ingroup raul
 */
template <typename T>
class List : Deletable, Noncopyable
{
public:

	/** A node in a List.
	 *
	 * This is exposed so the user can allocate Nodes in different thread
	 * than the list reader, and insert (e.g. via an Event) it later in the
	 * reader thread.
	 */
	class Node : public Raul::Deletable {
	public:
		explicit Node(T elem) : _elem(elem) {}
		virtual ~Node() {}

		template <typename Y>
		explicit Node(const typename List<Y>::Node& copy)
			: _elem(copy._elem), _prev(copy._prev), _next(copy._next)
		{}

		Node*     prev() const   { return _prev.get(); }
		void      prev(Node* ln) { _prev = ln; }
		Node*     next() const   { return _next.get(); }
		void      next(Node* ln) { _next = ln; }
		T&        elem()         { return _elem;}
		const T&  elem() const   { return _elem; }

	private:
		T               _elem;
		AtomicPtr<Node> _prev;
		AtomicPtr<Node> _next;
	};

	List(size_t size=0, Node* head=NULL, Node* tail=NULL)
		: _size(size)
		, _end_iter(this)
		, _const_end_iter(this)
	{
		_head = head;
		_tail = tail;
		_end_iter._listnode = NULL;
		_const_end_iter._listnode = NULL;
	}

	~List();

	void push_back(Node* elem); ///< Realtime Safe
	void push_back(T& elem);    ///< NOT Realtime Safe

	void append(List<T>& list);

	void clear();

	/// Valid only in the write thread
	unsigned size() const { return static_cast<unsigned>(_size.get()); }

	/// Valid for any thread
	bool empty() { return (_head.get() == NULL); }

	class iterator;

	/** Realtime safe const iterator for a List. */
	class const_iterator {
	public:
		explicit const_iterator(const List<T>* const list)
			: _list(list)
			, _listnode(NULL)
		{}

		explicit const_iterator(const iterator& i)
			: _list(i._list)
			, _listnode(i._listnode)
		{}

		inline const T&        operator*() { return _listnode->elem(); }
		inline const T*        operator->() { return &_listnode->elem(); }
		inline const_iterator& operator++() {
			_listnode = _listnode->next();
			return *this;
		}

		inline bool operator!=(const const_iterator& iter) const {
			return (_listnode != iter._listnode);
		}
		inline bool operator!=(const iterator& iter) const {
			return (_listnode != iter._listnode);
		}
		inline bool operator==(const const_iterator& iter) const {
			return (_listnode == iter._listnode);
		}
		inline bool operator==(const iterator& iter) const {
			return (_listnode == iter._listnode);
		}

		inline       typename List<T>::Node* node()       { return _listnode; }
		inline const typename List<T>::Node* node() const { return _listnode; }

		friend class List<T>;

	private:
		const List<T>*                _list;
		const typename List<T>::Node* _listnode;
	};

	/** Realtime safe iterator for a List. */
	class iterator {
	public:
		explicit iterator(List<T>* const list)
			: _list(list)
			, _listnode(NULL)
		{}

		inline T&        operator*()  { return _listnode->elem(); }
		inline T*        operator->() { return &_listnode->elem(); }
		inline iterator& operator++() {
			_listnode = _listnode->next();
			return *this;
		}

		inline bool operator!=(const const_iterator& iter) const {
			return (_listnode != iter._listnode);
		}
		inline bool operator!=(const iterator& iter) const {
			return (_listnode != iter._listnode);
		}
		inline bool operator==(const const_iterator& iter) const {
			return (_listnode == iter._listnode);
		}
		inline bool operator==(const iterator& iter) const {
			return (_listnode == iter._listnode);
		}

		friend class List<T>;
		friend class List<T>::const_iterator;

	private:
		const List<T>*          _list;
		typename List<T>::Node* _listnode;
	};

	void chop_front(List<T>& front, size_t front_size, Node* front_tail);

	Node* erase(const iterator iter);

	iterator find(const T& val);

	iterator       begin();
	const_iterator begin() const;
	const iterator end()   const;

	T&       front()       { return *begin(); }
	const T& front() const { return *begin(); }

	Node*       head()       { return _head.get(); }
	const Node* head() const { return _head.get(); }

private:
	AtomicPtr<Node> _head;
	AtomicPtr<Node> _tail; ///< writer only
	AtomicInt       _size;
	iterator        _end_iter;
	const_iterator  _const_end_iter;
};

template <typename T>
List<T>::~List<T>()
{
	clear();
}

template <typename T>
inline typename List<T>::iterator
List<T>::begin()
{
	typename List<T>::iterator iter(this);

	iter._listnode = _head.get();

	return iter;
}

template <typename T>
inline typename List<T>::const_iterator
List<T>::begin() const
{
	typename List<T>::const_iterator iter(this);
	iter._listnode = _head.get();
	return iter;
}

template <typename T>
inline const typename List<T>::iterator
List<T>::end() const
{
	return _end_iter;
}

/** Clear the list, deleting all Nodes contained (but NOT their contents!)
 *
 * Not realtime safe.
 */
template <typename T>
void
List<T>::clear()
{
	Node* node = _head.get();
	Node* next = NULL;

	while (node) {
		next = node->next();
		delete node;
		node = next;
	}

	_head = 0;
	_tail = 0;
	_size = 0;
}

/** Add an element to the list.
 *
 * Thread safe (may be called while another thread is reading the list).
 * Realtime safe.
 */
template <typename T>
void
List<T>::push_back(Node* const ln)
{
	assert(ln);

	ln->next(NULL);

	if ( ! _head.get()) { // empty
		ln->prev(NULL);
		_tail = ln;
		_head = ln;
	} else {
		ln->prev(_tail.get());
		_tail.get()->next(ln);
		_tail = ln;
	}
	++_size;
}

/** Add an element to the list.
 *
 * Thread safe (may be called while another thread is reading the list).
 * NOT realtime safe (a Node is allocated).
 */
template <typename T>
void
List<T>::push_back(T& elem)
{
	Node* const ln = new Node(elem);

	assert(ln);

	ln->next(NULL);

	if ( ! _head.get()) { // empty
		ln->prev(NULL);
		_tail = ln;
		_head = ln;
	} else {
		ln->prev(_tail.get());
		_tail.get()->next(ln);
		_tail = ln;
	}
	++_size;
}

/** Append a list to this list.
 *
 * This operation is fast ( O(1) ).
 * The appended list is not safe to use concurrently with this call.
 * The appended list will be empty after this call.
 *
 * Thread safe (may be called while another thread is reading the list).
 * Realtime safe.
 */
template <typename T>
void
List<T>::append(List<T>& list)
{
	Node* const my_head    = _head.get();
	Node* const my_tail    = _tail.get();
	Node* const other_head = list._head.get();
	Node* const other_tail = list._tail.get();

	assert((my_head && my_tail) || (!my_head && !my_tail));
	assert((other_head && other_tail) || (!other_head && !other_tail));

	// Appending to an empty list
	if (my_head == NULL && my_tail == NULL) {
		_tail = other_tail;
		_head = other_head;
		_size = list._size;
	} else if (other_head != NULL && other_tail != NULL) {

		other_head->prev(my_tail);

		// FIXME: atomicity an issue? _size < true size is probably fine...
		// no guarantee an iteration runs exactly size times though.  verify/document this.
		// assuming comment above that says tail is writer only, this is fine
		my_tail->next(other_head);
		_tail = other_tail;
		_size += list.size();
	}

	list._head = NULL;
	list._tail = NULL;
	list._size = 0;
}

/** Find an element in the list.
 *
 * This will return the first element equal to @a val found in the list.
 */
template <typename T>
typename List<T>::iterator
List<T>::find(const T& val)
{
	for (iterator i = begin(); i != end(); ++i)
		if (*i == val)
			return i;

	return end();
}

/** Remove an element from the list using an iterator.
 *
 * This function is realtime safe - it is the caller's responsibility to
 * delete the returned Node, or there will be a leak.
 * Thread safe (safe to call while another thread reads the list).
 * @a iter is invalid immediately following this call.
 */
template <typename T>
typename List<T>::Node*
List<T>::erase(const iterator iter)
{
	assert((_head.get() && _tail.get()) || (!_head.get() && !_tail.get()));

	Node* const n = iter._listnode;

	if (n) {
		Node* const prev = n->prev();
		Node* const next = n->next();

		// Removing the head (or the only element)
		if (n == _head.get())
			_head = next;

		// Removing the tail (or the only element)
		if (n == _tail.get())
			_tail = _tail.get()->prev();

		if (prev)
			n->prev()->next(next);

		if (next)
			n->next()->prev(prev);

		--_size;
	}

	assert((_head.get() && _tail.get()) || (!_head.get() && !_tail.get()));
	return n;
}

template <typename T>
void
List<T>::chop_front(List<T>& front, size_t front_size, Node* front_tail)
{
	assert(front_tail);
	assert((front._head.get() && front._tail.get()) || (!front._head.get() && !front._tail.get()));
	assert((_head.get() && _tail.get()) || (!_head.get() && !_tail.get()));
	front._size = front_size;
	front._head = _head;
	front._tail = front_tail;
	Node* new_head = front_tail->next();
	if (new_head) {
		new_head->prev(NULL);
		_head = new_head;
	} else {
		// FIXME: race?
		_head = NULL;
		_tail = NULL;
	}
	_size -= front_size;
	front_tail->next(NULL);
	assert((front._head.get() && front._tail.get()) || (!front._head.get() && !front._tail.get()));
	assert((_head.get() && _tail.get()) || (!_head.get() && !_tail.get()));
}

} // namespace Raul

#endif // RAUL_LIST_HPP

