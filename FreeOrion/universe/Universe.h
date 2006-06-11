// -*- C++ -*-
#ifndef _Universe_h_
#define _Universe_h_

#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
#include <boost/serialization/shared_ptr.hpp>
#endif

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

#ifndef _Enums_h_
#include "Enums.h"
#endif

#ifndef _Predicates_h_
#include "Predicates.h"
#endif

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif
#ifndef BOOST_GRAPH_ADJACENCY_LIST_HPP
#include <boost/graph/adjacency_list.hpp>
#endif
#ifndef BOOST_FILTERED_GRAPH_HPP
#include <boost/graph/filtered_graph.hpp>
#endif

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <set>

class System;

#ifdef __GNUC__
  // GCC doesn't allow us to forward-declare PlayerSetupData
#  ifndef _MultiplayerCommon_h_
#    include "../util/MultiplayerCommon.h"
#  endif
#else
  struct PlayerSetupData;
#endif

class Empire;
struct UniverseObjectVisitor;
class XMLElement;

class Universe
{
protected:
    typedef std::map<int, UniverseObject*> ObjectMap;           ///< container type used to hold the objects in the universe; keyed by ID number
    typedef std::map<int, std::set<int> > ObjectKnowledgeMap;   ///< container type used to hold sets of IDs of Empires which known information about an object (or deleted object); keyed by object ID number

public:
    /** the types of universe shapes available in FreeOrion*/
    enum Shape {
        SPIRAL_2,      ///< a two-armed spiral galaxy
        SPIRAL_3,      ///< a three-armed spiral galaxy
        SPIRAL_4,      ///< a four-armed spiral galaxy
        CLUSTER,       ///< a cluster galaxy
        ELLIPTICAL,    ///< an elliptical galaxy
        IRREGULAR,     ///< an irregular galaxy
        RING,          ///< a ring galaxy
        GALAXY_SHAPES  ///< the number of shapes in this enum (leave this last)
    };

    /** types of Univervse ages*/
    enum Age {
        AGE_YOUNG,
        AGE_MATURE,
        AGE_ANCIENT,
        NUM_UNIVERSE_AGES    // keep this last, the number of universe age options
    };

    /** types of Planet Density */
    enum PlanetDensity {
        PD_LOW,
        PD_AVERAGE,
        PD_HIGH,
        NUM_UNIVERSE_PLANET_DENSITIES        //keep this last, the number of planet density options
    };

    /** types of starlane frequencies */
    enum StarlaneFrequency{
        LANES_NONE, 
        LANES_FEW, 
        LANES_SOME, 
        LANES_SEVERAL, 
        LANES_MANY, 
        LANES_VERY_MANY,
        NUM_STARLANE_FREQENCIES    // keep this last, the number of starlane frequency options
    };

    /** types of starlane frequencies */
    enum SpecialsFrequency{
        SPECIALS_NONE, 
        SPECIALS_RARE, 
        SPECIALS_UNCOMMON, 
        SPECIALS_COMMON, 
        NUM_SPECIALS_FREQENCIES    // keep this last, the number of specials frequency options
    };

    /** Set to true to make everything visible for everyone. Useful for debugging. */
    static const bool ALL_OBJECTS_VISIBLE;

    struct vertex_system_pointer_t {typedef boost::vertex_property_tag kind;}; ///< a system graph property map type
    typedef boost::property<vertex_system_pointer_t, System*,
                            boost::property<boost::vertex_index_t, int> > 
    vertex_property_t; ///< a system graph property map type
    typedef boost::property<boost::edge_weight_t, double> 
    edge_property_t; ///< a system graph property map type

    typedef ObjectMap::const_iterator            const_iterator;   ///< a const_iterator for sequences over the objects in the universe
    typedef ObjectMap::iterator                  iterator;         ///< an iterator for sequences over the objects in the universe

    typedef std::vector<const UniverseObject*>   ConstObjectVec;   ///< the return type of FindObjects()
    typedef std::vector<UniverseObject*>         ObjectVec;        ///< the return type of the non-const FindObjects()
    typedef std::vector<int>                     ObjectIDVec;      ///< the return type of FindObjectIDs()

    /** \name Signal Types */ //@{
    typedef boost::signal<void (const UniverseObject *)> UniverseObjectDeleteSignalType; ///< emitted just before the UniverseObject is deleted
    //@}


    /** \name Structors */ //@{
    Universe(); ///< default ctor
    Universe(const XMLElement& elem); ///< ctor that constructs a Universe object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Universe object
    const Universe& operator=(Universe& rhs); ///< assignment operator (move semantics)
    virtual ~Universe(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    const UniverseObject* Object(int id) const; ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists
    UniverseObject* Object(int id);  ///< returns a pointer to the universe object with ID number \a id, or 0 if none exists

    template <class T> const T* Object(int id) const; ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.
    template <class T> T* Object(int id);  ///< returns a pointer to the object of type T with ID number \a id. Returns 0 if none exists or the object with ID \a id is not of type T.

    /** returns all the objects that match \a visitor */
    ConstObjectVec FindObjects(const UniverseObjectVisitor& visitor) const;

    /** returns all the objects that match \a visitor */
    ObjectVec FindObjects(const UniverseObjectVisitor& visitor);

    /** returns all the objects of type T. */
    template <class T> std::vector<const T*> FindObjects() const;

    /** returns all the objects of type T. */
    template <class T> std::vector<T*> FindObjects();

    /** returns the IDs of all the objects that match \a visitor */
    ObjectIDVec FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** returns the IDs of all the objects of type T. */
    template <class T> ObjectIDVec FindObjectIDs() const;

    iterator begin() { return m_objects.begin();}
    iterator end() { return m_objects.end();}

    const_iterator begin() const  {return m_objects.begin();}   ///< returns the begin const_iterator for the objects in the universe
    const_iterator end() const    {return m_objects.end();}     ///< returns the end const_iterator for the objects in the universe


    const UniverseObject* DestroyedObject(int id) const;          ///< returns a pointer to the destroyed universe object with ID number \a id, or 0 if none exists
    
    const_iterator beginDestroyed() const  {return m_destroyed_objects.begin();}   ///< returns the begin const_iterator for the destroyed objects from the universe
    const_iterator endDestroyed() const    {return m_destroyed_objects.end();}     ///< returns the end const_iterator for the destroyed objects from the universe


    double LinearDistance(int system1, int system2) const; ///< returns the straight-line distance between the systems with the given IDs. \throw std::out_of_range This function will throw if either system ID is out of range.

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the shortest path from \a
        system1 to \a system2, and the distance travelled to get there.  If no such path exists, the list will be empty.
        Note that the path returned may be via one or more starlane, or may be "offroad".  The path is calculated using
        the visiblity for empire \a empire_id, or without regard to visibility if \a empire_id == ALL_EMPIRES.  \throw
        std::out_of_range This function will throw if either system ID is out of range. */
    std::pair<std::list<System*>, double> ShortestPath(int system1, int system2, int empire_id = ALL_EMPIRES) const;

    /** returns the sequence of systems, including \a system1 and \a system2, that defines the path with the fewest
        jumps from \a system1 to \a system2, and the number of jumps to get there.  If no such path exists, the list
        will be empty.  The path is calculated using the visiblity for empire \a empire_id, or without regard to
        visibility if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function will throw if either system
        ID is out of range. */
    std::pair<std::list<System*>, int> LeastJumpsPath(int system1, int system2, int empire_id = ALL_EMPIRES) const;

    /** returns the systems that are one starlane hop away from system \a system.  The returned systems are indexed by
        distance from \a system.  The neighborhood is calculated using the visiblity for empire \a empire_id, or without
        regard to visibility if \a empire_id == ALL_EMPIRES.  \throw std::out_of_range This function will throw if the
        system ID is out of range. */
    std::map<double, System*> ImmediateNeighbors(int system, int empire_id = ALL_EMPIRES) const;

    virtual XMLElement XMLEncode(int empire_id = ALL_EMPIRES) const; ///< constructs an XMLElement from a Universe object with visibility restrictions for the given empire

    mutable UniverseObjectDeleteSignalType UniverseObjectDeleteSignal; ///< the state changed signal object for this UniverseObject
    //@}

    /** \name Mutators */ //@{
    void SetUniverse(const XMLElement& elem); ///< wipes out the current object map and sets the map to the XMLElement passed in.

    /** inserts object \a obj into the universe; returns the ID number assigned to the object, or -1 on failure.
        \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
        passing it to Insert().*/
    int               Insert(UniverseObject* obj);

    /** inserts object \a obj of given ID into the universe; returns true on proper insert , or false on failure.
        \note Universe gains ownership of \a obj once it is inserted; the caller should \a never delete \a obj after
        passing it to InsertID().
        Useful mostly for times when ID needs to be consistant on client and server*/
    bool              InsertID(UniverseObject* obj, int id );

    /** generates systems and planets, assigns homeworlds and populates them with people, industry and bases, and places starting fleets.  Uses predefined galaxy shapes.  */
    void              CreateUniverse(int size, Shape shape, Age age, StarlaneFrequency starlane_freq, PlanetDensity planet_density, 
                                     SpecialsFrequency specials_freq, int players, int ai_players, 
                                     const std::vector<PlayerSetupData>& player_setup_data = std::vector<PlayerSetupData>());

    /** Applies all Effects from Buildings, Specials, Techs, etc. */
    void              ApplyEffects();

    /** Reconstructs the per-empire system graph views needed to calculate routes based on visibility. */
    void              RebuildEmpireViewSystemGraphs();

    /** removes the object with ID number \a id from the universe's map of existing objects and places it into the map of destroyed objects.
        removes the object from any containing UniverseObjects, though leaves the object's own records of what contained it intact, so that
        this information may be retained for later reference */
    void              Destroy(int id);

    /** removes from the universe (whether existing or destroyed) and deletes the object with ID number \a id; returns true if such an object was found, false otherwise*/
    bool              Delete(int id);

    /** marks an object for destruction by the Destroy effect. */
    void              EffectDestroy(int id);
    //@}

    /** returns the size of the galaxy map.  Does not measure absolute distances; the ratio between map coordinates and actual distance varies
        depending on universe size */
    static double UniverseWidth() {return s_universe_width;}

    int GenerateObjectID( );  ///< generates an object ID for a future object. Usually used by the server to service new ID requests

    typedef std::vector<std::vector<std::set<System*> > > AdjacencyGrid;

    static const bool& InhibitUniverseObjectSignals() {return s_inhibit_universe_object_signals;}

    /** HACK! This must be set to the encoding empire's id when serializing a Universe, so that only the relevant parts
        of the Universe are serialized.  The use of this global variable is done just so I don't have to rewrite any
        custom boost::serialization classes that implement empire-dependent visibility. */
    static int s_encoding_empire;

protected:
    typedef std::vector< std::vector<double> > DistanceMatrix;

    // declare main graph types, including properties declared above
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, 
                                  vertex_property_t, edge_property_t>
    SystemGraph;

    // declare types for iteration over graph
    typedef SystemGraph::vertex_iterator   VertexIterator;
    typedef SystemGraph::out_edge_iterator OutEdgeIterator;

    struct EdgeVisibilityFilter
    {
        EdgeVisibilityFilter();
        EdgeVisibilityFilter(const SystemGraph* graph, int empire_id);
        template <typename EdgeDescriptor>
        bool operator()(const EdgeDescriptor& edge) const;
        static bool CanSeeAtLeastOneSystem(const Empire* empire, int system1, int system2);
    private:
        const SystemGraph* m_graph;
        const Empire* m_empire;
    };
    typedef boost::filtered_graph<SystemGraph, EdgeVisibilityFilter> EmpireViewSystemGraph;
    typedef std::map<int, boost::shared_ptr<EmpireViewSystemGraph> > EmpireViewSystemGraphMap;

    // declare property map types for properties declared above
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::const_type ConstSystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, vertex_system_pointer_t>::type       SystemPointerPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::const_type   ConstIndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::vertex_index_t>::type         IndexPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::const_type    ConstEdgeWeightPropertyMap;
    typedef boost::property_map<SystemGraph, boost::edge_weight_t>::type          EdgeWeightPropertyMap;

    // factory class
    template <class T> class NumberedElementFactory
    {
    public:
        typedef T* (*Generator)(const XMLElement&); ///< this defines the function signature for object generators

        /** \name Structors */ //@{
        NumberedElementFactory(); ///< ctor
        //@}

        /** \name Accessors */ //@{
        /** Generates objects whose tag name contains but is not limited to the generator name  */
        T* GenerateObject(const XMLElement& elem) const;
        //@}

        /** \name Mutators */ //@{
        /** adds (or overrides) a new generator that can generate subclass objects described by \a name */
        void AddGenerator(const std::string& name, Generator gen);
        //@}

    private:
        /** mapping from strings to functions that can create the type of object that corresponds to the string */
        std::map<std::string, Generator> m_generators;
    };

    void GenerateIrregularGalaxy(int stars, Age age, AdjacencyGrid& adjacency_grid);   ///< creates an irregular galaxy and stores the empire homeworlds in the homeworlds vector

    void PopulateSystems(PlanetDensity density, SpecialsFrequency specials_freq);  ///< Will generate planets for all systems that have empty object maps (ie those that aren't homeworld systems)
    
    void GenerateStarlanes(StarlaneFrequency freq, const AdjacencyGrid& adjacency_grid); ///< creates starlanes and adds them systems already generated
    bool ConnectedWithin(int system1, int system2, int maxLaneJumps, std::vector<std::set<int> >& laneSetArray); // used by GenerateStarlanes.  Determines if two systems are connected by maxLaneJumps or less edges on graph
    void CullAngularlyTooCloseLanes(double maxLaneUVectDotProd, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems); // Removes lanes from passed graph that are angularly too close to eachother
    void CullTooLongLanes(double maxLaneLength, std::vector<std::set<int> >& laneSetArray, std::vector<System*> &systems); // Removes lanes from passed graph that are too long
    void GrowSpanningTrees(std::vector<int> roots, std::vector<std::set<int> >& potentialLaneSetArray, std::vector<std::set<int> >& laneSetArray); // grows trees to connect stars...  takes an array of sets of potential starlanes for each star, and puts the starlanes of the tree into another set
    
    void InitializeSystemGraph(); ///< resizes the system graph to the appropriate size and populates m_system_distances 
    void GenerateHomeworlds(int players, std::vector<int>& homeworlds);  ///< Picks systems to host homeworlds, generates planets for them, stores the ID's of the homeworld planets into the homeworld vector

    /// Will create empire objects, assign them homeworlds, setup the homeworld population, industry, and starting fleets
    /// NOTE: does nothing if executed client-side. This is a hack to deal with the
    /// dependency on ServerEmpireManager -- jdb
    void GenerateEmpires(int players, std::vector<int>& homeworlds, const std::vector<PlayerSetupData>& player_setup_data);

    void DestroyImpl(int id);

    ObjectMap m_objects;                                    ///< note that for the system graph algorithms to work more easily, the first N elements should be the N systems
    ObjectMap m_destroyed_objects;                          ///< objects that have been destroyed from the universe.  for the server: all of them;  for clients, only those that the local client knows about, not including previously-seen objects that the client no longer can see
    ObjectKnowledgeMap m_destroyed_object_knowers;          ///< keyed by (destroyed) object ID, map of sets of Empires' IDs that know the objects have been destroyed (ie. could see the object when it was destroyed)
    DistanceMatrix m_system_distances;                      ///< the straight-line distances between all the systems; this is an lower-triangular matrix, so only access the elements in (highID, lowID) order
    SystemGraph m_system_graph;                             ///< a graph in which the systems are vertices and the starlanes are edges
    EmpireViewSystemGraphMap m_empire_system_graph_views;   ///< a map of empire IDs to the views of the system graph by those empires
    NumberedElementFactory<UniverseObject> m_factory;       ///< generates new object IDs for all new objects
    int m_last_allocated_id;
    std::set<int> m_marked_destroyed;

    static double s_universe_width;

private:
    static bool s_inhibit_universe_object_signals;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
#if (10 * __GNUC__ + __GNUC_MINOR__ > 33) && (!defined _UniverseObject_h_)
#  include "UniverseObject.h"
#endif

template <class Archive>
void Universe::serialize(Archive& ar, const unsigned int version)
{
    ObjectMap objects;
    if (Archive::is_saving::value) {
        for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
            if (Universe::ALL_OBJECTS_VISIBLE ||
                it->second->GetVisibility(s_encoding_empire) != UniverseObject::NO_VISIBILITY ||
                universe_object_cast<System*>(it->second))
            {
                objects.insert(*it);
            }
        }
    }
    ar  & BOOST_SERIALIZATION_NVP(s_universe_width)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(m_last_allocated_id);
    if (Archive::is_loading::value)
        m_objects = objects;
}

template <class T> 
const T* Universe::Object(int id) const
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T> 
T* Universe::Object(int id)
{
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T>
std::vector<const T*> Universe::FindObjects() const
{
    std::vector<const T*> retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (const T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<T*> Universe::FindObjects()
{
    std::vector<T*> retval;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
Universe::ObjectIDVec Universe::FindObjectIDs() const
{
    Universe::ObjectIDVec retval;
    for (ObjectMap::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(it->first);
    }
    return retval;
}


template <typename EdgeDescriptor>
bool Universe::EdgeVisibilityFilter::operator()(const EdgeDescriptor& edge) const
{
    return m_empire && m_graph ? CanSeeAtLeastOneSystem(m_empire, boost::source(edge, *m_graph), boost::target(edge, *m_graph)) : false;
}

template <class T>
Universe::NumberedElementFactory<T>::NumberedElementFactory()
{}

template <class T>
T* Universe::NumberedElementFactory<T>::GenerateObject(const XMLElement& elem) const ///< returns a heap-allocated subclass object of the appropriate type
{
    T* retval = 0;

    for (typename std::map<std::string, Generator>::const_iterator it = m_generators.begin(); it != m_generators.end(); ++it) {
        // is the object type name a prefix of the tag?
        std::string tag_name = elem.Tag();
        std::string tag_alpha = tag_name.substr(0, tag_name.find_first_of("0123456789"));

        if (tag_alpha == it->first) {
            retval = it->second(elem);
            break;
        }
    }
    return retval;
}

template <class T>
void Universe::NumberedElementFactory<T>::AddGenerator(const std::string& name, Generator gen)
{
    m_generators[name] = gen;
}

#endif // _Universe_h_
