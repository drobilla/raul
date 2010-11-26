/* This file is part of Raul.
 * Copyright (C) 2007-2009 David Robillard <http://drobilla.net>
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
#include <boost/utility.hpp>
#include "raul/Deletable.hpp"
#include "raul/AtomicPtr.hpp"
#include "raul/AtomicInt.hpp"

namespace Raul {


/** A realtime safe, (partially) thread safe doubly-linked list.
 *
 * Elements can be added safely while another thread is reading the list.
 * Like a typical ringbuffer, this is single-reader single-writer threadsafe
 * only.  See documentation for specific functions for specifics.
 */
template <typename T>
class List : public Raul::Deletable, public boost::noncopyable
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
		explicit const_iterator(const List<T>* const list);
		const_iterator(const iterator& i)
		: _list(i._list), _listnode(i._listnode) {}

		inline const T&        operator*();
		inline const T*        operator->();
		inline const_iterator& operator++();
		inline bool            operator!=(const const_iterator& iter) const;
		inline bool            operator!=(const iterator& iter) const;
		inline bool            operator==(const const_iterator& iter) const;
		inline bool            operator==(const iterator& iter) const;

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
		explicit iterator(List<T>* const list);

		inline T&        operator*();
		inline T*        operator->();
		inline iterator& operator++();
		inline bool      operator!=(const iterator& iter) const;
		inline bool      operator!=(const const_iterator& iter) const;
		inline bool      operator==(const iterator& iter) const;
		inline bool      operator==(const const_iterator& iter) const;

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


} // namespace Raul

#endif // RAUL_LIST_HPP

#include "ListImpl.hpp"

