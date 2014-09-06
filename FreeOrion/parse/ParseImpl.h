// -*- C++ -*-
#ifndef _ParseImpl_h_
#define _ParseImpl_h_

#include "ReportParseError.h"
#include "../universe/Tech.h"

#include <boost/filesystem/path.hpp>
#include <boost/spirit/include/qi.hpp>

#include <GG/Clr.h>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        void (std::set<std::string>&),
        parse::skipper_type
    > tags_rule;
    tags_rule& tags_parser();


    typedef boost::spirit::qi::rule<
        token_iterator,
        std::vector<boost::shared_ptr<const Effect::EffectsGroup> > (),
        skipper_type
    > effects_group_rule;
    effects_group_rule& effects_group_parser();


    typedef boost::spirit::qi::rule<
        token_iterator,
        GG::Clr (),
        qi::locals<
            unsigned int,
            unsigned int,
            unsigned int
        >,
        skipper_type
    > color_parser_rule;
    color_parser_rule& color_parser();


    typedef boost::spirit::qi::rule<
        token_iterator,
        ItemSpec (),
        qi::locals<UnlockableItemType>,
        skipper_type
    > item_spec_parser_rule;
    item_spec_parser_rule& item_spec_parser();


    void parse_file_common(const boost::filesystem::path& path,
                           const lexer& l,
                           std::string& filename,
                           std::string& file_contents,
                           text_iterator& first,
                           token_iterator& it);

    template <typename Rules, typename Arg1>
    bool parse_file(const boost::filesystem::path& path, Arg1& arg1)
    {
        std::string filename;
        std::string file_contents;
        text_iterator first;
        token_iterator it;

        const lexer& l = lexer::instance();

        parse_file_common(path, l, filename, file_contents, first, it);

        boost::spirit::qi::in_state_type in_state;

        static Rules rules;

        bool success = boost::spirit::qi::phrase_parse(it, l.end(), rules.start(boost::phoenix::ref(arg1)), in_state("WS")[l.self]);

        std::ptrdiff_t distance = std::distance(first, parse::detail::s_end);

        return success && (!distance || distance == 1 && *first == '\n');
    }
} }

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
    namespace lib {
        void init();

        bool buildings(const boost::filesystem::path& path,
            std::map<std::string,
            BuildingType*>& building_types);

        bool fields(const boost::filesystem::path& path,
            std::map<std::string, FieldType*>& field_types);

        bool specials(const boost::filesystem::path& path,
            std::map<std::string, Special*>& specials_);

        bool species(const boost::filesystem::path& path,
            std::map<std::string, Species*>& species_);

        bool techs(const boost::filesystem::path& path,
            TechManager::TechContainer& techs_,
            std::map<std::string, TechCategory*>& tech_categories,
            std::set<std::string>& categories_seen);

        bool items(const boost::filesystem::path& path,
            std::vector<ItemSpec>& items_);

        bool ship_parts(const boost::filesystem::path& path,
            std::map<std::string, PartType*>& parts);

        bool ship_hulls(const boost::filesystem::path& path,
            std::map<std::string, HullType*>& hulls);

        bool ship_designs(const boost::filesystem::path& path,
            std::map<std::string, ShipDesign*>& designs);

        bool fleet_plans(const boost::filesystem::path& path,
            std::vector<FleetPlan*>& fleet_plans_);

        bool monster_fleet_plans(const boost::filesystem::path& path,
            std::vector<MonsterFleetPlan*>& monster_fleet_plans_);

        bool alignments(const boost::filesystem::path& path,
            std::vector<Alignment>& alignments_,
            std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups);

        bool statistics(const boost::filesystem::path& path,
            std::map<std::string, const ValueRef::ValueRefBase<double>*>& stats_);

        bool encyclopedia_articles(const boost::filesystem::path& path,
            Encyclopedia& enc);

        bool keymaps(const boost::filesystem::path& path,
            std::map<std::string, std::map<int, int> >& nkm);
    }
}

#endif
