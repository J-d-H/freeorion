import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import TechsListsAI
import AIDependencies
import AIstate
import traceback
import ColonisationAI

inProgressTechs={}


def get_research_index():
    empire_id = fo.empireID()
    research_index = empire_id % 2
    if foAI.foAIstate.aggression >=fo.aggression.aggressive: #maniacal
        research_index = 2 + (empire_id % 3) # so indices [2,3,4]
    elif foAI.foAIstate.aggression >=fo.aggression.typical:
        research_index += 1
    return research_index


def generate_research_orders():
    """generate research orders"""
    universe=fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted',{})
    print "Research Queue Management:"
    resource_production = empire.resourceProduction(fo.resourceType.research)
    print "\nTotal Current Research Points: %.2f\n"%resource_production
    print "Techs researched and available for use:"
    completed_techs = sorted(list(get_completed_techs()))
    tlist = completed_techs+3*[" "]
    tlines = zip( tlist[0::3],  tlist[1::3],  tlist[2::3])
    for tline in tlines:
        print "%25s  %25s  %25s"%tline
    print""
    
    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    inProgressTechs.clear()
    tech_turns_left = {}
    if research_queue_list:
        print "Techs currently at head of Research Queue:"
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            if element.allocation > 0.0:
                inProgressTechs[element.tech]=True
            this_tech=fo.getTech(element.tech)
            if not this_tech:
                print "Error: can't retrieve tech ",  element.tech
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            #unlocked_items = [(uli.name,  uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                print "    %25s  allocated %6.2f RP -- unlockable items: %s "%(element.tech,  element.allocation,  unlocked_items)
            else:
                print "    %25s  allocated %6.2f RP   --  missing preReqs: %s   -- unlockable items: %s "%(element.tech,  element.allocation,  missing_prereqs,  unlocked_items)
        print
    #
    # set starting techs, or after turn 100 add any additional default techs
    #
    if (fo.currentTurn() == 1) or ((fo.currentTurn() < 5) and (len(research_queue_list) == 0)):
        research_index = get_research_index()
        new_tech = TechsListsAI.primary_meta_techs(index = research_index)
        print "Empire %s (%d) is selecting research index %d"%(empire.name,  empire_id,  research_index)
        #pLTs_to_enqueue = (set(new_tech)-(set(completed_techs)|set(research_queue_list)))
        pLTs_to_enqueue = new_tech[:]
        tech_base = set(completed_techs+research_queue_list)
        techs_to_add = []
        for tech in pLTs_to_enqueue:
            if tech not in  tech_base:
                this_tech=fo.getTech(tech)
                if this_tech is None:
                    print "Error: desired tech '%s' appears to not exist"%tech
                    continue
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in tech_base]
                techs_to_add.extend( missing_prereqs+[tech] )
                tech_base.update(  missing_prereqs+[tech]  )
        cum_cost=0
        print "  Enqueued Tech:  %20s \t\t %8s \t %s"%("Name",  "Cost",  "CumulativeCost")
        for name in techs_to_add:
            try:
                enqueue_res = fo.issueEnqueueTechOrder(name, -1)
                if enqueue_res == 1:
                    this_tech=fo.getTech(name)
                    this_cost=0
                    if this_tech:
                        this_cost = this_tech.researchCost(empire_id)
                        cum_cost += this_cost
                    print "    Enqueued Tech: %20s \t\t %8.0f \t %8.0f" % ( name,  this_cost,  cum_cost)
                else:
                    print "    Error: failed attempt to enqueued Tech: " + name
            except:
                print "    Error: failed attempt to enqueued Tech: " + name
                print "    Error: exception triggered and caught:  ",  traceback.format_exc()
        if foAI.foAIstate.aggression <= fo.aggression.cautious:
            research_queue_list = get_research_queue_techs()
            def_techs = TechsListsAI.defense_techs_1()
            for def_tech in def_techs:
                if def_tech not in research_queue_list[:5]  and  empire.getTechStatus(def_tech) != fo.techStatus.complete:
                    res=fo.issueEnqueueTechOrder(def_tech, min(3,  len(research_queue_list)))
                    print "Empire is very defensive,  so attempted to fast-track %s,  got result %d"%(def_tech, res)
        if False and foAI.foAIstate.aggression >= fo.aggression.aggressive: #with current stats of Conc Camps, disabling this fast-track
            research_queue_list = get_research_queue_techs()
            if "CON_CONC_CAMP" in research_queue_list:
                insert_idx = min(40,  research_queue_list.index("CON_CONC_CAMP"))
            else:
                insert_idx=max(0,  min(40, len(research_queue_list)-10))
            if "SHP_DEFLECTOR_SHIELD" in research_queue_list:
                insert_idx = min(insert_idx,  research_queue_list.index("SHP_DEFLECTOR_SHIELD"))
            for cc_tech in [  "CON_ARCH_PSYCH",  "CON_CONC_CAMP"]:
                if   cc_tech not in research_queue_list[:insert_idx+1]  and  empire.getTechStatus(cc_tech) != fo.techStatus.complete:
                    res=fo.issueEnqueueTechOrder(cc_tech, insert_idx)
                    print "Empire is very aggressive,  so attempted to fast-track %s,  got result %d"%(cc_tech, res)

        print""

        generate_default_research_order()
        print "\n\nAll techs:"
        alltechs = fo.techs() # returns names of all techs
        for tname in alltechs:
            print tname
        print "\n-------------------------------\nAll unqueued techs:"
        #coveredTechs = new_tech+completed_techs
        for tname in [tn for tn in alltechs if tn not in tech_base]:
            print tname

    elif fo.currentTurn() >100:
        generate_default_research_order()

    research_queue_list = get_research_queue_techs()
    num_techs_accelerated = 1 # will ensure leading tech doesn't get dislodged
    gotGGG = empire.getTechStatus("PRO_ORBITAL_GEN") == fo.techStatus.complete
    gotSymBio = empire.getTechStatus("GRO_SYMBIOTIC_BIO") == fo.techStatus.complete
    gotXenoGen = empire.getTechStatus("GRO_XENO_GENETICS") == fo.techStatus.complete
    #
    # Consider accelerating techs; priority is
    # Supply/Detect range
    # xeno arch
    # ast / GG
    # gro xeno gen
    # distrib thought
    # quant net
    # pro sing gen
    # death ray 1 cleanup

    #
    # Supply range and detection range
    if False: # disabled for now, otherwise just to help with cold-folding /  organization
        if len(foAI.foAIstate.colonisablePlanetIDs)==0:
            bestColonySiteScore = 0
        else:
            bestColonySiteScore= foAI.foAIstate.colonisablePlanetIDs[0][1]
        if len(foAI.foAIstate.colonisableOutpostIDs)==0:
            bestOutpostSiteScore = 0
        else:
            bestOutpostSiteScore= foAI.foAIstate.colonisableOutpostIDs[0][1]
        needImprovedScouting = ( bestColonySiteScore <150  or   bestOutpostSiteScore < 200 )

        if needImprovedScouting:
            if empire.getTechStatus("CON_ORBITAL_CON") != fo.techStatus.complete:
                num_techs_accelerated += 1
                if  ( "CON_ORBITAL_CON" not in research_queue_list[:1 + num_techs_accelerated]  ) and  (
                                (empire.getTechStatus("PRO_FUSION_GEN") == fo.techStatus.complete) or ( "PRO_FUSION_GEN"  in research_queue_list[:1+num_techs_accelerated]  )):
                    res=fo.issueEnqueueTechOrder("CON_ORBITAL_CON", num_techs_accelerated)
                    print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%("CON_ORBITAL_CON", res)
            elif empire.getTechStatus("CON_CONTGRAV_ARCH") != fo.techStatus.complete:
                num_techs_accelerated += 1
                if  ( "CON_CONTGRAV_ARCH" not in research_queue_list[:1+num_techs_accelerated]  ) and  (
                    empire.getTechStatus("CON_METRO_INFRA") == fo.techStatus.complete):
                    for supply_tech in [_s_tech for _s_tech in ["CON_ARCH_MONOFILS",  "CON_CONTGRAV_ARCH"] if  (empire.getTechStatus(_s_tech) != fo.techStatus.complete)]:
                        res=fo.issueEnqueueTechOrder(supply_tech, num_techs_accelerated)
                        print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%(supply_tech, res)
            elif empire.getTechStatus("CON_GAL_INFRA") != fo.techStatus.complete:
                num_techs_accelerated += 1
                if  ( "CON_GAL_INFRA" not in research_queue_list[:1+num_techs_accelerated]  ) and  (
                    empire.getTechStatus("PRO_SINGULAR_GEN") == fo.techStatus.complete):
                    res=fo.issueEnqueueTechOrder("CON_GAL_INFRA", num_techs_accelerated)
                    print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%("CON_GAL_INFRA", res)
            else:
                pass
            research_queue_list = get_research_queue_techs()
                #could add more supply tech

            if False and (empire.getTechStatus("SPY_DETECT_2") != fo.techStatus.complete): #disabled for now, detect2
                num_techs_accelerated += 1
                if  ( "SPY_DETECT_2" not in research_queue_list[:2+num_techs_accelerated]  ) and  (empire.getTechStatus("PRO_FUSION_GEN") == fo.techStatus.complete) :
                    if "CON_ORBITAL_CON" not in research_queue_list[:1+num_techs_accelerated]:
                        res=fo.issueEnqueueTechOrder("SPY_DETECT_2", num_techs_accelerated)
                    else:
                        CO_idx = research_queue_list.index( "CON_ORBITAL_CON")
                        res=fo.issueEnqueueTechOrder("SPY_DETECT_2", CO_idx+1)
                    print "Empire has poor colony/outpost prospects,  so attempted to fast-track %s,  got result %d"%("CON_ORBITAL_CON", res)
                research_queue_list = get_research_queue_techs()

    #
    # check to accelerate xeno_arch
    if True: #just to help with cold-folding /  organization
        if (ColonisationAI.gotRuins and empire.getTechStatus("LRN_XENOARCH") != fo.techStatus.complete and
                        foAI.foAIstate.aggression >= fo.aggression.typical):
            if "LRN_ARTIF_MINDS" in research_queue_list:
                insert_idx = 7+ research_queue_list.index("LRN_ARTIF_MINDS")
            elif "GRO_SYMBIOTIC_BIO" in research_queue_list:
                insert_idx = research_queue_list.index("GRO_SYMBIOTIC_BIO") + 1
            else:
                insert_idx = num_techs_accelerated
            if "LRN_XENOARCH" not in research_queue_list[:insert_idx]:
                for xenoTech in [  "LRN_XENOARCH",  "LRN_TRANSLING_THT",  "LRN_PHYS_BRAIN" ,  "LRN_ALGO_ELEGANCE"]:
                    if (empire.getTechStatus(xenoTech) != fo.techStatus.complete) and (  xenoTech  not in research_queue_list[:(insert_idx+4)])  :
                        res=fo.issueEnqueueTechOrder(xenoTech,insert_idx)
                        num_techs_accelerated += 1
                        print "ANCIENT_RUINS: have an ancient ruins, so attempted to fast-track %s  to enable LRN_XENOARCH,  got result %d"%(xenoTech, res)
                research_queue_list = get_research_queue_techs()

    if not enemies_sighted:
        # params = [ (tech, gate, target_slot, add_tech_list), ]
        params = [("GRO_XENO_GENETICS", "PRO_EXOBOTS", "PRO_EXOBOTS", ["GRO_GENETIC_MED", "GRO_XENO_GENETICS"]),
                  ("PRO_EXOBOTS", "PRO_SENTIENT_AUTOMATION", "PRO_SENTIENT_AUTOMATION", ["PRO_EXOBOTS"]),
                  ("PRO_SENTIENT_AUTOMATION", "PRO_NANOTECH_PROD", "PRO_NANOTECH_PROD", ["PRO_SENTIENT_AUTOMATION"]),
                  ("PRO_INDUSTRY_CENTER_I", "GRO_SYMBIOTIC_BIO", "GRO_SYMBIOTIC_BIO", ["PRO_ROBOTIC_PROD","PRO_FUSION_GEN","PRO_INDUSTRY_CENTER_I"]),
                  ("GRO_SYMBIOTIC_BIO", "SHP_ORG_HULL", "SHP_ZORTRIUM_PLATE", ["GRO_SYMBIOTIC_BIO"]),
                 ]
        for (tech, gate, target_slot, add_tech_list) in params:
            num_added_here = 0
            if empire.getTechStatus(tech) == fo.techStatus.complete:
                break
            if tech_turns_left.get(gate, 0) not in [0,1,2]: # needs to exclude -1, the flag for no predicted completion
                continue
            if target_slot in research_queue_list:
                target_index = 1 + research_queue_list.index(target_slot)
            else:
                target_index = num_techs_accelerated
            for move_tech in add_tech_list:
                print "for tech %s, target_slot %s, target_index:%s  ; num_techs_accelerated:%s"%(move_tech, target_slot, target_index, num_techs_accelerated)
                if empire.getTechStatus(move_tech) == fo.techStatus.complete:
                    continue
                if target_index <= num_techs_accelerated:
                    num_techs_accelerated += 1
                if move_tech not in research_queue_list[:1 + target_index]:
                    res = fo.issueEnqueueTechOrder(move_tech, target_index)
                    print "Research: To prioritize %s, have advanced %s to slot %d"%(tech, move_tech, target_index)
                    target_index += 1
    #
    # check to accelerate asteroid or GG tech
    if True: #just to help with cold-folding /  organization
        if ColonisationAI.gotAst and empire.getTechStatus("SHP_ASTEROID_HULLS") != fo.techStatus.complete and (
                        ("SHP_ASTEROID_HULLS" not in research_queue_list[num_techs_accelerated])): #if needed but not top item, will block acceleration of pro_orb_gen
            if "SHP_ASTEROID_HULLS" not in research_queue_list[:2+num_techs_accelerated]:
                if "GRO_SYMBIOTIC_BIO" in research_queue_list:
                    insert_idx = 1+ research_queue_list.index("GRO_SYMBIOTIC_BIO")
                else:
                    insert_idx = num_techs_accelerated
                for ast_tech in ["SHP_ASTEROID_HULLS", "PRO_MICROGRAV_MAN"]:
                    if (empire.getTechStatus(ast_tech) != fo.techStatus.complete) and (  ast_tech  not in research_queue_list[:insert_idx+2])  :
                        res=fo.issueEnqueueTechOrder(ast_tech,insert_idx)
                        num_techs_accelerated += 1
                        print "Asteroids: have colonized an asteroid belt, so attempted to fast-track %s ,  got result %d"%(ast_tech, res)
                research_queue_list = get_research_queue_techs()
        elif ColonisationAI.gotGG and empire.getTechStatus("PRO_ORBITAL_GEN") != fo.techStatus.complete  and (
                    "PRO_ORBITAL_GEN" not in research_queue_list[:3+num_techs_accelerated]):
            if "GRO_SYMBIOTIC_BIO" in research_queue_list:
                insert_idx = 1+ research_queue_list.index("GRO_SYMBIOTIC_BIO")
            else:
                insert_idx = num_techs_accelerated
            res=fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN",insert_idx)
            num_techs_accelerated += 1
            print "GasGiant: have colonized a gas giant, so attempted to fast-track %s, got result %d"%("PRO_ORBITAL_GEN", res)
            research_queue_list = get_research_queue_techs()
    #
    # assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if True: #just to help with cold-folding /  organization
        if gotGGG and gotSymBio and (not gotXenoGen) and foAI.foAIstate.aggression >= fo.aggression.cautious:
            mostAdequate=0
            for specName in ColonisationAI.empireColonizers:
                environs={}
                thisSpec = fo.getSpecies(specName)
                if not thisSpec: 
                    continue
                for ptype in [fo.planetType.swamp,  fo.planetType.radiated,  fo.planetType.toxic,  fo.planetType.inferno,  fo.planetType.barren,  fo.planetType.tundra,  fo.planetType.desert,  fo.planetType.terran,  fo.planetType.ocean,  fo.planetType.asteroids]:
                    environ=thisSpec.getPlanetEnvironment(ptype)
                    environs.setdefault(environ, []).append(ptype)
                mostAdequate = max(mostAdequate,  len(environs.get( fo.planetEnvironment.adequate, [])))
            if mostAdequate==0:
                insert_idx = num_techs_accelerated
                for xgTech in [ "GRO_XENO_GENETICS", "GRO_GENETIC_ENG" ]:
                    if   xgTech not in research_queue_list[:1+num_techs_accelerated]  and  empire.getTechStatus(xgTech) != fo.techStatus.complete:
                        res=fo.issueEnqueueTechOrder(xgTech, insert_idx)
                        num_techs_accelerated += 1
                        print "Empire has poor colonizers,  so attempted to fast-track %s,  got result %d"%(xgTech, res)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate distrib thought
    if True: #just to help with cold-folding /  organization
        if empire.getTechStatus("LRN_DISTRIB_THOUGHT") != fo.techStatus.complete:
            got_telepathy = False
            for specName in ColonisationAI.empireSpecies:
                thisSpec=fo.getSpecies(specName)
                if thisSpec and ("TELEPATHIC" in list(thisSpec.tags)):
                    got_telepathy = True
                    break
            if (foAI.foAIstate.aggression > fo.aggression.cautious) and (empire.population() > ([300, 100][got_telepathy])):
                insert_idx = num_techs_accelerated
                for dt_ech in [ "LRN_DISTRIB_THOUGHT", "LRN_PSIONICS", "LRN_TRANSLING_THT",  "LRN_PHYS_BRAIN" ]:
                    if   dt_ech not in research_queue_list[:4+num_techs_accelerated]  and  empire.getTechStatus(dt_ech) != fo.techStatus.complete:
                        res=fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                        num_techs_accelerated += 1
                        fmt_str = "Empire has a telepathic race, so attempted to fast-track %s (got result %d)"
                        fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                        print fmt_str%( dt_ech, res,  resource_production,  empire.population(),  fo.currentTurn())
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate quant net
    if True: #just to help with cold-folding /  organization
        if (foAI.foAIstate.aggression > fo.aggression.cautious) and( ColonisationAI.empire_status.get('researchers', 0) >= 40 ):
            if empire.getTechStatus("LRN_QUANT_NET") != fo.techStatus.complete:
                insert_idx = num_techs_accelerated
                for qnTech in [ "LRN_NDIM_SUBSPACE", "LRN_QUANT_NET" ]:
                    if   qnTech not in research_queue_list[:2+num_techs_accelerated]  and  empire.getTechStatus(qnTech) != fo.techStatus.complete:
                        res=fo.issueEnqueueTechOrder(qnTech, insert_idx)
                        num_techs_accelerated += 1
                        print "Empire has many researchers, so attempted to fast-track %s (got result %d) on turn %d"%(qnTech, res,  fo.currentTurn())
                research_queue_list = get_research_queue_techs()

    #
    # if we own a blackhole, accelerate sing_gen and conc camp
    if True: #just to help with cold-folding /  organization
        if ( fo.currentTurn() >50 and len (AIstate.empireStars.get(fo.starType.blackHole,  []))!=0 and 
                    foAI.foAIstate.aggression > fo.aggression.cautious and (empire.getTechStatus(AIDependencies.sing_tech_name) != fo.techStatus.complete) ):
            #sing_tech_list = [  "LRN_GRAVITONICS" ,  "PRO_SINGULAR_GEN"]   # formerly also "CON_ARCH_PSYCH",  "CON_CONC_CAMP",  
            sing_gen_tech = fo.getTech(AIDependencies.sing_tech_name)
            sing_tech_list =  [pre_req for pre_req in sing_gen_tech.recursivePrerequisites(empire_id) if (empire.getTechStatus(pre_req) != fo.techStatus.complete) ]
            sing_tech_list += [ AIDependencies.sing_tech_name ]
            for singTech in sing_tech_list:
                if singTech  not in research_queue_list[:num_techs_accelerated+1  ]:
                    res=fo.issueEnqueueTechOrder(singTech,num_techs_accelerated)
                    num_techs_accelerated += 1
                    print "have a black hole star outpost/colony, so attempted to fast-track %s,  got result %d"%(singTech, res)
            research_queue_list = get_research_queue_techs()

    #
    # if got deathray from Ruins, remove most prereqs from queue
    if True: #just to help with cold-folding /  organization
        if  empire.getTechStatus("SHP_WEAPON_4_1" ) == fo.techStatus.complete:
            this_tech=fo.getTech("SHP_WEAPON_4_1")
            if this_tech:
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq in research_queue_list]
                if  len(missing_prereqs) > 2 : #leave plasma 4 and 3 if up to them already
                    for preReq in missing_prereqs:  #sorted(missing_prereqs,  reverse=True)[2:]
                        if preReq in research_queue_list:
                            res = fo.issueDequeueTechOrder(preReq)
                    if "SHP_WEAPON_4_2" in research_queue_list: #(should be)
                        idx = research_queue_list.index("SHP_WEAPON_4_2")
                        res=fo.issueEnqueueTechOrder("SHP_WEAPON_4_2",  max(0,  idx-15) )
            research_queue_list = get_research_queue_techs()


def generate_default_research_order():
    """
    Generate default research orders.
    Add cheapest technology from possible researches
    until current turn point totally spent.
    """

    empire = fo.getEmpire()
    total_rp = empire.resourceProduction(fo.resourceType.research)

    # get all usable researchable techs not already completed or queued for research

    excluded_techs = TechsListsAI.unusable_techs()
    queued_techs = get_research_queue_techs()

    def is_possible(tech_name):
        return all([empire.getTechStatus(tech_name) == fo.techStatus.researchable,
                   empire.getTechStatus(tech_name) != fo.techStatus.complete,
                   tech_name not in excluded_techs,
                   tech_name not in queued_techs])

    # (cost, name) for all tech that possible to add to queue, cheapest last
    possible = sorted(
        [(fo.getTech(tech).researchCost(empire.empireID), tech) for tech in fo.techs() if is_possible(tech)],
        reverse=True)

    print "Techs in possible list after enqueues to Research Queue:"
    for _, tech in possible:
        print "    " + tech
    print

    # iterate through techs in order of cost
    total_spent = fo.getEmpire().researchQueue.totalSpent
    print "enqueuing techs.  already spent RP: %s total RP: %s" % (total_spent, total_rp)

    while total_rp > 0 and possible:
        cost, name = possible.pop()  # get chipest
        total_rp -= cost
        fo.issueEnqueueTechOrder(name, -1)
        print "    enqueued tech " + name + "  :  cost: " + str(cost) + "RP"
    print


def get_possible_projects():
    """get possible projects"""
    preliminaryProjects = []
    technames = fo.techs() # returns names of all techs
    empire = fo.getEmpire()
    for techname in technames:
        if empire.getTechStatus(techname) == fo.techStatus.researchable:
            preliminaryProjects.append(techname)

    unusableTechs = TechsListsAI.unusable_techs()
    return set(preliminaryProjects)-set(unusableTechs)


def get_completed_techs():
    """get completed and available for use techs"""
    empire = fo.getEmpire()
    return [tech for tech in fo.techs() if empire.getTechStatus(tech) == fo.techStatus.complete]


def get_research_queue_techs():
    """ Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]

