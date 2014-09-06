// -*- C++ -*-
#ifndef _ValueRef_h_
#define _ValueRef_h_

#include "Condition.h"
#include "../util/Export.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>

#include <map>
#include <set>

class UniverseObject;

FO_COMMON_API std::string DoubleToString(double val, int digits, bool always_show_sign);
FO_COMMON_API const std::string& UserString(const std::string& str);
FO_COMMON_API boost::format FlexibleFormat(const std::string& string_to_format);
FO_COMMON_API std::string FormatedDescriptionPropertyNames(ValueRef::ReferenceType ref_type
                                                           , const std::vector<std::string>& property_names);

namespace ValueRef {
    FO_COMMON_API std::string ReconstructName(const std::vector<std::string>& property_name
                                              , ValueRef::ReferenceType ref_type);

    FO_COMMON_API MeterType     NameToMeter(std::string name);
    FO_COMMON_API std::string   MeterToName(MeterType meter);

    FO_COMMON_API std::string   ReconstructName(const std::vector<std::string>& property_name
                                                , ReferenceType ref_type);

    template <class T>
    inline bool ConstantExpr(const ValueRefBase<T>* expr)
    {
        assert(expr);
        if (dynamic_cast<const Constant<T>*>(expr))
            return true;
        else if (dynamic_cast<const Variable<T>*>(expr))
            return false;
        else if (const Operation<T>* op = dynamic_cast<const Operation<T>*>(expr))
            return ConstantExpr(op->LHS()) && ConstantExpr(op->RHS());
        return false;
    }
}

struct ScriptingContext {
    /** Empty context.  Useful for evaluating ValueRef::Constant that don't
      * depend on their context. */
    ScriptingContext()
    {}

    /** Context with only a source object.  Useful for evaluating effectsgroup
      * scope and activation conditions that have no external candidates or
      * effect target to propegate. */
    explicit ScriptingContext(TemporaryPtr<const UniverseObject> source_) :
        source(source_)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_) :
        source(source_),
        effect_target(target_)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_,
                     const boost::any& current_value_) :
        source(source_),
        effect_target(target_),
        current_value(current_value_)
    {}

    /** For evaluating ValueRef in an Effect::Execute function.  Keeps input
      * context, but specifies the current value. */
    ScriptingContext(const ScriptingContext& parent_context,
                     const boost::any& current_value_) :
        source(parent_context.source),
        effect_target(parent_context.effect_target),
        condition_root_candidate(parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(current_value_)
    {}

    /** For recusrive evaluation of Conditions.  Keeps source and effect_target
      * from input context, but sets local candidate with input object, and if
      * there is no root candidate in the parent context, then the input object
      * becomes the root candidate. */
    ScriptingContext(const ScriptingContext& parent_context,
                     TemporaryPtr<const UniverseObject> condition_local_candidate) :
        source(                     parent_context.source),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate ?
                                        parent_context.condition_root_candidate :
                                        condition_local_candidate),                 // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(  condition_local_candidate),                     // new local candidate
        current_value(              parent_context.current_value)
    {}

    ScriptingContext(TemporaryPtr<const UniverseObject> source_, TemporaryPtr<UniverseObject> target_,
                     const boost::any& current_value_,
                     TemporaryPtr<const UniverseObject> condition_root_candidate_,
                     TemporaryPtr<const UniverseObject> condition_local_candidate_) :
        source(source_),
        condition_root_candidate(condition_root_candidate_),
        condition_local_candidate(condition_local_candidate_),
        current_value(current_value_)
    {}

    TemporaryPtr<const UniverseObject>  source;
    TemporaryPtr<UniverseObject>        effect_target;
    TemporaryPtr<const UniverseObject>  condition_root_candidate;
    TemporaryPtr<const UniverseObject>  condition_local_candidate;
    const boost::any                    current_value;
};



///////////////////////////////////////////////////////////
// ValueRefBase                                          //
///////////////////////////////////////////////////////////

/** The base class for all ValueRef classes.  This class provides the public
  * interface for a ValueRef expression tree. */
template <class T>
struct FO_COMMON_API ValueRef::ValueRefBase
{
    virtual ~ValueRefBase() {} ///< virtual dtor

    virtual bool        operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        return true;
    }
    bool                operator!=(const ValueRef::ValueRefBase<T>& rhs) const { return !(*this == rhs); }

    /** Evaluates the expression tree and return the results; \a context
      * is used to fill in any instances of the "Value" variable or references
      * to objects such as the source, effect target, or condition candidates
      * that exist in the tree. */
    virtual T           Eval(const ScriptingContext& context) const = 0;

    /** Evaluates the expression tree with an empty context.  Useful for
      * evaluating expressions that do not depend on context. */
    T                   Eval() const { return Eval(::ScriptingContext()); }

    virtual bool        RootCandidateInvariant() const { return false; }
    virtual bool        LocalCandidateInvariant() const { return false; }
    virtual bool        TargetInvariant() const { return false; }
    virtual bool        SourceInvariant() const { return false; }

    virtual std::string Description() const = 0;
    virtual std::string Dump() const = 0; ///< returns a text description of this type of special

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {}
};

template struct FO_COMMON_API ValueRef::ValueRefBase<int>;
template struct FO_COMMON_API ValueRef::ValueRefBase<double>;
template struct FO_COMMON_API ValueRef::ValueRefBase<std::string>;
template struct FO_COMMON_API ValueRef::ValueRefBase<PlanetSize>;
template struct FO_COMMON_API ValueRef::ValueRefBase<PlanetType>;
template struct FO_COMMON_API ValueRef::ValueRefBase<PlanetEnvironment>;
template struct FO_COMMON_API ValueRef::ValueRefBase<UniverseObjectType>;
template struct FO_COMMON_API ValueRef::ValueRefBase<StarType>;



///////////////////////////////////////////////////////////
// Constant                                              //
///////////////////////////////////////////////////////////

/** the constant value leaf ValueRef class. */
template <class T>
struct FO_COMMON_API ValueRef::Constant : public ValueRef::ValueRefBase<T>
{
    Constant(T value) : m_value(value) {} ///< basic ctor

    virtual bool        operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::Constant<T>& rhs_ = static_cast<const ValueRef::Constant<T>&>(rhs);

        return m_value == rhs_.m_value;
    }
    T                   Value() const { return m_value; }
    virtual T           Eval(const ScriptingContext& context) const { return m_value; };
    virtual bool        RootCandidateInvariant() const { return true; }
    virtual bool        LocalCandidateInvariant() const { return true; }
    virtual bool        TargetInvariant() const { return true; }
    virtual bool        SourceInvariant() const { return true; }

    virtual std::string Description() const { return UserString(boost::lexical_cast<std::string>(m_value)); }
    virtual std::string Dump() const;

private:
    T m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
            & BOOST_SERIALIZATION_NVP(m_value);
    }
};

namespace ValueRef {
    template <>
    inline std::string Constant<int>::Description() const;

    template <>
    inline std::string Constant<double>::Description() const;

    template <>
    inline std::string Constant<std::string>::Description() const;

    template <>
    inline std::string Constant<PlanetSize>::Dump() const;

    template <>
    inline std::string Constant<PlanetType>::Dump() const;

    template <>
    inline std::string Constant<PlanetEnvironment>::Dump() const;

    template <>
    inline std::string Constant<UniverseObjectType>::Dump() const;

    template <>
    inline std::string Constant<StarType>::Dump() const;

    template <>
    inline std::string Constant<int>::Dump() const;

    template <>
    inline std::string Constant<double>::Dump() const;

    template <>
    inline std::string Constant<std::string>::Dump() const;
}

template struct FO_COMMON_API ValueRef::Constant<int>;
template struct FO_COMMON_API ValueRef::Constant<double>;
template struct FO_COMMON_API ValueRef::Constant<std::string>;
template struct FO_COMMON_API ValueRef::Constant<PlanetSize>;
template struct FO_COMMON_API ValueRef::Constant<PlanetType>;
template struct FO_COMMON_API ValueRef::Constant<PlanetEnvironment>;
template struct FO_COMMON_API ValueRef::Constant<UniverseObjectType>;
template struct FO_COMMON_API ValueRef::Constant<StarType>;



///////////////////////////////////////////////////////////
// Variable                                              //
///////////////////////////////////////////////////////////

/** The variable value ValueRef class.  The value returned by this node is
  * taken from the gamestate, most often from the Source or Target objects. */
template <class T>
struct FO_COMMON_API ValueRef::Variable : public ValueRef::ValueRefBase<T>
{
    Variable(ReferenceType ref_type, const std::vector<std::string>& property_name)
        : m_ref_type(ref_type)
        , m_property_name(property_name.begin(), property_name.end())
    { }
    Variable(ReferenceType ref_type, const std::string& property_name = "")
        : m_ref_type(ref_type)
        , m_property_name() 
    {
        m_property_name.push_back(property_name);
    }

    virtual bool                    operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::Variable<T>& rhs_ = static_cast<const ValueRef::Variable<T>&>(rhs);
        return (m_ref_type == rhs_.m_ref_type) && (m_property_name == rhs_.m_property_name);
    }

    ReferenceType                   GetReferenceType() const { return m_ref_type; }
    const std::vector<std::string>& PropertyName() const { return m_property_name; }
    virtual T                       Eval(const ScriptingContext& context) const; // only specialized definitions available
    virtual bool                    RootCandidateInvariant() const {
        return m_ref_type != CONDITION_ROOT_CANDIDATE_REFERENCE;
    }
    virtual bool                    LocalCandidateInvariant() const {
        return m_ref_type != CONDITION_LOCAL_CANDIDATE_REFERENCE;
    }
    virtual bool                    TargetInvariant() const {
        return m_ref_type != EFFECT_TARGET_REFERENCE && m_ref_type != EFFECT_TARGET_VALUE_REFERENCE;
    }
    virtual bool                    SourceInvariant() const {
        return m_ref_type != SOURCE_REFERENCE;
    }
    virtual std::string             Description() const {
        return FormatedDescriptionPropertyNames(m_ref_type, m_property_name);
    }
    virtual std::string             Dump() const {
        return ReconstructName(m_property_name, m_ref_type);
    }

protected:
    mutable ReferenceType       m_ref_type;
    std::vector<std::string>    m_property_name;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
            & BOOST_SERIALIZATION_NVP(m_ref_type)
            & BOOST_SERIALIZATION_NVP(m_property_name);
    }
};

namespace ValueRef {
    template <>
    inline PlanetSize Variable<PlanetSize>::Eval(const ScriptingContext& context) const;

    template <>
    inline PlanetType Variable<PlanetType>::Eval(const ScriptingContext& context) const;

    template <>
    inline PlanetEnvironment Variable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

    template <>
    inline UniverseObjectType Variable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

    template <>
    inline StarType Variable<StarType>::Eval(const ScriptingContext& context) const;

    template <>
    inline double Variable<double>::Eval(const ScriptingContext& context) const;

    template <>
    inline int Variable<int>::Eval(const ScriptingContext& context) const;
}

template struct FO_COMMON_API ValueRef::Variable<int>;
template struct FO_COMMON_API ValueRef::Variable<double>;
template struct FO_COMMON_API ValueRef::Variable<PlanetSize>;
template struct FO_COMMON_API ValueRef::Variable<PlanetType>;
template struct FO_COMMON_API ValueRef::Variable<PlanetEnvironment>;
template struct FO_COMMON_API ValueRef::Variable<UniverseObjectType>;
template struct FO_COMMON_API ValueRef::Variable<StarType>;



///////////////////////////////////////////////////////////
// Statistic                                             //
///////////////////////////////////////////////////////////

namespace ValueRef {
    namespace impl {
        template<class T>
        FO_COMMON_API T ReduceData(Statistic<T> const * const that, const std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values);
    }
}

/** The variable statistic class.   The value returned by this node is
  * computed from the general gamestate; the value of the indicated
  * \a property_name is computed for each object that matches
  * \a sampling_condition and the statistic indicated by \a stat_type is
  * calculated from them and returned. */
template <class T>
struct FO_COMMON_API ValueRef::Statistic : public ValueRef::Variable<T>
{
    Statistic(const std::vector<std::string>& property_name,
              StatisticType stat_type,
              const Condition::ConditionBase* sampling_condition)
        : Variable<T>(ValueRef::NON_OBJECT_REFERENCE, property_name)
        , m_stat_type(stat_type)
        , m_sampling_condition(sampling_condition)
    { }
    ~Statistic() {
        delete m_sampling_condition;
    }

    virtual bool                    operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::Statistic<T>& rhs_ = static_cast<const ValueRef::Statistic<T>&>(rhs);

        if (m_stat_type != rhs_.m_stat_type)
            return false;
        if (this->m_property_name != rhs_.m_property_name)
            return false;

        if (m_sampling_condition == rhs_.m_sampling_condition) {
            // check next member
        }
        else if (!m_sampling_condition || !rhs_.m_sampling_condition) {
            return false;
        }
        else {
            if (*m_sampling_condition != *(rhs_.m_sampling_condition))
                return false;
        }

        return true;
    }

    StatisticType                   GetStatisticType() const     { return m_stat_type; }
    const Condition::ConditionBase* GetSamplingCondition() const { return m_sampling_condition; }

    virtual T                       Eval(const ScriptingContext& context) const
    {
        // the only statistic that can be computed on non-number property types
        // and that is itself of a non-number type is the most common value
        if (m_stat_type != MODE)
            throw std::runtime_error("ValueRef evaluated with an invalid StatisticType for the return type.");

        Condition::ObjectSet condition_matches;
        GetConditionMatches(context, condition_matches, m_sampling_condition);

        if (condition_matches.empty())
            return T(-1);   // should be INVALID_T of enum types

        // evaluate property for each condition-matched object
        std::map<TemporaryPtr<const UniverseObject>, T> object_property_values;
        GetObjectPropertyValues(context, condition_matches, object_property_values);

        // count number of each result, tracking which has the most occurances
        std::map<T, unsigned int> histogram;
        typename std::map<T, unsigned int>::const_iterator most_common_property_value_it = histogram.begin();
        unsigned int max_seen(0);

        for (typename std::map<TemporaryPtr<const UniverseObject>, T>::const_iterator it = object_property_values.begin();
            it != object_property_values.end(); ++it)
        {
            const T& property_value = it->second;

            typename std::map<T, unsigned int>::iterator hist_it = histogram.find(property_value);
            if (hist_it == histogram.end())
                hist_it = histogram.insert(std::make_pair(property_value, 0)).first;
            unsigned int& num_seen = hist_it->second;

            num_seen++;

            if (num_seen > max_seen) {
                most_common_property_value_it = hist_it;
                max_seen = num_seen;
            }
        }

        // return result (property value) that occured most frequently
        return most_common_property_value_it->first;
    }

    virtual bool                    RootCandidateInvariant() const {
        return ValueRef::Variable<T>::RootCandidateInvariant() && m_sampling_condition->RootCandidateInvariant();
    }
    virtual bool                    LocalCandidateInvariant() const {
        // don't need to check if sampling condition is LocalCandidateInvariant, as
        // all conditions aren't, but that refers to their own local candidate.  no
        // condition is explicitly dependent on the parent context's local candidate.
        return ValueRef::Variable<T>::LocalCandidateInvariant();
    }
    virtual bool                    TargetInvariant() const {
        return ValueRef::Variable<T>::TargetInvariant() && m_sampling_condition->TargetInvariant();
    }
    virtual bool                    SourceInvariant() const {
        return ValueRef::Variable<T>::SourceInvariant() && m_sampling_condition->SourceInvariant();
    }

    virtual std::string             Description() const {
        return UserString("DESC_STATISTIC");
    }

    virtual std::string             Dump() const {
        return "Statistic";
    }

protected:
    /** Gets the set of objects in the Universe that match the sampling condition. */
    void    GetConditionMatches(const ScriptingContext& context,
                                Condition::ObjectSet& condition_targets,
                                const Condition::ConditionBase* condition) const
    {
        condition_targets.clear();
        if (!condition)
            return;
        condition->Eval(context, condition_targets);
    }

    /** Evaluates the property for the specified objects. */
    void    GetObjectPropertyValues(const ScriptingContext& context,
                                    const Condition::ObjectSet& objects,
                                    std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values) const
    {
        object_property_values.clear();
        //Logger().debugStream() << "ValueRef::Statistic<T>::GetObjectPropertyValues source: " << source->Dump()
        //                       << " sampling condition: " << m_sampling_condition->Dump()
        //                       << " property name final: " << this->PropertyName().back();
        ReferenceType original_ref_type = this->m_ref_type;
        this->m_ref_type = ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE;
        for (Condition::ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it) {
            T property_value = this->Variable<T>::Eval(ScriptingContext(context, *it));
            object_property_values[*it] = property_value;
        }
        this->m_ref_type = original_ref_type;
    }

private:
    StatisticType                   m_stat_type;
    const Condition::ConditionBase* m_sampling_condition;

    /** Computes the statistic from the specified set of property values. */
    template <class T>
    friend FO_COMMON_API T impl::ReduceData(Statistic<T> const * const that, const std::map<TemporaryPtr<const UniverseObject>, T>& object_property_values);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Variable)
            & BOOST_SERIALIZATION_NVP(m_stat_type)
            & BOOST_SERIALIZATION_NVP(m_sampling_condition);
    }
};

namespace ValueRef {
    template <>
    inline double Statistic<double>::Eval(const ScriptingContext& context) const;

    template <>
    inline int Statistic<int>::Eval(const ScriptingContext& context) const;

    template <>
    inline std::string Statistic<std::string>::Eval(const ScriptingContext& context) const;
}

template struct FO_COMMON_API ValueRef::Statistic<int>;
template struct FO_COMMON_API ValueRef::Statistic<double>;
template struct FO_COMMON_API ValueRef::Statistic<std::string>;



///////////////////////////////////////////////////////////
// ComplexVariable                                       //
///////////////////////////////////////////////////////////

/** The complex variable ValueRef class. The value returned by this node
  * is taken from the gamestate. */
template <class T>
struct FO_COMMON_API ValueRef::ComplexVariable : public ValueRef::Variable<T>
{
    explicit ComplexVariable(const std::string& variable_name,
                             const ValueRefBase<int>* int_ref1 = 0,
                             const ValueRefBase<int>* int_ref2 = 0,
                             const ValueRefBase<std::string>* string_ref1 = 0,
                             const ValueRefBase<std::string>* string_ref2 = 0)
        : Variable<T>(ValueRef::NON_OBJECT_REFERENCE, std::vector<std::string>(1, variable_name))
        , m_int_ref1(int_ref1)
        , m_int_ref2(int_ref2)
        , m_string_ref1(string_ref1)
        , m_string_ref2(string_ref2)
    {
        //std::cout << "ComplexVariable: " << variable_name << ", "
        //          << int_ref1 << ", " << int_ref2 << ", "
        //          << string_ref1 << ", " << string_ref2 << std::endl;
    }

    ~ComplexVariable()
    {
        delete m_int_ref1;
        delete m_int_ref2;
        delete m_string_ref1;
        delete m_string_ref2;
    }

    virtual bool operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::ComplexVariable<T>& rhs_ = static_cast<const ValueRef::ComplexVariable<T>&>(rhs);

        if (this->m_property_name != rhs_.m_property_name)
            return false;

        if (m_int_ref1 == rhs_.m_int_ref1) {
            // check next member
        }
        else if (!m_int_ref1 || !rhs_.m_int_ref1) {
            return false;
        }
        else {
            if (*m_int_ref1 != *(rhs_.m_int_ref1))
                return false;
        }

        if (m_int_ref2 == rhs_.m_int_ref2) {
            // check next member
        }
        else if (!m_int_ref2 || !rhs_.m_int_ref2) {
            return false;
        }
        else {
            if (*m_int_ref2 != *(rhs_.m_int_ref2))
                return false;
        }

        if (m_string_ref1 == rhs_.m_string_ref1) {
            // check next member
        }
        else if (!m_string_ref1 || !rhs_.m_string_ref1) {
            return false;
        }
        else {
            if (*m_string_ref1 != *(rhs_.m_string_ref1))
                return false;
        }

        if (m_string_ref2 == rhs_.m_string_ref2) {
            // check next member
        }
        else if (!m_string_ref2 || !rhs_.m_string_ref2) {
            return false;
        }
        else {
            if (*m_string_ref2 != *(rhs_.m_string_ref2))
                return false;
        }

        return true;
    }

    const ValueRefBase<int>*        IntRef1() const { return m_int_ref1; }
    const ValueRefBase<int>*        IntRef2() const { return m_int_ref2; }
    const ValueRefBase<std::string>*StringRef1() const { return m_string_ref1; }
    const ValueRefBase<std::string>*StringRef2() const { return m_string_ref2; }
    virtual T                       Eval(const ScriptingContext& context) const;
    virtual bool                    RootCandidateInvariant() const
    {
        return ValueRef::Variable<T>::RootCandidateInvariant()
            && (!m_int_ref1 || m_int_ref1->RootCandidateInvariant())
            && (!m_int_ref2 || m_int_ref2->RootCandidateInvariant())
            && (!m_string_ref1 || m_string_ref1->RootCandidateInvariant())
            && (!m_string_ref2 || m_string_ref2->RootCandidateInvariant());
    }
    virtual bool                    LocalCandidateInvariant() const
    {
        return (!m_int_ref1 || m_int_ref1->LocalCandidateInvariant())
            && (!m_int_ref2 || m_int_ref2->LocalCandidateInvariant())
            && (!m_string_ref1 || m_string_ref1->LocalCandidateInvariant())
            && (!m_string_ref2 || m_string_ref2->LocalCandidateInvariant());
    }

    virtual bool                    TargetInvariant() const
    {
        return (!m_int_ref1 || m_int_ref1->TargetInvariant())
            && (!m_int_ref2 || m_int_ref2->TargetInvariant())
            && (!m_string_ref1 || m_string_ref1->TargetInvariant())
            && (!m_string_ref2 || m_string_ref2->TargetInvariant());
    }
    virtual bool                    SourceInvariant() const
    {
        return (!m_int_ref1 || m_int_ref1->SourceInvariant())
            && (!m_int_ref2 || m_int_ref2->SourceInvariant())
            && (!m_string_ref1 || m_string_ref1->SourceInvariant())
            && (!m_string_ref2 || m_string_ref2->SourceInvariant());
    }
    virtual std::string             Description() const { return UserString("DESC_COMPLEX"); }
    virtual std::string             Dump() const { return "ComplexVariable"; }

protected:
    const ValueRefBase<int>*        m_int_ref1;
    const ValueRefBase<int>*        m_int_ref2;
    const ValueRefBase<std::string>*m_string_ref1;
    const ValueRefBase<std::string>*m_string_ref2;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Variable)
            & BOOST_SERIALIZATION_NVP(m_int_ref1)
            & BOOST_SERIALIZATION_NVP(m_int_ref2)
            & BOOST_SERIALIZATION_NVP(m_string_ref1)
            & BOOST_SERIALIZATION_NVP(m_string_ref2);
    }
};

namespace ValueRef {
    template <>
    inline PlanetSize ComplexVariable<PlanetSize>::Eval(const ScriptingContext& context) const;

    template <>
    inline PlanetType ComplexVariable<PlanetType>::Eval(const ScriptingContext& context) const;

    template <>
    inline PlanetEnvironment ComplexVariable<PlanetEnvironment>::Eval(const ScriptingContext& context) const;

    template <>
    inline UniverseObjectType ComplexVariable<UniverseObjectType>::Eval(const ScriptingContext& context) const;

    template <>
    inline StarType ComplexVariable<StarType>::Eval(const ScriptingContext& context) const;

    template <>
    inline double ComplexVariable<double>::Eval(const ScriptingContext& context) const;

    template <>
    inline int ComplexVariable<int>::Eval(const ScriptingContext& context) const;

    template <>
    inline std::string ComplexVariable<std::string>::Eval(const ScriptingContext& context) const;
}

template struct FO_COMMON_API ValueRef::ComplexVariable<int>;
template struct FO_COMMON_API ValueRef::ComplexVariable<double>;
template struct FO_COMMON_API ValueRef::ComplexVariable<std::string>;
template struct FO_COMMON_API ValueRef::ComplexVariable<PlanetSize>;
template struct FO_COMMON_API ValueRef::ComplexVariable<PlanetType>;
template struct FO_COMMON_API ValueRef::ComplexVariable<PlanetEnvironment>;
template struct FO_COMMON_API ValueRef::ComplexVariable<UniverseObjectType>;
template struct FO_COMMON_API ValueRef::ComplexVariable<StarType>;



///////////////////////////////////////////////////////////
// StaticCast                                            //
///////////////////////////////////////////////////////////

/** The variable static_cast class.  The value returned by this node is taken
  * from the ctor \a value_ref parameter's FromType value, static_cast to
  * ToType. */
template <class FromType, class ToType>
struct FO_COMMON_API ValueRef::StaticCast : public ValueRef::Variable<ToType>
{
    StaticCast(const ValueRef::Variable<FromType>* value_ref)
        : ValueRef::Variable<ToType>(value_ref->GetReferenceType()
        , value_ref->PropertyName())
        , m_value_ref(value_ref)
    { }
    ~StaticCast()
    {
        delete m_value_ref;
    }

    virtual bool        operator==(const ValueRef::ValueRefBase<ToType>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::StaticCast<FromType, ToType>& rhs_ =
            static_cast<const ValueRef::StaticCast<FromType, ToType>&>(rhs);

        if (m_value_ref == rhs_.m_value_ref) {
            // check next member
        }
        else if (!m_value_ref || !rhs_.m_value_ref) {
            return false;
        }
        else {
            if (*m_value_ref != *(rhs_.m_value_ref))
                return false;
        }

        return true;
    }

    virtual ToType      Eval(const ScriptingContext& context) const
    {
        return static_cast<ToType>(m_value_ref->Eval(context));
    }
    virtual bool        RootCandidateInvariant() const { return m_value_ref->RootCandidateInvariant(); }
    virtual bool        LocalCandidateInvariant() const { return m_value_ref->LocalCandidateInvariant(); }
    virtual bool        TargetInvariant() const { return m_value_ref->TargetInvariant(); }
    virtual bool        SourceInvariant() const { return m_value_ref->SourceInvariant(); }
    virtual std::string Description() const { return m_value_ref->Description(); }
    virtual std::string Dump() const { return m_value_ref->Dump(); }
    const ValueRefBase<FromType>*   GetValueRef() const { return m_value_ref; }

private:
    const ValueRefBase<FromType>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
            & BOOST_SERIALIZATION_NVP(m_value_ref);
    }
};



///////////////////////////////////////////////////////////
// StringCast                                            //
///////////////////////////////////////////////////////////

/** The variable lexical_cast to string class.  The value returned by this node
  * is taken from the ctor \a value_ref parameter's FromType value,
  * lexical_cast to std::string */
template <class FromType>
struct FO_COMMON_API ValueRef::StringCast : public ValueRef::Variable<std::string>
{
    StringCast(const ValueRef::Variable<FromType>* value_ref)
        : ValueRef::Variable<std::string>(value_ref->GetReferenceType()
        , value_ref->PropertyName())
        , m_value_ref(value_ref)
    { }
    ~StringCast()
    {
        delete m_value_ref;
    }

    virtual bool        operator==(const ValueRef::ValueRefBase<std::string>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::StringCast<FromType>& rhs_ =
            static_cast<const ValueRef::StringCast<FromType>&>(rhs);

        if (m_value_ref == rhs_.m_value_ref) {
            // check next member
        }
        else if (!m_value_ref || !rhs_.m_value_ref) {
            return false;
        }
        else {
            if (*m_value_ref != *(rhs_.m_value_ref))
                return false;
        }

        return true;
    }
    virtual std::string Eval(const ScriptingContext& context) const
    {
        return boost::lexical_cast<std::string>(m_value_ref->Eval(context));
    }
    virtual bool        RootCandidateInvariant() const { return m_value_ref->RootCandidateInvariant(); }
    virtual bool        LocalCandidateInvariant() const { return m_value_ref->LocalCandidateInvariant(); }
    virtual bool        TargetInvariant() const { return m_value_ref->TargetInvariant(); }
    virtual bool        SourceInvariant() const { return m_value_ref->SourceInvariant(); }
    virtual std::string Description() const { return m_value_ref->Description(); }
    virtual std::string Dump() const { return m_value_ref->Dump(); }
    const ValueRefBase<FromType>*   GetValueRef() const { return m_value_ref; }

private:
    const ValueRefBase<FromType>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
            & BOOST_SERIALIZATION_NVP(m_value_ref);
    }
};

namespace ValueRef {
    template <>
    inline std::string StringCast<double>::Eval(const ScriptingContext& context) const;

    template <>
    inline std::string StringCast<int>::Eval(const ScriptingContext& context) const;
}

template struct FO_COMMON_API ValueRef::StringCast<int>;
template struct FO_COMMON_API ValueRef::StringCast<double>;



///////////////////////////////////////////////////////////
// UserStringLookup                                      //
///////////////////////////////////////////////////////////

/** Looks up a string ValueRef and returns the UserString equivalent. */
struct FO_COMMON_API ValueRef::UserStringLookup : public ValueRef::Variable<std::string> {
    UserStringLookup(const ValueRef::Variable<std::string>* value_ref);
    ~UserStringLookup();
    virtual bool        operator==(const ValueRef::ValueRefBase<std::string>& rhs) const;
    virtual std::string Eval(const ScriptingContext& context) const;
    virtual bool        RootCandidateInvariant() const;
    virtual bool        LocalCandidateInvariant() const;
    virtual bool        TargetInvariant() const;
    virtual bool        SourceInvariant() const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
    const ValueRefBase<std::string>*    GetValueRef() const { return m_value_ref; }

private:
    const ValueRefBase<std::string>* m_value_ref;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase<std::string>)
            & BOOST_SERIALIZATION_NVP(m_value_ref);
    }
};



///////////////////////////////////////////////////////////
// Operation                                             //
///////////////////////////////////////////////////////////

/** An arithmetic operation node ValueRef class.  One of addition, subtraction,
  * mutiplication, division, or unary negation is performed on the child(ren)
  * of this node, and the result is returned. */
template <class T>
struct FO_COMMON_API ValueRef::Operation : public ValueRef::ValueRefBase<T>
{
    Operation(OpType op_type, const ValueRefBase<T>* operand1, const ValueRefBase<T>* operand2)
        : m_op_type(op_type), m_operand1(operand1), m_operand2(operand2) ///< binary operation ctor
    { }
    Operation(OpType op_type, const ValueRefBase<T>* operand)
        : m_op_type(op_type), m_operand1(operand), m_operand2(0)         ///< unary operation ctor
    { }
    ~Operation()
    {
        delete m_operand1;
        delete m_operand2;
    }

    virtual bool            operator==(const ValueRef::ValueRefBase<T>& rhs) const
    {
        if (&rhs == this)
            return true;
        if (typeid(rhs) != typeid(*this))
            return false;
        const ValueRef::Operation<T>& rhs_ = static_cast<const ValueRef::Operation<T>&>(rhs);

        if (m_operand1 == rhs_.m_operand1) {
            // check next member
        }
        else if (!m_operand1 || !rhs_.m_operand1) {
            return false;
        }
        else {
            if (*m_operand1 != *(rhs_.m_operand1))
                return false;
        }

        if (m_operand2 == rhs_.m_operand2) {
            // check next member
        }
        else if (!m_operand2 || !rhs_.m_operand2) {
            return false;
        }
        else {
            if (*m_operand2 != *(rhs_.m_operand2))
                return false;
        }

        return true;
}
    OpType                  GetOpType() const { return m_op_type; }
    const ValueRefBase<T>*  LHS() const { return m_operand1; }
    const ValueRefBase<T>*  RHS() const { return m_operand2; }
    virtual T               Eval(const ScriptingContext& context) const
    {
        switch (m_op_type) {
        case PLUS:
            return T(m_operand1->Eval(context) +
                m_operand2->Eval(context));
            break;
        case MINUS:
            return T(m_operand1->Eval(context) -
                m_operand2->Eval(context));
            break;
        default:
            throw std::runtime_error("ValueRef evaluated with an unknown or invalid OpType.");
            break;
        }
    }
    virtual bool            RootCandidateInvariant() const
    {
        if (m_op_type == RANDOM_UNIFORM)
            return false;
        if (m_operand1 && !m_operand1->RootCandidateInvariant())
            return false;
        if (m_operand2 && !m_operand2->RootCandidateInvariant())
            return false;
        return true;
    }
    virtual bool            LocalCandidateInvariant() const
    {
        if (m_op_type == RANDOM_UNIFORM)
            return false;
        if (m_operand1 && !m_operand1->LocalCandidateInvariant())
            return false;
        if (m_operand2 && !m_operand2->LocalCandidateInvariant())
            return false;
        return true;
    }
    virtual bool            TargetInvariant() const
    {
        if (m_op_type == RANDOM_UNIFORM)
            return false;
        if (m_operand1 && !m_operand1->TargetInvariant())
            return false;
        if (m_operand2 && !m_operand2->TargetInvariant())
            return false;
        return true;
    }
    virtual bool            SourceInvariant() const
    {
        if (m_op_type == RANDOM_UNIFORM)
            return false;
        if (m_operand1 && !m_operand1->SourceInvariant())
            return false;
        if (m_operand2 && !m_operand2->SourceInvariant())
            return false;
        return true;
    }
    virtual std::string     Description() const
    {
        if (m_op_type == NEGATE) {
            //Logger().debugStream() << "Operation is negation";
            if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
                OpType op_type = rhs->GetOpType();
                if (op_type == PLUS || op_type == MINUS ||
                    op_type == TIMES || op_type == DIVIDE ||
                    op_type == NEGATE || op_type == EXPONENTIATE)
                    return "-(" + m_operand1->Description() + ")";
            }
            else {
                return "-" + m_operand1->Description();
            }
        }

        if (m_op_type == ABS)
            return "abs(" + m_operand1->Description() + ")";
        if (m_op_type == LOGARITHM)
            return "log(" + m_operand1->Description() + ")";
        if (m_op_type == SINE)
            return "sin(" + m_operand1->Description() + ")";
        if (m_op_type == COSINE)
            return "cos(" + m_operand1->Description() + ")";
        if (m_op_type == MINIMUM)
            return "min(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";
        if (m_op_type == MAXIMUM)
            return "max(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";
        if (m_op_type == RANDOM_UNIFORM)
            return "random(" + m_operand1->Description() + ", " + m_operand1->Description() + ")";


        bool parenthesize_lhs = false;
        bool parenthesize_rhs = false;
        if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
            OpType op_type = lhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                (op_type == EXPONENTIATE || op_type == TIMES || op_type == DIVIDE ||
                op_type == PLUS || op_type == MINUS || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES || m_op_type == DIVIDE) &&
                (op_type == PLUS || op_type == MINUS) || op_type == NEGATE)
                )
                parenthesize_lhs = true;
        }
        if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
            OpType op_type = rhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                (op_type == EXPONENTIATE || op_type == TIMES || op_type == DIVIDE ||
                op_type == PLUS || op_type == MINUS || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES || m_op_type == DIVIDE) &&
                (op_type == PLUS || op_type == MINUS) || op_type == NEGATE)
                )
                parenthesize_rhs = true;
        }

        std::string retval;
        if (parenthesize_lhs)
            retval += '(' + m_operand1->Description() + ')';
        else
            retval += m_operand1->Description();

        switch (m_op_type) {
        case PLUS:          retval += " + "; break;
        case MINUS:         retval += " - "; break;
        case TIMES:         retval += " * "; break;
        case DIVIDE:        retval += " / "; break;
        case EXPONENTIATE:  retval += " ^ "; break;
        default:            retval += " ? "; break;
        }

        if (parenthesize_rhs)
            retval += '(' + m_operand2->Description() + ')';
        else
            retval += m_operand2->Description();

        return retval;
    }
    virtual std::string     Dump() const
    {
        if (m_op_type == NEGATE) {
            if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
                OpType op_type = rhs->GetOpType();
                if (op_type == PLUS || op_type == MINUS ||
                    op_type == TIMES || op_type == DIVIDE ||
                    op_type == NEGATE || op_type == EXPONENTIATE)
                    return "-(" + m_operand1->Dump() + ")";
            }
            else {
                return "-" + m_operand1->Dump();
            }
        }

        if (m_op_type == ABS)
            return "abs(" + m_operand1->Dump() + ")";
        if (m_op_type == LOGARITHM)
            return "log(" + m_operand1->Dump() + ")";
        if (m_op_type == SINE)
            return "sin(" + m_operand1->Dump() + ")";
        if (m_op_type == COSINE)
            return "cos(" + m_operand1->Dump() + ")";
        if (m_op_type == MINIMUM)
            return "min(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";
        if (m_op_type == MAXIMUM)
            return "max(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";
        if (m_op_type == RANDOM_UNIFORM)
            return "random(" + m_operand1->Dump() + ", " + m_operand1->Dump() + ")";


        bool parenthesize_lhs = false;
        bool parenthesize_rhs = false;
        if (const ValueRef::Operation<T>* lhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand1)) {
            OpType op_type = lhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                (op_type == EXPONENTIATE || op_type == TIMES || op_type == DIVIDE ||
                op_type == PLUS || op_type == MINUS || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES || m_op_type == DIVIDE) &&
                (op_type == PLUS || op_type == MINUS) || op_type == NEGATE)
                )
                parenthesize_lhs = true;
        }
        if (const ValueRef::Operation<T>* rhs = dynamic_cast<const ValueRef::Operation<T>*>(m_operand2)) {
            OpType op_type = rhs->GetOpType();
            if (
                (m_op_type == EXPONENTIATE &&
                (op_type == EXPONENTIATE || op_type == TIMES || op_type == DIVIDE ||
                op_type == PLUS || op_type == MINUS || op_type == NEGATE)
                ) ||
                ((m_op_type == TIMES || m_op_type == DIVIDE) &&
                (op_type == PLUS || op_type == MINUS) || op_type == NEGATE)
                )
                parenthesize_rhs = true;
        }

        std::string retval;
        if (parenthesize_lhs)
            retval += '(' + m_operand1->Dump() + ')';
        else
            retval += m_operand1->Dump();

        switch (m_op_type) {
        case PLUS:          retval += " + "; break;
        case MINUS:         retval += " - "; break;
        case TIMES:         retval += " * "; break;
        case DIVIDE:        retval += " / "; break;
        case EXPONENTIATE:  retval += " ^ "; break;
        default:            retval += " ? "; break;
        }

        if (parenthesize_rhs)
            retval += '(' + m_operand2->Dump() + ')';
        else
            retval += m_operand2->Dump();

        return retval;
    }

private:
    OpType                  m_op_type;
    const ValueRefBase<T>*  m_operand1;
    const ValueRefBase<T>*  m_operand2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ValueRefBase)
            & BOOST_SERIALIZATION_NVP(m_op_type)
            & BOOST_SERIALIZATION_NVP(m_operand1)
            & BOOST_SERIALIZATION_NVP(m_operand2);
    }
};

namespace ValueRef {
    template <>
    inline std::string Operation<std::string>::Eval(const ScriptingContext& context) const;

    template <>
    inline double Operation<double>::Eval(const ScriptingContext& context) const;

    template <>
    inline int Operation<int>::Eval(const ScriptingContext& context) const;

    template <>
    inline std::string Operation<double>::Description() const;
}

template struct FO_COMMON_API ValueRef::Operation<int>;
template struct FO_COMMON_API ValueRef::Operation<double>;
template struct FO_COMMON_API ValueRef::Operation<std::string>;

#endif // _ValueRef_h_
