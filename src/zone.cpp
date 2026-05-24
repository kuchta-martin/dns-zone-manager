#include "zone.hpp"

#include <cstddef>

// ── Copy / assignment ────────────────────────────────────────────────────────

CZone::CZone(const CZone& other) : CRec(other) {
    for (const auto& r : other.m_recs)
        m_recs.push_back(r->clone());
}

CZone& CZone::operator=(const CZone& other) {
    if (this == &other) return *this;
    CRec::operator=(other);
    m_recs.clear(); // unique_ptr handles deletion
    for (const auto& r : other.m_recs)
        m_recs.push_back(r->clone());
    return *this;
}

// ── Mutation ─────────────────────────────────────────────────────────────────

bool CZone::add(const CRec& rec) {
    for (const auto& r : m_recs) {
        if (rec.conflictsWith(*r) || r->conflictsWith(rec) || rec.equals(*r))
            return false;
    }
    m_recs.push_back(rec.clone());
    return true;
}

bool CZone::del(const CRec& rec) {
    for (auto it = m_recs.begin(); it != m_recs.end(); ++it) {
        if ((*it)->equals(rec)) {
            m_recs.erase(it);
            return true;
        }
    }
    return false;
}

// ── Query ────────────────────────────────────────────────────────────────────

CSearchResult CZone::search(const std::string& query) const {
    CSearchResult result;
    const std::size_t dot = query.rfind('.');

    if (dot == std::string::npos) {
        // Flat search: match all records by name in this zone.
        for (const auto& r : m_recs)
            if (r->name() == query)
                result.addResult(r.get());
    } else {
        // Dot-path: last component names the sub-zone to recurse into.
        const std::string zoneName = query.substr(dot + 1);
        const std::string rest     = query.substr(0, dot);
        for (const auto& r : m_recs) {
            const auto* z = dynamic_cast<const CZone*>(r.get());
            if (z && z->name() == zoneName)
                return z->search(rest);
        }
    }
    return result;
}

// ── CRec interface ───────────────────────────────────────────────────────────

std::unique_ptr<CRec> CZone::clone() const {
    return std::make_unique<CZone>(*this);
}

bool CZone::equals(const CRec& other) const {
    const auto* o = dynamic_cast<const CZone*>(&other);
    return o && m_name == o->m_name;
}

bool CZone::conflictsWith(const CRec& other) const {
    return name() == other.name();
}

void CZone::printAddr(std::ostream& os) const {
    os << m_name;
    for (std::size_t i = 0; i < m_recs.size(); ++i) {
        os << "\n";
        m_recs[i]->printStructured(os, "", i == m_recs.size() - 1);
    }
}

void CZone::printStructured(std::ostream& os,
                             const std::string& prefix,
                             bool isLast) const {
    os << " " << prefix << (isLast ? "\\- " : "+- ") << m_name;
    const std::string newPrefix = prefix + (isLast ? "   " : "|  ");
    for (std::size_t i = 0; i < m_recs.size(); ++i) {
        os << "\n";
        m_recs[i]->printStructured(os, newPrefix, i == m_recs.size() - 1);
    }
}
