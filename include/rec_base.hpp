#pragma once

#include <memory>
#include <ostream>
#include <string>

/**
 * Abstract base for all DNS resource records.
 *
 * Concrete leaf types: CRecA, CRecAAAA, CRecMX, CRecCNAME, CRecSPF.
 * Composite type: CZone (holds owned records and nested zones).
 *
 * Ownership model:
 *   - CZone stores records as std::unique_ptr<CRec>.
 *   - CSearchResult holds *non-owning* raw pointers — the producing zone
 *     must outlive any search result derived from it.
 */
class CRec {
    friend class CZone;

public:
    CRec(std::string name, std::string type)
        : m_name(std::move(name)), m_type(std::move(type)) {}

    virtual ~CRec() = default;

    std::string name() const { return m_name; }
    std::string type() const { return m_type; }

    friend std::ostream& operator<<(std::ostream& os, const CRec& r) {
        r.printAddr(os);
        return os;
    }

protected:
    /// Returns true if adding this record would conflict with @p other.
    /// Overridden by CRecCNAME and CZone (name-uniqueness constraints).
    virtual bool conflictsWith(const CRec& /*other*/) const { return false; }

    /// Returns a heap-allocated deep copy. Caller takes ownership.
    virtual std::unique_ptr<CRec> clone() const = 0;

    /// Formats this record as a single line: "name type addr..."
    virtual void printAddr(std::ostream& os) const = 0;

    /// Formats this record as a tree node with ASCII-art connectors.
    virtual void printStructured(std::ostream& os,
                                 const std::string& prefix,
                                 bool isLast) const {
        os << " " << prefix << (isLast ? "\\- " : "+- ") << *this;
    }

    virtual bool equals(const CRec& other) const = 0;

    std::string m_name;
    std::string m_type;
};
