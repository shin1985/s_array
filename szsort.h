#ifndef __SORTING_STRINGS_TERMINATED_BY_ZERO_H_
#define __SORTING_STRINGS_TERMINATED_BY_ZERO_H_


// This header provides a function to sort keys in dictionary order
// by using multikey quicksort. And, the function can find same keys in
// a given keyset.
//
// StringSort::Sort() receives 2 random access iterators (or pointers),
// depth, and 2 functions (GetLetter and HandleSameKeys). Letters with
// depth+ are used for sorting. GetLetter is used to get letters from keys,
// and HandleSameKeys is called when same keys appear.
//
// GetLetter must receive a key (value type of an iterator) and depth,
// and return a letter (unsigned-compatible).
//
// HandleSameKeys must receive 2 iterators (or pointers) and return true or
// false. If this function returns false, sorting will stop without being
// completed. If the existence of same keys is insignificant, please don't
// pass HandleSameKeys so that a default handler is used. The default
// handler does nothing and returns true.


// For std::size_t.
#include <cstddef>

// For std::iter_swap().
#include <algorithm>


namespace StringSort {

using std::size_t;
const char *mapdata;

// Default handler does nothing.
struct DefaultHandler {
  template <typename RandomAccessIterator>
  bool operator()(RandomAccessIterator l, RandomAccessIterator r) {
    return true;
  }
};

// Compare a pair of keys.
template <typename RandomAccessIterator, typename GetLetter>
inline int CompareKeys(
    RandomAccessIterator lhs, RandomAccessIterator rhs,
    size_t depth, GetLetter get_letter) {
  unsigned l, r;
  do {
    l = get_letter(*lhs, depth);
    r = get_letter(*rhs, depth);
    ++depth;
  } while (l != '\0' && l == r);
  return (l < r) ? -1 : (l > r);
}

// Choose the median from 3 values.
template <typename LetterType>
inline LetterType GetMedian(LetterType l, LetterType m, LetterType r) {
  if (l < m) {
    if (m < r) return m;
    else if (l < r) return r;
    return l;
  } else if (l < r) return l;
  else if (m < r) return r;
  return m;
}

// Find same keys and call HandleSameKeys for them.
template <typename RandomAccessIterator, typename GetLetter,
          typename HandleSameKeys>
bool FindSameKeys(
    RandomAccessIterator l, RandomAccessIterator r, size_t depth,
    GetLetter get_letter, HandleSameKeys handle_same_keys) {
  RandomAccessIterator start = l;
  for (++l; l < r; ++l) {
    if (CompareKeys(start, l, depth, get_letter)) {
      if (l - start > 1 && !handle_same_keys(start, l))
        return false;
      start = l;
    }
  }
  return (r - start > 1) ? handle_same_keys(start, r) : true;
}

// Insertion sort for small range.
template <typename RandomAccessIterator, typename GetLetter,
          typename HandleSameKeys>
bool InsertionSort(
    RandomAccessIterator l, RandomAccessIterator r, size_t depth,
    GetLetter get_letter, HandleSameKeys handle_same_keys) {
  int equal_flag = 0;
  RandomAccessIterator i, j;
  for (i = l + 1; i < r; ++i) {
    for (j = i; j > l; --j) {
      int compare = CompareKeys(j - 1, j, depth, get_letter);
      equal_flag |= (compare == 0);
      if (compare <= 0)
        break;
      std::iter_swap(j - 1, j);
    }
  }
  if (equal_flag)
    return FindSameKeys(l, r, depth, get_letter, handle_same_keys);
  return true;
}

// Sort an array by multi-key quick sort.
template <typename RandomAccessIterator, typename GetLetter,
          typename HandleSameKeys>
bool Sort(const char *mapdata, RandomAccessIterator l, RandomAccessIterator r, size_t depth,
                 GetLetter get_letter, HandleSameKeys handle_same_keys) {
					 StringSort::mapdata = mapdata;
  while (r - l > 10) {
    unsigned pivot;
    RandomAccessIterator pl = l, pr = r, pivot_l = l, pivot_r = r;
    pivot = GetMedian(get_letter(*l, depth),
                      get_letter(*(l + (r - l) / 2), depth),
                      get_letter(*(r - 1), depth));

    // Move keys less than the pivot to left.
    // Move keys greater than the pivot to right.
    for (;;) {
      int diff;
      while (pl < pr && (diff = get_letter(*pl, depth) - pivot) <= 0) {
        if (!diff)
          std::iter_swap(pl, pivot_l), ++pivot_l;
        ++pl;
      }
      while (pl < pr && (diff = get_letter(*--pr, depth) - pivot) >= 0) {
        if (!diff)
          std::iter_swap(pr, --pivot_r);
      }
      if (pl >= pr)
        break;
      std::iter_swap(pl, pr);
      ++pl;
    }
    while (pivot_l > l)
      std::iter_swap(--pivot_l, --pl);
    while (pivot_r < r)
      std::iter_swap(pivot_r, pr), ++pivot_r, ++pr;

    // To avoid stack over flow.
    // Use recursive call except for the largest range.
    if (pl - l > pr - pl || r - pr > pr - pl) {
      if (pr - pl > 1) {
        if (pivot != '\0') {
          if (!Sort(mapdata, pl, pr, depth + 1, get_letter, handle_same_keys))
            return false;
        } else if (!handle_same_keys(pl, pr))
          return false;
      }
      if (pl - l < r - pr) {
        if (pl - l > 1 && !Sort(mapdata, l, pl, depth, get_letter, handle_same_keys))
          return false;
        l = pr;
      } else {
        if (r - pr > 1 && !Sort(mapdata, pr, r, depth, get_letter, handle_same_keys))
          return false;
        r = pl;
      }
    } else {
      if (pl - l > 1 && !Sort(mapdata, l, pl, depth, get_letter, handle_same_keys))
        return false;
      if (r - pr > 1 && !Sort(mapdata, pr, r, depth, get_letter, handle_same_keys))
        return false;
      l = pl, r = pr;
      if (pr - pl > 1) {
        if (pivot != '\0')
          ++depth;
        else {
          if (!handle_same_keys(pl, pr))
            return false;
          l = r;
        }
      }
    }
  }
  if (r - l > 1)
    return InsertionSort(l, r, depth, get_letter, handle_same_keys);
  return true;
}

// Sort an array by multi-key quick sort.
template <typename RandomAccessIterator, typename GetLetter>
inline void Sort(const char *mapdata, RandomAccessIterator l, RandomAccessIterator r, size_t depth,
                 GetLetter get_letter) {
  Sort(mapdata, l, r, depth, get_letter, DefaultHandler());
}

}  // namespace StringSort.


#endif  // __SORTING_STRINGS_TERMINATED_BY_ZERO_H_
