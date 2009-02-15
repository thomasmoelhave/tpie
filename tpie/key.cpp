#include <tpie/key.h>

using namespace tpie::ami;

key_range::key_range() : m_min(0), m_max(0) {
}

key_range::key_range(kb_key min_key, kb_key max_key) : m_min(min_key), m_max(max_key) {
}


