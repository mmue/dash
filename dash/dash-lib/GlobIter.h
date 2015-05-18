/* 
 * dash-lib/GlobIter.h
 *
 * author(s): Karl Fuerlinger, LMU Munich 
 */
/* @DASH_HEADER@ */

#ifndef GLOBITER_H_INCLUDED
#define GLOBITER_H_INCLUDED

#include "Pattern.h"
#include "GlobRef.h"

namespace dash {

// KF: check 
typedef long gptrdiff_t;

template<typename T>
class GlobIter 
: public GlobPtr<T>,
  public std::iterator<
            std::random_access_iterator_tag,
				    T, gptrdiff_t, GlobIter<T>, GlobRef<T> > {
protected:
  GlobMem<T> * m_globmem;
  Pattern<1> * m_pattern;
  size_t       m_idx;
  
public:
  GlobIter() : GlobPtr<T>() {
    m_globmem = nullptr;
    m_pattern = nullptr;
    m_idx=0;
  }

  GlobIter(
    GlobMem<T> * mem,
	  Pattern<1> & pat,
	  size_t       idx = 0)
  : GlobPtr<T>(mem->begin()) {
    m_globmem = mem;
    m_pattern = &pat;
    m_idx = idx;
  }

  GlobIter(const GlobIter<T>& other) {
    m_globmem = other.m_globmem;
    m_pattern = other.m_pattern;
    m_idx     = other.m_idx;
  }
  
  GlobRef<T> operator*() {
    auto coord = m_pattern->sizespec().coords(m_idx);
    auto unit  = m_pattern->index_to_unit(coord);
    auto elem  = m_pattern->index_to_elem(coord);
    GlobPtr<T> ptr = m_globmem->get_globptr(unit, elem);
    return GlobRef<T>(ptr);
  }  

  GlobRef<T> operator[](gptrdiff_t n) {
    auto coord = m_pattern->sizespec().coords(n);
    auto unit  = m_pattern->index_to_unit(coord);
    auto elem  = m_pattern->index_to_elem(coord);
    GlobPtr<T> ptr = m_globmem->get_globptr(unit, elem);
    return GlobRef<T>(ptr);
  }

  // prefix++ operator
  GlobIter<T>& operator++() {
    m_idx++;
    return *this;
  }
  
  // postfix++ operator
  GlobIter<T> operator++(int) {
    GlobIter<T> result = *this;
    m_idx++;
    return result;
  }

  // prefix-- operator
  GlobIter<T>& operator--() {
    m_idx--;
    return *this;
  }
  
  // postfix-- operator
  GlobIter<T> operator--(int) {
    GlobIter<T> result = *this;
    m_idx--;
    return result;
  }
  
  GlobIter<T>& operator+=(gptrdiff_t n) {
    m_idx+=n;
    return *this;
  }
  
  GlobIter<T>& operator-=(gptrdiff_t n) {
    m_idx-=n;
    return *this;
  }

  GlobIter<T> operator+(gptrdiff_t n) const {
    GlobIter<T> res(m_globmem, *m_pattern, m_idx+n);
    return res;
  }

  GlobIter<T> operator-(gptrdiff_t n) const {
    GlobIter<T> res(m_globmem, *m_pattern, m_idx-n);
    return res;
  }

  bool operator<(const GlobIter<T>& other) const {
    //std::cerr<<"Operator<"<<std::endl;
    return m_idx < other.m_idx;
  }

  bool operator<=(const GlobIter<T>& other) const {
    return m_idx <= other.m_idx;
  }

  bool operator>(const GlobIter<T>& other) const {
    //std::cerr<<"Operator>"<<std::endl;
    return m_idx > other.m_idx;
  }

  bool operator>=(const GlobIter<T>& other) const {
    return m_idx >= other.m_idx;
  }

  bool operator==(const GlobIter<T>& other) const {
    return m_idx == other.m_idx;
  }

  bool operator!=(const GlobIter<T>& other) const {
    return m_idx != other.m_idx;
  }
};

};

#endif /* GLOBPTR_H_INCLUDED */
