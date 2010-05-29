// -*- C++ -*-
#ifndef _Parser_h_
#define _Parser_h_

#define PHOENIX_LIMIT 12
#define BOOST_SPIRIT_CLOSURE_LIMIT PHOENIX_LIMIT

#include "Enums.h"
#include "ShipDesign.h"
#include "Tech.h"
#include "../Empire/Empire.h"

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_attribute.hpp>
#include <boost/spirit/include/phoenix1.hpp>
#include <boost/tuple/tuple.hpp>

#include <stdexcept>
#include <set>
#include <string>

#include <GG/Clr.h>

////////////////////////////////////////////////////////////
// Forward Declarations                                   //
////////////////////////////////////////////////////////////
namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
    class EffectBase;
}
class Special;
class Species;
class FocusType;
class BuildingType;
class Tech;
class ShipDesign;
struct TechCategory;
struct FleetPlan;

////////////////////////////////////////////////////////////
// Scanner                                                //
////////////////////////////////////////////////////////////
struct Skip : boost::spirit::classic::grammar<Skip>
{
    template <class ScannerT>
    struct definition
    {
        definition(const Skip&)
        {
            using namespace boost::spirit::classic;
            skip = space_p | comment_p("//") | comment_p("/*", "*/");
        }
        boost::spirit::classic::rule<ScannerT> skip;
        const boost::spirit::classic::rule<ScannerT>& start() const {return skip;}
    };
};
extern const Skip skip_p;

typedef boost::spirit::classic::scanner<const char*, boost::spirit::classic::scanner_policies<boost::spirit::classic::skip_parser_iteration_policy<Skip> > > ScannerBase;
typedef boost::spirit::classic::as_lower_scanner<ScannerBase>::type Scanner;

struct NameClosure : boost::spirit::classic::closure<NameClosure, std::string>
{
    member1 this_;
};

struct ColourClosure : boost::spirit::classic::closure<ColourClosure, GG::Clr, unsigned int, unsigned int, unsigned int, unsigned int>
{
    member1 this_;
    member2 r;
    member3 g;
    member4 b;
    member5 a;
};

////////////////////////////////////////////////////////////
// Condition Parser                                       //
////////////////////////////////////////////////////////////
struct ConditionClosure : boost::spirit::classic::closure<ConditionClosure, Condition::ConditionBase*>
{
    member1 this_;
};

extern boost::spirit::classic::rule<Scanner, ConditionClosure::context_t> condition_p;


////////////////////////////////////////////////////////////
// Effect Parser                                          //
////////////////////////////////////////////////////////////
struct EffectClosure : boost::spirit::classic::closure<EffectClosure, Effect::EffectBase*>
{
    member1 this_;
};

extern boost::spirit::classic::rule<Scanner, EffectClosure::context_t> effect_p;


////////////////////////////////////////////////////////////
// Top Level Parsers                                      //
////////////////////////////////////////////////////////////
struct BuildingTypeClosure : boost::spirit::classic::closure<BuildingTypeClosure, BuildingType*, std::string,
                                                             std::string, double, int, double, Condition::ConditionBase*,
                                                             std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                             std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 build_cost;
    member5 build_time;
    member6 maintenance_cost;
    member7 location;
    member8 effects_groups;
    member9 graphic;
};

struct SpecialClosure : boost::spirit::classic::closure<SpecialClosure, Special*, std::string, std::string,
                                                        std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                        std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 effects_groups;
    member5 graphic;
};

struct SpeciesClosure : boost::spirit::classic::closure<SpeciesClosure, Species*, std::string, std::string,
                                                        std::vector<FocusType>,
                                                        std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                        std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 foci;
    member5 effects_groups;
    member6 graphic;
};

struct CategoryClosure : boost::spirit::classic::closure<CategoryClosure, TechCategory*, std::string, std::string, GG::Clr>
{
    member1 this_;
    member2 name;
    member3 graphic;
    member4 colour;
};

struct TechClosure : boost::spirit::classic::closure<TechClosure, Tech*, std::string, std::string, std::string,
                                                     std::string, TechType, double, int,
                                                     std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                     std::set<std::string>, std::vector<ItemSpec>, std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 short_description;
    member5 category;
    member6 tech_type;
    member7 research_cost;
    member8 research_turns;
    member9 effects_groups;
    member10 prerequisites;
    member11 unlocked_items;
    member12 graphic;
};

struct ItemSpecClosure : boost::spirit::classic::closure<ItemSpecClosure, ItemSpec, UnlockableItemType, std::string>
{
    member1 this_;
    member2 type;
    member3 name;
};

struct PartStatsClosure : boost::spirit::classic::closure<PartStatsClosure, PartTypeStats, double, double,
                                                          double, double, double, double, CombatFighterType,
                                                          double, double, double, int>
{
    member1 this_;
    member2 damage;
    member3 rate;
    member4 range;
    member5 speed;
    member6 stealth;
    member7 structure;
    member8 fighter_type;
    member9 anti_ship_damage;
    member10 anti_fighter_damage;
    member11 detection;
    member12 capacity;
};

struct PartClosure : boost::spirit::classic::closure<PartClosure, PartType*, std::string, std::string, ShipPartClass,
                                                     PartTypeStats, double, int, std::vector<ShipSlotType>,
                                                     Condition::ConditionBase*,
                                                     std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                     std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 part_class;
    member5 stats;
    member6 cost;
    member7 build_time;
    member8 mountable_slot_types;
    member9 location;
    member10 effects_groups;
    member11 graphic;
};

struct HullStatsClosure : boost::spirit::classic::closure<HullStatsClosure, HullTypeStats, double,
                                                          double, double, double, double>
{
    member1 this_;
    member2 battle_speed;
    member3 starlane_speed;
    member4 fuel;
    member5 stealth;
    member6 structure;
};

struct HullClosure : boost::spirit::classic::closure<HullClosure, HullType*, std::string, std::string,
                                                     HullTypeStats, double, int, std::vector<HullType::Slot>,
                                                     Condition::ConditionBase*,
                                                     std::vector<boost::shared_ptr<const Effect::EffectsGroup> >,
                                                     std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 stats;
    member5 cost;
    member6 build_time;
    member7 slots;
    member8 location;
    member9 effects_groups;
    member10 graphic;
};

struct ShipDesignClosure : boost::spirit::classic::closure<ShipDesignClosure, ShipDesign*, std::string, std::string,
                                                           std::string, std::vector<std::string>,
                                                           std::string, std::string, bool>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 hull;
    member5 parts;
    member6 graphic;
    member7 model;
    member8 name_desc_in_stringtable;
};

struct FleetPlanClosure : boost::spirit::classic::closure<FleetPlanClosure, FleetPlan, std::string,
                                                          std::vector<std::string> >
{
    member1 this_;
    member2 name;
    member3 ship_designs;
};

extern boost::spirit::classic::rule<Scanner, BuildingTypeClosure::context_t> building_type_p;
extern boost::spirit::classic::rule<Scanner, SpecialClosure::context_t>      special_p;
extern boost::spirit::classic::rule<Scanner, SpeciesClosure::context_t>      species_p;
extern boost::spirit::classic::rule<Scanner, CategoryClosure::context_t>     category_p;
extern boost::spirit::classic::rule<Scanner, TechClosure::context_t>         tech_p;
extern boost::spirit::classic::rule<Scanner, ItemSpecClosure::context_t>     item_spec_p;
extern boost::spirit::classic::rule<Scanner, PartStatsClosure::context_t>    part_stats_p;
extern boost::spirit::classic::rule<Scanner, PartClosure::context_t>         part_p;
extern boost::spirit::classic::rule<Scanner, HullClosure::context_t>         hull_p;
extern boost::spirit::classic::rule<Scanner, ShipDesignClosure::context_t>   ship_design_p;
extern boost::spirit::classic::rule<Scanner, FleetPlanClosure::context_t>    fleet_plan_p;

#endif // _Parser_h_
