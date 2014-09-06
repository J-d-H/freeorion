#include "Parse.h"
#include "ParseImpl.h"

namespace parse {
    FO_COMMON_API void init() { lib::init(); }

    FO_COMMON_API bool buildings(const boost::filesystem::path& path,
                                std::map<std::string, BuildingType*>& building_types)
    {
        return lib::buildings(path, building_types);
    }

    FO_COMMON_API bool fields(const boost::filesystem::path& path,
                             std::map<std::string, FieldType*>& field_types)
    {
        return lib::fields(path, field_types);
    }

    FO_COMMON_API bool specials(const boost::filesystem::path& path,
                               std::map<std::string, Special*>& specials_)
    {
        return lib::specials(path, specials_);
    }

    FO_COMMON_API bool species(const boost::filesystem::path& path,
                              std::map<std::string, Species*>& species_)
    {
        return lib::species(path, species_);
    }

    FO_COMMON_API bool techs(const boost::filesystem::path& path,
                            TechManager::TechContainer& techs_,
                            std::map<std::string, TechCategory*>& tech_categories,
                            std::set<std::string>& categories_seen)
    {
        return lib::techs(path, techs_, tech_categories, categories_seen);
    }

    FO_COMMON_API bool items(const boost::filesystem::path& path,
                            std::vector<ItemSpec>& items_)
    {
        return lib::items(path, items_);
    }

    FO_COMMON_API bool ship_parts(const boost::filesystem::path& path,
                                 std::map<std::string, PartType*>& parts)
    {
        return lib::ship_parts(path, parts);
    }

    FO_COMMON_API bool ship_hulls(const boost::filesystem::path& path,
                                 std::map<std::string, HullType*>& hulls)
    {
        return lib::ship_hulls(path, hulls);
    }

    FO_COMMON_API bool ship_designs(const boost::filesystem::path& path,
                                   std::map<std::string, ShipDesign*>& designs)
    {
        return lib::ship_designs(path, designs);
    }

    FO_COMMON_API bool fleet_plans(const boost::filesystem::path& path,
                                  std::vector<FleetPlan*>& fleet_plans_)
    {
        return lib::fleet_plans(path, fleet_plans_);
    }

    FO_COMMON_API bool monster_fleet_plans(const boost::filesystem::path& path,
                                          std::vector<MonsterFleetPlan*>& monster_fleet_plans_)
    {
        return lib::monster_fleet_plans(path, monster_fleet_plans_);
    }

    FO_COMMON_API bool alignments(const boost::filesystem::path& path,
                                 std::vector<Alignment>& alignments_,
                                 std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups)
    {
        return lib::alignments(path, alignments_, effects_groups);
    }

    FO_COMMON_API bool statistics(const boost::filesystem::path& path,
                                 std::map<std::string, const ValueRef::ValueRefBase<double>*>& stats_)
    {
        return lib::statistics(path, stats_);
    }

    FO_COMMON_API bool encyclopedia_articles(const boost::filesystem::path& path,
                                            Encyclopedia& enc)
    {
        return lib::encyclopedia_articles(path, enc);
    }

    FO_COMMON_API bool keymaps(const boost::filesystem::path& path,
                              std::map<std::string, std::map<int, int> >& nkm)
    {
        return lib::keymaps(path, nkm);
    }
}