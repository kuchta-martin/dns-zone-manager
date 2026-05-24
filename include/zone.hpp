#pragma once

#include "rec_base.hpp"
#include "search_result.hpp"

#include <memory>
#include <ostream>
#include <string>
#include <vector>

/**
 * A DNS zone: a named, owning container for resource records and sub-zones.
 *
 * Records are stored as std::unique_ptr<CRec> — no manual memory management.
 * CZone itself is also a CRec, so zones can be nested to arbitrary depth.
 *
 * Conflict rules enforced by add():
 *   - No exact duplicates (equals()).
 *   - CNAME record blocks all other records with the same name.
 *   - Zone name must be unique within the parent zone.
 */
class CZone : public CRec {
    std::vector<std::unique_ptr<CRec>> m_recs; ///< Owned records (leaves + sub-zones)

public:
    explicit CZone(std::string name) : CRec(std::move(name), "zone") {}

    CZone(const CZone& other);
    CZone& operator=(const CZone& other);

    /// Adds a deep copy of @p rec if it does not conflict with existing records.
    /// Returns false (and leaves the zone unchanged) on conflict or duplicate.
    bool add(const CRec& rec);

    /// Removes the first record that equals @p rec.
    /// Returns false if no such record exists.
    bool del(const CRec& rec);

    /**
     * Searches for records, with optional dot-path zone navigation.
     *
     *   search("progtest")         — flat: returns all records named "progtest".
     *   search("progtest.fit.cvut")— recursive: navigate fit → cvut, then flat.
     *
     * @note The returned CSearchResult holds non-owning pointers into this zone.
     *       This zone must outlive the result.
     */
    CSearchResult search(const std::string& query) const;

    friend std::ostream& operator<<(std::ostream& os, const CZone& z) {
        z.printAddr(os);
        os << "\n";
        return os;
    }

protected:
    std::unique_ptr<CRec> clone()                     const override;
    bool equals(const CRec& other)                    const override;
    bool conflictsWith(const CRec& other)             const override;
    void printAddr(std::ostream& os)                  const override;
    void printStructured(std::ostream& os,
                         const std::string& prefix,
                         bool isLast)                 const override;
};
