#pragma once

#include "rec_base.hpp"

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Ordered collection of matching DNS records returned by CZone::search().
 *
 * Stores non-owning raw pointers — no copies of records are made.
 * The CZone that produced this result must outlive it.
 */
class CSearchResult {
    std::vector<CRec*> m_results;

public:
    std::size_t size() const { return m_results.size(); }

    /// Returns the record at @p index by reference.
    /// @throws std::out_of_range if index ≥ size().
    CRec& operator[](std::size_t index) const {
        if (index < m_results.size()) return *m_results[index];
        throw std::out_of_range(std::to_string(index));
    }

    friend std::ostream& operator<<(std::ostream& os, const CSearchResult& r) {
        for (const CRec* rec : r.m_results)
            os << *rec << "\n";
        return os;
    }

    void addResult(CRec* rec)                   { m_results.push_back(rec); }
    void addResult(const CSearchResult& other)  {
        for (CRec* rec : other.m_results)
            m_results.push_back(rec);
    }
};
