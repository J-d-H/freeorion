// -*- C++ -*-
#ifndef _Parse_h_
#define _Parse_h_

#include "../universe/Tech.h"
#include "../util/Export.h"

#include <boost/filesystem/path.hpp>


class Alignment;
class BuildingType;
class FieldType;
class FleetPlan;
class HullType;
class MonsterFleetPlan;
class PartType;
class ShipDesign;
class Special;
class Species;
struct Encyclopedia;

namespace parse {
    FO_COMMON_API void init();

    FO_COMMON_API bool buildings(const boost::filesystem::path& path,
                                std::map<std::string,
                                BuildingType*>& building_types);

    FO_COMMON_API bool fields(const boost::filesystem::path& path,
                             std::map<std::string, FieldType*>& field_types);

    FO_COMMON_API bool specials(const boost::filesystem::path& path,
                               std::map<std::string, Special*>& specials_);

    FO_COMMON_API bool species(const boost::filesystem::path& path,
                              std::map<std::string, Species*>& species_);

    FO_COMMON_API bool techs(const boost::filesystem::path& path,
                            TechManager::TechContainer& techs_,
                            std::map<std::string, TechCategory*>& tech_categories,
                            std::set<std::string>& categories_seen);

    FO_COMMON_API bool items(const boost::filesystem::path& path,
                            std::vector<ItemSpec>& items_);

    FO_COMMON_API bool ship_parts(const boost::filesystem::path& path,
                                 std::map<std::string, PartType*>& parts);

    FO_COMMON_API bool ship_hulls(const boost::filesystem::path& path,
                                 std::map<std::string, HullType*>& hulls);

    FO_COMMON_API bool ship_designs(const boost::filesystem::path& path,
                                   std::map<std::string, ShipDesign*>& designs);

    FO_COMMON_API bool fleet_plans(const boost::filesystem::path& path,
                                  std::vector<FleetPlan*>& fleet_plans_);

    FO_COMMON_API bool monster_fleet_plans(const boost::filesystem::path& path,
                                          std::vector<MonsterFleetPlan*>& monster_fleet_plans_);

    FO_COMMON_API bool alignments(const boost::filesystem::path& path,
                                 std::vector<Alignment>& alignments_,
                                 std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups);

    FO_COMMON_API bool statistics(const boost::filesystem::path& path,
                                 std::map<std::string, const ValueRef::ValueRefBase<double>*>& stats_);

    FO_COMMON_API bool encyclopedia_articles(const boost::filesystem::path& path,
                                            Encyclopedia& enc);

    FO_COMMON_API bool keymaps(const boost::filesystem::path& path,
                              std::map<std::string, std::map<int, int> >& nkm);
}

#endif
