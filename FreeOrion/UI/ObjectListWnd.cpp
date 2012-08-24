#include "ObjectListWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "FleetButton.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/System.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>

namespace
{ enum VIS_DISPLAY { SHOW_VISIBLE, SHOW_PREVIOUSLY_VISIBLE, SHOW_DESTROYED }; }

////////////////////////////////////////////////
// FilterDialog
////////////////////////////////////////////////
class FilterDialog : public CUIWnd {
public:
    FilterDialog(GG::X x, GG::Y y,
                 const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >& vis_filters,
                 const Condition::ConditionBase* const condition_filter) :
        CUIWnd(UserString("FILTERS"), x, y, GG::X(400), GG::Y(250), GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL),
        m_vis_filters(vis_filters),
        m_accept_changes(false),
        m_filters_layout(0),
        m_cancel_button(0),
        m_apply_button(0)
    {
        Init(condition_filter);
    }

    ~FilterDialog() {
        for (std::vector<Condition::ConditionBase*>::iterator it = m_conditions.begin();
             it != m_conditions.end(); ++it)
        { delete *it; }
        m_conditions.clear();
    }

    bool    ChangesAccepted()
    { return m_accept_changes; }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >    GetVisibilityFilters() const
    { return m_vis_filters; }

    // caller takes ownership of returned ConditionBase*
    Condition::ConditionBase*                               GetConditionFilter() {
        if (m_conditions.empty()) {
            return new Condition::All();
        } else if (m_conditions.size() == 1) {
            Condition::ConditionBase* retval = *(m_conditions.begin());
            m_conditions.clear();
            return retval;
        } else {
            std::vector<const Condition::ConditionBase*> params;
            for (std::vector<Condition::ConditionBase*>::const_iterator it = m_conditions.begin();
                 it != m_conditions.end(); ++it)
            { params.push_back(*it); }
            Condition::ConditionBase* retval = new Condition::And(params);
            m_conditions.clear();
            return retval;
        }
    }

private:
    ValueRef::ValueRefBase<std::string>*  CopyStringValueRef(const ValueRef::ValueRefBase<std::string>* const value_ref) {
        if (const ValueRef::Constant<std::string>* constant =
            dynamic_cast<const ValueRef::Constant<std::string>*>(value_ref))
        { return new ValueRef::Constant<std::string>(constant->Value()); }
        return new ValueRef::Constant<std::string>("");
    }
    ValueRef::ValueRefBase<int>*          CopyIntValueRef(const ValueRef::ValueRefBase<int>* const value_ref) {
        if (const ValueRef::Constant<int>* constant =
            dynamic_cast<const ValueRef::Constant<int>*>(value_ref))
        { return new ValueRef::Constant<int>(constant->Value()); }
        return new ValueRef::Constant<int>(0);
    }
    ValueRef::ValueRefBase<double>*       CopyDoubleValueRef(const ValueRef::ValueRefBase<double>* const value_ref) {
        if (const ValueRef::Constant<double>* constant =
            dynamic_cast<const ValueRef::Constant<double>*>(value_ref))
        { return new ValueRef::Constant<double>(constant->Value()); }
        return new ValueRef::Constant<double>(0.0);
    }

    Condition::ConditionBase*                   CopyCondition(const Condition::ConditionBase* const condition) {
        if (dynamic_cast<const Condition::Source* const>(condition)) {
            return new Condition::Source();

        } else if (dynamic_cast<const Condition::Homeworld* const>(condition)) {
            return new Condition::Homeworld();

        } else if (dynamic_cast<const Condition::Building* const>(condition)) {

        }

        return new Condition::All();
    }

    void    Init(const Condition::ConditionBase* const condition_filter) {
        if (m_filters_layout)
            delete m_filters_layout;
        m_filters_layout = new GG::Layout(GG::X0, GG::Y0, GG::X(390), GG::Y(90), 4, 6);
        AttachChild(m_filters_layout);

        m_filters_layout->SetMinimumColumnWidth(0, GG::X(ClientUI::Pts()*8));
        m_filters_layout->SetColumnStretch(0, 0.0);

        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr color = ClientUI::TextColor();

        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("VISIBLE"),              font, color),
                              1, 0, GG::ALIGN_CENTER);
        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("PREVIOUSLY_VISIBLE"),   font, color),
                              2, 0, GG::ALIGN_CENTER);
        m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, UserString("DESTROYED"),            font, color),
                              3, 0, GG::ALIGN_CENTER);

        int col = 1;
        for (std::map<UniverseObjectType, std::set<VIS_DISPLAY> >::const_iterator uot_it = m_vis_filters.begin();
             uot_it != m_vis_filters.end(); ++uot_it, ++col)
        {
            const UniverseObjectType& uot = uot_it->first;
            const std::string& uot_label = UserString(GG::GetEnumMap<UniverseObjectType>().FromEnum(uot));
            const std::set<VIS_DISPLAY>& vis_display = uot_it->second;

            m_filters_layout->SetColumnStretch(col, 1.0);

            m_filters_layout->Add(new GG::TextControl(GG::X0, GG::Y0, uot_label, font, color, GG::FORMAT_CENTER),
                                  0, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);

            CUIStateButton* button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 1, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);

            button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_PREVIOUSLY_VISIBLE) != vis_display.end());
            m_filters_layout->Add(button, 2, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);

            button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, " ", GG::FORMAT_CENTER);
            button->SetCheck(vis_display.find(SHOW_DESTROYED) != vis_display.end());
            m_filters_layout->Add(button, 3, col, GG::ALIGN_CENTER | GG::ALIGN_VCENTER);
            GG::Connect(button->CheckedSignal,  &FilterDialog::UpdateVisfiltersFromStateButtons,    this);
        }

        if (!condition_filter) {
            m_conditions.clear();

        } else if (const Condition::And* const and_condition = dynamic_cast<const Condition::And* const>(condition_filter)) {
            const std::vector<const Condition::ConditionBase*>& operands = and_condition->Operands();
            for (std::vector<const Condition::ConditionBase*>::const_iterator it = operands.begin();
                 it != operands.end(); ++it)
            {
                Condition::ConditionBase* copied_condition = CopyCondition(*it);
                if (copied_condition)
                    m_conditions.push_back(copied_condition);
            }

        } else {
            Condition::ConditionBase* copied_condition = CopyCondition(condition_filter);
            if (copied_condition)
                m_conditions.push_back(copied_condition);
        }

        m_cancel_button = new CUIButton(GG::X0, GG::Y0, GG::X(ClientUI::Pts()*8), UserString("CANCEL"), font);
        AttachChild(m_cancel_button);
        GG::Connect(m_cancel_button->ClickedSignal, &FilterDialog::CancelClicked,   this);

        m_apply_button = new CUIButton(GG::X0, GG::Y0, GG::X(ClientUI::Pts()*8), UserString("APPLY"), font);
        AttachChild(m_apply_button);
        GG::Connect(m_apply_button->ClickedSignal, &FilterDialog::AcceptClicked,   this);

        GG::Pt button_lr = this->ClientSize();
        m_cancel_button->MoveTo(GG::Pt(button_lr.x - m_cancel_button->Width(),
                                       button_lr.y - m_cancel_button->Height()));
        button_lr = button_lr - GG::Pt(m_apply_button->Width() + GG::X(3), GG::Y0);
        m_apply_button->MoveTo(GG::Pt(button_lr.x - m_apply_button->Width(),
                                      button_lr.y - m_apply_button->Height()));
    }

    void    AcceptClicked() {
        m_accept_changes = true;
        m_done = true;
    }

    void    CancelClicked() {
        m_accept_changes = false;
        m_done = true;
    }

    void    UpdateVisfiltersFromStateButtons(bool button_checked) {
        std::vector<std::vector<const GG::Wnd*> > layout_cells = m_filters_layout->Cells();

        int col = 1;
        for (std::map<UniverseObjectType, std::set<VIS_DISPLAY> >::iterator uot_it = m_vis_filters.begin();
             uot_it != m_vis_filters.end(); ++uot_it, ++col)
        {
            const UniverseObjectType& uot = uot_it->first;
            std::set<VIS_DISPLAY>& vis_display = uot_it->second;
            vis_display.clear();

            const GG::Wnd* wnd = layout_cells[1][col];
            const CUIStateButton* button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_VISIBLE);

            wnd = layout_cells[2][col];
            button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_PREVIOUSLY_VISIBLE);

            wnd = layout_cells[3][col];
            button = dynamic_cast<const CUIStateButton*>(wnd);
            if (button && button->Checked())
                m_vis_filters[uot].insert(SHOW_DESTROYED);
        }
    }

    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >    m_vis_filters;
    std::vector<Condition::ConditionBase*>                  m_conditions;

    bool        m_accept_changes;

    GG::Layout* m_filters_layout;
    GG::Button* m_cancel_button;
    GG::Button* m_apply_button;
};

namespace {
    static std::string EMPTY_STRING;

    std::vector<boost::shared_ptr<GG::Texture> > ObjectTextures(const UniverseObject* obj) {
    std::vector<boost::shared_ptr<GG::Texture> > retval;

    if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
        if (const ShipDesign* design = ship->Design())
            retval.push_back(ClientUI::ShipDesignIcon(design->ID()));
        else
            retval.push_back(ClientUI::ShipDesignIcon(INVALID_OBJECT_ID));  // default icon
    } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
        boost::shared_ptr<GG::Texture> head_icon = FleetHeadIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
        retval.push_back(head_icon);
        boost::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
        retval.push_back(size_icon);
    } else if (const System* system = universe_object_cast<const System*>(obj)) {
        StarType star_type = system->GetStarType();
        ClientUI* ui = ClientUI::GetClientUI();
        boost::shared_ptr<GG::Texture> disc_texture = ui->GetModuloTexture(
            ClientUI::ArtDir() / "stars", ClientUI::StarTypeFilePrefixes()[star_type], system->ID());
        retval.push_back(disc_texture);
        boost::shared_ptr<GG::Texture> halo_texture = ui->GetModuloTexture(
            ClientUI::ArtDir() / "stars", ClientUI::HaloStarTypeFilePrefixes()[star_type], system->ID());
        retval.push_back(halo_texture);
    } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
        // don't have any icons for each planet type, so use generic / default object icon

    } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(building->BuildingTypeName());
        retval.push_back(texture);
    }
    if (retval.empty())
        retval.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "generic_object.png", true));
    return retval;
}

    const std::string& ObjectName(const UniverseObject* obj) {
    if (!obj)
        return EMPTY_STRING;
    if (const System* system = universe_object_cast<const System*>(obj))
        return system->ApparentName(HumanClientApp::GetApp()->EmpireID());
    return obj->PublicName(HumanClientApp::GetApp()->EmpireID());
}

    std::pair<std::string, GG::Clr> ObjectEmpireNameAndColour(const UniverseObject* obj) {
    if (!obj)
        return std::make_pair("", ClientUI::TextColor());
    if (const Empire* empire = Empires().Lookup(obj->Owner()))
        return std::make_pair(empire->Name(), empire->Color());
    return std::make_pair("", ClientUI::TextColor());
}
}

////////////////////////////////////////////////
// ObjectPanel
////////////////////////////////////////////////
class ObjectPanel : public GG::Control {
public:
    ObjectPanel(GG::X w, GG::Y h, const UniverseObject* obj, bool expanded, bool has_contents, int indent = 0) :
        Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
        m_object_id(obj ? obj->ID() : INVALID_OBJECT_ID),
        m_indent(indent),
        m_expanded(expanded),
        m_has_contents(has_contents),
        m_expand_button(0),
        m_dot(0),
        m_icon(0),
        m_name_label(0),
        m_empire_label(0)
    {
        Refresh();
    }

    int                 ObjectID() const { return m_object_id; }

    virtual void        Render() {
        GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
        GG::FlatRectangle(UpperLeft(), LowerRight(), background_clr, ClientUI::WndOuterBorderColor(), 1u);
    }

    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (old_size != Size())
            DoLayout();
    }

    void                Refresh() {
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr clr = ClientUI::TextColor();
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

        delete m_dot;           m_dot = 0;
        delete m_expand_button; m_expand_button = 0;
        delete m_icon;          m_icon = 0;
        delete m_name_label;    m_name_label = 0;

        if (m_has_contents) {
            m_expand_button = new GG::Button(GG::X0, GG::Y0, GG::X(16), GG::Y(16),
                                                "", font, GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
            AttachChild(m_expand_button);
            GG::Connect(m_expand_button->ClickedSignal, &ObjectPanel::ExpandCollapseButtonPressed, this);

            if (m_expanded) {
                m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "minusnormal.png"     , true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
                m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "minusclicked.png"    , true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
                m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "minusmouseover.png"  , true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
            } else {
                m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "plusnormal.png"   , true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
                m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "plusclicked.png"  , true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
                m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "plusmouseover.png", true), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
            }
        } else {
            m_dot = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(16), GG::Y(16),
                                          ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "dot.png", true),
                                          style, GG::Flags<GG::WndFlag>());
            AttachChild(m_dot);
        }

        const UniverseObject* obj = GetUniverseObject(m_object_id);
        std::vector<boost::shared_ptr<GG::Texture> > textures = ObjectTextures(obj);

        m_icon = new MultiTextureStaticGraphic(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(),
                                                textures, std::vector<GG::Flags<GG::GraphicStyle> >(textures.size(), style));
        AttachChild(m_icon);

        m_name_label = new GG::TextControl(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(), ObjectName(obj), font, clr, GG::FORMAT_LEFT);
        AttachChild(m_name_label);

        std::pair<std::string, GG::Clr> empire_and_colour = ObjectEmpireNameAndColour(obj);
        m_empire_label = new GG::TextControl(GG::X0, GG::Y0, GG::X(Value(ClientHeight())), ClientHeight(), empire_and_colour.first, font, empire_and_colour.second, GG::FORMAT_LEFT);
        AttachChild(m_empire_label);

        DoLayout();
    }

    void                SetHasContents(bool has_contents)
    { m_has_contents = has_contents; }

    mutable boost::signal<void ()>  ExpandCollapseSignal;
private:
    void                DoLayout() {
        const GG::Y ICON_HEIGHT(ClientHeight());
        const GG::X ICON_WIDTH(Value(ClientHeight()));

        GG::X indent(ICON_WIDTH * m_indent);
        GG::X left = indent;
        GG::Y top(GG::Y0);
        GG::Y bottom(ClientHeight());
        GG::X PAD(3);

        GG::X ctrl_width = ICON_WIDTH;

        if (m_expand_button) {
            m_expand_button->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        } else if (m_dot) {
            m_dot->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        }
        left += ctrl_width + PAD;

        m_icon->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;

        ctrl_width = GG::X(ClientUI::Pts()*14) - indent;    // so second column all line up
        m_name_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;

        ctrl_width = GG::X(ClientUI::Pts()*8);
        m_empire_label->SizeMove(GG::Pt(left, top), GG::Pt(left + ctrl_width, bottom));
        left += ctrl_width + PAD;
    }

    void                ExpandCollapseButtonPressed() {
        m_expanded = !m_expanded;
        ExpandCollapseSignal();
    }

    int                         m_object_id;
    int                         m_indent;
    bool                        m_expanded;
    bool                        m_has_contents;

    GG::Button*                 m_expand_button;
    GG::StaticGraphic*          m_dot;
    MultiTextureStaticGraphic*  m_icon;
    GG::TextControl*            m_name_label;
    GG::TextControl*            m_empire_label;
};

////////////////////////////////////////////////
// ObjectRow
////////////////////////////////////////////////
class ObjectRow : public GG::ListBox::Row {
public:
    ObjectRow(GG::X w, GG::Y h, const UniverseObject* obj, bool expanded,
              int container_object_panel,
              const std::set<int>& contained_object_panels, int indent) :
        GG::ListBox::Row(w, h, "ObjectRow", GG::ALIGN_CENTER, 1),
        m_panel(0),
        m_container_object_panel(container_object_panel),
        m_contained_object_panels(contained_object_panels)
    {
        SetName("ObjectRow");
        SetChildClippingMode(ClipToClient);
        SetDragDropDataType("ObjectRow");
        m_panel = new ObjectPanel(w, h, obj, expanded, !m_contained_object_panels.empty(), indent);
        push_back(m_panel);
        GG::Connect(m_panel->ExpandCollapseSignal,  &ObjectRow::ExpandCollapseClicked, this);
    }

    int                     ObjectID() const {
        if (m_panel)
            return m_panel->ObjectID();
        return INVALID_OBJECT_ID;
    }

    int                     ContainedByPanel() const
    { return m_container_object_panel; }

    const std::set<int>& ContainedPanels() const
    { return m_contained_object_panels; }

    void                    SetContainedPanels(const std::set<int>& contained_object_panels) {
        m_contained_object_panels = contained_object_panels;
        m_panel->SetHasContents(!m_contained_object_panels.empty());
        m_panel->Refresh();
    }

    void                    Update()
    { m_panel->Refresh(); }

    /** This function overridden because otherwise, rows don't expand
        * larger than their initial size when resizing the list. */
    void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::ListBox::Row::SizeMove(ul, lr);
        //std::cout << "ObjectRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (!empty() && old_size != Size() && m_panel)
            m_panel->Resize(Size());
    }

    void                    ExpandCollapseClicked()
    { ExpandCollapseSignal(m_panel ? m_panel->ObjectID() : INVALID_OBJECT_ID); }

    mutable boost::signal<void (int)>   ExpandCollapseSignal;
private:
    ObjectPanel*        m_panel;
    int                 m_container_object_panel;
    std::set<int>       m_contained_object_panels;
};

////////////////////////////////////////////////
// ObjectListBox
////////////////////////////////////////////////
class ObjectListBox : public CUIListBox {
public:
    ObjectListBox() :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_object_change_connections(),
        m_collapsed_objects(),
        m_filter_condition(0),
        m_visibilities()
    {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();

        m_filter_condition = new Condition::All();

        m_visibilities[OBJ_BUILDING].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_BUILDING].insert(SHOW_PREVIOUSLY_VISIBLE);
        m_visibilities[OBJ_SHIP].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_FLEET].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_PLANET].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_PLANET].insert(SHOW_PREVIOUSLY_VISIBLE);
        m_visibilities[OBJ_SYSTEM].insert(SHOW_VISIBLE);
        m_visibilities[OBJ_SYSTEM].insert(SHOW_PREVIOUSLY_VISIBLE);

        GG::Connect(GetUniverse().UniverseObjectDeleteSignal,   &ObjectListBox::UniverseObjectDeleted,  this);
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "ObjectListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "ObjectListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

    static GG::Y    ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    const Condition::ConditionBase* const                       FilterCondition() const
    { return m_filter_condition; }

    const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >  Visibilities() const
    { return m_visibilities; }

    void            CollapseObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            for (GG::ListBox::iterator row_it = this->begin(); row_it != this->end(); ++row_it)
                if (const ObjectRow* object_row = dynamic_cast<const ObjectRow*>(*row_it))
                    m_collapsed_objects.insert(object_row->ObjectID());
        } else {
            m_collapsed_objects.insert(object_id);
        }
        Refresh();
    }

    void            ExpandObject(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID) {
            m_collapsed_objects.clear();
        } else {
            m_collapsed_objects.erase(object_id);
        }
        Refresh();
    }

    bool            ObjectCollapsed(int object_id) const {
        if (object_id == INVALID_OBJECT_ID)
            return false;
        return m_collapsed_objects.find(object_id) != m_collapsed_objects.end();
    }

    void            SetFilterCondition(Condition::ConditionBase* condition) {
        m_filter_condition = condition;
        Refresh();
    }

    void            SetVisibilityFilters(const std::map<UniverseObjectType, std::set<VIS_DISPLAY> >& vis) {
        if (vis != m_visibilities) {
            m_visibilities = vis;
            Refresh();
        }
    }

    void            ClearContents() {
        Clear();
        for (std::map<int, boost::signals::connection>::iterator it = m_object_change_connections.begin();
             it != m_object_change_connections.end(); ++it)
        { it->second.disconnect(); }
        m_object_change_connections.clear();
    }

    bool            ObjectShown(int object_id, UniverseObjectType type, bool assume_visible_without_checking = false) {
        if (object_id == INVALID_OBJECT_ID)
            return false;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        if (GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).find(object_id) != GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id).end())
            return m_visibilities[type].find(SHOW_DESTROYED) != m_visibilities[type].end();

        if (assume_visible_without_checking || GetUniverse().GetObjectVisibilityByEmpire(object_id, client_empire_id))
            return m_visibilities[type].find(SHOW_VISIBLE) != m_visibilities[type].end();

        return m_visibilities[type].find(SHOW_PREVIOUSLY_VISIBLE) != m_visibilities[type].end();
    }

    void            Refresh() {
        std::size_t first_visible_queue_row = std::distance(this->begin(), this->FirstRowShown());
        ClearContents();

        const ObjectMap& visible_objects = GetUniverse().Objects();
        const ObjectMap& known_objects = GetUniverse().EmpireKnownObjects(HumanClientApp::GetApp()->EmpireID());

        std::set<int> already_filtered_objects;

        bool nested = true;

        if (!nested) {
        } else {
            // sort objects by containment associations
            std::set<int>                   systems;
            std::map<int, std::set<int> >   system_fleets;
            std::map<int, std::set<int> >   fleet_ships;
            std::map<int, std::set<int> >   system_planets;
            std::map<int, std::set<int> >   planet_buildings;

            for (ObjectMap::const_iterator it = visible_objects.const_begin(); it != visible_objects.const_end(); ++it) {
                int object_id = it->first;
                already_filtered_objects.insert(object_id);
                const UniverseObject* obj = it->second;

                if (const System* system = universe_object_cast<const System*>(obj)) {
                    if (ObjectShown(object_id, OBJ_SYSTEM, true))
                        systems.insert(object_id);

                } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_FLEET, true))
                        system_fleets[fleet->SystemID()].insert(object_id);

                } else if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
                    if (ObjectShown(object_id, OBJ_SHIP, true))
                        fleet_ships[ship->FleetID()].insert(object_id);

                } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_PLANET, true))
                        system_planets[planet->SystemID()].insert(object_id);

                } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
                    if (ObjectShown(object_id, OBJ_BUILDING, true))
                        planet_buildings[building->PlanetID()].insert(object_id);
                }
            }
            for (ObjectMap::const_iterator it = known_objects.const_begin(); it != known_objects.const_end(); ++it) {
                int object_id = it->first;
                if (already_filtered_objects.find(object_id) != already_filtered_objects.end())
                    continue;
                const UniverseObject* obj = it->second;

                if (const System* system = universe_object_cast<const System*>(obj)) {
                    if (ObjectShown(object_id, OBJ_SYSTEM))
                        systems.insert(object_id);

                } else if (const Fleet* fleet = universe_object_cast<const Fleet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_FLEET))
                        system_fleets[fleet->SystemID()].insert(object_id);

                } else if (const Ship* ship = universe_object_cast<const Ship*>(obj)) {
                    if (ObjectShown(object_id, OBJ_SHIP))
                        fleet_ships[ship->FleetID()].insert(object_id);

                } else if (const Planet* planet = universe_object_cast<const Planet*>(obj)) {
                    if (ObjectShown(object_id, OBJ_PLANET))
                        system_planets[planet->SystemID()].insert(object_id);

                } else if (const Building* building = universe_object_cast<const Building*>(obj)) {
                    if (ObjectShown(object_id, OBJ_BUILDING))
                        planet_buildings[building->PlanetID()].insert(object_id);
                }
            }


            int indent = 0;

            // add system rows
            for (std::set<int>::const_iterator sys_it = systems.begin(); sys_it != systems.end(); ++sys_it) {
                int system_id = *sys_it;

                std::map<int, std::set<int> >::const_iterator sp_it = system_planets.find(system_id);
                std::map<int, std::set<int> >::const_iterator sf_it = system_fleets.find(system_id);
                bool has_contents_system_row = sp_it != system_planets.end() || sf_it != system_fleets.end();
                std::set<int> system_contents;
                if (sp_it != system_planets.end())
                    system_contents = sp_it->second;
                if (sf_it != system_fleets.end())
                    system_contents.insert(sf_it->second.begin(), sf_it->second.end());

                AddObjectRow(system_id, INVALID_OBJECT_ID, system_contents, indent);

                if (!has_contents_system_row || ObjectCollapsed(system_id))
                    continue;

                ++indent;
                // add planet rows in this system
                if (sp_it != system_planets.end()) {
                    const std::set<int>& planets = sp_it->second;
                    for (std::set<int>::const_iterator planet_it = planets.begin(); planet_it != planets.end(); ++planet_it) {
                        int planet_id = *planet_it;

                        std::map<int, std::set<int> >::const_iterator pb_it = planet_buildings.find(planet_id);
                        bool has_contents_planet_row = pb_it != planet_buildings.end();

                        AddObjectRow(planet_id, system_id,
                                     pb_it != planet_buildings.end() ? pb_it->second : std::set<int>(),
                                     indent);

                        if (!has_contents_planet_row || ObjectCollapsed(planet_id))
                            continue;

                        ++indent;
                        // add building rows on this planet
                        if (pb_it != planet_buildings.end()) {
                            const std::set<int>& buildings = pb_it->second;
                            for (std::set<int>::const_iterator building_it = buildings.begin(); building_it != buildings.end(); ++building_it) {
                                AddObjectRow(*building_it, planet_id, std::set<int>(), indent);
                            }
                        }
                        indent--;
                    }
                }

                // add fleet rows in this system
                if (sf_it != system_fleets.end()) {
                    const std::set<int>& fleets = sf_it->second;
                    for (std::set<int>::const_iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                        int fleet_id = *fleet_it;

                        std::map<int, std::set<int> >::const_iterator fs_it = fleet_ships.find(fleet_id);
                        bool has_contents_fleet_row = fs_it != fleet_ships.end();

                        AddObjectRow(fleet_id, system_id, 
                                     fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                     indent);

                        if (!has_contents_fleet_row || ObjectCollapsed(fleet_id))
                            continue;

                        ++indent;
                        // add ship rows on this fleet
                        if (fs_it != fleet_ships.end()) {
                            const std::set<int>& ships = fs_it->second;
                            for (std::set<int>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                                AddObjectRow(*ship_it, fleet_id, std::set<int>(), indent);
                            }
                        }
                        indent--;
                    }
                }
                indent--;
            }

            // add fleets outside systems...
            std::map<int, std::set<int> >::const_iterator sf_it = system_fleets.find(INVALID_OBJECT_ID);
            if (sf_it != system_fleets.end()) {
                const std::set<int>& fleets = sf_it->second;
                for (std::set<int>::const_iterator fleet_it = fleets.begin(); fleet_it != fleets.end(); ++fleet_it) {
                    int fleet_id = *fleet_it;

                    std::map<int, std::set<int> >::const_iterator fs_it = fleet_ships.find(fleet_id);
                    bool has_contents_fleet_row = fs_it != fleet_ships.end();

                    AddObjectRow(fleet_id, INVALID_OBJECT_ID,
                                 fs_it != fleet_ships.end() ? fs_it->second : std::set<int>(),
                                 indent);

                    if (!has_contents_fleet_row || ObjectCollapsed(fleet_id))
                        continue;

                    ++indent;
                    // add ship rows on this fleet
                    if (fs_it != fleet_ships.end()) {
                        const std::set<int>& ships = fs_it->second;
                        for (std::set<int>::const_iterator ship_it = ships.begin(); ship_it != ships.end(); ++ship_it) {
                            AddObjectRow(*ship_it, fleet_id, std::set<int>(), indent);
                        }
                    }
                    indent--;
                }
            }
        }

        if (!this->Empty())
            this->BringRowIntoView(--this->end());
        if (first_visible_queue_row < this->NumRows())
            this->BringRowIntoView(boost::next(this->begin(), first_visible_queue_row));
    }

    void            UpdateObjectPanel(int object_id = INVALID_OBJECT_ID) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* row = dynamic_cast<ObjectRow*>(*it)) {
                row->Update();
                return;
            }
        }
    }

private:
    void            AddObjectRow(int object_id, int container, const std::set<int>& contents, int indent)
    { AddObjectRow(object_id, container, contents, indent, this->end()); }

    void            AddObjectRow(int object_id, int container, const std::set<int>& contents,
                                 int indent, GG::ListBox::iterator it)
    {
        const UniverseObject* obj = GetUniverseObject(object_id);
        if (!obj)
            return;
        const GG::Pt row_size = ListRowSize();
        ObjectRow* object_row = new ObjectRow(row_size.x, row_size.y, obj, !ObjectCollapsed(object_id),
                                              container, contents, indent);
        this->Insert(object_row, it);
        object_row->Resize(row_size);
        GG::Connect(object_row->ExpandCollapseSignal,   &ObjectListBox::ObjectExpandCollapseClicked,    this, boost::signals::at_front);
        m_object_change_connections[obj->ID()].disconnect();
        m_object_change_connections[obj->ID()] = GG::Connect(obj->StateChangedSignal, boost::bind(&ObjectListBox::ObjectStateChanged, this, obj->ID()), boost::signals::at_front);
    }

    // Removes row of indicated object, and all contained rows, recursively.
    // Also updates contents tracking of containing row, if any.
    void            RemoveObjectRow(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        int container_object_id = INVALID_OBJECT_ID;
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it)) {
                if (object_row->ObjectID() == object_id) {
                    container_object_id = object_row->ContainedByPanel();
                    RemoveObjectRow(it);
                    break;
                }
            }
        }

        if (container_object_id == INVALID_OBJECT_ID)
            return;

        // remove this row from parent row's contents
        for (GG::ListBox::iterator it = this->begin(); it != this->end(); ++it) {
            if (ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it)) {
                if (object_row->ObjectID() == container_object_id) {
                    const std::set<int>& contents = object_row->ContainedPanels();
                    std::set<int> new_contents;
                    for (std::set<int>::const_iterator contents_it = contents.begin();
                         contents_it != contents.end(); ++contents_it)
                    {
                        if (*contents_it != object_id)
                            new_contents.insert(*contents_it);
                    }
                    object_row->SetContainedPanels(new_contents);
                    object_row->Update();
                    break;
                }
            }
        }
    }

    // Removes row at indicated iterator location and its contained rows.
    // Does not adjust containing row.
    void            RemoveObjectRow(GG::ListBox::iterator it) {
        if (it == this->end())
            return;
        ObjectRow* object_row = dynamic_cast<ObjectRow*>(*it);

        // recursively remove contained rows first
        const std::set<int>& contents = object_row->ContainedPanels();
        for (unsigned int i = 0; i < contents.size(); ++i) {
            GG::ListBox::iterator next_it = it; ++next_it;
            if (next_it == this->end())
                break;
            ObjectRow* contained_row = dynamic_cast<ObjectRow*>(*next_it);
            if (!contained_row)
                continue;
            // remove only rows that are contained by this row
            if (contained_row->ContainedByPanel() != object_row->ObjectID())
                break;
            RemoveObjectRow(next_it);
        }

        // erase this row and remove any signals related to it
        m_object_change_connections[object_row->ObjectID()].disconnect();
        m_object_change_connections.erase(object_row->ObjectID());
        this->Erase(it);
    }

    void            ObjectExpandCollapseClicked(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        if (ObjectCollapsed(object_id))
            ExpandObject(object_id);
        else
            CollapseObject(object_id);
    }

    void            ObjectStateChanged(int object_id) {
        if (object_id == INVALID_OBJECT_ID)
            return;
        const UniverseObject* obj = GetUniverseObject(object_id);
        Logger().debugStream() << "ObjectListBox::ObjectStateChanged: " << obj->Name();
        if (!obj)
            return;
        if (universe_object_cast<const Ship*>(obj) ||
            universe_object_cast<const Building*>(obj))
        {
            UpdateObjectPanel(object_id);
        } else if (universe_object_cast<const Fleet*>(obj) ||
                   universe_object_cast<const Planet*>(obj) ||
                   universe_object_cast<const System*>(obj))
        {
            Refresh();
        }
    }

    void            UniverseObjectDeleted(const UniverseObject* obj) {
        if (obj)
            RemoveObjectRow(obj->ID());
    }

    std::map<int, boost::signals::connection>           m_object_change_connections;
    std::set<int>                                       m_collapsed_objects;
    Condition::ConditionBase*                           m_filter_condition;
    std::map<UniverseObjectType, std::set<VIS_DISPLAY> >m_visibilities;
};

////////////////////////////////////////////////
// ObjectListWnd
////////////////////////////////////////////////
ObjectListWnd::ObjectListWnd(GG::X w, GG::Y h) :
    CUIWnd(UserString("MAP_BTN_OBJECTS"), GG::X1, GG::Y1, w - 1, h - 1, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_list_box(0),
    m_filter_button(0),
    m_sort_button(0),
    m_columns_button(0),
    m_collapse_button(0)
{
    m_list_box = new ObjectListBox();
    m_list_box->SetHiliteColor(GG::CLR_ZERO);
    m_list_box->SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);
    GG::Connect(m_list_box->DoubleClickedSignal,    &ObjectListWnd::ObjectDoubleClicked,    this);
    GG::Connect(m_list_box->RightClickedSignal,     &ObjectListWnd::ObjectRightClicked,     this);
    AttachChild(m_list_box);

    m_filter_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("FILTERS"));
    GG::Connect(m_filter_button->ClickedSignal,     &ObjectListWnd::FilterClicked,          this);
    AttachChild(m_filter_button);

    m_sort_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("SORT"));
    GG::Connect(m_sort_button->ClickedSignal,       &ObjectListWnd::SortClicked,            this);
    AttachChild(m_sort_button);
    m_sort_button->Disable();

    m_columns_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLUMNS"));
    GG::Connect(m_columns_button->ClickedSignal,    &ObjectListWnd::ColumnsClicked,         this);
    AttachChild(m_columns_button);
    m_columns_button->Disable();

    m_collapse_button = new CUIButton(GG::X0, GG::Y0, GG::X(30), UserString("COLLAPSE_ALL"));
    GG::Connect(m_collapse_button->ClickedSignal,   &ObjectListWnd::CollapseExpandClicked,  this);
    AttachChild(m_collapse_button);
    m_collapse_button->Disable();

    DoLayout();
}

void ObjectListWnd::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*7);
    GG::Y BUTTON_HEIGHT = m_filter_button->Height();
    int PAD(3);

    GG::Pt button_ul(GG::X0, ClientHeight() - BUTTON_HEIGHT);

    m_filter_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_sort_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_columns_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_collapse_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul += GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_list_box->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth(), button_ul.y));

    SetMinSize(GG::Pt(5*BUTTON_WIDTH, 6*BUTTON_HEIGHT));
}

void ObjectListWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ObjectListWnd::Refresh()
{ m_list_box->Refresh(); }

void ObjectListWnd::ObjectDoubleClicked(GG::ListBox::iterator it) {
    int object_id = ObjectInRow(it);
    if (object_id != INVALID_OBJECT_ID)
        ObjectDoubleClickedSignal(object_id);
    ClientUI::GetClientUI()->ZoomToObject(object_id);
}

void ObjectListWnd::ObjectRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    int object_id = ObjectInRow(it);
    if (object_id == INVALID_OBJECT_ID)
        return;

    // create popup menu with diplomacy options in it
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DUMP"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            ObjectDumpSignal(object_id);
            break;
        }
        default:
            break;
        }
    }
}

int ObjectListWnd::ObjectInRow(GG::ListBox::iterator it) const {
    if (it == m_list_box->end())
        return INVALID_OBJECT_ID;

    if (ObjectRow* obj_row = dynamic_cast<ObjectRow*>(*it))
        return obj_row->ObjectID();

    return INVALID_OBJECT_ID;
}

void ObjectListWnd::FilterClicked() {
    FilterDialog dlg(GG::X(100), GG::Y(100),
                     m_list_box->Visibilities(), m_list_box->FilterCondition());
    dlg.Run();

    if (dlg.ChangesAccepted()) {
        m_list_box->SetVisibilityFilters(dlg.GetVisibilityFilters());
        m_list_box->SetFilterCondition(dlg.GetConditionFilter());
    }
}

void ObjectListWnd::SortClicked() {
}

void ObjectListWnd::ColumnsClicked() {
}

void ObjectListWnd::CollapseExpandClicked() {
}
