#ifndef PIRE_PARTITION_H
#define PIRE_PARTITION_H


#include <assert.h>
#include "stl.h"
#include "stub/singleton.h"

namespace Pire {

/*
* A class which forms a disjoint set of pairwise equivalent items,
* depending on given equivalence relation.
*/
template<class T, class Eq>
class Partition {
private:
	typedef std::map< T, std::pair< size_t, std::vector<T> > > Set;

public:
	Partition(const Eq& eq)
		: m_eq(eq)
		, m_maxidx(0)
	{
	}

	/// Appends a new item into partition, creating new equivalience class if neccessary.
	void Append(const T& t) {
		DoAppend(m_set, t);
	}

	typedef typename Set::const_iterator ConstIterator;

	ConstIterator Begin() const {
		return m_set.begin();
	}
	ConstIterator End() const {
		return m_set.end();
	}
	size_t Size() const {
		return m_set.size();
	}
	bool Empty() const {
		return m_set.empty();
	}

	/// Returns an item equal to @p t. It is guaranteed that:
	/// - representative(a) equals representative(b) iff a is equivalent to b;
	/// - representative(a) is equivalent to a.
	const T& Representative(const T& t) const
	{
		typename std::map<T, T>::const_iterator it = m_inv.find(t);
		if (it != m_inv.end())
			return it->second;
		else
			return DefaultValue<T>();
	}
	
	bool Contains(const T& t) const
	{
		return m_inv.find(t) != m_inv.end();
	}

	/// Returns an index of set containing @p t. It is guaranteed that:
	/// - index(a) equals index(b) iff a is equivalent to b;
	/// - 0 <= index(a) < size().
	size_t Index(const T& t) const
	{
		typename std::map<T, T>::const_iterator it = m_inv.find(t);
		if (it == m_inv.end())
			throw Error("Partition::index(): attempted to obtain an index of nonexistent item");
		typename Set::const_iterator it2 = m_set.find(it->second);
		assert(it2 != m_set.end());
		return it2->second.first;
	}
	/// Returns the whole equivalence class of @p t (i.e. item @p i
	/// is returned iff representative(i) == representative(t)).
	const std::vector<T>& Klass(const T& t) const
	{
		typename std::map<T, T>::const_iterator it = m_inv.find(t);
		if (it == m_inv.end())
			throw Error("Partition::index(): attempted to obtain an index of nonexistent item");
		ConstIterator it2 = m_set.find(it->second);
		assert(it2 != m_set.end());
		return it2->second.second;
	}

	bool operator == (const Partition& rhs) const { return m_set == rhs.m_set; }
	bool operator != (const Partition& rhs) const { return !(*this == rhs); }

	/// Splits the current sets into smaller ones, using given equivalence relation.
	/// Requires given relation imply previous one (set either in ctor or
	/// in preceeding calls to split()), but performs faster.
	/// Replaces previous relation with given one.
	void Split(const Eq& eq)
	{
		m_eq = eq;

		for (typename Set::iterator sit = m_set.begin(), sie = m_set.end(); sit != sie; ++sit)
			if (sit->second.second.size() > 1) {
				std::vector<T>& v = sit->second.second;
				typename std::vector<T>::iterator bound = std::partition(v.begin(), v.end(), std::bind2nd(m_eq, v[0]));
				if (bound == v.end())
					continue;

				Set delta;
				for (typename std::vector<T>::iterator it = bound, ie = v.end(); it != ie; ++it)
					DoAppend(delta, *it);

				v.erase(bound, v.end());
				m_set.insert(delta.begin(), delta.end());
			}
	}

private:
	Eq m_eq;
	Set m_set;
	std::map<T, T> m_inv;
	size_t m_maxidx;

	void DoAppend(Set& set, const T& t)
	{
		typename Set::iterator it = set.begin();
		typename Set::iterator end = set.end();
		for (; it != end; ++it)
			if (m_eq(it->first, t)) {
				it->second.second.push_back(t);
				m_inv[t] = it->first;
				break;
			}

		if (it == end) {
			// Begin new set
			std::vector<T> v(1, t);
			set.insert(std::make_pair(t, std::make_pair(m_maxidx++, v)));
			m_inv[t] = t;
		}
	}
};

// Mainly for debugging
template<class T, class Eq>
yostream& operator << (yostream& stream, const Partition<T, Eq>& partition)
{
	stream << "Partition {" << std::endl;
	for (typename Partition<T, Eq>::ConstIterator it = partition.Begin(), ie = partition.End(); it != ie; ++it) {
		stream << "    Class " << it->second.first << " \"" << it->first << "\" { ";
		bool first = false;
		for (typename std::vector<T>::const_iterator iit = it->second.second.begin(), iie = it->second.second.end(); iit != iie; ++iit) {
			if (first)
				stream << ", ";
			else
				first = true;
			stream << *iit;
		}
		stream << " }" << std::endl;
	}
	stream << "}";
	return stream;
}

}


#endif