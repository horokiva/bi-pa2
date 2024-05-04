#ifndef PROGTEST
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <memory>
#endif /* PROGTEST */

class CIterator;

class CLandRegister
{
public:
    CLandRegister(); // Default constructor
    ~CLandRegister(); // Default destructor

    bool add(const std::string& city, const std::string& addr,
             const std::string& region, unsigned long long id);

    bool del(const std::string& city, const std::string& addr);

    bool del(const std::string    & region, unsigned long long id);

    bool getOwner(const std::string& city, const std::string& addr,
                  std::string& owner) const;

    bool getOwner(const std::string& region, unsigned long long id,
                  std::string& owner) const;

    bool newOwner(const std::string& city, const std::string& addr,
                  const std::string& owner);

    bool newOwner(const std::string& region, unsigned long long id,
                  const std::string& owner);

    size_t count(const std::string& owner) const;

    CIterator listByAddr() const;

    CIterator listByOwner(const std::string& owner) const;

    void printAll();

private:

    struct m_Property {

        // Properties
        std::string m_City;
        std::string m_Address;
        std::string m_Region;
        unsigned long long m_ID;
        std::string m_Owner;
        unsigned long long m_AcquisitionTimestamp;


        // Methods
        bool operator < (const m_Property& otherProperty) const;
        bool operator == (const m_Property& otherProperty) const;
        static bool CompareLessThan(const m_Property* lhs, const m_Property* rhs);
        static bool CompareIDLessThan(const m_Property* lhs, const m_Property* rhs);

        // Constructor
        m_Property(const std::string& city, const std::string& addr, const std::string& region, unsigned long long id);
        m_Property(const std::string& city, const std::string address);
        m_Property(const std::string& region, unsigned long long id);
        ~m_Property();
    };

    friend class CIterator;
    std::vector<m_Property*> sortedByCityAddr;
    std::vector<m_Property*> sortedByOwner;
    std::vector<m_Property*> sortedByRegionId;
    unsigned long long m_NextAcquisitionOrder = 0;
};

class CIterator
{
public:
    CIterator(const CLandRegister& landRegister, std::vector<CLandRegister::m_Property*> sortedProperties);
    ~CIterator();

    bool atEnd() const;
    void next();
    std::string city() const;
    std::string addr() const;
    std::string region() const;
    unsigned id() const;
    std::string owner() const;
private:
    friend class CLandRegister;
    const CLandRegister &landRegister;
    size_t currentIndex;
    std::vector<CLandRegister::m_Property*> sortedProperties;
};

CLandRegister::CLandRegister() {}

CLandRegister::~CLandRegister() {}

CLandRegister::m_Property::m_Property(const std::string& city, const std::string& address, const std::string& region, unsigned long long id)
        : m_City(city), m_Address(address), m_Region(region), m_ID(id) {}

CLandRegister::m_Property::m_Property(const std::string& city, const std::string address) : m_City(city), m_Address(address) {}

CLandRegister::m_Property::m_Property(const std::string &region, unsigned long long id) : m_Region(region), m_ID(id) {}

CLandRegister::m_Property::~m_Property() {}

CIterator::CIterator(const CLandRegister& landRegister, std::vector<CLandRegister::m_Property*> sortedProperties)
        : landRegister(landRegister), currentIndex(0), sortedProperties(sortedProperties) {}

CIterator::~CIterator() {}

bool CLandRegister::m_Property::operator < (const m_Property& otherProperty) const
{
    // Sort by name and if names are same sort by address
    if (this->m_City == otherProperty.m_City)
    {
        return this->m_Address < otherProperty.m_Address;
    }

    return this->m_City < otherProperty.m_City;
}

bool CLandRegister::m_Property::operator == (const m_Property& otherProperty) const
{
    return this->m_City == otherProperty.m_City && this->m_Address == otherProperty.m_Address;
}

bool CLandRegister::m_Property::CompareLessThan(const m_Property* lhs, const m_Property* rhs)
{
    return lhs->operator<(*rhs);
}

bool CLandRegister::m_Property::CompareIDLessThan(const CLandRegister::m_Property *lhs,
                                                  const CLandRegister::m_Property *rhs) {
    return lhs->m_AcquisitionTimestamp < rhs->m_AcquisitionTimestamp;
}

bool CLandRegister::add(const std::string& city, const std::string& address, const std::string& region, unsigned long long id) {
    if(city.empty() || address.empty() || region.empty()) {
        return false;
    }
    std::string locCity(city);
    std::string locAddr(address);
    std::string locRegion(region);
    unsigned long long locId(id);

    m_Property* newProperty = new m_Property(locCity, locAddr, locRegion, locId);

    // Search for company with specific ID
    auto listCityAddressIt = std::lower_bound(sortedByCityAddr.begin(),
                                              sortedByCityAddr.end(),
                                              newProperty,
                                              CLandRegister::m_Property::CompareLessThan);

    auto listRegionIdIt = std::lower_bound(sortedByRegionId.begin(),
                                           sortedByRegionId.end(),
                                           newProperty,
                                           [&](const m_Property* lhs, const m_Property* rhs) {
                                               if (lhs->m_Region == rhs->m_Region)
                                               {
                                                   return lhs->m_ID < rhs->m_ID;
                                               }

                                               return lhs->m_Region < rhs->m_Region; });

    if (listCityAddressIt != sortedByCityAddr.end() &&
        (*listCityAddressIt)->m_City == newProperty->m_City &&
          (*listCityAddressIt)->m_Address == newProperty->m_Address) {
        delete newProperty;
        return false;
    }

    if (listRegionIdIt != sortedByRegionId.end() &&
        (*listRegionIdIt)->m_Region == newProperty->m_Region
        && (*listRegionIdIt)->m_ID == newProperty->m_ID) {
        delete newProperty;
        return false;
    }

    // Insert the property into both lists
    sortedByCityAddr.insert(listCityAddressIt, newProperty);
    sortedByRegionId.insert(listRegionIdIt, newProperty);
    newProperty->m_AcquisitionTimestamp = m_NextAcquisitionOrder++;

    return true;
}

bool CLandRegister::del(const std::string& city, const std::string& address)
{
    if (city.empty() || address.empty()) {
        return false;
    }

    std::string locCity(city);
    std::string locAddress(address);

    m_Property* searchProperty = new m_Property(locCity, locAddress);

    auto listCityAddressIt = std::lower_bound(sortedByCityAddr.begin(), sortedByCityAddr.end(), searchProperty,
                                              CLandRegister::m_Property::CompareLessThan);

    if (listCityAddressIt != sortedByCityAddr.end() &&  (*listCityAddressIt)->m_City == searchProperty->m_City
        && (*listCityAddressIt)->m_Address == searchProperty->m_Address) {
        m_Property* toRemove = (*listCityAddressIt);
        searchProperty->m_Region = toRemove->m_Region;
        searchProperty->m_ID = toRemove->m_ID;
        searchProperty->m_AcquisitionTimestamp = toRemove->m_AcquisitionTimestamp;

        // Remove pointers from list
        sortedByCityAddr.erase(listCityAddressIt);

        auto listRegionIdIt = std::lower_bound(sortedByRegionId.begin(), sortedByRegionId.end(), searchProperty,
                                            [&](const m_Property* lhs, const m_Property* rhs) {
                                                if (lhs->m_Region == rhs->m_Region)
                                                {
                                                    return lhs->m_ID < rhs->m_ID;
                                                }

                                                return lhs->m_Region < rhs->m_Region; });

        sortedByRegionId.erase(listRegionIdIt);

        // Free memory
        delete searchProperty;
        delete toRemove;

        return true;
    }

    // Free memory
    delete searchProperty;
    return false;
}

bool CLandRegister::del(const std::string &region, unsigned long long id)
{
    if (region.empty()) {
        return false;
    }

    std::string locRegion(region);
    unsigned long long locId(id);

    m_Property* searchProperty = new m_Property(locRegion, locId);

    auto listRegionIdIt = std::lower_bound(sortedByRegionId.begin(), sortedByRegionId.end(), searchProperty,
                                        [&](const m_Property* lhs, const m_Property* rhs) {
                                            if (lhs->m_Region == rhs->m_Region)
                                            {
                                                return lhs->m_ID < rhs->m_ID;
                                            }

                                            return lhs->m_Region < rhs->m_Region; });

    if (listRegionIdIt != sortedByRegionId.end() && (*listRegionIdIt)->m_Region == searchProperty->m_Region
        && (*listRegionIdIt)->m_ID == searchProperty->m_ID) {
        m_Property* toRemove = (*listRegionIdIt);
        searchProperty->m_City = toRemove->m_City;
        searchProperty->m_Address = toRemove->m_Address;
        searchProperty->m_AcquisitionTimestamp = toRemove->m_AcquisitionTimestamp;

        sortedByRegionId.erase(listRegionIdIt);

        auto listCityAddressIt = std::lower_bound(sortedByCityAddr.begin(), sortedByCityAddr.end(), searchProperty,
                                                  CLandRegister::m_Property::CompareLessThan);

        // Remove pointers from list
        sortedByCityAddr.erase(listCityAddressIt);

        // Free memory
        delete searchProperty;
        delete toRemove;

        return true;
    }


    // Free memory
    delete searchProperty;
    return false;
}

bool CLandRegister::getOwner(const std::string& city, const std::string& address, std::string& owner) const {
    if (city.empty() || address.empty()) {
        return false;
    }

    std::string locCity(city);
    std::string locAddress(address);

    m_Property* searchProperty = new m_Property(locCity, locAddress);


    auto listCityAddressIt = std::lower_bound(sortedByCityAddr.begin(), sortedByCityAddr.end(), searchProperty,
                                              CLandRegister::m_Property::CompareLessThan);

    if (listCityAddressIt != sortedByCityAddr.end() && (*listCityAddressIt)->m_City == searchProperty->m_City
        && (*listCityAddressIt)->m_Address == searchProperty->m_Address) {
        owner = (*listCityAddressIt)->m_Owner;

        delete searchProperty;
        return true;
    }

    delete searchProperty;
    return false;
}

bool CLandRegister::getOwner(const std::string& region, unsigned long long id, std::string& owner) const {
    if (region.empty()) {
        return false;
    }

    std::string locRegion(region);
    unsigned long long locId(id);

    m_Property* searchProperty = new m_Property(locRegion, locId);

    auto listRegionIdIt = std::lower_bound(sortedByRegionId.begin(), sortedByRegionId.end(), searchProperty,
                                        [&](const m_Property* lhs, const m_Property* rhs) {
                                            if (lhs->m_Region == rhs->m_Region)
                                            {
                                                return lhs->m_ID < rhs->m_ID;
                                            }

                                            return lhs->m_Region < rhs->m_Region; });

    if (listRegionIdIt != sortedByRegionId.end() && (*listRegionIdIt)->m_Region == searchProperty->m_Region
        && (*listRegionIdIt)->m_ID == searchProperty->m_ID) {
        owner = (*listRegionIdIt)->m_Owner;

        delete searchProperty;
        return true;
    }

    delete searchProperty;
    return false;
}

bool CLandRegister::newOwner(const std::string& city, const std::string& address, const std::string& owner)
{
    if (city.empty() || address.empty()) {
        return false;
    }

    std::string locCity(city);
    std::string locAddress(address);

    m_Property* searchProperty = new m_Property(locCity, locAddress);

    auto listCityAddressIt = std::lower_bound(sortedByCityAddr.begin(), sortedByCityAddr.end(), searchProperty,
                                              CLandRegister::m_Property::CompareLessThan);

    if (listCityAddressIt != sortedByCityAddr.end() && (*listCityAddressIt)->m_City == searchProperty->m_City
        && (*listCityAddressIt)->m_Address == searchProperty->m_Address && (*listCityAddressIt)->m_Owner != owner) {
        (*listCityAddressIt)->m_Owner = owner;
        (*listCityAddressIt)->m_AcquisitionTimestamp = m_NextAcquisitionOrder++;


        delete searchProperty;
        return true;
    }

    delete searchProperty;
    return false;
}

bool CLandRegister::newOwner(const std::string& region, unsigned long long id, const std::string& owner) {
    if (region.empty()) {
        return false;
    }

    std::string locRegion(region);
    unsigned long long locId(id);

    m_Property* searchProperty = new m_Property(locRegion, locId);

    auto listRegionIdIt = std::lower_bound(sortedByRegionId.begin(), sortedByRegionId.end(), searchProperty,
                                        [&](const m_Property* lhs, const m_Property* rhs) {
                                            if (lhs->m_Region == rhs->m_Region)
                                            {
                                                return lhs->m_ID < rhs->m_ID;
                                            }

                                            return lhs->m_Region < rhs->m_Region; });

    if (listRegionIdIt != sortedByRegionId.end() && (*listRegionIdIt)->m_Region == searchProperty->m_Region
        && (*listRegionIdIt)->m_ID == searchProperty->m_ID && (*listRegionIdIt)->m_Owner != owner) {
        (*listRegionIdIt)->m_Owner = owner;
        (*listRegionIdIt)->m_AcquisitionTimestamp = m_NextAcquisitionOrder++;

        delete searchProperty;
        return true;
    }

    delete searchProperty;
    return false;
}

CIterator CLandRegister::listByAddr() const {
    std::vector<m_Property*> sortedProperties = sortedByCityAddr;
    return CIterator(*this, sortedProperties);
}


CIterator CLandRegister::listByOwner(const std::string& owner) const
{
    std::vector<m_Property*> ownedProperties;

    for (const auto& property : sortedByRegionId) {
        if (strcasecmp(property->m_Owner.c_str(), owner.c_str()) == 0) {
            ownedProperties.push_back(property);
        }
    }

    std::sort(ownedProperties.begin(), ownedProperties.end(),
              [&](const m_Property* lhs, const m_Property* rhs) { return lhs->m_AcquisitionTimestamp < rhs->m_AcquisitionTimestamp ;});

    CIterator iterator(*this, ownedProperties);
    return iterator;
}

size_t CLandRegister::count(const std::string& owner) const
{
    return std::count_if(sortedByCityAddr.begin(), sortedByCityAddr.end(),
                         [&](const m_Property* p) {
                             return strcasecmp(p->m_Owner.c_str(), owner.c_str()) == 0;
                         });
}

bool CIterator::atEnd() const
{
    return currentIndex >= sortedProperties.size();
}

void CIterator::next()
{
    if(!atEnd())
    {
        currentIndex++;
    }
}

std::string CIterator::city() const
{
    std::cout << sortedProperties[currentIndex]->m_City << std::endl;
    return (!atEnd()) ? sortedProperties[currentIndex]->m_City : "";
}

std::string CIterator::addr() const
{
    return (!atEnd()) ? sortedProperties[currentIndex]->m_Address : "";
}

std::string CIterator::owner() const
{
    return (!atEnd()) ? sortedProperties[currentIndex]->m_Owner : "";
}

std::string CIterator::region() const
{
    return (!atEnd()) ? sortedProperties[currentIndex]->m_Region : "";
}

unsigned CIterator::id() const
{
    return (!atEnd()) ? sortedProperties[currentIndex]->m_ID : 0;
}


#ifndef __PROGTEST__
static void test0 ()
{
    CLandRegister x;
    std::string owner;

    assert ( x . add ( "Prague", "Thakurova", "Dejvice", 12345 ) );
    assert ( x . add ( "Prague", "Evropska", "Vokovice", 12345 ) );
    assert ( x . add ( "Prague", "Technicka", "Dejvice", 9873 ) );
    assert ( x . add ( "Plzen", "Evropska", "Plzen mesto", 78901 ) );
    assert ( x . add ( "Liberec", "Evropska", "Librec", 4552 ) );
    CIterator i0 = x . listByAddr ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Liberec"
             && i0 . addr () == "Evropska"
             && i0 . region () == "Librec"
             && i0 . id () == 4552
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Plzen"
             && i0 . addr () == "Evropska"
             && i0 . region () == "Plzen mesto"
             && i0 . id () == 78901
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Evropska"
             && i0 . region () == "Vokovice"
             && i0 . id () == 12345
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Technicka"
             && i0 . region () == "Dejvice"
             && i0 . id () == 9873
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Thakurova"
             && i0 . region () == "Dejvice"
             && i0 . id () == 12345
             && i0 . owner () == "" );
    i0 . next ();
    assert ( i0 . atEnd () );

    assert ( x . count ( "" ) == 5 );
    CIterator i1 = x . listByOwner ( "" );
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Prague"
             && i1 . addr () == "Thakurova"
             && i1 . region () == "Dejvice"
             && i1 . id () == 12345
             && i1 . owner () == "" );
    i1 . next ();
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Prague"
             && i1 . addr () == "Evropska"
             && i1 . region () == "Vokovice"
             && i1 . id () == 12345
             && i1 . owner () == "" );
    i1 . next ();
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Prague"
             && i1 . addr () == "Technicka"
             && i1 . region () == "Dejvice"
             && i1 . id () == 9873
             && i1 . owner () == "" );
    i1 . next ();
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Plzen"
             && i1 . addr () == "Evropska"
             && i1 . region () == "Plzen mesto"
             && i1 . id () == 78901
             && i1 . owner () == "" );
    i1 . next ();
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Liberec"
             && i1 . addr () == "Evropska"
             && i1 . region () == "Librec"
             && i1 . id () == 4552
             && i1 . owner () == "" );
    i1 . next ();
    assert ( i1 . atEnd () );

    assert ( x . count ( "CVUT" ) == 0 );
    CIterator i2 = x . listByOwner ( "CVUT" );
    assert ( i2 . atEnd () );

    assert ( x . newOwner ( "Prague", "Thakurova", "CVUT" ) );
    assert ( x . newOwner ( "Dejvice", 9873, "CVUT" ) );
    assert ( x . newOwner ( "Plzen", "Evropska", "Anton Hrabis" ) );
    assert ( x . newOwner ( "Librec", 4552, "Cvut" ) );
    assert ( x . getOwner ( "Prague", "Thakurova", owner ) && owner == "CVUT" );
    assert ( x . getOwner ( "Dejvice", 12345, owner ) && owner == "CVUT" );
    assert ( x . getOwner ( "Prague", "Evropska", owner ) && owner == "" );
    assert ( x . getOwner ( "Vokovice", 12345, owner ) && owner == "" );
    assert ( x . getOwner ( "Prague", "Technicka", owner ) && owner == "CVUT" );
    assert ( x . getOwner ( "Dejvice", 9873, owner ) && owner == "CVUT" );
    assert ( x . getOwner ( "Plzen", "Evropska", owner ) && owner == "Anton Hrabis" );
    assert ( x . getOwner ( "Plzen mesto", 78901, owner ) && owner == "Anton Hrabis" );
    assert ( x . getOwner ( "Liberec", "Evropska", owner ) && owner == "Cvut" );
    assert ( x . getOwner ( "Librec", 4552, owner ) && owner == "Cvut" );
    CIterator i3 = x . listByAddr ();
    assert ( ! i3 . atEnd ()
             && i3 . city () == "Liberec"
             && i3 . addr () == "Evropska"
             && i3 . region () == "Librec"
             && i3 . id () == 4552
             && i3 . owner () == "Cvut" );
    i3 . next ();
    assert ( ! i3 . atEnd ()
             && i3 . city () == "Plzen"
             && i3 . addr () == "Evropska"
             && i3 . region () == "Plzen mesto"
             && i3 . id () == 78901
             && i3 . owner () == "Anton Hrabis" );
    i3 . next ();
    assert ( ! i3 . atEnd ()
             && i3 . city () == "Prague"
             && i3 . addr () == "Evropska"
             && i3 . region () == "Vokovice"
             && i3 . id () == 12345
             && i3 . owner () == "" );
    i3 . next ();
    assert ( ! i3 . atEnd ()
             && i3 . city () == "Prague"
             && i3 . addr () == "Technicka"
             && i3 . region () == "Dejvice"
             && i3 . id () == 9873
             && i3 . owner () == "CVUT" );
    i3 . next ();
    assert ( ! i3 . atEnd ()
             && i3 . city () == "Prague"
             && i3 . addr () == "Thakurova"
             && i3 . region () == "Dejvice"
             && i3 . id () == 12345
             && i3 . owner () == "CVUT" );
    i3 . next ();
    assert ( i3 . atEnd () );

    assert ( x . count ( "cvut" ) == 3 );
    CIterator i4 = x . listByOwner ( "cVuT" );
    assert ( ! i4 . atEnd ()
             && i4 . city () == "Prague"
             && i4 . addr () == "Thakurova"
             && i4 . region () == "Dejvice"
             && i4 . id () == 12345
             && i4 . owner () == "CVUT" );
    i4 . next ();
    assert ( ! i4 . atEnd ()
             && i4 . city () == "Prague"
             && i4 . addr () == "Technicka"
             && i4 . region () == "Dejvice"
             && i4 . id () == 9873
             && i4 . owner () == "CVUT" );
    i4 . next ();
    assert ( ! i4 . atEnd ()
             && i4 . city () == "Liberec"
             && i4 . addr () == "Evropska"
             && i4 . region () == "Librec"
             && i4 . id () == 4552
             && i4 . owner () == "Cvut" );
    i4 . next ();
    assert ( i4 . atEnd () );

    assert ( x . newOwner ( "Plzen mesto", 78901, "CVut" ) );
    assert ( x . count ( "CVUT" ) == 4 );
    CIterator i5 = x . listByOwner ( "CVUT" );
    assert ( ! i5 . atEnd ()
             && i5 . city () == "Prague"
             && i5 . addr () == "Thakurova"
             && i5 . region () == "Dejvice"
             && i5 . id () == 12345
             && i5 . owner () == "CVUT" );
    i5 . next ();
    assert ( ! i5 . atEnd ()
             && i5 . city () == "Prague"
             && i5 . addr () == "Technicka"
             && i5 . region () == "Dejvice"
             && i5 . id () == 9873
             && i5 . owner () == "CVUT" );
    i5 . next ();
    assert ( ! i5 . atEnd ()
             && i5 . city () == "Liberec"
             && i5 . addr () == "Evropska"
             && i5 . region () == "Librec"
             && i5 . id () == 4552
             && i5 . owner () == "Cvut" );
    i5 . next ();
    assert ( ! i5 . atEnd ()
             && i5 . city () == "Plzen"
             && i5 . addr () == "Evropska"
             && i5 . region () == "Plzen mesto"
             && i5 . id () == 78901
             && i5 . owner () == "CVut" );
    i5 . next ();
    assert ( i5 . atEnd () );

    assert ( x . del ( "Liberec", "Evropska" ) );
    assert ( x . del ( "Plzen mesto", 78901 ) );
    assert ( x . count ( "cvut" ) == 2 );
    CIterator i6 = x . listByOwner ( "cVuT" );
    assert ( ! i6 . atEnd ()
             && i6 . city () == "Prague"
             && i6 . addr () == "Thakurova"
             && i6 . region () == "Dejvice"
             && i6 . id () == 12345
             && i6 . owner () == "CVUT" );
    i6 . next ();
    assert ( ! i6 . atEnd ()
             && i6 . city () == "Prague"
             && i6 . addr () == "Technicka"
             && i6 . region () == "Dejvice"
             && i6 . id () == 9873
             && i6 . owner () == "CVUT" );
    i6 . next ();
    assert ( i6 . atEnd () );

    assert ( x . add ( "Liberec", "Evropska", "Librec", 4552 ) );
}

static void test1 ()
{
    CLandRegister x;
    std::string owner;

    assert ( x . add ( "Prague", "Thakurova", "Dejvice", 12345 ) );
    assert ( x . add ( "Prague", "Evropska", "Vokovice", 12345 ) );
    assert ( x . add ( "Prague", "Technicka", "Dejvice", 9873 ) );
    assert ( ! x . add ( "Prague", "Technicka", "Hradcany", 7344 ) );
    assert ( ! x . add ( "Brno", "Bozetechova", "Dejvice", 9873 ) );
    assert ( !x . getOwner ( "Prague", "THAKUROVA", owner ) );
    assert ( !x . getOwner ( "Hradcany", 7343, owner ) );
    CIterator i0 = x . listByAddr ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Evropska"
             && i0 . region () == "Vokovice"
             && i0 . id () == 12345
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Technicka"
             && i0 . region () == "Dejvice"
             && i0 . id () == 9873
             && i0 . owner () == "" );
    i0 . next ();
    assert ( ! i0 . atEnd ()
             && i0 . city () == "Prague"
             && i0 . addr () == "Thakurova"
             && i0 . region () == "Dejvice"
             && i0 . id () == 12345
             && i0 . owner () == "" );
    i0 . next ();
    assert ( i0 . atEnd () );

    assert ( x . newOwner ( "Prague", "Thakurova", "CVUT" ) );
    assert ( ! x . newOwner ( "Prague", "technicka", "CVUT" ) );
    assert ( ! x . newOwner ( "prague", "Technicka", "CVUT" ) );
    assert ( ! x . newOwner ( "dejvice", 9873, "CVUT" ) );
    assert ( ! x . newOwner ( "Dejvice", 9973, "CVUT" ) );
    assert ( ! x . newOwner ( "Dejvice", 12345, "CVUT" ) );
    assert ( x . count ( "CVUT" ) == 1 );
    CIterator i1 = x . listByOwner ( "CVUT" );
    assert ( ! i1 . atEnd ()
             && i1 . city () == "Prague"
             && i1 . addr () == "Thakurova"
             && i1 . region () == "Dejvice"
             && i1 . id () == 12345
             && i1 . owner () == "CVUT" );
    i1 . next ();
    assert ( i1 . atEnd () );

    assert ( ! x . del ( "Brno", "Technicka" ) );
    assert ( ! x . del ( "Karlin", 9873 ) );
    assert ( x . del ( "Prague", "Technicka" ) );
    assert ( ! x . del ( "Prague", "Technicka" ) );
    assert ( ! x . del ( "Dejvice", 9873 ) );
    assert (x . add ("Tokyo", "Nagana", "Tokyo City", 12020203993));
}

int main ( void )
{
    test0 ();
    test1 ();
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */