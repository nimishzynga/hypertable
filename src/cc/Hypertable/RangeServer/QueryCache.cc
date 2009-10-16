/** -*- c++ -*-
 * Copyright (C) 2009 Doug Judd (Hypertable, Inc.)
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "Common/Compat.h"
#include <cassert>
#include <iostream>

#include "QueryCache.h"

using namespace Hypertable;
using std::pair;

bool QueryCache::insert(Key *key, uint32_t table_id, const char *row,
			boost::shared_array<uint8_t> &result,
			uint32_t result_length) {
  ScopedLock lock(m_mutex);
  LookupHashIndex &hash_index = m_cache.get<1>();
  LookupHashIndex::iterator lookup_iter;
  uint64_t length = result_length + 32 + strlen(row);
  
  if (length > m_max_memory)
    return false;

  if ((lookup_iter = hash_index.find(*key)) != hash_index.end())
    hash_index.erase(lookup_iter);

  // make room
  if (m_avail_memory < length) {
    Cache::iterator iter = m_cache.begin();
    while (iter != m_cache.end()) {
      m_avail_memory += (*iter).result_length + 32;
      iter = m_cache.erase(iter);
      if (m_avail_memory >= length)
	break;
    }
  }

  if (m_avail_memory < length)
    return false;

  QueryCacheEntry entry(*key, table_id, row, result, result_length);

  pair<Sequence::iterator, bool> insert_result = m_cache.push_back(entry);
  assert(insert_result.second);

  m_avail_memory -= length;

  return true;
}


bool QueryCache::lookup(Key *key, boost::shared_array<uint8_t> &result,
			uint32_t *lenp) {
  ScopedLock lock(m_mutex);
  LookupHashIndex &hash_index = m_cache.get<1>();
  LookupHashIndex::iterator iter;

  if (m_lookup_count > 0 && (m_lookup_count % 1000) == 0) {
    HT_INFOF("QueryCache hit rate %f",
             ((double)m_hit_count / (double)m_lookup_count)*100.0);
    m_lookup_count = m_hit_count = 0;
  }

  m_lookup_count++;

  if ((iter = hash_index.find(*key)) == hash_index.end())
    return false;

  QueryCacheEntry entry = *iter;

  hash_index.erase(iter);

  pair<Sequence::iterator, bool> insert_result = m_cache.push_back(entry);
  assert(insert_result.second);

  result = (*insert_result.first).result;
  *lenp = (*insert_result.first).result_length;

  m_hit_count++;

  return true;
}

void QueryCache::invalidate(uint32_t table_id, const char *row) {
  ScopedLock lock(m_mutex);
  InvalidateHashIndex &hash_index = m_cache.get<2>();
  InvalidateHashIndex::iterator iter;
  RowKey row_key(table_id, row);
  pair<InvalidateHashIndex::iterator, InvalidateHashIndex::iterator> p = hash_index.equal_range(row_key);
  uint64_t length;

  for (; p.first != p.second; ++p.first) {
    length = (*p.first).result_length + 32 + (*p.first).row_key.row.length();
    /** HT_ASSERT((*p.first).row_key.table_id == table_id &&
        strcmp((*p.first).row_key.row.c_str(), row) == 0); **/
    m_avail_memory += length;
    hash_index.erase(p.first);
  }

}


void QueryCache::dump() {
  ScopedLock lock(m_mutex);
  Sequence &index0 = m_cache.get<0>();
  LookupHashIndex &index1 = m_cache.get<1>();
  InvalidateHashIndex &index2 = m_cache.get<2>();

  std::cout << "index0:" << std::endl;
  for (Sequence::iterator iter = index0.begin(); iter != index0.end(); ++iter) {
    QueryCacheEntry entry(*iter);
    entry.dump();
  }

  std::cout << "index1:" << std::endl;
  for (LookupHashIndex::iterator iter = index1.begin(); iter != index1.end(); ++iter) {
    QueryCacheEntry entry(*iter);
    entry.dump();
  }

  std::cout << "index2:" << std::endl;
  for (InvalidateHashIndex::iterator iter = index2.begin(); iter != index2.end(); ++iter) {
    QueryCacheEntry entry(*iter);
    entry.dump();
  }
}